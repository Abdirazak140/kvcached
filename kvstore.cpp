#include <unordered_map>
#include <string>
#include <iostream>

struct Node{
    std::string value {};
    void *next {};
};

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

class KVStore{
    private:
        int max_capacity;
        std::unordered_map<std::string, Node> store {};

        
    public:
        KVStore(int max_capacity){
            max_capacity = max_capacity;
        }

        int set(std::string key, std::string value){
            auto it = store.find(key);
            
            if (it != store.end()){
                it->second.value = std::move(value);
                return 0;
            }
            else{
                Node node;
                node.value = std::move(value);
                
                auto [it, inserted] = store.insert_or_assign(std::move(key) , std::move(node));
                            
                if (inserted){
                    return 0;
                }
                else{
                    return 1;
                }
            }
        }

        int get(std::string key, std::string_view value){
            auto it = store.find(key);
            
            if (it != store.end()){
                value = it->second.value;
                return 0;
            }
            else{
                return 1;
            }
        }

        int remove(std::string key){
            auto it = store.find(key);

            if (it != store.end()){
                store.erase(it);
                return 0;
            }
            else{
                return 1;
            }
        }

};