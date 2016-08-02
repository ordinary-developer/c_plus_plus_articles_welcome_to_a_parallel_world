#include <thread>
#include <iostream>
#include <string>

int main() {
    auto func = [](const std::string& first, const std::string& second)
    {
        std::cout << first << second << std::endl;
    };
    std::thread thread(func, "Hello, ", "threads!");
    thread.join();

    return 0;
}
