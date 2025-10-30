#include "utilities.h"

#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<iomanip>
#include<algorithm>


int utilities::string_to_decimal(const std::string &str) {
    int value;
    std::stringstream(str) >> value;
    return value;
}

std::string utilities::int_to_string_hex(const int x, const int fill) {
    std::stringstream s;
    s << std::setfill('0') << std::setw(fill) << std::hex << x;
    std::string temp = s.str();
    temp = temp.substr(temp.length() - fill, fill);
    std::ranges::transform(temp, temp.begin(), ::toupper);
    return temp;
}

std::string utilities::str_expand(std::string data, const int length, const char fillChar, const bool back) {
    if (back) {
        if (length <= data.length()) {
            return data.substr(0, length);
        }
        for (size_t i = length - data.length(); i > 0; i--) {
            data += fillChar;
        }
    } else {
        if (length <= data.length()) {
            return data.substr(data.length() - length, length);
        }
        for (size_t i = length - data.length(); i > 0; i--) {
            data = fillChar + data;
        }
    }
    return data;
}

int utilities::string_hex_to_int(const std::string &x, bool *success) {
    if (success) *success = false;
    if (x.empty() || x == " " || x == "-1") {
        return 0;
    }
    std::string trimmed = x;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    if (trimmed.empty()) {
        return 0;
    }

    try {
        int result = stoul(trimmed, nullptr, 16);
        if (success) *success = true;
        return result;
    } catch (const std::exception &e) {
        std::cerr << "Error converting '" << x << "' to hex: " << e.what() << std::endl;
        return 0;
    }
}

std::string utilities::stringToHexString(const std::string &input) {
    static const auto lut = "0123456789ABCDEF";
    const size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

bool utilities::checkWhiteSpace(const char x) {
    if (x == ' ' || x == '\t') {
        return true;
    }
    return false;
}

bool utilities::checkCommentLine(const std::string &line) {
    if (line[0] == '.') {
        return true;
    }
    return false;
}

bool utilities::if_all_num(const std::string &x) {
    // Handle empty string
    if (x.empty()) {
        return false;
    }

    int start = 0;

    // Allow leading negative sign
    if (x[0] == '-' || x[0] == '+') {
        if (x.length() == 1) {
            return false; // Just a sign, not a number
        }
        start = 1;
    }

    // Check remaining characters are digits
    for (int i = start; i < x.length(); i++) {
        if (!isdigit(x[i])) {
            return false;
        }
    }

    return true;
}

void utilities::first_non_whitespace(const std::string &line, int &index, bool &status, std::string &data,
                                     const bool readTillEnd) {
    data = "";
    status = false; // Default to false

    // Boundary check
    if (index < 0 || index >= line.length()) {
        return;
    }

    if (readTillEnd) {
        // Read from index to end of line
        data = line.substr(index);
        status = !data.empty();
        return;
    }

    // Read until whitespace
    while (index < line.length() && !checkWhiteSpace(line[index])) {
        data += line[index];
        index++;
    }

    status = !data.empty();

    while (index < line.length() && checkWhiteSpace(line[index])) {
        index++;
    }
}

void utilities::readByteOperand(const std::string &line, int &index, bool &status, std::string &data) {
    data = "";
    status = true;
    if (line[index] == 'C') {
        data += line[index++];
        const char identifier = line[index++];
        data += identifier;
        while (index < line.length() && line[index] != identifier) {
            //Copy all data until next identifier regardless of whitespace
            data += line[index];
            index++;
        }
        data += identifier;
        index++; //Shift to next of identifier
    } else {
        while (index < line.length() && !checkWhiteSpace(line[index])) {
            //In no whitespace then data
            data += line[index];
            index++;
        }
    }

    if (data.empty()) {
        status = false;
    }

    while (index < line.length() && checkWhiteSpace(line[index])) {
        //Increase index to pass all whitespace
        index++;
    }
}

void utilities::write_to_file(std::ofstream &file, const std::string &data, const bool newline) {
    if (newline) {
        file << data << std::endl;
    } else {
        file << data;
    }
}

std::string utilities::getRealOpcode(std::string opcode) {
    if (opcode[0] == '+' || opcode[0] == '@') {
        return opcode.substr(1, opcode.length() - 1);
    }
    return opcode;
}

char utilities::getFlagFormat(const std::string &data) {
    if (data[0] == '#' || data[0] == '+' || data[0] == '@' || data[0] == '=') {
        return data[0];
    }
    return ' ';
}

StrEval::StrEval(const std::string &data) {
    stored_data_ = data;
    index = 0;
}

int StrEval::get_result() {
    int result = term();
    while (peek() == '+' || peek() == '-') {
        if (get() == '+') {
            result += term();
        } else {
            result -= term();
        }
    }
    return result;
}

int StrEval::term() {
    int result = factor();
    while (peek() == '*' || peek() == '/') {
        if (get() == '*') {
            result *= factor();
        } else {
            result /= factor();
        }
    }
    return result;
}

int StrEval::factor() {
    if (peek() >= '0' && peek() <= '9') {
        return number();
    }
    if (peek() == '(') {
        get();
        const int result = get_result();
        get();
        return result;
    }
    if (peek() == '-') {
        get();
        return -factor();
    }
    return 0;
}

int StrEval::number() {
    int result = get() - '0';
    while (peek() >= '0' && peek() <= '9') {
        result = 10 * result + get() - '0';
    }
    return result;
}

char StrEval::get() {
    return stored_data_[index++];
}

char StrEval::peek() const {
    return stored_data_[index];
}
