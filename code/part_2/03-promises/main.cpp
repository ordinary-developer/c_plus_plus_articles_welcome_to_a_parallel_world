#include <future>
#include <thread>
#include <limits>
#include <memory>

int main() {
    auto spPromise = std::make_shared<std::promise<void>>();
    std::future<void> waiter = spPromise->get_future();
    auto call = [spPromise](size_t value) {
        size_t i = std::numeric_limits<size_t>::max();
        while (i--) 
            if (value == i) 
                spPromise->set_value();
    };
    std::thread thread(call, std::numeric_limits<size_t>::max() - 500);
    thread.detach();
    waiter.get();

    return 0;
}
