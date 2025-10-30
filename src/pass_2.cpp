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

    is_comment_ = fileLine[index] == '.';
    if (is_comment_) {
        utilities::first_non_whitespace(fileLine, index, tempStatus, comment, true);
        return true;
    }
    address = utilities::string_hex_to_int(readTillTab(fileLine, index));
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
    utilities::first_non_whitespace(fileLine, index, tempStatus, comment, true);
    return true;
}

void pass_2::writeTextRecord(const bool lastRecord) {
    if (lastRecord) {
        if (!currentRecord.empty()) {
            // Write last text record
            writeData = utilities::int_to_string_hex(static_cast<int>(currentRecord.length() / 2), 2) + '^' +
                        currentRecord;
            utilities::write_to_file(objectFile, writeData);
            currentRecord = "";
        }
        return;
    }
    if (!objectCode.empty()) {
        if (currentRecord.empty()) {
            writeData = "T^" + utilities::int_to_string_hex(
                            address + utilities::string_hex_to_int(
                                tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress), 6) + '^';
            utilities::write_to_file(objectFile, writeData, false);
        }
        if ((currentRecord + objectCode).length() > 60) {
            writeData = utilities::int_to_string_hex(static_cast<int>(currentRecord.length() / 2), 2) + '^' +
                        currentRecord;
            utilities::write_to_file(objectFile, writeData);
            currentRecord = "";
            writeData = "T^" + utilities::int_to_string_hex(
                            address + utilities::string_hex_to_int(
                                tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress), 6) + '^';
            utilities::write_to_file(objectFile, writeData, false);
        }

        currentRecord += objectCode;
    } else {
        if (opcode == "START" || opcode == "END" || opcode == "BASE" || opcode == "NOBASE" || opcode == "LTORG" ||
            opcode == "ORG" || opcode == "EQU") {
        } else {
            // Write current record if exists
            if (!currentRecord.empty()) {
                writeData = utilities::int_to_string_hex(static_cast<int>(currentRecord.length() / 2), 2) + '^' +
                            currentRecord;
                utilities::write_to_file(objectFile, writeData);
            }
            currentRecord = "";
        }
    }
}

