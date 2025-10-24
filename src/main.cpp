#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>

#include "pass_1.h"
#include "table_store.h"

void print_help(const char* program_name)
{
    std::cout << "Usage: " << program_name << " [OPTIONS] <input_file> <output_file>\n"
        << "\nOptions:\n"
        << "  -h           Display this help message\n"
        << "  -i           Output intermediate files\n"
        << "\nArguments:\n"
        << "  input_file   Path to the input file\n"
        << "  output_file  Path to the output file\n";
}

int main(const int argc, char* argv[])
{
    std::string input_file;
    std::string output_file;

    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "-h") == 0)
        {
            print_help(argv[0]);
            return 0;
        }
        if (std::strcmp(argv[i], "-i") == 0)
        {
            bool output_intermediate = false;
            output_intermediate = true;
        }
        else if (input_file.empty())
        {
            input_file = argv[i];
        }
        else if (output_file.empty())
        {
            output_file = argv[i];
        }
        else
        {
            std::cerr << "Error: Too many arguments\n";
            print_help(argv[0]);
            return 1;
        }
    }

    if (input_file.empty() || output_file.empty())
    {
        std::cerr << "Error: Both Input and Output files are required\n";
        print_help(argv[0]);
        return 1;
    }

    if (!std::filesystem::exists(input_file))
    {
        std::cerr << "Error: Input file '" << input_file << "' does not exist\n";
        return 1;
    }

    // std::cout << "Input file: " << input_file << "\n";
    // std::cout << "Output file: " << output_file << "\n";
    // std::cout << "Output intermediate files: " << (output_intermediate ? "yes" : "no") << "\n";

    auto tables = table_store();
    auto first_pass = pass_1(input_file, &tables);
    for (auto symbol_table = tables.SYMTAB; const auto& [fst, snd] : symbol_table)
    {
        std::cout << fst << std::setw(20) << snd.address << std::setw(20) << snd.blockNumber <<
            std::setw(20) << snd.exists << std::setw(20)
            << snd.name << std::setw(20) << snd.relative << "\n";
    }

    return 0;
}
