#ifndef SIC_XE_ASSEMBLER_PASS_1_H
#define SIC_XE_ASSEMBLER_PASS_1_H
#include <fstream>
#include <string>
#include <utility>
#include <iostream>

#include "table_store.h"


class pass_1 {
    std::string fileName;
    bool error_flag = false;
    table_store *tableStore;
    std::ifstream SourceFile;
    std::ofstream intermediateFile;
    std::ofstream errorFile;
    int program_length{};
    std::string *BLocksNumToName{};
    std::string firstExecutable_Sec;
    std::string fileLine;
    std::string writeData, writeDataSuffix, writeDataPrefix;
    int startAddress{}, LOCCTR{}, saveLOCCTR{}, lineNumber = 0, lastDeltaLOCCTR = 0, lineNumberDelta = 0;
    int index = 0;

    std::string currentBlockName = "DEFAULT";
    int currentBlockNumber = 0;
    int totalBlocks = 1;

    bool statusCode{};
    std::string label, opcode, operand, comment;
    std::string tempOperand;
public:

    auto get_first_executable_sec() -> std::string;

    auto get_blocks_num_to_name() const -> std::string *;

    auto get_program_length() const -> int;

    pass_1(std::string filename, table_store *tables, const std::string &intermediateFileName,
           const std::string &errorFileName);;

    auto get_error() const -> bool;

    ~pass_1() {
        SourceFile.close();
        intermediateFile.close();
        errorFile.close();
    }

    auto run_pass_1() -> void;

    auto eval_expr(std::string expression, bool &relative) -> void;

    auto handle_LTORG(std::string &litPrefix) -> void;
};


#endif //SIC_XE_ASSEMBLER_PASS_1_H
