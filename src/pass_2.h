#ifndef SIC_XE_ASSEMBLER_PASS_2_H
#define SIC_XE_ASSEMBLER_PASS_2_H

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "table_store.h"

class pass_2 {
        std::string fileName;
        table_store *tableStore;
        std::ifstream intermediateFile;
        std::ofstream errorFile;
        std::ofstream objectFile;
        std::ofstream listingFile;
        bool hasError = false;
        bool is_comment_{};
        std::string label, opcode, operand, comment;
        std::string operand1, operand2;

        int lineNumber{}, blockNumber = 0, address{}, startAddress{};
        std::string objectCode, writeData, currentRecord, modificationRecord, endRecord;
        int program_counter{}, base_register_value{};
        bool not_base = true;

        std::string *blocks_num_to_name;
        std::string firstExecutable_Sec;
        int program_length;

        static auto readTillTab(const std::string &data, int &index) -> std::string;
        auto readIntermediateFile() -> bool;
        auto writeTextRecord(bool lastRecord = false) -> void;
        auto createObjectCodeFormat34() -> std::string;
        auto writeEndRecord(bool write = true) -> void;

    public:
        pass_2(std::string filename, table_store *tables, const std::string &intermediateFileName,
               const std::string &objectFileName, const std::string &listingFileName,
               const std::string &errorFileName, std::string *blocksNumToName,
               const std::string &firstExecutableSec, int progLength);

        auto run_pass_2() -> void;

        ~pass_2();

        auto has_error() const -> bool;
};


#endif //SIC_XE_ASSEMBLER_PASS_2_H
