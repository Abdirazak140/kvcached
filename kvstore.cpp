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
        Node *head;
        Node *tail;

    public:
        KVStore(int max_capacity) : capacity {max_capacity} {}

        int set(std::string key, std::string value){
            auto it = store.find(key);
            
            if (it != store.end()){
                Node *temp = &(it->second);
                it->second.prev->next = it->second.next;
                temp->prev = nullptr;
                temp->next = head;
                head = temp;

                it->second.value = std::move(value);
            }
            else{
                Node node;
                node.value = std::move(value);
                node.next = head;
                head = &node;
                
                store[key] = node;               
            }

            return 0;
        }

        int get(std::string key, std::string *value){
            auto it = store.find(key);
            
            if (it != store.end()){
                Node *temp = &(it->second);
                it->second.prev->next = it->second.next;
                temp->prev = nullptr;
                temp->next = head;
                head = temp;

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
                it->second.prev->next = it->second.next;

                store.erase(it);
 
                return 0;
            }
            else{
                return 1;
            }
        }
        
};

