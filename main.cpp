#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <chrono>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>


enum class Opcodes{
    SET,
    GET,
    DELETE
};

struct Command{
    Opcodes opcode;
    std::string key;
    std::string value;
    int socket;
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
addrinfo *initialize_address_info(int ai_family=AF_INET);

// Worker operations forward declerations
void worker_handler();

std::string kv_set(std::string key, std::string value);
std::string kv_get(std::string key);
std::string kv_delete(std::string key);


void signal_handler(int){
    running = false;
}


int main(){
    const int listen_queue_size = 10;
    const unsigned int cores = std::thread::hardware_concurrency();
    auto address_info = initialize_address_info();
    
    int main_sock = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);

    if (main_sock == -1){
        std:perror("Failed to initialize main socket.");
    }

    if (bind(main_sock, address_info->ai_addr, address_info->ai_addrlen) == -1){
        std::perror("Failed to bind socket.");
    }

    if (listen(main_sock, listen_queue_size) == -1){
        std::perror("Failed to listen at socket.");
    }

    std::vector<std::thread> workers {};

    for (auto i {0}; i < cores; ++i){
        std::thread t (worker_handler);
        workers.push_back(std::move(t));
    }

    signal(SIGINT, signal_handler);
    pollfd listener {};
    listener.fd = main_sock;
    listener.events = POLLIN;

    while (running){
        int poll_res = poll(&listener, 1, 0);

        if (poll_res == -1 || listener.revents == -1){
            running = false;
            std::perror("Pollin error");
            break;
        }
        else if (listener.revents == POLLIN){
            sockaddr_storage client_address {};
            socklen_t client_address_len = sizeof(client_address);
            Command command {};
            command.socket = accept(main_sock, reinterpret_cast<sockaddr *>(&client_address), &client_address_len);

            if (command.socket == -1){
                std::perror("Failed to accept connection with client");
                continue;
            }

            char buffer[1024] {};

            if (recv(command.socket, buffer, sizeof(buffer), 0) == -1){
                std::perror("Failed to receive data from client");
                continue;
            }

            bool is_valid = parse_and_validate_query(buffer, &command);

            if (!is_valid){
                continue;
            }

            tasks.push_back(command);
        }
    }


    for (auto& worker : workers){
        worker.join();
    }

    for (auto& t : tasks){
        close(t.socket);
    }

    close(main_sock);
    freeaddrinfo(address_info);
}


addrinfo *initialize_address_info(int ai_family){
    addrinfo base, *res {};

    base.ai_family = ai_family;
    base.ai_socktype = SOCK_STREAM;
    base.ai_flags = AI_PASSIVE;
    base.ai_protocol = 0;

    getaddrinfo(NULL, "5050", &base, &res);

    return res;
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

    query = trim(query.substr(opcode_pos + 1, query.length() - (opcode_pos + 1)));

    if (query.length() == 0){
        std::cout << "Key not provided" << "\n";
        return false;
    }

    auto key_pos = query.find_first_of(" ");

    if (key_pos == std::string::npos){
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


void worker_handler(){

    while (running){
        std::this_thread::sleep_for(std::chrono::seconds(2));

        mtx.lock();
        if (!tasks.empty()){
            const Command task = tasks.back();
            tasks.erase(--tasks.end());
            
            mtx.unlock();

            std::cout << "Thread " << std::this_thread::get_id() << " executing the command: ";

            std::string result {};

            switch (task.opcode)
            {
                case Opcodes::SET:
                    result = kv_set(task.key, task.value);
                    break;
                
                case Opcodes::GET:
                    result = kv_get(task.key);
                    break;

                case Opcodes::DELETE:
                    result = kv_delete(task.key);
                    break;
            }

            auto bytes_sent = send(task.socket, result.data(), result.size(), 0);

            if (bytes_sent == -1){
                std::perror("Failed to send result");
            }
            else{
                std::cout << "Sent " << bytes_sent << " bytes" << "\n";
            }

            close(task.socket);
        }   
        else{
            mtx.unlock();
        }
    }
}


std::string kv_set(std::string key, std::string value){
    auto [it, inserted] = kv_store.insert_or_assign(key, value);
    
    if (inserted){
        return "Success";
    }
    else{
        return "Failed";
    }
}

std::string kv_get(std::string key){
    auto it = kv_store.find(key);

    if (it != kv_store.end()){
        std::cout << "Retrieved value: " << it->second << "\n";
        return it->second;
    }
    else{
        return "Failed to get";
    }
}

std::string kv_delete(std::string key){
    auto it = kv_store.find(key);

    if (it != kv_store.end()){
        kv_store.erase(it);
        return "Success";
    }
    else{
        return "Failed to delete";
    }
}

