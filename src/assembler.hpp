#pragma once
#include <string>
#include <vector>
#include <map>
#include "tokenizer.hpp"
#include "instruction_validator.hpp"

struct AssemblerOutput {
  std::vector<std::string> object_code;
  std::vector<std::string> errors;
  std::map<std::string, int> symbol_table;
};

class Assembler {
public:
  Assembler();
  
  // Process an assembly file and generate object code
  AssemblerOutput assemble(const std::string& filename);
  
private:
  Tokenizer tokenizer_;
  InstructionValidator validator_;
  
  // Symbol table for labels
  std::map<std::string, int> symbol_table_;
  
  // Program counter
  int location_counter_ = 0;
  
  // Processing methods
  void process_first_pass(const std::vector<std::string>& source_lines, std::vector<TokenizedLine>& tokenized_lines, std::vector<std::string>& errors);
  void process_second_pass(const std::vector<TokenizedLine>& tokenized_lines, std::vector<std::string>& object_code, std::vector<std::string>& errors);
  
  // Helper methods
  int calculate_instruction_length(const TokenizedLine& line);
};
