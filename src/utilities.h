#ifndef SIC_XE_ASSEMBLER_UTILITIES_H
#define SIC_XE_ASSEMBLER_UTILITIES_H

#include<string>
#include<iomanip>

class utilities {
public:
    static int string_to_decimal(const std::string &str);

    static std::string get_string(char c);

    static std::string int_to_string_hex(int x, int fill = 5);

    static std::string str_expand(std::string data, int length, char fillChar, bool back = false);

    static int string_hex_to_int(const std::string &x, bool *success = nullptr);

    static std::string stringToHexString(const std::string &input);

    static bool checkWhiteSpace(char x);

    static bool checkCommentLine(const std::string &line);

    static bool if_all_num(const std::string &x);

    static void first_non_whitespace(const std::string &line, int &index, bool &status, std::string &data,
                                     bool readTillEnd = false);

    static void readByteOperand(const std::string &line, int &index, bool &status, std::string &data);

    static void write_to_file(std::ofstream &file, const std::string &data, bool newline = true);

    static std::string getRealOpcode(std::string opcode);

    static char getFlagFormat(const std::string &data);
};

class StrEval {
public:
    int get_result();

    explicit StrEval(const std::string &data);

private:
    std::string stored_data_;
    int index;

    [[nodiscard]] char peek() const;

    char get();

    int term();

    int factor();

    int number();
};


#endif //SIC_XE_ASSEMBLER_UTILITIES_H
