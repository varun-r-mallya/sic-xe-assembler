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

std::string utilities::getString(const char c) {
    std::string s(1, c);
    return s;
}

std::string utilities::intToStringHex(const int x, const int fill) {
    std::stringstream s;
    s << std::setfill('0') << std::setw(fill) << std::hex << x;
    std::string temp = s.str();
    temp = temp.substr(temp.length() - fill, fill);
    std::ranges::transform(temp, temp.begin(), ::toupper);
    return temp;
}

std::string utilities::expandString(std::string data, const int length, const char fillChar, const bool back) {
    if (back) {
        if (length <= data.length()) {
            return data.substr(0, length);
        }
        for (int i = length - data.length(); i > 0; i--) {
            data += fillChar;
        }
    } else {
        if (length <= data.length()) {
            return data.substr(data.length() - length, length);
        } else {
            for (int i = length - data.length(); i > 0; i--) {
                data = fillChar + data;
            }
        }
    }
    return data;
}

int utilities::stringHexToInt(const std::string &x) {
    return stoul(x, nullptr, 16);
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
    bool all_num = true;
    int i = 0;
    while (all_num && (i < x.length())) {
        all_num &= isdigit(x[i++]);
    }
    return all_num;
}

void utilities::readFirstNonWhiteSpace(const std::string &line, int &index, bool &status, std::string &data,
                                       const bool readTillEnd) {
    data = "";
    status = true;
    if (readTillEnd) {
        data = line.substr(index, line.length() - index);
        if (data.empty()) {
            status = false;
        }
        return;
    }
    while (index < line.length() && !checkWhiteSpace(line[index])) {
        //If no whitespace then data
        data += line[index];
        index++;
    }

    if (data.empty()) {
        status = false;
    }

    while (index < line.length() && checkWhiteSpace(line[index])) {
        //Increase index to pass all whitespace
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

void utilities::writeToFile(std::ofstream &file, const std::string &data, const bool newline) {
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

EvaluateString::EvaluateString(const std::string &data) {
    storedData = data;
    index = 0;
}

int EvaluateString::getResult() {
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

int EvaluateString::term() {
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

int EvaluateString::factor() {
    if (peek() >= '0' && peek() <= '9') {
        return number();
    }
    if (peek() == '(') {
        get();
        const int result = getResult();
        get();
        return result;
    }
    if (peek() == '-') {
        get();
        return -factor();
    }
    return 0;
}

int EvaluateString::number() {
    int result = get() - '0';
    while (peek() >= '0' && peek() <= '9') {
        result = 10 * result + get() - '0';
    }
    return result;
}

char EvaluateString::get() {
    return storedData[index++];
}

char EvaluateString::peek() const {
    return storedData[index];
}
