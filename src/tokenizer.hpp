#pragma once
#include <string>
#include <vector>
#include <optional>

// Represents a tokenized assembly instruction line
struct TokenizedLine {
  std::string label;             // Optional label (empty if none)
  std::string operation;         // Mnemonic or directive
  std::string operand;           // Operand field (may contain commas for multiple operands)
  std::string comment;           // Optional comment (empty if none)
  int line_number;               // Source line number
  bool has_error;                // True if line has syntax error
  std::string error_message;     // Error message if any
};

class Tokenizer {
public:
  explicit Tokenizer();
  
  // Process a line of assembly code and return tokenized result
  TokenizedLine tokenize(const std::string& line, int line_number);
  
private:
  // Helper methods
  bool is_valid_label(const std::string& label);
  bool is_valid_operation(const std::string& operation);
  bool is_valid_operand(const std::string& operand, const std::string& operation);
  
  // Trim whitespace from start and end of string
  static std::string trim(const std::string& str);
};
