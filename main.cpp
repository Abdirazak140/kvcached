#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <chrono>


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

const std::unordered_map<std::string, Opcodes> opcode_mapper {
    {"SET", Opcodes::SET},
    {"GET", Opcodes::GET},
    {"DELETE", Opcodes::DELETE},
};

std::unordered_map<std::string, std::string> kv_store {};

std::vector<Command> tasks {};

std::atomic_bool running = true;

std::mutex mtx;

// Preprocessing forward declerations
std::string listen();
bool parse_and_validate_query(std::string query, Command *command);
std::string_view trim(std::string_view str);

// Worker operations forward declerations
void worker_handler();
void kv_set(std::string key, std::string value);
std::string kv_get(std::string key);
void kv_delete(std::string key);


int main(){
    const unsigned int cores = std::thread::hardware_concurrency();

    std::vector<std::thread> workers {};

    for (auto i {0}; i < cores; ++i){
        std::thread t (worker_handler);
        workers.push_back(std::move(t));
    }

    while (running){
        std::string query = listen();
        Command command {};

        bool is_valid = parse_and_validate_query(query, &command);

        if (!is_valid){
            continue;
        }

        tasks.push_back(command);
    }


    for (auto& worker : workers){
        worker.join();
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
        std::cout << "Invalid command: " << opcode << "\n";
        return false;
    }

    command->opcode = it->second;

    std::string_view query = trim(query.substr(opcode_pos + 1));

    if (query.length() == 0){
        std::cout << "Key not provided" << "\n";
        return false;
    }

    auto key_pos = query.find_first_of(" ");

    if (key_pos == std::string::npos){
        command->key = query;
        return true;
    }
    else{
        command->key = query.substr(0, key_pos);
    }

    
    std::string_view query = trim(query.substr(key_pos + 1));

    if (query.length() == 0){
        if (command->opcode == Opcodes::SET){
            std::cout << "Value not provided" << "\n";
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


void worker_handler(){

    while (running){
        std::this_thread::sleep_for(std::chrono::seconds(2));

        mtx.lock();
        if (!tasks.empty()){
            const Command task = *tasks.end();
            tasks.erase(tasks.end());

            mtx.unlock();

            switch (task.opcode)
            {
            case Opcodes::SET:
                kv_set(task.key, task.value);
                break;
            
            case Opcodes::GET:
                kv_get(task.key);
                break;

            case Opcodes::DELETE:
                kv_delete(task.key);
                break;
            }
            
        }
    }
}


void kv_set(std::string key, std::string value){
    kv_store.insert(key, value);
}

std::string kv_get(std::string key){
    auto it = kv_store.find(key);
    return it->second;
}

void kv_delete(std::string key){
    auto it = kv_store.find(key);
    kv_store.erase(it);
}