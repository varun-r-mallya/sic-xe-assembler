#include "instruction_validator.hpp"
#include "opcode_table.hpp"

auto InstructionValidator::isValid() -> std::string {
  switch (num_of_bytes()) {
  case 1:
    validate_case(1);
    break;
  case 2:
    validate_case(2);
    break;
  case 3:
  case 4:
    validate_case(3);
    break;
  default:
    validity_ = false;
    error_message = "Invalid size of instruction";
  }
  if (validity_) {
    return name_of_instruction_;
  }
  return std::string("INVALID").append("\n").append(error_message);
}

auto InstructionValidator::num_of_bytes() const -> std::size_t {
  const std::size_t size_of_string = instruction_.size();
  if (size_of_string % 2) {
    return -1;
  }
  return size_of_string / 2;
}

auto InstructionValidator::hex_to_byte_for_first_byte(const char *hex) -> uint8_t {
  auto nybble = [](char c) -> uint8_t {
    return (c <= '9') ? c - '0' : (toupper(c) - 'A' + 10);
  };
  return (nybble(hex[0]) << 4) | nybble(hex[1]);
}

auto InstructionValidator::validate_case(int size_of_instruction_) -> void {
  auto opcode = hex_to_byte_for_first_byte(instruction_.data());

  const auto &formatting = opcode_table[opcode];
  if (formatting.valid) {
    name_of_instruction_ = formatting.mnemonic;
    if (formatting.fmt1 == size_of_instruction_) {
      validity_ = true;
    } else {
      validity_ = false;
      error_message = "Invalid number of operands for " + name_of_instruction_;
    }
  } else {
    validity_ = false;
    error_message = "Opcode not found";
  }
}
