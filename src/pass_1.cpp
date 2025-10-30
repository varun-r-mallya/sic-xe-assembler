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
    return first_executable_section_;
}

std::string *pass_1::get_blocks_num_to_name() const {
    return blocks_num_to_name_;
}

int pass_1::get_program_length() const {
    return program_length;
}

pass_1::pass_1(std::string filename, table_store *tables, const std::string &intermediateFileName,
               const std::string &errorFileName) {
    this->file_name_ = std::move(filename);
    this->table_store_ = tables;
    source_file_.open(file_name_);
    intermediate_file_.open(intermediateFileName);
    error_file_.open(errorFileName);
    run_pass_1();
}

bool pass_1::get_error() const {
    return error_flag;
}

void pass_1::run_pass_1() {
    // utilities::writeToFile(errorFile, "**********PASS1************");
    utilities::write_to_file(intermediate_file_, "Line\tAddress\tBlock\tLabel\tOPCODE\tOPERAND\tComment");

    getline(source_file_, file_line_);
    line_number_ += 5;
    while (utilities::checkCommentLine(file_line_)) {
        write_data_ = to_string(line_number_) + "\t" + file_line_;
        utilities::write_to_file(intermediate_file_, write_data_);
        getline(source_file_, file_line_); // read first input line
        line_number_ += 5;
        index = 0;
    }

    utilities::first_non_whitespace(file_line_, index, status_code_, label);
    utilities::first_non_whitespace(file_line_, index, status_code_, opcode);

    if (opcode == "START") {
        utilities::first_non_whitespace(file_line_, index, status_code_, operand);
        utilities::first_non_whitespace(file_line_, index, status_code_, comment, true);
        start_address_ = utilities::string_hex_to_int(operand);
        location_counter = start_address_;
        write_data_ = to_string(line_number_) + "\t" + utilities::int_to_string_hex(
                          location_counter - last_location_counter) + "\t" +
                      to_string(current_blk_num) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment;
        utilities::write_to_file(intermediate_file_, write_data_);

        getline(source_file_, file_line_);
        line_number_ += 5;
        index = 0;
        utilities::first_non_whitespace(file_line_, index, status_code_, label);
        utilities::first_non_whitespace(file_line_, index, status_code_, opcode);
    } else {
        start_address_ = 0;
        location_counter = 0;
    }

    while (opcode != "END") {
        if (!utilities::checkCommentLine(file_line_)) {
            if (!label.empty()) {
                // Label found
                if (table_store_->SYMTAB[label].exists == 'n') {
                    table_store_->SYMTAB[label].name = label;
                    table_store_->SYMTAB[label].address = utilities::int_to_string_hex(location_counter);
                    table_store_->SYMTAB[label].relative = 1;
                    table_store_->SYMTAB[label].exists = 'y';
                    table_store_->SYMTAB[label].blockNumber = current_blk_num;
                } else {
                    write_data_ = "Line: " + to_string(line_number_) + " : Duplicate symbol for '" + label +
                                  "'. Previously defined at " + table_store_->SYMTAB[label].address;
                    utilities::write_to_file(error_file_, write_data_);
                    error_flag = true;
                }
            }
            if (table_store_->OPTAB[utilities::getRealOpcode(opcode)].exists == 'y') {
                // Search for opcode in tableStore->OPTAB
                if (table_store_->OPTAB[utilities::getRealOpcode(opcode)].format == 3) {
                    location_counter += 3;
                    last_location_counter += 3;
                    if (utilities::getFlagFormat(opcode) == '+') {
                        location_counter += 1;
                        last_location_counter += 1;
                    }
                    if (utilities::getRealOpcode(opcode) == "RSUB") {
                        operand = " ";
                    } else {
                        utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                        if (operand[operand.length() - 1] == ',') {
                            utilities::first_non_whitespace(file_line_, index, status_code_, temp_operand_);
                            operand += temp_operand_;
                        }
                    }

                    if (utilities::getFlagFormat(operand) == '=') {
                        temp_operand_ = operand.substr(1, operand.length() - 1);
                        if (temp_operand_ == "*") {
                            temp_operand_ = "X'" + utilities::int_to_string_hex(
                                                location_counter - last_location_counter, 6) + "'";
                        }
                        if (table_store_->LITTAB[temp_operand_].exists == 'n') {
                            table_store_->LITTAB[temp_operand_].value = temp_operand_;
                            table_store_->LITTAB[temp_operand_].exists = 'y';
                            table_store_->LITTAB[temp_operand_].address = "?";
                            table_store_->LITTAB[temp_operand_].blockNumber = -1;
                        }
                    }
                } else if (table_store_->OPTAB[utilities::getRealOpcode(opcode)].format == 1) {
                    operand = " ";
                    location_counter += table_store_->OPTAB[utilities::getRealOpcode(opcode)].format;
                    last_location_counter += table_store_->OPTAB[utilities::getRealOpcode(opcode)].format;
                } else {
                    location_counter += table_store_->OPTAB[utilities::getRealOpcode(opcode)].format;
                    last_location_counter += table_store_->OPTAB[utilities::getRealOpcode(opcode)].format;
                    utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                    if (operand[operand.length() - 1] == ',') {
                        utilities::first_non_whitespace(file_line_, index, status_code_, temp_operand_);
                        operand += temp_operand_;
                    }
                }
            } else if (opcode == "WORD") {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                location_counter += 3;
                last_location_counter += 3;
            } else if (opcode == "RESW") {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                location_counter += 3 * utilities::string_to_decimal(operand);
                last_location_counter += 3 * utilities::string_to_decimal(operand);
            } else if (opcode == "RESB") {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                location_counter += utilities::string_to_decimal(operand);
                last_location_counter += utilities::string_to_decimal(operand);
            } else if (opcode == "BYTE") {
                utilities::readByteOperand(file_line_, index, status_code_, operand);
                if (operand[0] == 'X') {
                    location_counter += (static_cast<int>(operand.length()) - 3) / 2;
                    last_location_counter += (static_cast<int>(operand.length()) - 3) / 2;
                } else if (operand[0] == 'C') {
                    location_counter += static_cast<int>(operand.length()) - 3;
                    last_location_counter += static_cast<int>(operand.length()) - 3;
                }
                // else{
                //   writeData = "Line: "+to_string(line)+" : Invalid operand for BYTE. Found " + operand;
                //   utilities::writeToFile(errorFile,writeData);
                // }
            } else if (opcode == "BASE") {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);
            } else if (opcode == "LTORG") {
                operand = " ";
                handle_LTORG(write_data_suffix_);
            } else if (opcode == "ORG") {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);

                char lastByte = operand[operand.length() - 1];
                while (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*') {
                    utilities::first_non_whitespace(file_line_, index, status_code_, temp_operand_);
                    operand += temp_operand_;
                    lastByte = operand[operand.length() - 1];
                }

                int tempVariable;
                tempVariable = save_location_counter;
                save_location_counter = location_counter;
                location_counter = tempVariable;

                if (table_store_->SYMTAB[operand].exists == 'y') {
                    location_counter = utilities::string_hex_to_int(table_store_->SYMTAB[operand].address);
                } else {
                    bool relative;
                    // set error_flag to false
                    error_flag = false;
                    eval_expr(operand, relative);
                    if (!error_flag) {
                        location_counter = utilities::string_hex_to_int(temp_operand_);
                    }
                    error_flag = false; // reset error_flag
                }
            } else if (opcode == "USE") {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                table_store_->BLOCKS[current_block_].LOCCTR = utilities::int_to_string_hex(location_counter);

                if (table_store_->BLOCKS[operand].exists == 'n') {
                    // cout<<"Creating block: "<<operand<<endl;
                    table_store_->BLOCKS[operand].exists = 'y';
                    table_store_->BLOCKS[operand].name = operand;
                    table_store_->BLOCKS[operand].number = total_blocks_++;
                    table_store_->BLOCKS[operand].LOCCTR = "0";
                }
                current_blk_num = table_store_->BLOCKS[operand].number;
                current_block_ = table_store_->BLOCKS[operand].name;
                location_counter = utilities::string_hex_to_int(table_store_->BLOCKS[operand].LOCCTR);
            } else if (opcode == "EQU") {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                temp_operand_ = "";
                bool relative;

                if (operand == "*") {
                    temp_operand_ = utilities::int_to_string_hex(location_counter - last_location_counter, 6);
                    relative = true;
                } else if (utilities::if_all_num(operand)) {
                    temp_operand_ = utilities::int_to_string_hex(utilities::string_to_decimal(operand), 6);
                    relative = false;
                } else {
                    char lastByte = operand[operand.length() - 1];

                    while (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*') {
                        utilities::first_non_whitespace(file_line_, index, status_code_, temp_operand_);
                        operand += temp_operand_;
                        lastByte = operand[operand.length() - 1];
                    }

                    eval_expr(operand, relative);
                }

                table_store_->SYMTAB[label].name = label;
                table_store_->SYMTAB[label].address = temp_operand_;
                table_store_->SYMTAB[label].relative = relative;
                table_store_->SYMTAB[label].blockNumber = current_blk_num;
                last_location_counter = location_counter - utilities::string_hex_to_int(temp_operand_);
            } else {
                utilities::first_non_whitespace(file_line_, index, status_code_, operand);
                write_data_ = "Line: " + to_string(line_number_) + " : Invalid OPCODE. Found " + opcode;
                utilities::write_to_file(error_file_, write_data_);
                error_flag = true;
            }
            utilities::first_non_whitespace(file_line_, index, status_code_, comment, true);
            if (opcode == "EQU" && table_store_->SYMTAB[label].relative == 0) {
                write_data_ = write_data_prefix_ + to_string(line_number_) + "\t" +
                              utilities::int_to_string_hex(location_counter - last_location_counter) + "\t" + " " + "\t"
                              +
                              label + "\t" + opcode + "\t" + operand + "\t" + comment + write_data_suffix_;
            } else {
                write_data_ = write_data_prefix_ + to_string(line_number_) + "\t" +
                              utilities::int_to_string_hex(location_counter - last_location_counter) + "\t" +
                              to_string(current_blk_num) + "\t" + label + "\t" + opcode + "\t" +
                              operand + "\t" + comment + write_data_suffix_;
            }
            write_data_prefix_ = "";
            write_data_suffix_ = "";
        } else {
            write_data_ = to_string(line_number_) + "\t" + file_line_;
        }
        utilities::write_to_file(intermediate_file_, write_data_);

        table_store_->BLOCKS[current_block_].LOCCTR = utilities::int_to_string_hex(location_counter);
        getline(source_file_, file_line_);
        line_number_ += 5 + line_number_diff;
        line_number_diff = 0;
        index = 0;
        last_location_counter = 0;
        utilities::first_non_whitespace(file_line_, index, status_code_, label); // Parse label
        utilities::first_non_whitespace(file_line_, index, status_code_, opcode); // Parse OPCODE
    }

    if (opcode == "END") {
        first_executable_section_ = table_store_->SYMTAB[label].address;
        table_store_->SYMTAB[first_executable_section_].name = label;
        table_store_->SYMTAB[first_executable_section_].address = first_executable_section_;
    }

    utilities::first_non_whitespace(file_line_, index, status_code_, operand);
    utilities::first_non_whitespace(file_line_, index, status_code_, comment, true);
    current_block_ = "DEFAULT";
    current_blk_num = 0;
    location_counter = utilities::string_hex_to_int(table_store_->BLOCKS[current_block_].LOCCTR);

    handle_LTORG(write_data_suffix_);

    write_data_ = to_string(line_number_) + "\t" + utilities::int_to_string_hex(
                      location_counter - last_location_counter) + "\t" +
                  " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + write_data_suffix_;
    utilities::write_to_file(intermediate_file_, write_data_);

    int LocctrArr[total_blocks_];
    blocks_num_to_name_ = new string[total_blocks_];
    for (const auto &[fst, snd]: table_store_->BLOCKS) {
        LocctrArr[snd.number] = utilities::string_hex_to_int(snd.LOCCTR);
        blocks_num_to_name_[snd.number] = fst;
    }

    for (int i = 1; i < total_blocks_; i++) {
        LocctrArr[i] += LocctrArr[i - 1];
    }

    for (const auto &[fst, snd]: table_store_->BLOCKS) {
        if (snd.startAddress == "?") {
            table_store_->BLOCKS[fst].startAddress = utilities::int_to_string_hex(LocctrArr[snd.number - 1]);
        }
    }

    program_length = LocctrArr[total_blocks_ - 1] - start_address_;
}

