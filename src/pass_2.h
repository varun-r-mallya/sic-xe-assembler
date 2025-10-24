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

        bool isComment;
        std::string label, opcode, operand, comment;
        std::string operand1, operand2;

        int lineNumber, blockNumber, address, startAddress;
        std::string objectCode, writeData, currentRecord, modificationRecord, endRecord, write_R_Data;
        int program_counter, base_register_value;
        bool nobase;

        std::string *BLocksNumToName;
        std::string firstExecutable_Sec;
        int program_length;

        static std::string readTillTab(const std::string &data, int &index);
        bool readIntermediateFile();
        void writeRRecord();
        void writeTextRecord(bool lastRecord = false);
        std::string createObjectCodeFormat34();
        void writeEndRecord(bool write = true);

    public:
        pass_2(std::string filename, table_store *tables, const std::string &intermediateFileName,
               const std::string &objectFileName, const std::string &listingFileName,
               const std::string &errorFileName, std::string *blocksNumToName,
               const std::string &firstExecutableSec, int progLength);

        void run_pass_2();

        ~pass_2();
};


#endif //SIC_XE_ASSEMBLER_PASS_2_H
