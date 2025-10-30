#include "pass_1.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "utilities.h"

using namespace std;

// Helper function to format output with tabs (matching functional version)
string formatIntermediateLine(const string &line, const string &address, const string &block,
                              const string &label, const string &opcode, const string &operand,
                              const string &comment) {
    return line + "\t" + address + "\t" + block + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment;
}

std::string pass_1::get_first_executable_sec() {
    return firstExecutable_Sec;
}

std::string * pass_1::get_blocks_num_to_name() const {
    return BLocksNumToName;
}

int pass_1::get_program_length() const {
    return program_length;
}

bool pass_1::get_error() const {
    return error_flag;
}

void pass_1::run_pass_1() {
    utilities::writeToFile(errorFile, "**********PASS1************");
    utilities::writeToFile(intermediateFile, "Line\tAddress\tBlock\tLabel\tOPCODE\tOPERAND\tComment");

    getline(SourceFile, fileLine);
    lineNumber += 5;
    while (utilities::checkCommentLine(fileLine)) {
        writeData = to_string(lineNumber) + "\t" + fileLine;
        utilities::writeToFile(intermediateFile, writeData);
        getline(SourceFile, fileLine); // read first input line
        lineNumber += 5;
        index = 0;
    }

    utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, label);
    utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, opcode);

    if (opcode == "START") {
        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, comment, true);
        startAddress = utilities::stringHexToInt(operand);
        LOCCTR = startAddress;
        writeData = to_string(lineNumber) + "\t" + utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR) + "\t" +
                   to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment;
        utilities::writeToFile(intermediateFile, writeData);

        getline(SourceFile, fileLine);
        lineNumber += 5;
        index = 0;
        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, label);
        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, opcode);
    } else {
        startAddress = 0;
        LOCCTR = 0;
    }

    while (opcode != "END") {
        if (!utilities::checkCommentLine(fileLine)) {
            if (!label.empty()) {
                // Label found
                if (tableStore->SYMTAB[label].exists == 'n') {
                    tableStore->SYMTAB[label].name = label;
                    tableStore->SYMTAB[label].address = utilities::int_to_string_hex(LOCCTR);
                    tableStore->SYMTAB[label].relative = 1;
                    tableStore->SYMTAB[label].exists = 'y';
                    tableStore->SYMTAB[label].blockNumber = currentBlockNumber;
                } else {
                    writeData = "Line: " + to_string(lineNumber) + " : Duplicate symbol for '" + label +
                                "'. Previously defined at " + tableStore->SYMTAB[label].address;
                    utilities::writeToFile(errorFile, writeData);
                    error_flag = true;
                }
            }
            if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].exists == 'y') {
                // Search for opcode in tableStore->OPTAB
                if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 3) {
                    LOCCTR += 3;
                    lastDeltaLOCCTR += 3;
                    if (utilities::getFlagFormat(opcode) == '+') {
                        LOCCTR += 1;
                        lastDeltaLOCCTR += 1;
                    }
                    if (utilities::getRealOpcode(opcode) == "RSUB") {
                        operand = " ";
                    } else {
                        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                        if (operand[operand.length() - 1] == ',') {
                            utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, tempOperand);
                            operand += tempOperand;
                        }
                    }

                    if (utilities::getFlagFormat(operand) == '=') {
                        tempOperand = operand.substr(1, operand.length() - 1);
                        if (tempOperand == "*") {
                            tempOperand = "X'" + utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR, 6) + "'";
                        }
                        if (tableStore->LITTAB[tempOperand].exists == 'n') {
                            tableStore->LITTAB[tempOperand].value = tempOperand;
                            tableStore->LITTAB[tempOperand].exists = 'y';
                            tableStore->LITTAB[tempOperand].address = "?";
                            tableStore->LITTAB[tempOperand].blockNumber = -1;
                        }
                    }
                } else if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 1) {
                    operand = " ";
                    LOCCTR += tableStore->OPTAB[utilities::getRealOpcode(opcode)].format;
                    lastDeltaLOCCTR += tableStore->OPTAB[utilities::getRealOpcode(opcode)].format;
                } else {
                    LOCCTR += tableStore->OPTAB[utilities::getRealOpcode(opcode)].format;
                    lastDeltaLOCCTR += tableStore->OPTAB[utilities::getRealOpcode(opcode)].format;
                    utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                    if (operand[operand.length() - 1] == ',') {
                        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, tempOperand);
                        operand += tempOperand;
                    }
                }
            } else if (opcode == "WORD") {
                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                LOCCTR += 3;
                lastDeltaLOCCTR += 3;
            } else if (opcode == "RESW") {
                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                LOCCTR += 3 * utilities::string_to_decimal(operand);
                lastDeltaLOCCTR += 3 * utilities::string_to_decimal(operand);
            } else if (opcode == "RESB") {
                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                LOCCTR += utilities::string_to_decimal(operand);
                lastDeltaLOCCTR += utilities::string_to_decimal(operand);
            } else if (opcode == "BYTE") {
                utilities::readByteOperand(fileLine, index, statusCode, operand);
                if (operand[0] == 'X') {
                    LOCCTR += (static_cast<int>(operand.length()) - 3) / 2;
                    lastDeltaLOCCTR += (static_cast<int>(operand.length()) - 3) / 2;
                } else if (operand[0] == 'C') {
                    LOCCTR += static_cast<int>(operand.length()) - 3;
                    lastDeltaLOCCTR += static_cast<int>(operand.length()) - 3;
                }
                // else{
                //   writeData = "Line: "+to_string(line)+" : Invalid operand for BYTE. Found " + operand;
                //   utilities::writeToFile(errorFile,writeData);
                // }
            } else if (opcode == "BASE") {
                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
            } else if (opcode == "LTORG") {
                operand = " ";
                handle_LTORG(writeDataSuffix, lineNumberDelta, lineNumber, LOCCTR, lastDeltaLOCCTR,
                             currentBlockNumber);
            } else if (opcode == "ORG") {
                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);

                char lastByte = operand[operand.length() - 1];
                while (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*') {
                    utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, tempOperand);
                    operand += tempOperand;
                    lastByte = operand[operand.length() - 1];
                }

                int tempVariable;
                tempVariable = saveLOCCTR;
                saveLOCCTR = LOCCTR;
                LOCCTR = tempVariable;

                if (tableStore->SYMTAB[operand].exists == 'y') {
                    LOCCTR = utilities::stringHexToInt(tableStore->SYMTAB[operand].address);
                } else {
                    bool relative;
                    // set error_flag to false
                    error_flag = false;
                    evaluateExpression(operand, relative);
                    if (!error_flag) {
                        LOCCTR = utilities::stringHexToInt(tempOperand);
                    }
                    error_flag = false; // reset error_flag
                }
            } else if (opcode == "USE") {
                // cout<<"Changing block"<<endl;
                // for(auto const& it: tableStore->BLOCKS){
                //   cout<<it.second.name<<":"<<it.second.LOCCTR<<endl;
                // }
                //
                // cout<<"Current block number: "<<currentBlockNumber<<l;
                // cout<<"Current LOCCTR: "<<LOCCTR<<endl;

                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                tableStore->BLOCKS[currentBlockName].LOCCTR = utilities::int_to_string_hex(LOCCTR);

                if (tableStore->BLOCKS[operand].exists == 'n') {
                    // cout<<"Creating block: "<<operand<<endl;
                    tableStore->BLOCKS[operand].exists = 'y';
                    tableStore->BLOCKS[operand].name = operand;
                    tableStore->BLOCKS[operand].number = totalBlocks++;
                    tableStore->BLOCKS[operand].LOCCTR = "0";
                }

                // cout<<"Changing to: "<<operand<<endl;
                // for(auto const& it: tableStore->BLOCKS){
                //   cout<<it.second.name<<":"<<it.second.LOCCTR<<endl;
                // }

                currentBlockNumber = tableStore->BLOCKS[operand].number;
                currentBlockName = tableStore->BLOCKS[operand].name;
                LOCCTR = utilities::stringHexToInt(tableStore->BLOCKS[operand].LOCCTR);

                // cout<<"Current block number: "<<currentBlockNumber<<endl;
                // cout<<"Current LOCCTR: "<<LOCCTR<<endl;
            } else if (opcode == "EQU") {
                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                tempOperand = "";
                bool relative;

                if (operand == "*") {
                    tempOperand = utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR, 6);
                    relative = true;
                } else if (utilities::if_all_num(operand)) {
                    tempOperand = utilities::int_to_string_hex(utilities::string_to_decimal(operand), 6);
                    relative = false;
                } else {
                    char lastByte = operand[operand.length() - 1];

                    while (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*') {
                        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, tempOperand);
                        operand += tempOperand;
                        lastByte = operand[operand.length() - 1];
                    }

                    // Code for reading whole operand
                    evaluateExpression(operand, relative);
                }

                tableStore->SYMTAB[label].name = label;
                tableStore->SYMTAB[label].address = tempOperand;
                tableStore->SYMTAB[label].relative = relative;
                tableStore->SYMTAB[label].blockNumber = currentBlockNumber;
                lastDeltaLOCCTR = LOCCTR - utilities::stringHexToInt(tempOperand);
            } else {
                utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
                writeData = "Line: " + to_string(lineNumber) + " : Invalid OPCODE. Found " + opcode;
                utilities::writeToFile(errorFile, writeData);
                error_flag = true;
            }
            utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, comment, true);
            if (opcode == "EQU" && tableStore->SYMTAB[label].relative == 0) {
                writeData = writeDataPrefix + to_string(lineNumber) + "\t" +
                           utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR) + "\t" + " " + "\t" +
                           label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
            } else {
                writeData = writeDataPrefix + to_string(lineNumber) + "\t" +
                           utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR) + "\t" +
                           to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" +
                           operand + "\t" + comment + writeDataSuffix;
            }
            writeDataPrefix = "";
            writeDataSuffix = "";
        } else {
            writeData = to_string(lineNumber) + "\t" + fileLine;
        }
        utilities::writeToFile(intermediateFile, writeData);

        tableStore->BLOCKS[currentBlockName].LOCCTR = utilities::int_to_string_hex(LOCCTR);
        // Update LOCCTR of block after every instruction
        getline(SourceFile, fileLine); // Read next line
        lineNumber += 5 + lineNumberDelta;
        lineNumberDelta = 0;
        index = 0;
        lastDeltaLOCCTR = 0;
        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, label); // Parse label
        utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, opcode); // Parse OPCODE
    }

    if (opcode == "END") {
        firstExecutable_Sec = tableStore->SYMTAB[label].address;
        tableStore->SYMTAB[firstExecutable_Sec].name = label;
        tableStore->SYMTAB[firstExecutable_Sec].address = firstExecutable_Sec;
    }

    utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, operand);
    utilities::readFirstNonWhiteSpace(fileLine, index, statusCode, comment, true);

    /*Change to deafult before dumping literals*/
    currentBlockName = "DEFAULT";
    currentBlockNumber = 0;
    LOCCTR = utilities::stringHexToInt(tableStore->BLOCKS[currentBlockName].LOCCTR);

    handle_LTORG(writeDataSuffix, lineNumberDelta, lineNumber, LOCCTR, lastDeltaLOCCTR, currentBlockNumber);

    writeData = to_string(lineNumber) + "\t" + utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR) + "\t" +
               " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
    utilities::writeToFile(intermediateFile, writeData);

    int LocctrArr[totalBlocks];
    BLocksNumToName = new string[totalBlocks];
    for (auto const &it: tableStore->BLOCKS) {
        LocctrArr[it.second.number] = utilities::stringHexToInt(it.second.LOCCTR);
        BLocksNumToName[it.second.number] = it.first;
    }

    for (int i = 1; i < totalBlocks; i++) {
        LocctrArr[i] += LocctrArr[i - 1];
    }

    for (auto const &it: tableStore->BLOCKS) {
        if (it.second.startAddress == "?") {
            tableStore->BLOCKS[it.first].startAddress = utilities::int_to_string_hex(LocctrArr[it.second.number - 1]);
        }
    }

    program_length = LocctrArr[totalBlocks - 1] - startAddress;
}

