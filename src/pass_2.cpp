#include "pass_2.h"
#include "utilities.h"

#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "table_store.h"

std::string pass_2::readTillTab(const std::string &data, int &index) {
    std::string tempBuffer;

    while (index < data.length() && data[index] != '\t') {
        tempBuffer += data[index];
        index++;
    }
    index++;
    if (tempBuffer == " ") {
        tempBuffer = "-1";
    }
    return tempBuffer;
}

bool pass_2::readIntermediateFile() {
    std::string fileLine;
    bool tempStatus;
    int index = 0;
    if (!getline(intermediateFile, fileLine)) {
        return false;
    }
    lineNumber = utilities::string_to_decimal(readTillTab(fileLine, index));

    isComment = fileLine[index] == '.';
    if (isComment) {
        utilities::readFirstNonWhiteSpace(fileLine, index, tempStatus, comment, true);
        return true;
    }
    address = utilities::stringHexToInt(readTillTab(fileLine, index));
    if (const std::string tempBuffer = readTillTab(fileLine, index); tempBuffer == " ") {
        blockNumber = -1;
    } else {
        blockNumber = utilities::string_to_decimal(tempBuffer);
    }
    label = readTillTab(fileLine, index);
    if (label == "-1") {
        label = " ";
    }
    opcode = readTillTab(fileLine, index);
    if (opcode == "BYTE") {
        utilities::readByteOperand(fileLine, index, tempStatus, operand);
    } else {
        operand = readTillTab(fileLine, index);
        if (operand == "-1") {
            operand = " ";
        }
    }
    utilities::readFirstNonWhiteSpace(fileLine, index, tempStatus, comment, true);
    return true;
}

void pass_2::writeTextRecord(const bool lastRecord) {
    if (lastRecord) {
        if (currentRecord.length() > 0) {
            // Write last text record
            writeData = utilities::intToStringHex(currentRecord.length() / 2, 2) + '^' + currentRecord;
            utilities::writeToFile(objectFile, writeData);
            currentRecord = "";
        }
        return;
    }
    if (objectCode != "") {
        if (currentRecord.length() == 0) {
            writeData = "T^" + utilities::intToStringHex(
                            address + utilities::stringHexToInt(
                                tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress), 6) + '^';
            utilities::writeToFile(objectFile, writeData, false);
        }
        // What is objectCode length > 60
        if ((currentRecord + objectCode).length() > 60) {
            // Write current record
            writeData = utilities::intToStringHex(currentRecord.length() / 2, 2) + '^' + currentRecord;
            utilities::writeToFile(objectFile, writeData);

            // Initialize new text currentRecord
            currentRecord = "";
            writeData = "T^" + utilities::intToStringHex(
                            address + utilities::stringHexToInt(
                                tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress), 6) + '^';
            utilities::writeToFile(objectFile, writeData, false);
        }

        currentRecord += objectCode;
    } else {
        /*Assembler directive which doesn't result in address genrenation*/
        if (opcode == "START" || opcode == "END" || opcode == "BASE" || opcode == "NOBASE" || opcode == "LTORG" ||
            opcode == "ORG" || opcode == "EQU") {
            /*DO nothing*/
        } else {
            // Write current record if exists
            if (currentRecord.length() > 0) {
                writeData = utilities::intToStringHex(currentRecord.length() / 2, 2) + '^' + currentRecord;
                utilities::writeToFile(objectFile, writeData);
            }
            currentRecord = "";
        }
    }
}

