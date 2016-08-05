#include <thread>
#include <mutex>
#include <chrono>

int main() {
    auto call = [](std::mutex& first, std::mutex& second) {
        std::lock(first, second);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        first.unlock();
        second.unlock();
    };

    std::mutex first;
    std::mutex second;
    std::thread firstThread(call, std::ref(first), std::ref(second));
    std::thread secondThread(call, std::ref(second), std::ref(first));
    firstThread.join();
    secondThread.join();

    return 0;
}
