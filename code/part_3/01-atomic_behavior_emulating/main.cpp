#include <mutex>
#include <future>
#include <vector>
#include <cstdint>
#include <iostream>


int main() {
    uint32_t shared = 0;
    std::mutex mutex;

    std::vector<std::future<void>> futures;
    uint32_t taskCount{ 10 };
    for (uint32_t i = 0; i < taskCount; ++i) {
        futures.emplace_back(
                std::async(std::launch::deferred,
                          [&shared, &mutex]() -> void 
                            {
                              mutex.lock();
                              shared++;
                              mutex.unlock();
                            }
                         )
        );
    }

    for (auto& future : futures)
        future.get();

    std::cout << "[result] => " << shared << std::endl;

    return 0;
}
