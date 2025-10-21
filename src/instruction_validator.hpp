#include <string>
#include <cstdint>
#include <utility>

class InstructionValidator {
public:
  explicit InstructionValidator(std::string instruction)
      : instruction_(std::move(instruction)) {}
  std::string isValid();

private:
  std::string instruction_;
  bool validity_ = false;
  std::string name_of_instruction_;
  std::string error_message;

  [[nodiscard]] std::size_t num_of_bytes() const;

  static inline uint8_t hex_to_byte_for_first_byte(const char *hex);

  void validate_case(int size_of_instruction_);
};
