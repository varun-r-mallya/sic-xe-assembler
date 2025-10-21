#pragma once
#include <array>
#include <cstdint>
#include <string_view>

/*
 * Opcode table with index of opcode and indexed to mnemonic and formats in a
 * struct with 3 and 4 both represented by a 3 as there are no 4 only formats
 * and always comes with a 3. Used a constexpr lambda array to make sure no
 * hashing occurs at runtime and everything is calculated at compile time.
 */
struct Instruction {
  uint8_t opcode;
  std::string_view mnemonic;
  uint8_t fmt1;
  bool valid;
};

constexpr std::array<Instruction, 256> opcode_table = [] {
  std::array<Instruction, 256> table{};

  table[0x18] = {0x18, "ADD", 3, true};
  table[0x58] = {0x58, "ADDF", 3, true};
  table[0x90] = {0x90, "ADDR", 2, true};
  table[0x40] = {0x40, "AND", 3, true};
  table[0xB4] = {0xB4, "CLEAR", 2, true};
  table[0x28] = {0x28, "COMP", 3, true};
  table[0x88] = {0x88, "COMPF", 3, true};
  table[0xA0] = {0xA0, "COMPR", 2, true};
  table[0x24] = {0x24, "DIV", 3, true};
  table[0x64] = {0x64, "DIVF", 3, true};
  table[0x9C] = {0x9C, "DIVR", 2, true};
  table[0xC4] = {0xC4, "FIX", 1, true};
  table[0xC0] = {0xC0, "FLOAT", 1, true};
  table[0xF4] = {0xF4, "HIO", 1, true};
  table[0x3C] = {0x3C, "J", 3, true};
  table[0x30] = {0x30, "JEQ", 3, true};
  table[0x34] = {0x34, "JGT", 3, true};
  table[0x38] = {0x38, "JLT", 3, true};
  table[0x48] = {0x48, "JSUB", 3, true};
  table[0x00] = {0x00, "LDA", 3, true};
  table[0x68] = {0x68, "LDB", 3, true};
  table[0x50] = {0x50, "LDCH", 3, true};
  table[0x70] = {0x70, "LDF", 3, true};
  table[0x08] = {0x08, "LDL", 3, true};
  table[0x6C] = {0x6C, "LDS", 3, true};
  table[0x74] = {0x74, "LDT", 3, true};
  table[0x04] = {0x04, "LDX", 3, true};
  table[0xD0] = {0xD0, "LPS", 3, true};
  table[0x20] = {0x20, "MUL", 3, true};
  table[0x60] = {0x60, "MULF", 3, true};
  table[0x98] = {0x98, "MULR", 2, true};
  table[0xC8] = {0xC8, "NORM", 1, true};
  table[0x44] = {0x44, "OR", 3, true};
  table[0xD8] = {0xD8, "RD", 3, true};
  table[0xAC] = {0xAC, "RMO", 2, true};
  table[0x4C] = {0x4C, "RSUB", 3, true};
  table[0xA4] = {0xA4, "SHIFTL", 2, true};
  table[0xA8] = {0xA8, "SHIFTR", 2, true};
  table[0xF0] = {0xF0, "SIO", 1, true};
  table[0xEC] = {0xEC, "SSK", 3, true};
  table[0x0C] = {0x0C, "STA", 3, true};
  table[0x78] = {0x78, "STB", 3, true};
  table[0x54] = {0x54, "STCH", 3, true};
  table[0x80] = {0x80, "STF", 3, true};
  table[0xD4] = {0xD4, "STI", 3, true};
  table[0x14] = {0x14, "STL", 3, true};
  table[0x7C] = {0x7C, "STS", 3, true};
  table[0xE8] = {0xE8, "STSW", 3, true};
  table[0x84] = {0x84, "STT", 3, true};
  table[0x10] = {0x10, "STX", 3, true};
  table[0x1C] = {0x1C, "SUB", 3, true};
  table[0x5C] = {0x5C, "SUBF", 3, true};
  table[0x94] = {0x94, "SUBR", 2, true};
  table[0xB0] = {0xB0, "SVC", 2, true};
  table[0xE0] = {0xE0, "TD", 3, true};
  table[0xF8] = {0xF8, "TIO", 1, true};
  table[0x2C] = {0x2C, "TIX", 3, true};
  table[0xB8] = {0xB8, "TIXR", 2, true};
  table[0xDC] = {0xDC, "WD", 3, true};

  return table;
}();