void pass_1::eval_expr(std::string expression, bool &relative
) {
    if (!expression.empty() && expression[0] == '#') {
        string numPart = expression.substr(1);
        if (utilities::if_all_num(numPart)) {
            // It's an immediate numeric value, not a symbol
            temp_operand_ = utilities::int_to_string_hex(utilities::string_to_decimal(numPart), 6);
            relative = false;
            return;
        }
        // Remove '#' for symbol lookup
        expression = numPart;
    }

    // Handle pure numeric values
    if (utilities::if_all_num(expression)) {
        temp_operand_ = utilities::int_to_string_hex(utilities::string_to_decimal(expression), 6);
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

    for (size_t i = 0; i < expression.length();) {
        singleOperand = "";

        lastByte = expression[i];
        while ((lastByte != '+' && lastByte != '-' && lastByte != '/' && lastByte != '*') && i < expression.length()) {
            singleOperand += lastByte;
            lastByte = expression[++i];
        }

        if (table_store_->SYMTAB[singleOperand].exists == 'y') {
            // Check operand existence
            lastOperand = table_store_->SYMTAB[singleOperand].relative;
            valueTemp = to_string(utilities::string_hex_to_int(table_store_->SYMTAB[singleOperand].address));
        } else if ((!singleOperand.empty() || singleOperand != "?") && utilities::if_all_num(singleOperand)) {
            lastOperand = 0;
            valueTemp = singleOperand;
        } else {
            write_data_ = "Line: " + to_string(line_number_) + " : Cannot find symbol. Found " + singleOperand;
            utilities::write_to_file(error_file_, write_data_);
            Illegal = true;
            break;
        }

        if (lastOperand * lastOperator == 1) {
            // Check expressions legality
            write_data_ = "Line: " + to_string(line_number_) + " : Illegal expression";
            utilities::write_to_file(error_file_, write_data_);
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
            write_data_ = "Line: " + to_string(line_number_) + " : Illegal operator in expression. Found " +
                          singleOperator;
            utilities::write_to_file(error_file_, write_data_);
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
            StrEval tempOBJ(valueString);
            temp_operand_ = utilities::int_to_string_hex(tempOBJ.get_result());
        } else if (pairCount == 0) {
            /*absolute*/
            relative = false;
            StrEval tempOBJ(valueString);
            temp_operand_ = utilities::int_to_string_hex(tempOBJ.get_result());
        } else {
            write_data_ = "Line: " + to_string(line_number_) + " : Illegal expression";
            utilities::write_to_file(error_file_, write_data_);
            error_flag = true;
            temp_operand_ = "00000";
            relative = false;
        }
    } else {
        temp_operand_ = "00000";
        error_flag = true;
        relative = false;
    }
}

void pass_1::handle_LTORG(std::string &litPrefix) {
    litPrefix = "";
    for (const auto &[fst, snd]: table_store_->LITTAB) {
        string litAddress = snd.address;
        string litValue = snd.value;
        if (litAddress != "?") {
            /*Pass as already address is assigned*/
        } else {
            line_number_ += 5;
            line_number_diff += 5;
            table_store_->LITTAB[fst].address = utilities::int_to_string_hex(location_counter);
            table_store_->LITTAB[fst].blockNumber = current_blk_num;

            litPrefix += "\n" + to_string(line_number_) + "\t" + utilities::int_to_string_hex(location_counter) + "\t" +
                    to_string(current_blk_num) + "\t" + "*" + "\t" + "=" + litValue + "\t" + " " + "\t" + " ";

            if (litValue[0] == 'X') {
                location_counter += (static_cast<int>(litValue.length()) - 3) / 2;
                last_location_counter += (static_cast<int>(litValue.length()) - 3) / 2;
            } else if (litValue[0] == 'C') {
                location_counter += static_cast<int>(litValue.length()) - 3;
                last_location_counter += static_cast<int>(litValue.length()) - 3;
            }
        }
    }
}