void pass_1::evaluateExpression(std::string expression, bool &relative
) {
    if (!expression.empty() && expression[0] == '#') {
        string numPart = expression.substr(1);
        if (utilities::if_all_num(numPart)) {
            // It's an immediate numeric value, not a symbol
            tempOperand = utilities::int_to_string_hex(utilities::string_to_decimal(numPart), 6);
            relative = false;
            return;
        }
        // Remove '#' for symbol lookup
        expression = numPart;
    }

    // Handle pure numeric values
    if (utilities::if_all_num(expression)) {
        tempOperand = utilities::int_to_string_hex(utilities::string_to_decimal(expression), 6);
        relative = false;
        return;
    }

    string singleOperand = "?";
    string singleOperator = "?";
    string valueString;
    string valueTemp;
    string writeData;
    int lastOperand = 0, lastOperator = 0, pairCount = 0;
    char lastByte = ' ';
    bool Illegal = false;

    for (int i = 0; i < expression.length();) {
        singleOperand = "";

        lastByte = expression[i];
        while ((lastByte != '+' && lastByte != '-' && lastByte != '/' && lastByte != '*') && i < expression.length()) {
            singleOperand += lastByte;
            lastByte = expression[++i];
        }

        if (tableStore->SYMTAB[singleOperand].exists == 'y') {
            // Check operand existence
            lastOperand = tableStore->SYMTAB[singleOperand].relative;
            valueTemp = to_string(utilities::stringHexToInt(tableStore->SYMTAB[singleOperand].address));
        } else if ((!singleOperand.empty() || singleOperand != "?") && utilities::if_all_num(singleOperand)) {
            lastOperand = 0;
            valueTemp = singleOperand;
        } else {
            writeData = "Line: " + to_string(lineNumber) + " : Can't find symbol. Found " + singleOperand;
            utilities::writeToFile(errorFile, writeData);
            Illegal = true;
            break;
        }

        if (lastOperand * lastOperator == 1) {
            // Check expressions legality
            writeData = "Line: " + to_string(lineNumber) + " : Illegal expression";
            utilities::writeToFile(errorFile, writeData);
            error_flag = true;
            Illegal = true;
            break;
        }
        if ((singleOperator == "-" || singleOperator == "+" || singleOperator == "?") && lastOperand == 1) {
            if (singleOperator == "-") {
                pairCount--;
            } else {
                pairCount++;
            }
        }

        valueString += valueTemp;

        singleOperator = "";
        while (i < expression.length() && (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*')) {
            singleOperator += lastByte;
            lastByte = expression[++i];
        }

        if (singleOperator.length() > 1) {
            writeData = "Line: " + to_string(lineNumber) + " : Illegal operator in expression. Found " + singleOperator;
            utilities::writeToFile(errorFile, writeData);
            error_flag = true;
            Illegal = true;
            break;
        }

        if (singleOperator == "*" || singleOperator == "/") {
            lastOperator = 1;
        } else {
            lastOperator = 0;
        }

        valueString += singleOperator;
    }

    if (!Illegal) {
        if (pairCount == 1) {
            /*relative*/
            relative = true;
            EvaluateString tempOBJ(valueString);
            tempOperand = utilities::int_to_string_hex(tempOBJ.getResult());
        } else if (pairCount == 0) {
            /*absolute*/
            relative = false;
            // cout << valueString << endl;
            EvaluateString tempOBJ(valueString);
            tempOperand = utilities::int_to_string_hex(tempOBJ.getResult());
        } else {
            writeData = "Line: " + to_string(lineNumber) + " : Illegal expression";
            utilities::writeToFile(errorFile, writeData);
            error_flag = true;
            tempOperand = "00000";
            relative = false;
        }
    } else {
        tempOperand = "00000";
        error_flag = true;
        relative = false;
    }
}

void pass_1::handle_LTORG(std::string &litPrefix, int &lineNumberDelta, int lineNumber, int &LOCCTR,
                          int &lastDeltaLOCCTR, const int currentBlockNumber) const {
    litPrefix = "";
    for (const auto &[fst, snd]: tableStore->LITTAB) {
        string litAddress = snd.address;
        string litValue = snd.value;
        if (litAddress != "?") {
            /*Pass as already address is assigned*/
        } else {
            lineNumber += 5;
            lineNumberDelta += 5;
            tableStore->LITTAB[fst].address = utilities::int_to_string_hex(LOCCTR);
            tableStore->LITTAB[fst].blockNumber = currentBlockNumber;

            litPrefix += "\n" + to_string(lineNumber) + "\t" + utilities::int_to_string_hex(LOCCTR) + "\t" +
                        to_string(currentBlockNumber) + "\t" + "*" + "\t" + "=" + litValue + "\t" + " " + "\t" + " ";

            if (litValue[0] == 'X') {
                LOCCTR += (static_cast<int>(litValue.length()) - 3) / 2;
                lastDeltaLOCCTR += (static_cast<int>(litValue.length()) - 3) / 2;
            } else if (litValue[0] == 'C') {
                LOCCTR += static_cast<int>(litValue.length()) - 3;
                lastDeltaLOCCTR += static_cast<int>(litValue.length()) - 3;
            }
        }
    }
}
