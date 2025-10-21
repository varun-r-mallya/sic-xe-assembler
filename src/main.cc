/*
 * main.cc
 *
 * SIC/XE Assembler
 * Based on instruction validator from CSO-101 Assignment 7
 * Varun R Mallya
 * 23117144
 */

#include <iostream>
#include <string>
#include "assembler.hpp"

void print_usage() {
  std::cout << "Usage: ./assembler <input_file>" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Error: No input file specified." << std::endl;
    print_usage();
    return 1;
  }
  
  std::string input_file = argv[1];
  Assembler assembler;
  
  std::cout << "Assembling file: " << input_file << std::endl;
  
  AssemblerOutput output = assembler.assemble(input_file);
  
  // Print any errors
  if (!output.errors.empty()) {
    std::cout << "\nErrors:" << std::endl;
    for (const auto& error : output.errors) {
      std::cout << error << std::endl;
    }
    return 1;
  }
  
  // Print symbol table
  std::cout << "\nSymbol Table:" << std::endl;
  std::cout << "-----------------" << std::endl;
  for (const auto& [symbol, address] : output.symbol_table) {
    std::cout << symbol << ": " << std::hex << std::uppercase << address << std::endl;
  }
  std::cout << std::dec; // Reset to decimal output
  
  // Print object code
  std::cout << "\nObject Code:" << std::endl;
  std::cout << "-----------------" << std::endl;
  for (const auto& code : output.object_code) {
    std::cout << code << std::endl;
  }
  
  return 0;
}
