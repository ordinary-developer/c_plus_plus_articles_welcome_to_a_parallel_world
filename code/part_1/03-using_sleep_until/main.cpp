#include <thread>
#include <chrono>
#include <iostream>

int main() {
    auto minutes = std::chrono::system_clock::now() 
        + std::chrono::minutes(1);
    std::this_thread::sleep_until(minutes);
    // std::this_thread::sleep_for(std::chrono::minutes(1));
    
    std::cout << "The sleeping is overing" << std::endl;

    return 0;
}
