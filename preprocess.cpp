#include <string>
#include <iostream>
#include <instruction.h>


bool parse_and_validate_query(std::string_view query, Command *command){
    query = trim(query);

    auto opcode_pos = query.find_first_of(" ");
    std::string_view opcode = query.substr(0, opcode_pos);
    
    auto it = opcode_mapper.find(opcode);

    if (it == opcode_mapper.end()){
        std::cout << "Invalid command: " << opcode << "\n";
        return false;
    }

    command->opcode = it->second;

    query = trim(query.substr(opcode_pos + 1, query.length() - (opcode_pos + 1)));

    if (query.length() == 0){
        std::cout << "Key not provided" << "\n";
        return false;
    }

    auto key_pos = query.find_first_of(" ");

    if (key_pos == std::string_view::npos){
        command->key = query.substr(0, query.length() - 1);
        return true;
    }

    command->key = query.substr(0, key_pos);
    query = trim(query.substr(key_pos + 1, query.length() - (key_pos + 1)));

    if (query.length() == 0){
        if (command->opcode == Opcodes::SET){
            std::cout << "Value not provided" << "\n";
            return false;
        }
        return true;
    }

    command->value = query;

    return true;
}


std::string_view trim(std::string_view str){
    if (str.length() == 0){
        return str;
    }

    while (str.length() > 0){
        char c = str.front();

        if (c != ' '){
            break;
        }

        str = str.substr(1, str.length() - 1);
    }

    while (str.length() > 0){
        char c = str.back();

        if (c != ' '){
            break;
        }

        str = str.substr(0, str.length() - 1);
    }

    return str;
}