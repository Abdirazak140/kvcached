#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <unordered_map>
#include <string>


enum class Opcodes{
    SET,
    GET,
    DELETE
};

const inline std::unordered_map<std::string_view, Opcodes> opcode_mapper {
    {"SET", Opcodes::SET},
    {"GET", Opcodes::GET},
    {"DELETE", Opcodes::DELETE},
};

struct Command{
    Opcodes opcode;
    std::string key;
    std::string value;
    int socket;
};

#endif