#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <unordered_map>
#include <atomic>


enum class Opcodes{
    SET,
    GET,
    DELETE
};


struct Command{
    Opcodes opcode;
    std::string key;
    std::string value;
};


std::unordered_map<std::string, Opcodes> opcode_mapper {
    {"SET", Opcodes::SET},
    {"GET", Opcodes::GET},
    {"DELETE", Opcodes::DELETE},
};


std::atomic_bool running = true;


std::unordered_map<std::string, std::string> kv_store {};


int main(){
    const unsigned int cores = std::thread::hardware_concurrency();

    std::vector<std::thread> workers {};

    for (auto i {0}; i <= cores; ++i){
        std::thread t;
        workers.push_back(std::move(t));
    }

    while (running){
        std::string query = listen();
        Command command {};

        bool is_valid = parse_and_validate_query(query, &command);

        if (!is_valid){
            continue;
        }

        switch (command.opcode){
            case Opcodes::SET:
                /* code */
                break;
                
            case Opcodes::GET:
                break;

            case Opcodes::DELETE:
                break;

            default:
                break;
        }

    }
}


std::string listen(){
    std::string buffer {};
    std::getline(std::cin, buffer);

    return buffer;
}


bool parse_and_validate_query(std::string query, Command *command){
    query = trim(query);

    auto opcode_pos = query.find_first_of(" ");
    std::string opcode = query.substr(0, opcode_pos);
    
    auto it = opcode_mapper.find(opcode);

    if (it == opcode_mapper.end()){
        return false;
    }

    command->opcode = it->second;

    std::string_view query = trim(query.substr(opcode_pos + 1));

    if (query.length() == 0){
        return false;
    }

    auto key_pos = query.find_first_of(" ");

    std::string key = query.substr(0, key_pos);

    command->key = key;

    std::string_view query = trim(query.substr(key_pos + 1));

    if (query.length() == 0){
        if (command->opcode == Opcodes::SET){
            return false;
        }
        return true;
    }

    auto value_pos = query.find_first_of(" ");

    std::string value = query.substr(0, value_pos);

    command->value = value;

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

        str = str.substr(1);
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


