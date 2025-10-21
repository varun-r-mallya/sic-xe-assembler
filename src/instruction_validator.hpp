#pragma once
#include <string>
#include <cstdint>
#include "tokenizer.hpp"
#include "opcode_table.hpp"

class InstructionValidator {
public:
  explicit InstructionValidator();
  
  // Validate a tokenized instruction line
  bool validate(const TokenizedLine& line, std::string& error_message);
  
  // Generate machine code for a valid instruction
  std::string generateMachineCode(const TokenizedLine& line);
  
  // Get instruction information from mnemonic
  const Instruction* getInstructionInfo(const std::string& mnemonic);
  
private:
  bool validate_format1(const TokenizedLine& line, std::string& error_message);
  bool validate_format2(const TokenizedLine& line, std::string& error_message);
  bool validate_format3(const TokenizedLine& line, std::string& error_message);
  bool validate_format4(const TokenizedLine& line, std::string& error_message);
  bool validate_directive(const TokenizedLine& line, std::string& error_message);
  
  uint8_t get_register_number(const std::string& reg_name);
  bool is_valid_hex_constant(const std::string& hex);
};
