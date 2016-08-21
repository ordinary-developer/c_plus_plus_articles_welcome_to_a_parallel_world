#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Main thread id = " << std::this_thread::get_id()
              << std::endl;

    auto asyncDefault = std::async([]() -> void {
        std::cout << "Async default, Thread id = "
                  << std::this_thread::get_id() << std::endl;
    });

    auto asyncDeffered = 
        std::async(std::launch::deferred,
                   [](const std::string& str) -> void {
                       std::cout << "Async deffer, Thread id = "
                                 << std::this_thread::get_id() 
                                 << ", " << str << std::endl;
                   }, 
                   std::string("end string"));

    auto asyncDeffered2 = 
        std::async(std::launch::deferred, 
                   []() -> void
                   {
                      std::cout << "Async deffer2, Thread id = "
                                << std::this_thread::get_id()
                                << std::endl;
                   });

    auto trueAsync = 
        std::async(std::launch::async, 
                   []() -> void 
                   {
                       std::cout << "True async, Thread id = "
                                 << std::this_thread::get_id() 
                                 << std::endl;
                   });

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Sleep ended" << std::endl;
    asyncDefault.get();
    asyncDeffered.get();
    trueAsync.get();

    return 0;
}
