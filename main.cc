/*
 * main.cc
 *
 * SIC and SIC-XE Instruction Validator
 * CSO-101 Assignment 5 solution
 * Varun R Mallya
 * 23117144
 * Compile with `clang++ -Wall -Wextra -std=c++23 -O3 main.cc`
 */

#include <csignal>
#include <iostream>
#include <string>

#include "instruction_validator.hpp"
int main() {
  std::string input;
  std::cin >> input;
  InstructionValidator validator(input);
  std::cout << validator.isValid() << std::endl;
  return 0;
}
