#include "pass_2.h"
#include "utilities.h"

#include <string>
#include <fstream>
#include <vector>
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

void pass_2::writeTextRecord(bool lastRecord) {
}

std::string pass_2::createObjectCodeFormat34() {
}

void pass_2::writeEndRecord(bool write) {
}

pass_2::pass_2(std::string filename, table_store *tables, const std::string &intermediateFileName,
               const std::string &objectFileName, const std::string &listingFileName, const std::string &errorFileName,
               std::string *blocksNumToName, const std::string &firstExecutableSec, int progLength) {
}

void pass_2::run_pass_2() {
}

pass_2::~pass_2() {
}
