#include <iostream>
#include <future>
#include <exception>
#include <thread>
#include <memory>

int main() {
    auto spPromise = std::make_shared<std::promise<void>>();
    auto waiter = spPromise->get_future();
    auto call = [spPromise]() -> void {
        spPromise->set_exception(std::make_exception_ptr
                                            (std::bad_alloc()));
    };

    std::thread thread(call);
    try {
        waiter.get();
    } 
    catch (std::exception&) {
        std::cout << "catch exception" << std::endl;
    }
    thread.join();

    return 0;
}
