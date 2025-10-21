#include "tokenizer.hpp"
#include "opcode_table.hpp"
#include <algorithm>
#include <cctype>
#include <regex>

Tokenizer::Tokenizer() {}

std::string Tokenizer::trim(const std::string& str) {
  const auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) {
    return std::isspace(c);
  });
  
  const auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
    return std::isspace(c);
  }).base();
  
  return (start < end) ? std::string(start, end) : std::string();
}

TokenizedLine Tokenizer::tokenize(const std::string& line, int line_number) {
  TokenizedLine result;
  result.line_number = line_number;
  result.has_error = false;
  
  // Handle empty lines or comment-only lines
  std::string trimmed_line = trim(line);
  if (trimmed_line.empty() || trimmed_line[0] == '.') {
    if (!trimmed_line.empty()) {
      result.comment = trimmed_line;
    }
    result.operation = "COMMENT";
    return result;
  }
  
  // Extract comment if present
  size_t comment_pos = trimmed_line.find('.');
  if (comment_pos != std::string::npos) {
    result.comment = trim(trimmed_line.substr(comment_pos));
    trimmed_line = trim(trimmed_line.substr(0, comment_pos));
  }
  
  // Regex to match label, operation, and operand fields
  // Format: [label] operation [operand]
  std::regex line_regex(R"((?:([A-Za-z][A-Za-z0-9]*)\s+)?([A-Za-z+][A-Za-z0-9+]*)\s*(?:(.+))?)");
  std::smatch matches;
  
  if (std::regex_match(trimmed_line, matches, line_regex)) {
    if (matches.size() > 1 && matches[1].matched) {
      result.label = matches[1].str();
    }
    
    if (matches.size() > 2 && matches[2].matched) {
      result.operation = matches[2].str();
      // Convert to uppercase for consistency
      std::transform(result.operation.begin(), result.operation.end(), 
                    result.operation.begin(), ::toupper);
    } else {
      result.has_error = true;
      result.error_message = "Missing operation";
      return result;
    }
    
    if (matches.size() > 3 && matches[3].matched) {
      result.operand = trim(matches[3].str());
    }
  } else {
    result.has_error = true;
    result.error_message = "Invalid syntax";
    return result;
  }
  
  // Validate operation if it's not a directive
  if (result.operation != "START" && 
      result.operation != "END" && 
      result.operation != "BYTE" && 
      result.operation != "WORD" && 
      result.operation != "RESB" && 
      result.operation != "RESW") {
    
    // Check if it's a valid mnemonic
    const auto* instr = find_instruction_by_mnemonic(result.operation);
    if (!instr) {
      result.has_error = true;
      result.error_message = "Unknown operation: " + result.operation;
      return result;
    }
    
    // Validate operands based on format
    if (instr->fmt == 1 && !result.operand.empty()) {
      result.has_error = true;
      result.error_message = "Format 1 instruction should not have operands";
    } else if (instr->fmt == 2 && result.operand.empty()) {
      result.has_error = true;
      result.error_message = "Format 2 instruction requires register operands";
    } else if (instr->fmt == 3 && result.operand.empty() && result.operation != "RSUB") {
      result.has_error = true;
      result.error_message = "Format 3 instruction requires an operand";
    }
  }
  
  return result;
}

bool Tokenizer::is_valid_label(const std::string& label) {
  // Labels must start with a letter and contain only letters and numbers
  if (label.empty() || !std::isalpha(label[0])) {
    return false;
  }
  
  return std::all_of(label.begin(), label.end(), [](unsigned char c) {
    return std::isalnum(c);
  });
}

bool Tokenizer::is_valid_operation(const std::string& operation) {
  // Check if it's a valid mnemonic or directive
  const auto* instr = find_instruction_by_mnemonic(operation);
  return instr != nullptr || 
         operation == "START" || 
         operation == "END" || 
         operation == "BYTE" || 
         operation == "WORD" || 
         operation == "RESB" || 
         operation == "RESW";
}

bool Tokenizer::is_valid_operand(const std::string& operand, const std::string& operation) {
  // Basic validation for now - would need more sophisticated checking in a full assembler
  return !operand.empty() || operation == "RSUB";
}
