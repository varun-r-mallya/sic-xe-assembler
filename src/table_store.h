#ifndef SIC_XE_ASSEMBLER_TABLES_H
#define SIC_XE_ASSEMBLER_TABLES_H

#include<map>
#include<string>

struct struct_opcode {
    std::string opcode;
    int format;
    char exists;

    struct_opcode() {
        opcode = "undefined";
        format = 0;
        exists = 'n';
    }
};

struct struct_literal {
    std::string value;
    std::string address;
    char exists;
    int blockNumber = 0;

    struct_literal() {
        value = "";
        address = "?";
        blockNumber = 0;
        exists = 'n';
    }
};

struct struct_label {
    std::string address;
    std::string name;
    int relative;
    int blockNumber;
    char exists;

    struct_label() {
        name = "undefined";
        address = "0";
        blockNumber = 0;
        exists = 'n';
        relative = 0;
    }
};

struct struct_blocks {
    std::string startAddress;
    std::string name;
    std::string LOCCTR;
    int number;
    char exists;

    struct_blocks() {
        name = "undefined";
        startAddress = "?";
        exists = 'n';
        number = -1;
        LOCCTR = "0";
    }
};

struct struct_register {
    char num;
    char exists;

    struct_register() {
        num = 'F';
        exists = 'n';
    }
};

class table_store {
    void load_register_table();

    void load_blocks();

public:
    std::map<std::string, struct_label> SYMTAB;
    std::map<std::string, struct_opcode> OPTAB;
    std::map<std::string, struct_register> REGTAB;
    std::map<std::string, struct_literal> LITTAB;
    std::map<std::string, struct_blocks> BLOCKS;

    table_store();

    void load_op_table();
};

#endif //SIC_XE_ASSEMBLER_TABLES_H
