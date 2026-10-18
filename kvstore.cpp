#include <unordered_map>
#include <string>
#include <iostream>
#include <instruction.h>


class KVStore{
    private:
        struct Node{
            std::string value {};
            Node *next {};
            Node *prev {};
        };

        const int capacity { 1 };
        std::unordered_map<std::string, Node> store {};
        Node *head { nullptr };
        Node *tail { nullptr };

    public:
        explicit KVStore(int max_capacity) : capacity {max_capacity} {}

        int set(std::string key, std::string value){
            auto it = store.find(key);
            
            if (it != store.end()){
                Node * node = &(it->second);
                lru_remove(node);
                lru_add(node);

                it->second.value = std::move(value);
            }
            else{
                Node node;
                node.value = std::move(value);
                lru_add(&node);
                
                store[key] = node;               
            }

            return 0;
        }

        int get(std::string key, std::string *value){
            auto it = store.find(key);
            
            if (it != store.end()){
                Node * node = &(it->second);
                lru_remove(node);
                lru_add(node);

                value = &(it->second.value);

                return 0;
            }
            else{
                return 1;
            }
        }

        int remove(std::string key){
            auto it = store.find(key);

            if (it != store.end()){
                lru_remove(&(it->second));
                store.erase(it);
                
                return 0;
            }
            else{
                return 1;
            }
        }

    private:
        void lru_remove(Node* node){
            Node * prev_node = node->prev;
            Node * next_node = node->next;
            prev_node->next = next_node;
            next_node->prev = prev_node;
        }

        void lru_add(Node* node){
            head->prev = node;
            head = node; 
        }
};

