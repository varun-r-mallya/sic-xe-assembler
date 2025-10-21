#include "instruction_validator.hpp"
#include "opcode_table.hpp"
#include <regex>
#include <algorithm>

InstructionValidator::InstructionValidator() {}

bool InstructionValidator::validate(const TokenizedLine& line, std::string& error_message) {
  // If line already has an error from tokenizer, return that error
  if (line.has_error) {
    error_message = line.error_message;
    return false;
  }
  
  // Check if it's a directive
  if (line.operation == "START" || 
      line.operation == "END" || 
      line.operation == "BYTE" || 
      line.operation == "WORD" || 
      line.operation == "RESB" || 
      line.operation == "RESW") {
    return validate_directive(line, error_message);
  }
  
  // Look up the instruction
  const auto* instr = find_instruction_by_mnemonic(line.operation);
  if (!instr) {
    error_message = "Unknown operation: " + line.operation;
    return false;
  }
  
  // Validate based on format
  bool is_extended = false;
  std::string op = line.operation;
  
  // Check for extended format (format 4)
  if (!op.empty() && op[0] == '+') {
    is_extended = true;
    op = op.substr(1); // Remove + prefix
  }
  
  if (is_extended) {
    return validate_format4(line, error_message);
  }
  
  switch (instr->fmt) {
    case 1:
      return validate_format1(line, error_message);
    case 2:
      return validate_format2(line, error_message);
    case 3:
      return validate_format3(line, error_message);
    default:
      error_message = "Unknown instruction format";
      return false;
  }
}

bool InstructionValidator::validate_format1(const TokenizedLine& line, std::string& error_message) {
  if (!line.operand.empty()) {
    error_message = "Format 1 instruction should not have operands";
    return false;
  }
  return true;
}

bool InstructionValidator::validate_format2(const TokenizedLine& line, std::string& error_message) {
  if (line.operand.empty()) {
    error_message = "Format 2 instruction requires register operands";
    return false;
  }
  
  // Check for valid register operands (r1[,r2])
  std::regex reg_format(R"(([ABXLSTF]|PC|SW)(?:,([ABXLSTF]|PC|SW))?)");
  std::smatch matches;
  
  if (!std::regex_match(line.operand, matches, reg_format)) {
    error_message = "Invalid register operand format";
    return false;
  }
  
  return true;
}

bool InstructionValidator::validate_format3(const TokenizedLine& line, std::string& error_message) {
  // RSUB is special - it doesn't need an operand
  if (line.operation == "RSUB" && line.operand.empty()) {
    return true;
  }
  
  if (line.operand.empty()) {
    error_message = "Format 3 instruction requires an operand";
    return false;
  }
  
  // Basic check for format 3 operands - would need more sophisticated checking in a full assembler
  std::regex operand_format(R"((@|#)?[A-Za-z0-9_]+(?:,[X])?)");
  if (!std::regex_match(line.operand, operand_format)) {
    error_message = "Invalid format 3 operand syntax";
    return false;
  }
  
  return true;
}

bool InstructionValidator::validate_format4(const TokenizedLine& line, std::string& error_message) {
  // Format 4 is just an extended format 3
  return validate_format3(line, error_message);
}

bool InstructionValidator::validate_directive(const TokenizedLine& line, std::string& error_message) {
  if (line.operation == "START" || line.operation == "END") {
    // START and END should have a hex address as operand
    if (line.operation == "START" && line.operand.empty()) {
      error_message = "START directive requires an address operand";
      return false;
    }
    
    if (!line.operand.empty() && !is_valid_hex_constant(line.operand)) {
      error_message = "START/END operand must be a valid hex address";
      return false;
    }
    
  } else if (line.operation == "BYTE") {
    // BYTE should be C'...' or X'...'
    std::regex byte_format(R"((C|X)'([^']*)')");
    std::smatch matches;
    
    if (!std::regex_match(line.operand, matches, byte_format)) {
      error_message = "BYTE directive requires C'...' or X'...' format";
      return false;
    }
    
    if (matches[1] == "X") {
      std::string hex_value = matches[2];
      if (hex_value.size() % 2 != 0 || !is_valid_hex_constant(hex_value)) {
        error_message = "X'...' must contain an even number of valid hex digits";
        return false;
      }
    }
    
  } else if (line.operation == "WORD") {
    // WORD should be a decimal number
    std::regex word_format(R"(-?\d+)");
    if (!std::regex_match(line.operand, word_format)) {
      error_message = "WORD directive requires a decimal number";
      return false;
    }
    
  } else if (line.operation == "RESB" || line.operation == "RESW") {
    // RESB/RESW should be a positive decimal number
    std::regex res_format(R"(\d+)");
    if (!std::regex_match(line.operand, res_format)) {
      error_message = line.operation + " directive requires a positive decimal number";
      return false;
    }
  }
  
  return true;
}

uint8_t InstructionValidator::get_register_number(const std::string& reg_name) {
  if (reg_name == "A") return 0;
  if (reg_name == "X") return 1;
  if (reg_name == "L") return 2;
  if (reg_name == "B") return 3;
  if (reg_name == "S") return 4;
  if (reg_name == "T") return 5;
  if (reg_name == "F") return 6;
  if (reg_name == "PC") return 8;
  if (reg_name == "SW") return 9;
  return 0; // Default/error
}

bool InstructionValidator::is_valid_hex_constant(const std::string& hex) {
  return std::all_of(hex.begin(), hex.end(), [](unsigned char c) {
    return std::isxdigit(c);
  });
}

std::string InstructionValidator::generateMachineCode(const TokenizedLine& line) {
  // This would be implemented in a full assembler
  // For now, return a placeholder
  return "Machine code generation not implemented";
}

// Add this to the end of your instruction_validator.cc file

const Instruction* InstructionValidator::getInstructionInfo(const std::string& mnemonic) {
  // Handle extended format prefix '+'
  std::string op = mnemonic;
  if (!op.empty() && op[0] == '+') {
    op = op.substr(1);  // Remove + prefix
  }
  
  // Use the existing function from opcode_table.hpp
  return find_instruction_by_mnemonic(op);
}