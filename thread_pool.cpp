#include <thread>
#include <vector>


class ThreadPool{
private:
    std::vector<std::thread> workers {};

public:
    ThreadPool(void *worker_handler, const unsigned int cores = std::thread::hardware_concurrency()){
        
        for (auto i {0}; i < cores; ++i){
            std::thread t (worker_handler);
            workers.push_back(std::move(t));
        }
    }

    ~ThreadPool(){
        for (std::thread& t : workers){
            t.join();
        }
    }
};
