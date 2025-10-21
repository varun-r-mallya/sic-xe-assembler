/*
 * main.cc
 *
 * SIC and SIC-XE Instruction Validator
 * CSO-101 Assignment 7 solution
 * Varun R Mallya
 * 23117144
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
