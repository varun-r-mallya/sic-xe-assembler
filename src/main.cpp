#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>

#include "pass_1.h"
#include "table_store.h"

void print_help(const char *program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] <input_file> <output_file>\n"
            << "\nOptions:\n"
            << "  -h           Display this help message\n"
            << "  -i           Output intermediate files\n"
            << "\nArguments:\n"
            << "  input_file   Path to the input file\n"
            << "  output_file  Path to the output file\n";
}

int main(const int argc, char *argv[]) {
    std::string input_file;
    std::string output_file;
    bool output_intermediate = false;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        }
        if (std::strcmp(argv[i], "-i") == 0) {
            output_intermediate = true;
        } else if (input_file.empty()) {
            input_file = argv[i];
        } else if (output_file.empty()) {
            output_file = argv[i];
        } else {
            std::cerr << "Error: Too many arguments\n";
            print_help(argv[0]);
            return 1;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: Input file is required\n";
        print_help(argv[0]);
        return 1;
    }

    if (output_file.empty()) {
        output_file = input_file + ".out";
    }

    if (!std::filesystem::exists(input_file)) {
        std::cerr << "Error: Input file '" << input_file << "' does not exist\n";
        return 1;
    }

    auto tables = table_store();
    auto first_pass = pass_1(input_file, &tables);
    if (output_intermediate) {
        std::ofstream symtab_file("SYMTAB.txt");
        symtab_file << std::left << std::setw(10) << "Symbol"
                << std::setw(20) << "Address"
                << std::setw(20) << "Block Number"
                << std::setw(20) << "Exists"
                << std::setw(20) << "Name"
                << std::setw(20) << "Relative" << "\n";
        for (auto symbol_table = tables.SYMTAB; const auto &[fst, snd]: symbol_table) {
            symtab_file << std::left << std::setw(10) << fst
                    << std::setw(20) << snd.address
                    << std::setw(20) << snd.blockNumber
                    << std::setw(20) << snd.exists
                    << std::setw(20) << snd.name
                    << std::setw(20) << snd.relative << "\n";
        }
        std::ofstream blocks_file("LITTAB.txt");
        blocks_file << std::left << std::setw(10) << "Block"
                << std::setw(20) << "Address"
                << std::setw(20) << "Block Number"
                << std::setw(20) << "Exists"
                << std::setw(20) << "Value" << "\n";
        for (auto literal_table = tables.LITTAB; const auto &[fst, snd]: literal_table) {
            blocks_file << std::left << std::setw(10) << fst
                    << std::setw(20) << snd.address
                    << std::setw(20) << snd.blockNumber
                    << std::setw(20) << snd.exists
                    << std::setw(20) << snd.value << "\n";
        }
    }


    return 0;
}
