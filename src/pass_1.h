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

public:

    std::string get_first_executable_sec();

    std::string* get_blocks_num_to_name() const;

    int get_program_length() const;

    pass_1(std::string filename, table_store *tables, const std::string &intermediateFileName,
           const std::string &errorFileName) {
        this->fileName = std::move(filename);
        this->tableStore = tables;
        SourceFile.open(fileName);
        intermediateFile.open(intermediateFileName);
        errorFile.open(errorFileName);
        run_pass_1();
    };

    bool get_error() const;

    ~pass_1() {
        SourceFile.close();
        intermediateFile.close();
        errorFile.close();
    }

    void run_pass_1();

    void evaluateExpression(std::string expression, bool &relative, std::string &tempOperand, int lineNumber);

    void handle_LTORG(std::string &litPrefix, int &lineNumberDelta, int lineNumber, int &LOCCTR, int &lastDeltaLOCCTR,
                      int currentBlockNumber) const;
};


#endif //SIC_XE_ASSEMBLER_PASS_1_H
