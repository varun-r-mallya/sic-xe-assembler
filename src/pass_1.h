#ifndef SIC_XE_ASSEMBLER_PASS_1_H
#define SIC_XE_ASSEMBLER_PASS_1_H
#include <fstream>
#include <string>

#include "table_store.h"


class pass_1 {
    std::string file_name_;
    bool error_flag = false;
    table_store *table_store_;
    std::ifstream source_file_;
    std::ofstream intermediate_file_;
    std::ofstream error_file_;
    int program_length{};
    std::string *blocks_num_to_name_{};
    std::string first_executable_section_;
    std::string file_line_;
    std::string write_data_, write_data_suffix_, write_data_prefix_;
    int start_address_{}, location_counter{}, save_location_counter{}, line_number_ = 0, last_location_counter = 0,
            line_number_diff = 0;
    int index = 0;

    std::string current_block_ = "DEFAULT";
    int current_blk_num = 0;
    int total_blocks_ = 1;

    bool status_code_{};
    std::string label, opcode, operand, comment;
    std::string temp_operand_;

public:
    auto get_first_executable_sec() -> std::string;

    auto get_blocks_num_to_name() const -> std::string *;

    auto get_program_length() const -> int;

    pass_1(std::string filename, table_store *tables, const std::string &intermediateFileName,
           const std::string &errorFileName);;

    auto get_error() const -> bool;

    ~pass_1() {
        source_file_.close();
        intermediate_file_.close();
        error_file_.close();
    }

    auto run_pass_1() -> void;

    auto eval_expr(std::string expression, bool &relative) -> void;

    auto handle_LTORG(std::string &litPrefix) -> void;
};


#endif //SIC_XE_ASSEMBLER_PASS_1_H
