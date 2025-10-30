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
    if (!getline(intermediate_file_, fileLine)) {
        return false;
    }
    line_number_ = utilities::string_to_decimal(readTillTab(fileLine, index));

    is_comment_ = fileLine[index] == '.';
    if (is_comment_) {
        utilities::first_non_whitespace(fileLine, index, tempStatus, comment, true);
        return true;
    }
    address = utilities::string_hex_to_int(readTillTab(fileLine, index));
    if (const std::string tempBuffer = readTillTab(fileLine, index); tempBuffer == " ") {
        block_number_ = -1;
    } else {
        block_number_ = utilities::string_to_decimal(tempBuffer);
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
        if (!current_record_.empty()) {
            // Write last text record
            write_data_ = utilities::int_to_string_hex(static_cast<int>(current_record_.length() / 2), 2) + '^' +
                          current_record_;
            utilities::write_to_file(object_file_, write_data_);
            current_record_ = "";
        }
        return;
    }
    if (!object_code_.empty()) {
        if (current_record_.empty()) {
            write_data_ = "T^" + utilities::int_to_string_hex(
                              address + utilities::string_hex_to_int(
                                  tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress), 6) + '^';
            utilities::write_to_file(object_file_, write_data_, false);
        }
        if ((current_record_ + object_code_).length() > 60) {
            write_data_ = utilities::int_to_string_hex(static_cast<int>(current_record_.length() / 2), 2) + '^' +
                          current_record_;
            utilities::write_to_file(object_file_, write_data_);
            current_record_ = "";
            write_data_ = "T^" + utilities::int_to_string_hex(
                              address + utilities::string_hex_to_int(
                                  tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress), 6) + '^';
            utilities::write_to_file(object_file_, write_data_, false);
        }

        current_record_ += object_code_;
    } else {
        if (opcode == "START" || opcode == "END" || opcode == "BASE" || opcode == "NOBASE" || opcode == "LTORG" ||
            opcode == "ORG" || opcode == "EQU") {
        } else {
            // Write current record if exists
            if (!current_record_.empty()) {
                write_data_ = utilities::int_to_string_hex(static_cast<int>(current_record_.length() / 2), 2) + '^' +
                              current_record_;
                utilities::write_to_file(object_file_, write_data_);
            }
            current_record_ = "";
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
            write_data_ = "Line: " + std::to_string(line_number_) +
                          " Index based addressing not supported with Indirect addressing";
            utilities::write_to_file(error_file_, write_data_);
            has_error_ = true;
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
                write_data_ = "Line: " + std::to_string(line_number_) + " Immediate value exceeds format limit";
                utilities::write_to_file(error_file_, write_data_);
                has_error_ = true;
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
            write_data_ = "Line " + std::to_string(line_number_);
            write_data_ += " : Symbol does not exist. Found " + tempOperand;
            utilities::write_to_file(error_file_, write_data_);
            has_error_ = true;
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

            modification_record_ += "M^" + utilities::int_to_string_hex(address + 1, 6) + '^';
            if (halfBytes == 5)
                modification_record_ += "05";
            else
                modification_record_ += "03";
            modification_record_ += '\n';

            return object_code;
        }
        program_counter = address + utilities::string_hex_to_int(
                              tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress);
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
            modification_record_ += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress), 6) + '^';
            modification_record_ += (halfBytes == 5) ? "05" : "03";
            modification_record_ += '\n';

            return object_code;
        }
    } else if (utilities::getFlagFormat(operand) == '@') {
        std::string tempOperand = operand.substr(1, operand.length() - 1);
        if (tempOperand.substr(tempOperand.length() - 2, 2) == ",X" || tableStore->SYMTAB[tempOperand].exists == 'n') {
            write_data_ = "Line " + std::to_string(line_number_);
            write_data_ +=
                    " : Symbol does not exist. Index based addressing not supported with Indirect addressing ";
            utilities::write_to_file(error_file_, write_data_);
            has_error_ = true;
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
                              tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress);
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
                modification_record_ += "M^" + utilities::int_to_string_hex(
                    address + 1 + utilities::string_hex_to_int(
                        tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress), 6) + '^';
                modification_record_ += (halfBytes == 5) ? "05" : "03";
                modification_record_ += '\n';

                return object_code;
            }
        } else {
            // No base or pc based addressing in format 4
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 2, 2);
            object_code += '1';
            object_code += utilities::int_to_string_hex(operandAddress, halfBytes);
            modification_record_ += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress),
                6) + '^';
            modification_record_ += (halfBytes == 5) ? "05" : "03";
            modification_record_ += '\n';

            return object_code;
        }

        write_data_ = "Line: " + std::to_string(line_number_);
        write_data_ += "Cannot fit into program counter or base register based addressing.";
        utilities::write_to_file(error_file_, write_data_);
        has_error_ = true;
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
            modification_record_ += "M^" + utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->LITTAB[tempOperand].address) + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[tableStore->LITTAB[tempOperand].blockNumber]].startAddress),
                6) + '^';
            modification_record_ += utilities::int_to_string_hex(6, 2);
            modification_record_ += '\n';
        }

        if (tableStore->LITTAB[tempOperand].exists == 'n') {
            write_data_ = "Line " + std::to_string(line_number_) + " : Symbol doesn't exists. Found " + tempOperand;
            utilities::write_to_file(error_file_, write_data_);
            has_error_ = true;
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
                              tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress);
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

                modification_record_ += "M^" + utilities::int_to_string_hex(
                    address + 1 + utilities::string_hex_to_int(
                        tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress), 6) + '^';
                modification_record_ += (halfBytes == 5) ? "05" : "03";
                modification_record_ += '\n';

                return object_code;
            }
        } else {
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            object_code += '1';
            object_code += utilities::int_to_string_hex(operandAddress, halfBytes);

            modification_record_ += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress),
                6) + '^';
            modification_record_ += (halfBytes == 5) ? "05" : "03";
            modification_record_ += '\n';

            return object_code;
        }

        write_data_ = "Line: " + std::to_string(line_number_);
        write_data_ += "Cannot fit into program counter or base register based addressing.";
        utilities::write_to_file(error_file_, write_data_);
        has_error_ = true;
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
            write_data_ = "Line " + std::to_string(line_number_);
            write_data_ += " : Invalid operand. Found '" + operand + "'";
            utilities::write_to_file(error_file_, write_data_);
            has_error_ = true;
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            object_code += "0000";
            return object_code;
        }

        if (tableStore->SYMTAB[tempOperand].exists == 'n') {
            write_data_ = "Line " + std::to_string(line_number_);
            write_data_ += " : Symbol doesn't exists. Found " + tempOperand;
            utilities::write_to_file(error_file_, write_data_);
            has_error_ = true;
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
                              tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress);
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

                modification_record_ += "M^" + utilities::int_to_string_hex(
                    address + 1 + utilities::string_hex_to_int(
                        tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress), 6) + '^';
                modification_record_ += (halfBytes == 5) ? "05" : "03";
                modification_record_ += '\n';

                return object_code;
            }
        } else {
            // No base or pc based addressing in format 4
            object_code = utilities::int_to_string_hex(
                utilities::string_hex_to_int(tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode) + 3, 2);
            object_code += utilities::int_to_string_hex(xbpe + 1, 1);
            object_code += utilities::int_to_string_hex(operandAddress, halfBytes);

            /*add modifacation record here*/
            modification_record_ += "M^" + utilities::int_to_string_hex(
                address + 1 + utilities::string_hex_to_int(
                    tableStore->BLOCKS[blocks_num_to_name[block_number_]].startAddress), 6) + '^';
            modification_record_ += (halfBytes == 5) ? "05" : "03";
            modification_record_ += '\n';

            return object_code;
        }

        write_data_ = "Line: " + std::to_string(line_number_);
        write_data_ += "Cannot fit into program counter based or base register based addressing.";
        utilities::write_to_file(error_file_, write_data_);
        has_error_ = true;
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
        if (!end_record_.empty()) {
            utilities::write_to_file(object_file_, end_record_);
        } else {
            writeEndRecord(false);
        }
    }
    if (operand.empty() || operand == " ") {
        end_record_ = "E^" + utilities::int_to_string_hex(start_address_, 6);
    } else {
        const int firstExecutableAddress =
                utilities::string_hex_to_int(tableStore->SYMTAB[first_executable_section_].address);

        end_record_ = "E^" + utilities::int_to_string_hex(firstExecutableAddress, 6) + "\n";
    }
}

