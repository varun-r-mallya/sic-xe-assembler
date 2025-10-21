#include "assembler.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>  // Add this for setw and setfill

Assembler::Assembler() {}

AssemblerOutput Assembler::assemble(const std::string& filename) {
  AssemblerOutput output;
  std::vector<std::string> source_lines;
  std::vector<TokenizedLine> tokenized_lines;
  
  // Read the source file
  std::ifstream file(filename);
  if (!file.is_open()) {
    output.errors.push_back("Could not open file: " + filename);
    return output;
  }
  
  // Read all lines
  std::string line;
  while (std::getline(file, line)) {
    source_lines.push_back(line);
  }
  
  // Process first pass - build symbol table and validate syntax
  process_first_pass(source_lines, tokenized_lines, output.errors);
  
  // If there are errors, don't proceed to second pass
  if (!output.errors.empty()) {
    return output;
  }
  
  // Process second pass - generate object code
  process_second_pass(tokenized_lines, output.object_code, output.errors);
  
  // Copy symbol table to output
  output.symbol_table = symbol_table_;
  
  return output;
}

void Assembler::process_first_pass(
    const std::vector<std::string>& source_lines,
    std::vector<TokenizedLine>& tokenized_lines,
    std::vector<std::string>& errors) {
    
  location_counter_ = 0;
  symbol_table_.clear();
  bool program_started = false;
  bool program_ended = false;
  
  for (size_t i = 0; i < source_lines.size(); ++i) {
    const std::string& line = source_lines[i];
    TokenizedLine tokenized = tokenizer_.tokenize(line, i + 1);
    
    // Skip comment lines
    if (tokenized.operation == "COMMENT") {
      continue;
    }
    
    // Handle program start
    if (tokenized.operation == "START") {
      if (program_started) {
        std::ostringstream error;
        error << "Line " << (i+1) << ": Multiple START directives";
        errors.push_back(error.str());
        continue;
      }
      
      program_started = true;
      if (!tokenized.operand.empty()) {
        try {
          location_counter_ = std::stoi(tokenized.operand, nullptr, 16);
        } catch (const std::exception&) {
          std::ostringstream error;
          error << "Line " << (i+1) << ": Invalid START address";
          errors.push_back(error.str());
        }
      }
      tokenized_lines.push_back(tokenized);
      continue;
    }
    
    // Handle program end
    if (tokenized.operation == "END") {
      program_ended = true;
      tokenized_lines.push_back(tokenized);
      break;
    }
    
    // Make sure program has started
    if (!program_started) {
      std::ostringstream error;
      error << "Line " << (i+1) << ": Code before START directive";
      errors.push_back(error.str());
      continue;
    }
    
    // Validate the instruction
    std::string error_message;
    if (!validator_.validate(tokenized, error_message)) {
      std::ostringstream error;
      error << "Line " << (i+1) << ": " << error_message;
      errors.push_back(error.str());
      continue;
    }
    
    // Add label to symbol table if present
    if (!tokenized.label.empty()) {
      if (symbol_table_.find(tokenized.label) != symbol_table_.end()) {
        std::ostringstream error;
        error << "Line " << (i+1) << ": Duplicate label '" << tokenized.label << "'";
        errors.push_back(error.str());
      } else {
        symbol_table_[tokenized.label] = location_counter_;
      }
    }
    
    // Update location counter
    location_counter_ += calculate_instruction_length(tokenized);
    
    // Add to tokenized lines
    tokenized_lines.push_back(tokenized);
  }
  
  // Check if program ended properly
  if (!program_ended) {
    errors.push_back("Program has no END directive");
  }
}

void Assembler::process_second_pass(
    const std::vector<TokenizedLine>& tokenized_lines,
    std::vector<std::string>& object_code,
    std::vector<std::string>& errors) {  // We'll keep this parameter even if unused for now
    
  // Reset location counter
  location_counter_ = 0;
  
  for (const auto& line : tokenized_lines) {
    // Update location counter for each instruction
    if (line.operation == "START") {
      if (!line.operand.empty()) {
        try {
          location_counter_ = std::stoi(line.operand, nullptr, 16);
        } catch (const std::exception&) {
          // Error already reported in first pass
        }
      }
      continue;
    }
    
    if (line.operation == "END") {
      break;
    }
    
    // Generate object code for the instruction
    std::string code = validator_.generateMachineCode(line);
    
    // In a full assembler, we would replace symbolic operands with their addresses
    // from the symbol table and generate actual machine code
    
    // For now, just add a placeholder
    std::ostringstream obj_line;
    obj_line << std::hex << std::uppercase << std::setw(4) << std::setfill('0') 
             << location_counter_ << " " << code;
    object_code.push_back(obj_line.str());
    
    // Update location counter
    location_counter_ += calculate_instruction_length(line);
  }
}

int Assembler::calculate_instruction_length(const TokenizedLine& line) {
  // Directives
  if (line.operation == "RESW") {
    return 3 * std::stoi(line.operand);
  }
  if (line.operation == "RESB") {
    return std::stoi(line.operand);
  }
  if (line.operation == "WORD") {
    return 3;
  }
  if (line.operation == "BYTE") {
    // Calculate bytes based on C'...' or X'...' format
    if (line.operand.size() >= 3) {
      if (line.operand[0] == 'C') {
        // Character constant - 1 byte per character
        return line.operand.size() - 3; // Remove C''
      } else if (line.operand[0] == 'X') {
        // Hex constant - 1 byte per 2 hex digits
        return (line.operand.size() - 3) / 2; // Remove X''
      }
    }
    return 1; // Default
  }
  
  // Instructions
  if (line.operation[0] == '+') {
    // Format 4
    return 4;
  }
  
  // Use the validator to find the instruction format
  const auto* instr = validator_.getInstructionInfo(line.operation);
  if (instr) {
    return instr->fmt;
  }
  
  return 0; // Error case
}
