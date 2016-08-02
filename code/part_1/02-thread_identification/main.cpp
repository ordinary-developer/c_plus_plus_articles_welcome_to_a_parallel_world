#include <thread>
#include <iostream>


int main() {
    auto func = []() 
    {
        std::cout << "thread id: " << std::hex << std::this_thread::get_id()
                  << std::endl;
    };
    std::thread thread(func);
    thread.join();

    return 0;
}