std::string pass_2::createObjectCodeFormat34() {
    std::string objcode;
    int halfBytes;
    halfBytes = (utilities::getFlagFormat(opcode) == '+') ? 5 : 3;

    if (utilities::getFlagFormat(operand) == '#') {
        // Immediate
        if (operand.substr(operand.length() - 2, 2) == ",X") {
            // Error handling for Immediate with index based
            writeData = "Line: " + std::to_string(lineNumber) +
                        " Index based addressing not supported with Indirect addressing";
            utilities::writeToFile(errorFile, writeData);
            hasError = true;
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            objcode += (halfBytes == 5) ? "100000" : "0000";
            return objcode;
        }

        std::string tempOperand = operand.substr(1, operand.length() - 1);
        if (utilities::if_all_num(tempOperand) || (
                (tableStore->SYMTAB[tempOperand].exists == 'y') && (tableStore->SYMTAB[tempOperand].relative == 0))) {
            // Immediate integer value
            int immediateValue;

            if (utilities::if_all_num(tempOperand)) {
                immediateValue = utilities::string_to_decimal(tempOperand);
            } else {
                immediateValue = utilities::stringHexToInt(tableStore->SYMTAB[tempOperand].address);
            }
            /*Process Immediate value*/
            if (immediateValue >= (1 << 4 * halfBytes)) {
                // Can't fit it
                writeData = "Line: " + std::to_string(lineNumber) + " Immediate value exceeds format limit";
                utilities::writeToFile(errorFile, writeData);
                hasError = true;
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
                objcode += (halfBytes == 5) ? "100000" : "0000";
            } else {
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
                objcode += (halfBytes == 5) ? '1' : '0';
                objcode += utilities::intToStringHex(immediateValue, halfBytes);
            }
            return objcode;
        }
        if (tableStore->SYMTAB[tempOperand].exists == 'n') {
            writeData = "Line " + std::to_string(lineNumber);
            writeData += " : Symbol does not exist. Found " + tempOperand;
            utilities::writeToFile(errorFile, writeData);
            hasError = true;
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            objcode += (halfBytes == 5) ? "100000" : "0000";
            return objcode;
        }
        int operandAddress = utilities::stringHexToInt(tableStore->SYMTAB[tempOperand].address) +
                             utilities::stringHexToInt(
                                 tableStore->BLOCKS[BLocksNumToName[tableStore->SYMTAB[tempOperand].blockNumber]].
                                 startAddress);

        /*Process Immediate symbol value*/
        if (halfBytes == 5) {
            /*If format 4*/
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            objcode += '1';
            objcode += utilities::intToStringHex(operandAddress, halfBytes);

            /*add modifacation record here*/
            modificationRecord += "M^" + utilities::intToStringHex(address + 1, 6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return objcode;
        }

        /*Handle format 3*/
        program_counter = address + utilities::stringHexToInt(
                              tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress);
        program_counter += (halfBytes == 5) ? 4 : 3;
        int relativeAddress = operandAddress - program_counter;

        /*Try PC based*/
        if (relativeAddress >= (-2048) && relativeAddress <= 2047) {
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            objcode += '2';
            objcode += utilities::intToStringHex(relativeAddress, halfBytes);
            return objcode;
        }

        /*Try base based*/
        if (!nobase) {
            relativeAddress = operandAddress - base_register_value;
            if (relativeAddress >= 0 && relativeAddress <= 4095) {
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
                objcode += '4';
                objcode += utilities::intToStringHex(relativeAddress, halfBytes);
                return objcode;
            }
        }

        /*Use direct addressing with no PC or base*/
        if (operandAddress <= 4095) {
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            objcode += '0';
            objcode += utilities::intToStringHex(operandAddress, halfBytes);

            /*add modifacation record here*/
            modificationRecord += "M^" + utilities::intToStringHex(
                address + 1 + utilities::stringHexToInt(
                    tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress), 6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return objcode;
        }
    } else if (utilities::getFlagFormat(operand) == '@') {
        std::string tempOperand = operand.substr(1, operand.length() - 1);
        if (tempOperand.substr(tempOperand.length() - 2, 2) == ",X" || tableStore->SYMTAB[tempOperand].exists == 'n') {
            // Error handling for Indirect with index based
            writeData = "Line " + std::to_string(lineNumber);
            writeData +=
                    " : Symbol doesn't exists.Index based addressing not supported with Indirect addressing ";
            utilities::writeToFile(errorFile, writeData);
            hasError = true;
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
            objcode += (halfBytes == 5) ? "100000" : "0000";
            return objcode;
        }
        int operandAddress = utilities::stringHexToInt(tableStore->SYMTAB[tempOperand].address) +
                             utilities::stringHexToInt(
                                 tableStore->BLOCKS[BLocksNumToName[tableStore->SYMTAB[tempOperand].blockNumber]].
                                 startAddress);
        program_counter = address + utilities::stringHexToInt(
                              tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress);
        program_counter += (halfBytes == 5) ? 4 : 3;

        if (halfBytes == 3) {
            int relativeAddress = operandAddress - program_counter;
            if (relativeAddress >= (-2048) && relativeAddress <= 2047) {
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
                objcode += '2';
                objcode += utilities::intToStringHex(relativeAddress, halfBytes);
                return objcode;
            }

            if (!nobase) {
                relativeAddress = operandAddress - base_register_value;
                if (relativeAddress >= 0 && relativeAddress <= 4095) {
                    objcode = utilities::intToStringHex(
                        utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
                    objcode += '4';
                    objcode += utilities::intToStringHex(relativeAddress, halfBytes);
                    return objcode;
                }
            }

            if (operandAddress <= 4095) {
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
                objcode += '0';
                objcode += utilities::intToStringHex(operandAddress, halfBytes);

                /*add modifacation record here*/
                modificationRecord += "M^" + utilities::intToStringHex(
                    address + 1 + utilities::stringHexToInt(
                        tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress), 6) + '^';
                modificationRecord += (halfBytes == 5) ? "05" : "03";
                modificationRecord += '\n';

                return objcode;
            }
        } else {
            // No base or pc based addressing in format 4
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
            objcode += '1';
            objcode += utilities::intToStringHex(operandAddress, halfBytes);

            /*add modifacation record here*/
            modificationRecord += "M^" + utilities::intToStringHex(
                address + 1 + utilities::stringHexToInt(tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress),
                6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return objcode;
        }

        writeData = "Line: " + std::to_string(lineNumber);
        writeData += "Can't fit into program counter based or base register based addressing.";
        utilities::writeToFile(errorFile, writeData);
        hasError = true;
        objcode = utilities::intToStringHex(
            utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
        objcode += (halfBytes == 5) ? "100000" : "0000";

        return objcode;
    } else if (utilities::getFlagFormat(operand) == '=') {
        // Literals
        std::string tempOperand = operand.substr(1, operand.length() - 1);

        if (tempOperand == "*") {
            tempOperand = "X'" + utilities::intToStringHex(address, 6) + "'";
            /*Add modification record for value created by operand `*` */
            modificationRecord += "M^" + utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->LITTAB[tempOperand].address) + utilities::stringHexToInt(
                    tableStore->BLOCKS[BLocksNumToName[tableStore->LITTAB[tempOperand].blockNumber]].startAddress),
                6) + '^';
            modificationRecord += utilities::intToStringHex(6, 2);
            modificationRecord += '\n';
        }

        if (tableStore->LITTAB[tempOperand].exists == 'n') {
            writeData = "Line " + std::to_string(lineNumber) + " : Symbol doesn't exists. Found " + tempOperand;
            utilities::writeToFile(errorFile, writeData);
            hasError = true;
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            objcode += (halfBytes == 5) ? "000" : "0";
            objcode += "000";
            return objcode;
        }

        int operandAddress = utilities::stringHexToInt(tableStore->LITTAB[tempOperand].address) +
                             utilities::stringHexToInt(
                                 tableStore->BLOCKS[BLocksNumToName[tableStore->LITTAB[tempOperand].blockNumber]].
                                 startAddress);
        program_counter = address + utilities::stringHexToInt(
                              tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress);
        program_counter += (halfBytes == 5) ? 4 : 3;

        if (halfBytes == 3) {
            int relativeAddress = operandAddress - program_counter;
            if (relativeAddress >= (-2048) && relativeAddress <= 2047) {
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                objcode += '2';
                objcode += utilities::intToStringHex(relativeAddress, halfBytes);
                return objcode;
            }

            if (!nobase) {
                relativeAddress = operandAddress - base_register_value;
                if (relativeAddress >= 0 && relativeAddress <= 4095) {
                    objcode = utilities::intToStringHex(
                        utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                    objcode += '4';
                    objcode += utilities::intToStringHex(relativeAddress, halfBytes);
                    return objcode;
                }
            }

            if (operandAddress <= 4095) {
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                objcode += '0';
                objcode += utilities::intToStringHex(operandAddress, halfBytes);

                /*add modifacation record here*/
                modificationRecord += "M^" + utilities::intToStringHex(
                    address + 1 + utilities::stringHexToInt(
                        tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress), 6) + '^';
                modificationRecord += (halfBytes == 5) ? "05" : "03";
                modificationRecord += '\n';

                return objcode;
            }
        } else {
            // No base or pc based addressing in format 4
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            objcode += '1';
            objcode += utilities::intToStringHex(operandAddress, halfBytes);

            /*add modifacation record here*/
            modificationRecord += "M^" + utilities::intToStringHex(
                address + 1 + utilities::stringHexToInt(tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress),
                6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return objcode;
        }

        writeData = "Line: " + std::to_string(lineNumber);
        writeData += "Can't fit into program counter based or base register based addressing.";
        utilities::writeToFile(errorFile, writeData);
        hasError = true;
        objcode = utilities::intToStringHex(
            utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
        objcode += (halfBytes == 5) ? "100" : "0";
        objcode += "000";

        return objcode;
    } else {
        /*Handle direct addressing*/
        int xbpe = 0;
        std::string tempOperand = operand;
        // Handle indexed addressing - check for both ",X" and ", X" (with space)
        if (operand.length() >= 2) {
            if (std::string lastTwo = operand.substr(operand.length() - 2, 2); lastTwo == ",X") {
                tempOperand = operand.substr(0, operand.length() - 2);
                xbpe = 8;
            } else if (operand.length() >= 3 && operand.substr(operand.length() - 3, 3) == ", X") {
                // Handle space after comma
                tempOperand = operand.substr(0, operand.length() - 3);
                xbpe = 8;
            }
        }

        // Safety check for empty operand
        if (tempOperand.empty() || tempOperand == " ") {
            writeData = "Line " + std::to_string(lineNumber);
            writeData += " : Invalid operand. Found '" + operand + "'";
            utilities::writeToFile(errorFile, writeData);
            hasError = true;
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            objcode += "0000";
            return objcode;
        }

        if (tableStore->SYMTAB[tempOperand].exists == 'n') {
            writeData = "Line " + std::to_string(lineNumber);
            writeData += " : Symbol doesn't exists. Found " + tempOperand;
            utilities::writeToFile(errorFile, writeData);
            hasError = true;
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            if (halfBytes == 5)
                objcode += utilities::intToStringHex(xbpe + 1, 1) + "00";
            else
                objcode += utilities::intToStringHex(xbpe, 1);
            objcode += "000";
            return objcode;
        } else {
            int operandAddress = utilities::stringHexToInt(tableStore->SYMTAB[tempOperand].address) +
                                 utilities::stringHexToInt(
                                     tableStore->BLOCKS[BLocksNumToName[tableStore->SYMTAB[tempOperand].blockNumber]].
                                     startAddress);
            program_counter = address + utilities::stringHexToInt(
                                  tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress);
            if (halfBytes == 5)
                program_counter += 4;
            else
                program_counter += 3;

            if (halfBytes == 3) {
                int relativeAddress = operandAddress - program_counter;
                if (relativeAddress >= (-2048) && relativeAddress <= 2047) {
                    objcode = utilities::intToStringHex(
                        utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                    objcode += utilities::intToStringHex(xbpe + 2, 1);
                    objcode += utilities::intToStringHex(relativeAddress, halfBytes);
                    return objcode;
                }

                if (!nobase) {
                    relativeAddress = operandAddress - base_register_value;
                    if (relativeAddress >= 0 && relativeAddress <= 4095) {
                        objcode = utilities::intToStringHex(
                            utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3,
                            2);
                        objcode += utilities::intToStringHex(xbpe + 4, 1);
                        objcode += utilities::intToStringHex(relativeAddress, halfBytes);
                        return objcode;
                    }
                }

                if (operandAddress <= 4095) {
                    objcode = utilities::intToStringHex(
                        utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                    objcode += utilities::intToStringHex(xbpe, 1);
                    objcode += utilities::intToStringHex(operandAddress, halfBytes);

                    /*add modifacation record here*/
                    modificationRecord += "M^" + utilities::intToStringHex(
                        address + 1 + utilities::stringHexToInt(
                            tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress), 6) + '^';
                    modificationRecord += (halfBytes == 5) ? "05" : "03";
                    modificationRecord += '\n';

                    return objcode;
                }
            } else {
                // No base or pc based addressing in format 4
                objcode = utilities::intToStringHex(
                    utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                objcode += utilities::intToStringHex(xbpe + 1, 1);
                objcode += utilities::intToStringHex(operandAddress, halfBytes);

                /*add modifacation record here*/
                modificationRecord += "M^" + utilities::intToStringHex(
                    address + 1 + utilities::stringHexToInt(
                        tableStore->BLOCKS[BLocksNumToName[blockNumber]].startAddress), 6) + '^';
                modificationRecord += (halfBytes == 5) ? "05" : "03";
                modificationRecord += '\n';

                return objcode;
            }

            writeData = "Line: " + std::to_string(lineNumber);
            writeData += "Can't fit into program counter based or base register based addressing.";
            utilities::writeToFile(errorFile, writeData);
            hasError = true;
            objcode = utilities::intToStringHex(
                utilities::stringHexToInt(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            objcode += (halfBytes == 5)
                           ? (utilities::intToStringHex(xbpe + 1, 1) + "00")
                           : utilities::intToStringHex(xbpe, 1);
            objcode += "000";

            return objcode;
        }
    }
    return "";
}


void pass_2::writeEndRecord(const bool write) {
    if (write) {
        if (!endRecord.empty()) {
            utilities::writeToFile(objectFile, endRecord);
        } else {
            writeEndRecord(false);
        }
    }
    if (operand.empty() || operand == " ") {
        // If no operand of END
        endRecord = "E^" + utilities::intToStringHex(startAddress, 6);
    } else {
        // Make operand on end firstExecutableAddress

        const int firstExecutableAddress = utilities::stringHexToInt(tableStore->SYMTAB[firstExecutable_Sec].address);

        endRecord = "E^" + utilities::intToStringHex(firstExecutableAddress, 6) + "\n";
    }
}

pass_2::pass_2(std::string filename, table_store *tables, const std::string &intermediateFileName,
               const std::string &objectFileName, const std::string &listingFileName, const std::string &errorFileName,
               std::string *blocksNumToName, const std::string &firstExecutableSec, int progLength) {
    this->fileName = std::move(filename);
    this->tableStore = tables;
    this->BLocksNumToName = blocksNumToName;
    this->firstExecutable_Sec = firstExecutableSec;
    this->program_length = progLength;

    intermediateFile.open(intermediateFileName);
    if (!intermediateFile) {
        std::cerr << "Unable to open file: " << intermediateFileName << std::endl;
        exit(1);
    }

    objectFile.open(objectFileName);
    if (!objectFile) {
        std::cerr << "Unable to open file: " << objectFileName << std::endl;
        exit(1);
    }

    listingFile.open(listingFileName);
    if (!listingFile) {
        std::cerr << "Unable to open file: " << listingFileName << std::endl;
        exit(1);
    }

    errorFile.open(errorFileName, std::fstream::app);
    if (!errorFile) {
        std::cerr << "Unable to open file: " << errorFileName << std::endl;
        exit(1);
    }

    run_pass_2();
}

void pass_2::run_pass_2() {
    std::string tempBuffer;
    std::getline(intermediateFile, tempBuffer); // Discard heading line

    utilities::writeToFile(listingFile, "Line\tAddress\tLabel\tOPCODE\tOPERAND\tObjectCode\tComment");
    // utilities::writeToFile(errorFile, "\n\n************PASS2************");

    // Initialize variables
    objectCode = "";
    currentRecord = "";
    modificationRecord = "";
    blockNumber = 0;
    nobase = true;

    readIntermediateFile();
    while (isComment) {
        // Handle with previous comments
        writeData = std::to_string(lineNumber) + "\t" + comment;
        utilities::writeToFile(listingFile, writeData);
        readIntermediateFile();
    }

    if (opcode == "START") {
        startAddress = address;
        writeData = std::to_string(lineNumber) + "\t" + utilities::intToStringHex(address) + "\t" +
                    std::to_string(blockNumber) + "\t" + label + "\t" + opcode + "\t" + operand +
                    "\t" + objectCode + "\t" + comment;
        utilities::writeToFile(listingFile, writeData);
    } else {
        label = "";
        startAddress = 0;
        address = 0;
    }

    int program_section_length;
    if (tableStore->BLOCKS.size() <= 1) {
        program_section_length = program_length;
    } else {
        program_section_length = program_length;
    }

    writeData = "H^" + utilities::expandString(label, 6, ' ', true) + '^' +
                utilities::intToStringHex(address, 6) + '^' +
                utilities::intToStringHex(program_section_length, 6);
    utilities::writeToFile(objectFile, writeData);

    readIntermediateFile();

    while (opcode != "END") {
        if (!isComment) {
            if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].exists == 'y') {
                if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 1) {
                    objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode;
                } else if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 2) {
                    operand1 = operand.substr(0, operand.find(','));
                    operand2 = operand.substr(operand.find(',') + 1, operand.length() - operand.find(',') - 1);

                    if (operand2 == operand) {
                        // If not two operand i.e. a
                        if (utilities::getRealOpcode(opcode) == "SVC") {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                         utilities::intToStringHex(utilities::string_to_decimal(operand1), 1) + '0';
                        } else if (tableStore->REGTAB[operand1].exists == 'y') {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                         tableStore->REGTAB[operand1].num + '0';
                        } else {
                            objectCode = utilities::getRealOpcode(opcode) + '0' + '0';
                            writeData = "Line: " + std::to_string(lineNumber) + " Invalid Register name";
                            utilities::writeToFile(errorFile, writeData);
                            hasError = true;
                        }
                    } else {
                        // Two operands i.e. a,b
                        if (tableStore->REGTAB[operand1].exists == 'n') {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode + "00";
                            writeData = "Line: " + std::to_string(lineNumber) + " Invalid Register name";
                            utilities::writeToFile(errorFile, writeData);
                            hasError = true;
                        } else if (utilities::getRealOpcode(opcode) == "SHIFTR" || utilities::getRealOpcode(opcode) ==
                                   "SHIFTL") {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                         tableStore->REGTAB[operand1].num +
                                         utilities::intToStringHex(utilities::string_to_decimal(operand2), 1);
                        } else if (tableStore->REGTAB[operand2].exists == 'n') {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode + "00";
                            writeData = "Line: " + std::to_string(lineNumber) + " Invalid Register name";
                            utilities::writeToFile(errorFile, writeData);
                            hasError = true;
                        } else {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                         tableStore->REGTAB[operand1].num + tableStore->REGTAB[operand2].num;
                        }
                    }
                } else if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 3) {
                    if (utilities::getRealOpcode(opcode) == "RSUB") {
                        objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode;
                        objectCode += (utilities::getFlagFormat(opcode) == '+') ? "000000" : "0000";
                    } else {
                        objectCode = createObjectCodeFormat34();
                    }
                }
            } else if (opcode == "BYTE") {
                if (operand[0] == 'X') {
                    objectCode = operand.substr(2, operand.length() - 3);
                } else if (operand[0] == 'C') {
                    objectCode = utilities::stringToHexString(operand.substr(2, operand.length() - 3));
                }
            } else if (label == "*") {
                if (opcode[1] == 'C') {
                    objectCode = utilities::stringToHexString(opcode.substr(3, opcode.length() - 4));
                } else if (opcode[1] == 'X') {
                    objectCode = opcode.substr(3, opcode.length() - 4);
                }
            } else if (opcode == "WORD") {
                objectCode = utilities::intToStringHex(utilities::string_to_decimal(operand), 6);
            } else if (opcode == "BASE") {
                if (tableStore->SYMTAB[operand].exists == 'y') {
                    base_register_value = utilities::stringHexToInt(tableStore->SYMTAB[operand].address) +
                                          utilities::stringHexToInt(
                                              tableStore->BLOCKS[BLocksNumToName[tableStore->SYMTAB[operand].
                                                  blockNumber]].startAddress);
                    nobase = false;
                } else {
                    writeData = "Line " + std::to_string(lineNumber) + " : Symbol doesn't exists. Found " + operand;
                    utilities::writeToFile(errorFile, writeData);
                    hasError = true;
                }
                objectCode = "";
            } else if (opcode == "NOBASE") {
                if (nobase) {
                    writeData = "Line " + std::to_string(lineNumber) + ": Assembler wasn't using base addressing";
                    utilities::writeToFile(errorFile, writeData);
                    hasError = true;
                } else {
                    nobase = true;
                }
                objectCode = "";
            } else {
                objectCode = "";
            }

            // Write to text record if any generated
            writeTextRecord();

            if (blockNumber == -1 && address != -1) {
                writeData = std::to_string(lineNumber) + "\t" + utilities::intToStringHex(address) +
                            "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" +
                            objectCode + "\t" + comment;
            } else if (address == -1) {
                writeData = std::to_string(lineNumber) + "\t" + " " + "\t" + " " + "\t" + label +
                            "\t" + opcode + "\t" + operand + "\t" + objectCode + "\t" + comment;
            } else {
                writeData = std::to_string(lineNumber) + "\t" + utilities::intToStringHex(address) +
                            "\t" + std::to_string(blockNumber) + "\t" + label + "\t" + opcode + "\t" +
                            operand + "\t" + objectCode + "\t" + comment;
            }
        } else {
            writeData = std::to_string(lineNumber) + "\t" + comment;
        }
        utilities::writeToFile(listingFile, writeData);
        readIntermediateFile();
        objectCode = "";
    }

    writeTextRecord(true);

    writeEndRecord(false);

    if (!isComment) {
        writeData = std::to_string(lineNumber) + "\t" + utilities::intToStringHex(address) +
                    "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + "" +
                    "\t" + comment;
    } else {
        writeData = std::to_string(lineNumber) + "\t" + comment;
    }
    utilities::writeToFile(listingFile, writeData);

    while (readIntermediateFile()) {
        if (label == "*") {
            if (opcode[1] == 'C') {
                objectCode = utilities::stringToHexString(opcode.substr(3, opcode.length() - 4));
            } else if (opcode[1] == 'X') {
                objectCode = opcode.substr(3, opcode.length() - 4);
            }
            writeTextRecord();
        }
        writeData = std::to_string(lineNumber) + "\t" + utilities::intToStringHex(address) +
                    "\t" + std::to_string(blockNumber) + label + "\t" + opcode + "\t" + operand +
                    "\t" + objectCode + "\t" + comment;
        utilities::writeToFile(listingFile, writeData);
    }

    utilities::writeToFile(objectFile, modificationRecord, false);
    writeEndRecord(true);
}

pass_2::~pass_2() {
    intermediateFile.close();
    objectFile.close();
    listingFile.close();
    errorFile.close();
}

bool pass_2::has_error() const {
    return hasError;
}
