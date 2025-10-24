#ifndef SIC_XE_ASSEMBLER_PASS_1_H
#define SIC_XE_ASSEMBLER_PASS_1_H
#include <fstream>
#include <string>
#include <utility>

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
    pass_1(std::string filename, table_store *tables) {
        this->fileName = std::move(filename);
        this->tableStore = tables;
        SourceFile.open(fileName);

        // Extract directory path and filename from the source file path
        const size_t lastSlash = fileName.find_last_of("/\\");
        const std::string directory = (lastSlash != std::string::npos) ? fileName.substr(0, lastSlash + 1) : "";
        const std::string baseFileName = (lastSlash != std::string::npos) ? fileName.substr(lastSlash + 1) : fileName;

        intermediateFile.open(directory + "intermediate_" + baseFileName);
        errorFile.open(directory + "error_" + baseFileName);
        run_pass_1();
    };

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