std::string pass_2::createObjectCodeFormat34() {
    std::string object_code;
    int halfBytes;
    if ((utilities::getFlagFormat(opcode) == '+'))
        halfBytes = 5;
    else
        halfBytes = 3;

    if (utilities::getFlagFormat(operand) == '#') {
        if (operand.substr(operand.length() - 2, 2) == ",X") {
            // Error handling for Immediate with index based
            writeData = "Line: " + std::to_string(lineNumber) +
                        " Index based addressing not supported with Indirect addressing";
            utilities::write_to_file(errorFile, writeData);
            hasError = true;
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            object_code += (halfBytes == 5) ? "100000" : "0000";
            return object_code;
        }
        std::string tempOperand = operand.substr(1, operand.length() - 1);
        if (utilities::if_all_num(tempOperand) || (
                (tableStore->SYMTAB[tempOperand].exists == 'y') && (tableStore->SYMTAB[tempOperand].relative == 0))) {
            // Immediate integer value
            int immediateValue;

            if (utilities::if_all_num(tempOperand)) {
                immediateValue = utilities::string_to_decimal(tempOperand);
            } else {
                immediateValue = utilities::string_hex_to_int(tableStore->SYMTAB[tempOperand].address);
            }
            /*Process Immediate value*/
            if (immediateValue >= (1 << 4 * halfBytes)) {
                // Can't fit it
                writeData = "Line: " + std::to_string(lineNumber) + " Immediate value exceeds format limit";
                utilities::write_to_file(errorFile, writeData);
                hasError = true;
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
                object_code += (halfBytes == 5) ? "100000" : "0000";
            } else {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
                object_code += (halfBytes == 5) ? '1' : '0';
                object_code += utilities::int_to_string_hex(immediateValue, halfBytes);
            }
            return object_code;
        }
        if (tableStore->SYMTAB[tempOperand].exists == 'n') {
            writeData = "Line " + std::to_string(lineNumber);
            writeData += " : Symbol does not exist. Found " + tempOperand;
            utilities::write_to_file(errorFile, writeData);
            hasError = true;
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            object_code += (halfBytes == 5) ? "100000" : "0000";
            return object_code;
        }
        int operand_address = utilities::string_hex_to_int(tableStore->SYMTAB[tempOperand].address) +
                              utilities::string_hex_to_int(
                                  tableStore->BLOCKS[blocks_num_to_name[tableStore->SYMTAB[tempOperand].blockNumber]].
                                  startAddress);

        /*Process Immediate symbol value*/
        if (halfBytes == 5) {
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            object_code += '1';
            object_code += utilities::int_to_string_hex(operand_address, halfBytes);

            modificationRecord += "M^" + utilities::int_to_string_hex(address + 1, 6) + '^';
            if (halfBytes == 5)
                modificationRecord += "05";
            else
                modificationRecord += "03";
            modificationRecord += '\n';

            return object_code;
        }
        program_counter = address + utilities::string_hex_to_int(
                              tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress);
        if (halfBytes == 5)
            program_counter += 4;
        else
            program_counter += 3;
        int relative_address = operand_address - program_counter;

        if (relative_address >= (-2048) && relative_address <= 2047) {
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            object_code += '2';
            object_code += utilities::int_to_string_hex(relative_address, halfBytes);
            return object_code;
        }

        if (!not_base) {
            relative_address = operand_address - base_register_value;
            if (relative_address >= 0 && relative_address <= 4095) {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
                object_code += '4';
                object_code += utilities::int_to_string_hex(relative_address, halfBytes);
                return object_code;
            }
        }

        if (operand_address <= 4095) {
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 1, 2);
            object_code += '0';
            object_code += utilities::int_to_string_hex(operand_address, halfBytes);
            modificationRecord += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress), 6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return object_code;
        }
    } else if (utilities::getFlagFormat(operand) == '@') {
        std::string tempOperand = operand.substr(1, operand.length() - 1);
        if (tempOperand.substr(tempOperand.length() - 2, 2) == ",X" || tableStore->SYMTAB[tempOperand].exists == 'n') {
            writeData = "Line " + std::to_string(lineNumber);
            writeData +=
                    " : Symbol does not exist. Index based addressing not supported with Indirect addressing ";
            utilities::write_to_file(errorFile, writeData);
            hasError = true;
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
            object_code += (halfBytes == 5) ? "100000" : "0000";
            return object_code;
        }
        int operandAddress = utilities::string_hex_to_int(tableStore->SYMTAB[tempOperand].address) +
                             utilities::string_hex_to_int(
                                 tableStore->BLOCKS[blocks_num_to_name[tableStore->SYMTAB[tempOperand].blockNumber]].
                                 startAddress);
        program_counter = address + utilities::string_hex_to_int(
                              tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress);
        program_counter += (halfBytes == 5) ? 4 : 3;

        if (halfBytes == 3) {
            int relativeAddress = operandAddress - program_counter;
            if (relativeAddress >= (-2048) && relativeAddress <= 2047) {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
                object_code += '2';
                object_code += utilities::int_to_string_hex(relativeAddress, halfBytes);
                return object_code;
            }

            if (!not_base) {
                relativeAddress = operandAddress - base_register_value;
                if (relativeAddress >= 0 && relativeAddress <= 4095) {
                    object_code = utilities::int_to_string_hex(
                        utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2,
                        2);
                    object_code += '4';
                    object_code += utilities::int_to_string_hex(relativeAddress, halfBytes);
                    return object_code;
                }
            }

            if (operandAddress <= 4095) {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
                object_code += '0';
                object_code += utilities::int_to_string_hex(operandAddress, halfBytes);

                /*add modifacation record here*/
                modificationRecord += "M^" + utilities::int_to_string_hex(
                    address + 1 + utilities::string_hex_to_int(
                        tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress), 6) + '^';
                modificationRecord += (halfBytes == 5) ? "05" : "03";
                modificationRecord += '\n';

                return object_code;
            }
        } else {
            // No base or pc based addressing in format 4
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
            object_code += '1';
            object_code += utilities::int_to_string_hex(operandAddress, halfBytes);
            modificationRecord += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress),
                6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return object_code;
        }

        writeData = "Line: " + std::to_string(lineNumber);
        writeData += "Cannot fit into program counter or base register based addressing.";
        utilities::write_to_file(errorFile, writeData);
        hasError = true;
        object_code = utilities::int_to_string_hex(
            utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
        object_code += (halfBytes == 5) ? "100000" : "0000";

        return object_code;
    } else if (utilities::getFlagFormat(operand) == '=') {
        // Literals
        std::string tempOperand = operand.substr(1, operand.length() - 1);

        if (tempOperand == "*") {
            tempOperand = "X'" + utilities::int_to_string_hex(address, 6) + "'";
            /*Add modification record for value created by operand `*` */
            modificationRecord += "M^" + utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->LITTAB[tempOperand].address) + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[tableStore->LITTAB[tempOperand].blockNumber]].startAddress),
                6) + '^';
            modificationRecord += utilities::int_to_string_hex(6, 2);
            modificationRecord += '\n';
        }

        if (tableStore->LITTAB[tempOperand].exists == 'n') {
            writeData = "Line " + std::to_string(lineNumber) + " : Symbol doesn't exists. Found " + tempOperand;
            utilities::write_to_file(errorFile, writeData);
            hasError = true;
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            object_code += (halfBytes == 5) ? "000" : "0";
            object_code += "000";
            return object_code;
        }

        int operandAddress = utilities::string_hex_to_int(tableStore->LITTAB[tempOperand].address) +
                             utilities::string_hex_to_int(
                                 tableStore->BLOCKS[blocks_num_to_name[tableStore->LITTAB[tempOperand].blockNumber]].
                                 startAddress);
        program_counter = address + utilities::string_hex_to_int(
                              tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress);
        program_counter += (halfBytes == 5) ? 4 : 3;

        if (halfBytes == 3) {
            int relativeAddress = operandAddress - program_counter;
            if (relativeAddress >= (-2048) && relativeAddress <= 2047) {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                object_code += '2';
                object_code += utilities::int_to_string_hex(relativeAddress, halfBytes);
                return object_code;
            }

            if (!not_base) {
                relativeAddress = operandAddress - base_register_value;
                if (relativeAddress >= 0 && relativeAddress <= 4095) {
                    object_code = utilities::int_to_string_hex(
                        utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3,
                        2);
                    object_code += '4';
                    object_code += utilities::int_to_string_hex(relativeAddress, halfBytes);
                    return object_code;
                }
            }

            if (operandAddress <= 4095) {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                object_code += '0';
                object_code += utilities::int_to_string_hex(operandAddress, halfBytes);

                modificationRecord += "M^" + utilities::int_to_string_hex(
                    address + 1 + utilities::string_hex_to_int(
                        tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress), 6) + '^';
                modificationRecord += (halfBytes == 5) ? "05" : "03";
                modificationRecord += '\n';

                return object_code;
            }
        } else {
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            object_code += '1';
            object_code += utilities::int_to_string_hex(operandAddress, halfBytes);

            modificationRecord += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress),
                6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return object_code;
        }

        writeData = "Line: " + std::to_string(lineNumber);
        writeData += "Cannot fit into program counter or base register based addressing.";
        utilities::write_to_file(errorFile, writeData);
        hasError = true;
        object_code = utilities::int_to_string_hex(
            utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
        object_code += (halfBytes == 5) ? "100" : "0";
        object_code += "000";

        return object_code;
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
            utilities::write_to_file(errorFile, writeData);
            hasError = true;
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            object_code += "0000";
            return object_code;
        }

        if (tableStore->SYMTAB[tempOperand].exists == 'n') {
            writeData = "Line " + std::to_string(lineNumber);
            writeData += " : Symbol doesn't exists. Found " + tempOperand;
            utilities::write_to_file(errorFile, writeData);
            hasError = true;
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            if (halfBytes == 5)
                object_code += utilities::int_to_string_hex(xbpe + 1, 1) + "00";
            else
                object_code += utilities::int_to_string_hex(xbpe, 1);
            object_code += "000";
            return object_code;
        }
        int operandAddress = utilities::string_hex_to_int(tableStore->SYMTAB[tempOperand].address) +
                             utilities::string_hex_to_int(
                                 tableStore->BLOCKS[blocks_num_to_name[tableStore->SYMTAB[tempOperand].blockNumber]].
                                 startAddress);
        program_counter = address + utilities::string_hex_to_int(
                              tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress);
        if (halfBytes == 5)
            program_counter += 4;
        else
            program_counter += 3;

        // std::cout << halfBytes << std::endl;
        if (halfBytes == 3) {
            int relativeAddress = operandAddress - program_counter;
            if (relativeAddress >= (-2048) && relativeAddress <= 2047) {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                object_code += utilities::int_to_string_hex(xbpe + 2, 1);
                object_code += utilities::int_to_string_hex(relativeAddress, halfBytes);
                return object_code;
            }

            if (!not_base) {
                relativeAddress = operandAddress - base_register_value;
                if (relativeAddress >= 0 && relativeAddress <= 4095) {
                    object_code = utilities::int_to_string_hex(
                        utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3,
                        2);
                    object_code += utilities::int_to_string_hex(xbpe + 4, 1);
                    object_code += utilities::int_to_string_hex(relativeAddress, halfBytes);
                    return object_code;
                }
            }

            if (operandAddress <= 4095) {
                object_code = utilities::int_to_string_hex(
                    utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
                object_code += utilities::int_to_string_hex(xbpe, 1);
                object_code += utilities::int_to_string_hex(operandAddress, halfBytes);

                modificationRecord += "M^" + utilities::int_to_string_hex(
                    address + 1 + utilities::string_hex_to_int(
                        tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress), 6) + '^';
                modificationRecord += (halfBytes == 5) ? "05" : "03";
                modificationRecord += '\n';

                return object_code;
            }
        } else {
            // No base or pc based addressing in format 4
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            object_code += utilities::int_to_string_hex(xbpe + 1, 1);
            object_code += utilities::int_to_string_hex(operandAddress, halfBytes);

            /*add modifacation record here*/
            modificationRecord += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[blockNumber]].startAddress), 6) + '^';
            modificationRecord += (halfBytes == 5) ? "05" : "03";
            modificationRecord += '\n';

            return object_code;
        }

        writeData = "Line: " + std::to_string(lineNumber);
        writeData += "Cannot fit into program counter based or base register based addressing.";
        utilities::write_to_file(errorFile, writeData);
        hasError = true;
        object_code = utilities::int_to_string_hex(
            utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
        object_code += (halfBytes == 5)
                           ? (utilities::int_to_string_hex(xbpe + 1, 1) + "00")
                           : utilities::int_to_string_hex(xbpe, 1);
        object_code += "000";

        return object_code;
    }
    return "";
}