pass_2::pass_2(std::string filename, table_store *tables, const std::string &intermediateFileName,
               const std::string &objectFileName, const std::string &listingFileName, const std::string &errorFileName,
               std::string *blocksNumToName, const std::string &firstExecutableSec, int progLength) {
    this->fileName = std::move(filename);
    this->tableStore = tables;
    this->blocks_num_to_name = blocksNumToName;
    this->first_executable_section_ = firstExecutableSec;
    this->program_length = progLength;

    intermediate_file_.open(intermediateFileName);
    if (!intermediate_file_) {
        std::cerr << "Unable to open file: " << intermediateFileName << std::endl;
        exit(1);
    }

    object_file_.open(objectFileName);
    if (!object_file_) {
        std::cerr << "Unable to open file: " << objectFileName << std::endl;
        exit(1);
    }

    listing_file_.open(listingFileName);
    if (!listing_file_) {
        std::cerr << "Unable to open file: " << listingFileName << std::endl;
        exit(1);
    }

    error_file_.open(errorFileName, std::fstream::app);
    if (!error_file_) {
        std::cerr << "Unable to open file: " << errorFileName << std::endl;
        exit(1);
    }

    run_pass_2();
}

void pass_2::run_pass_2() {
    std::string tempBuffer;
    std::getline(intermediate_file_, tempBuffer);

    utilities::write_to_file(listing_file_, "Line\tAddress\tLabel\tOPCODE\tOPERAND\tObjectCode\tComment");
    // utilities::write_to_file(errorFile, "\n\nPASS2:");

    readIntermediateFile();
    while (is_comment_) {
        write_data_ = std::to_string(line_number_) + "\t" + comment;
        utilities::write_to_file(listing_file_, write_data_);
        readIntermediateFile();
    }

    if (opcode == "START") {
        start_address_ = address;
        write_data_ = std::to_string(line_number_) + "\t" + utilities::int_to_string_hex(address) + "\t" +
                      std::to_string(block_number_) + "\t" + label + "\t" + opcode + "\t" + operand +
                      "\t" + object_code_ + "\t" + comment;
        utilities::write_to_file(listing_file_, write_data_);
    } else {
        label = "";
        start_address_ = 0;
        address = 0;
    }

    const int program_section_length = program_length;

    write_data_ = "H^" + utilities::expandString(label, 6, ' ', true) + '^' +
                  utilities::int_to_string_hex(address, 6) + '^' +
                  utilities::int_to_string_hex(program_section_length, 6);
    utilities::write_to_file(object_file_, write_data_);

    readIntermediateFile();

    while (opcode != "END") {
        if (!is_comment_) {
            if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].exists == 'y') {
                if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 1) {
                    object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode;
                } else if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 2) {
                    operand1 = operand.substr(0, operand.find(','));
                    operand2 = operand.substr(operand.find(',') + 1, operand.length() - operand.find(',') - 1);

                    if (operand2 == operand) {
                        // If not two operand i.e. a
                        if (utilities::getRealOpcode(opcode) == "SVC") {
                            object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                           utilities::int_to_string_hex(utilities::string_to_decimal(operand1), 1) +
                                           '0';
                        } else if (tableStore->REGTAB[operand1].exists == 'y') {
                            object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                           tableStore->REGTAB[operand1].num + '0';
                        } else {
                            object_code_ = utilities::getRealOpcode(opcode) + '0' + '0';
                            write_data_ = "Line: " + std::to_string(line_number_) + " Invalid Register name";
                            utilities::write_to_file(error_file_, write_data_);
                            has_error_ = true;
                        }
                    } else {
                        // Two operands i.e. a,b
                        if (tableStore->REGTAB[operand1].exists == 'n') {
                            object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode + "00";
                            write_data_ = "Line: " + std::to_string(line_number_) + " Invalid Register name";
                            utilities::write_to_file(error_file_, write_data_);
                            has_error_ = true;
                        } else if (utilities::getRealOpcode(opcode) == "SHIFTR" || utilities::getRealOpcode(opcode) ==
                                   "SHIFTL") {
                            object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                           tableStore->REGTAB[operand1].num +
                                           utilities::int_to_string_hex(utilities::string_to_decimal(operand2), 1);
                        } else if (tableStore->REGTAB[operand2].exists == 'n') {
                            object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode + "00";
                            write_data_ = "Line: " + std::to_string(line_number_) + " Invalid Register name";
                            utilities::write_to_file(error_file_, write_data_);
                            has_error_ = true;
                        } else {
                            object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode +
                                           tableStore->REGTAB[operand1].num + tableStore->REGTAB[operand2].num;
                        }
                    }
                } else if (tableStore->OPTAB[utilities::getRealOpcode(opcode)].format == 3) {
                    if (utilities::getRealOpcode(opcode) == "RSUB") {
                        object_code_ = tableStore->OPTAB[utilities::getRealOpcode(opcode)].opcode;
                        object_code_ += (utilities::getFlagFormat(opcode) == '+') ? "000000" : "0000";
                    } else {
                        object_code_ = createObjectCodeFormat34();
                    }
                }
            } else if (opcode == "BYTE") {
                if (operand[0] == 'X') {
                    object_code_ = operand.substr(2, operand.length() - 3);
                } else if (operand[0] == 'C') {
                    object_code_ = utilities::stringToHexString(operand.substr(2, operand.length() - 3));
                }
            } else if (label == "*") {
                if (opcode[1] == 'C') {
                    object_code_ = utilities::stringToHexString(opcode.substr(3, opcode.length() - 4));
                } else if (opcode[1] == 'X') {
                    object_code_ = opcode.substr(3, opcode.length() - 4);
                }
            } else if (opcode == "WORD") {
                object_code_ = utilities::int_to_string_hex(utilities::string_to_decimal(operand), 6);
            } else if (opcode == "BASE") {
                if (tableStore->SYMTAB[operand].exists == 'y') {
                    base_register_value = utilities::string_hex_to_int(tableStore->SYMTAB[operand].address) +
                                          utilities::string_hex_to_int(
                                              tableStore->BLOCKS[blocks_num_to_name[tableStore->SYMTAB[operand].
                                                  blockNumber]].startAddress);
                    not_base = false;
                } else {
                    write_data_ = "Line " + std::to_string(line_number_) + " : Symbol doesn't exists. Found " + operand;
                    utilities::write_to_file(error_file_, write_data_);
                    has_error_ = true;
                }
                object_code_ = "";
            } else if (opcode == "NOBASE") {
                if (not_base) {
                    write_data_ = "Line " + std::to_string(line_number_) + ": Assembler wasn't using base addressing";
                    utilities::write_to_file(error_file_, write_data_);
                    has_error_ = true;
                } else {
                    not_base = true;
                }
                object_code_ = "";
            } else {
                object_code_ = "";
            }

            // Write to text record if any generated
            writeTextRecord();

            if (block_number_ == -1 && address != -1) {
                write_data_ = std::to_string(line_number_) + "\t" + utilities::int_to_string_hex(address) +
                              "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" +
                              object_code_ + "\t" + comment;
            } else if (address == -1) {
                write_data_ = std::to_string(line_number_) + "\t" + " " + "\t" + " " + "\t" + label +
                              "\t" + opcode + "\t" + operand + "\t" + object_code_ + "\t" + comment;
            } else {
                write_data_ = std::to_string(line_number_) + "\t" + utilities::int_to_string_hex(address) +
                              "\t" + std::to_string(block_number_) + "\t" + label + "\t" + opcode + "\t" +
                              operand + "\t" + object_code_ + "\t" + comment;
            }
        } else {
            write_data_ = std::to_string(line_number_) + "\t" + comment;
        }
        utilities::write_to_file(listing_file_, write_data_);
        readIntermediateFile();
        object_code_ = "";
    }

    writeTextRecord(true);

    writeEndRecord(false);

    if (!is_comment_) {
        write_data_ = std::to_string(line_number_) + "\t" + utilities::int_to_string_hex(address) +
                      "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + "" +
                      "\t" + comment;
    } else {
        write_data_ = std::to_string(line_number_) + "\t" + comment;
    }
    utilities::write_to_file(listing_file_, write_data_);

    while (readIntermediateFile()) {
        if (label == "*") {
            if (opcode[1] == 'C') {
                object_code_ = utilities::stringToHexString(opcode.substr(3, opcode.length() - 4));
            } else if (opcode[1] == 'X') {
                object_code_ = opcode.substr(3, opcode.length() - 4);
            }
            writeTextRecord();
        }
        write_data_ = std::to_string(line_number_) + "\t" + utilities::int_to_string_hex(address) +
                      "\t" + std::to_string(block_number_) + label + "\t" + opcode + "\t" + operand +
                      "\t" + object_code_ + "\t" + comment;
        utilities::write_to_file(listing_file_, write_data_);
    }

    utilities::write_to_file(object_file_, modification_record_, false);
    writeEndRecord(true);
}

pass_2::~pass_2() {
    intermediate_file_.close();
    object_file_.close();
    listing_file_.close();
    error_file_.close();
}

bool pass_2::has_error() const {
    return has_error_;
}
