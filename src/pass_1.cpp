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

pass_1::pass_1(std::string filename, table_store *tables, const std::string &intermediateFileName,
    const std::string &errorFileName) {
    this->fileName = std::move(filename);
    this->tableStore = tables;
    SourceFile.open(fileName);
    intermediateFile.open(intermediateFileName);
    errorFile.open(errorFileName);
    run_pass_1();
}

bool pass_1::get_error() const {
    return error_flag;
}

void pass_1::run_pass_1() {
    // utilities::writeToFile(errorFile, "**********PASS1************");
    utilities::write_to_file(intermediateFile, "Line\tAddress\tBlock\tLabel\tOPCODE\tOPERAND\tComment");

    getline(SourceFile, fileLine);
    lineNumber += 5;
    while (utilities::checkCommentLine(fileLine)) {
        writeData = to_string(lineNumber) + "\t" + fileLine;
        utilities::write_to_file(intermediateFile, writeData);
        getline(SourceFile, fileLine); // read first input line
        lineNumber += 5;
        index = 0;
    }

    utilities::first_non_whitespace(fileLine, index, statusCode, label);
    utilities::first_non_whitespace(fileLine, index, statusCode, opcode);

    if (opcode == "START") {
        utilities::first_non_whitespace(fileLine, index, statusCode, operand);
        utilities::first_non_whitespace(fileLine, index, statusCode, comment, true);
        startAddress = utilities::string_hex_to_int(operand);
        LOCCTR = startAddress;
        writeData = to_string(lineNumber) + "\t" + utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR) + "\t" +
                   to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment;
        utilities::write_to_file(intermediateFile, writeData);

        getline(SourceFile, fileLine);
        lineNumber += 5;
        index = 0;
        utilities::first_non_whitespace(fileLine, index, statusCode, label);
        utilities::first_non_whitespace(fileLine, index, statusCode, opcode);
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
                    utilities::write_to_file(errorFile, writeData);
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
                        utilities::first_non_whitespace(fileLine, index, statusCode, operand);
                        if (operand[operand.length() - 1] == ',') {
                            utilities::first_non_whitespace(fileLine, index, statusCode, tempOperand);
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
                    utilities::first_non_whitespace(fileLine, index, statusCode, operand);
                    if (operand[operand.length() - 1] == ',') {
                        utilities::first_non_whitespace(fileLine, index, statusCode, tempOperand);
                        operand += tempOperand;
                    }
                }
            } else if (opcode == "WORD") {
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);
                LOCCTR += 3;
                lastDeltaLOCCTR += 3;
            } else if (opcode == "RESW") {
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);
                LOCCTR += 3 * utilities::string_to_decimal(operand);
                lastDeltaLOCCTR += 3 * utilities::string_to_decimal(operand);
            } else if (opcode == "RESB") {
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);
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
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);
            } else if (opcode == "LTORG") {
                operand = " ";
                handle_LTORG(writeDataSuffix);
            } else if (opcode == "ORG") {
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);

                char lastByte = operand[operand.length() - 1];
                while (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*') {
                    utilities::first_non_whitespace(fileLine, index, statusCode, tempOperand);
                    operand += tempOperand;
                    lastByte = operand[operand.length() - 1];
                }

                int tempVariable;
                tempVariable = saveLOCCTR;
                saveLOCCTR = LOCCTR;
                LOCCTR = tempVariable;

                if (tableStore->SYMTAB[operand].exists == 'y') {
                    LOCCTR = utilities::string_hex_to_int(tableStore->SYMTAB[operand].address);
                } else {
                    bool relative;
                    // set error_flag to false
                    error_flag = false;
                    eval_expr(operand, relative);
                    if (!error_flag) {
                        LOCCTR = utilities::string_hex_to_int(tempOperand);
                    }
                    error_flag = false; // reset error_flag
                }
            } else if (opcode == "USE") {
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);
                tableStore->BLOCKS[currentBlockName].LOCCTR = utilities::int_to_string_hex(LOCCTR);

                if (tableStore->BLOCKS[operand].exists == 'n') {
                    // cout<<"Creating block: "<<operand<<endl;
                    tableStore->BLOCKS[operand].exists = 'y';
                    tableStore->BLOCKS[operand].name = operand;
                    tableStore->BLOCKS[operand].number = totalBlocks++;
                    tableStore->BLOCKS[operand].LOCCTR = "0";
                }
                currentBlockNumber = tableStore->BLOCKS[operand].number;
                currentBlockName = tableStore->BLOCKS[operand].name;
                LOCCTR = utilities::string_hex_to_int(tableStore->BLOCKS[operand].LOCCTR);
            } else if (opcode == "EQU") {
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);
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
                        utilities::first_non_whitespace(fileLine, index, statusCode, tempOperand);
                        operand += tempOperand;
                        lastByte = operand[operand.length() - 1];
                    }

                    eval_expr(operand, relative);
                }

                tableStore->SYMTAB[label].name = label;
                tableStore->SYMTAB[label].address = tempOperand;
                tableStore->SYMTAB[label].relative = relative;
                tableStore->SYMTAB[label].blockNumber = currentBlockNumber;
                lastDeltaLOCCTR = LOCCTR - utilities::string_hex_to_int(tempOperand);
            } else {
                utilities::first_non_whitespace(fileLine, index, statusCode, operand);
                writeData = "Line: " + to_string(lineNumber) + " : Invalid OPCODE. Found " + opcode;
                utilities::write_to_file(errorFile, writeData);
                error_flag = true;
            }
            utilities::first_non_whitespace(fileLine, index, statusCode, comment, true);
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
        utilities::write_to_file(intermediateFile, writeData);

        tableStore->BLOCKS[currentBlockName].LOCCTR = utilities::int_to_string_hex(LOCCTR);
        getline(SourceFile, fileLine);
        lineNumber += 5 + lineNumberDelta;
        lineNumberDelta = 0;
        index = 0;
        lastDeltaLOCCTR = 0;
        utilities::first_non_whitespace(fileLine, index, statusCode, label); // Parse label
        utilities::first_non_whitespace(fileLine, index, statusCode, opcode); // Parse OPCODE
    }

    if (opcode == "END") {
        firstExecutable_Sec = tableStore->SYMTAB[label].address;
        tableStore->SYMTAB[firstExecutable_Sec].name = label;
        tableStore->SYMTAB[firstExecutable_Sec].address = firstExecutable_Sec;
    }

    utilities::first_non_whitespace(fileLine, index, statusCode, operand);
    utilities::first_non_whitespace(fileLine, index, statusCode, comment, true);
    currentBlockName = "DEFAULT";
    currentBlockNumber = 0;
    LOCCTR = utilities::string_hex_to_int(tableStore->BLOCKS[currentBlockName].LOCCTR);

    handle_LTORG(writeDataSuffix);

    writeData = to_string(lineNumber) + "\t" + utilities::int_to_string_hex(LOCCTR - lastDeltaLOCCTR) + "\t" +
               " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
    utilities::write_to_file(intermediateFile, writeData);

    int LocctrArr[totalBlocks];
    BLocksNumToName = new string[totalBlocks];
    for (auto const &it: tableStore->BLOCKS) {
        LocctrArr[it.second.number] = utilities::string_hex_to_int(it.second.LOCCTR);
        BLocksNumToName[it.second.number] = it.first;
    }

    for (int i = 1; i < totalBlocks; i++) {
        LocctrArr[i] += LocctrArr[i - 1];
    }

    for (const auto &[fst, snd]: tableStore->BLOCKS) {
        if (snd.startAddress == "?") {
            tableStore->BLOCKS[fst].startAddress = utilities::int_to_string_hex(LocctrArr[snd.number - 1]);
        }
    }

    program_length = LocctrArr[totalBlocks - 1] - startAddress;
}

void pass_1::eval_expr(std::string expression, bool &relative
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
            valueTemp = to_string(utilities::string_hex_to_int(tableStore->SYMTAB[singleOperand].address));
        } else if ((!singleOperand.empty() || singleOperand != "?") && utilities::if_all_num(singleOperand)) {
            lastOperand = 0;
            valueTemp = singleOperand;
        } else {
            writeData = "Line: " + to_string(lineNumber) + " : Cannot find symbol. Found " + singleOperand;
            utilities::write_to_file(errorFile, writeData);
            Illegal = true;
            break;
        }

        if (lastOperand * lastOperator == 1) {
            // Check expressions legality
            writeData = "Line: " + to_string(lineNumber) + " : Illegal expression";
            utilities::write_to_file(errorFile, writeData);
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
            utilities::write_to_file(errorFile, writeData);
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
            EvaluateString tempOBJ(valueString);
            tempOperand = utilities::int_to_string_hex(tempOBJ.getResult());
        } else {
            writeData = "Line: " + to_string(lineNumber) + " : Illegal expression";
            utilities::write_to_file(errorFile, writeData);
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

void pass_1::handle_LTORG(std::string &litPrefix) {
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