void pass_2::writeEndRecord(const bool write) {
    if (write) {
        if (!endRecord.empty()) {
            utilities::write_to_file(objectFile, endRecord);
        } else {
            writeEndRecord(false);
        }
    }
    if (operand.empty() || operand == " ") {
        endRecord = "E^" + utilities::int_to_string_hex(startAddress, 6);
    } else {
        const int firstExecutableAddress =
                utilities::string_hex_to_int(tableStore->SYMTAB[firstExecutable_Sec].address);

        endRecord = "E^" + utilities::int_to_string_hex(firstExecutableAddress, 6) + "\n";
    }
}

pass_2::pass_2(std::string filename, table_store *tables, const std::string &intermediateFileName,
               const std::string &objectFileName, const std::string &listingFileName, const std::string &errorFileName,
               std::string *blocksNumToName, const std::string &firstExecutableSec, int progLength) {
    this->fileName = std::move(filename);
    this->tableStore = tables;
    this->blocks_num_to_name = blocksNumToName;
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
    std::getline(intermediateFile, tempBuffer);

    utilities::write_to_file(listingFile, "Line\tAddress\tLabel\tOPCODE\tOPERAND\tObjectCode\tComment");
    // utilities::write_to_file(errorFile, "\n\nPASS2:");

    readIntermediateFile();
    while (is_comment_) {
        writeData = std::to_string(lineNumber) + "\t" + comment;
        utilities::write_to_file(listingFile, writeData);
        readIntermediateFile();
    }

    if (opcode == "START") {
        startAddress = address;
        writeData = std::to_string(lineNumber) + "\t" + utilities::int_to_string_hex(address) + "\t" +
                    std::to_string(blockNumber) + "\t" + label + "\t" + opcode + "\t" + operand +
                    "\t" + objectCode + "\t" + comment;
        utilities::write_to_file(listingFile, writeData);
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
                utilities::int_to_string_hex(address, 6) + '^' +
                utilities::int_to_string_hex(program_section_length, 6);
    utilities::write_to_file(objectFile, writeData);

    readIntermediateFile();

    while (opcode != "END") {
        if (!is_comment_) {
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
                                         utilities::int_to_string_hex(utilities::string_to_decimal(operand1), 1) + '0';
                        } else if (tableStore->REGTAB[operand1].exists == 'y') {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                         tableStore->REGTAB[operand1].num + '0';
                        } else {
                            objectCode = utilities::getRealOpcode(opcode) + '0' + '0';
                            writeData = "Line: " + std::to_string(lineNumber) + " Invalid Register name";
                            utilities::write_to_file(errorFile, writeData);
                            hasError = true;
                        }
                    } else {
                        // Two operands i.e. a,b
                        if (tableStore->REGTAB[operand1].exists == 'n') {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode + "00";
                            writeData = "Line: " + std::to_string(lineNumber) + " Invalid Register name";
                            utilities::write_to_file(errorFile, writeData);
                            hasError = true;
                        } else if (utilities::getRealOpcode(opcode) == "SHIFTR" || utilities::getRealOpcode(opcode) ==
                                   "SHIFTL") {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                         tableStore->REGTAB[operand1].num +
                                         utilities::int_to_string_hex(utilities::string_to_decimal(operand2), 1);
                        } else if (tableStore->REGTAB[operand2].exists == 'n') {
                            objectCode = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode + "00";
                            writeData = "Line: " + std::to_string(lineNumber) + " Invalid Register name";
                            utilities::write_to_file(errorFile, writeData);
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
                objectCode = utilities::int_to_string_hex(utilities::string_to_decimal(operand), 6);
            } else if (opcode == "BASE") {
                if (tableStore->SYMTAB[operand].exists == 'y') {
                    base_register_value = utilities::string_hex_to_int(tableStore->SYMTAB[operand].address) +
                                          utilities::string_hex_to_int(
                                              tableStore->BLOCKS[blocks_num_to_name[tableStore->SYMTAB[operand].
                                                  blockNumber]].startAddress);
                    not_base = false;
                } else {
                    writeData = "Line " + std::to_string(lineNumber) + " : Symbol doesn't exists. Found " + operand;
                    utilities::write_to_file(errorFile, writeData);
                    hasError = true;
                }
                objectCode = "";
            } else if (opcode == "NOBASE") {
                if (not_base) {
                    writeData = "Line " + std::to_string(lineNumber) + ": Assembler wasn't using base addressing";
                    utilities::write_to_file(errorFile, writeData);
                    hasError = true;
                } else {
                    not_base = true;
                }
                objectCode = "";
            } else {
                objectCode = "";
            }

            // Write to text record if any generated
            writeTextRecord();

            if (blockNumber == -1 && address != -1) {
                writeData = std::to_string(lineNumber) + "\t" + utilities::int_to_string_hex(address) +
                            "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" +
                            objectCode + "\t" + comment;
            } else if (address == -1) {
                writeData = std::to_string(lineNumber) + "\t" + " " + "\t" + " " + "\t" + label +
                            "\t" + opcode + "\t" + operand + "\t" + objectCode + "\t" + comment;
            } else {
                writeData = std::to_string(lineNumber) + "\t" + utilities::int_to_string_hex(address) +
                            "\t" + std::to_string(blockNumber) + "\t" + label + "\t" + opcode + "\t" +
                            operand + "\t" + objectCode + "\t" + comment;
            }
        } else {
            writeData = std::to_string(lineNumber) + "\t" + comment;
        }
        utilities::write_to_file(listingFile, writeData);
        readIntermediateFile();
        objectCode = "";
    }

    writeTextRecord(true);

    writeEndRecord(false);

    if (!is_comment_) {
        writeData = std::to_string(lineNumber) + "\t" + utilities::int_to_string_hex(address) +
                    "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + "" +
                    "\t" + comment;
    } else {
        writeData = std::to_string(lineNumber) + "\t" + comment;
    }
    utilities::write_to_file(listingFile, writeData);

    while (readIntermediateFile()) {
        if (label == "*") {
            if (opcode[1] == 'C') {
                objectCode = utilities::stringToHexString(opcode.substr(3, opcode.length() - 4));
            } else if (opcode[1] == 'X') {
                objectCode = opcode.substr(3, opcode.length() - 4);
            }
            writeTextRecord();
        }
        writeData = std::to_string(lineNumber) + "\t" + utilities::int_to_string_hex(address) +
                    "\t" + std::to_string(blockNumber) + label + "\t" + opcode + "\t" + operand +
                    "\t" + objectCode + "\t" + comment;
        utilities::write_to_file(listingFile, writeData);
    }

    utilities::write_to_file(objectFile, modificationRecord, false);
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
