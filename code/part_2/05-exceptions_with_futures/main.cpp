#include <iostream>
#include <future>
#include <exception>

int main() {
    auto first = std::async([]() -> void {
        throw std::bad_alloc();
    });
    auto second = std::async([]() -> void {
        throw std::bad_alloc();
    });

    try {
        first.get();
    }
    catch (std::exception& ) {
        std::cout << "catch exception from the first" << std::endl;
    }

    second.wait();
    std::cout << "second has been ended" << std::endl;

    return 0;
}
