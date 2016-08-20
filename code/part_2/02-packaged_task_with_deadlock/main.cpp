#include <iostream>
#include <thread>
#include <future>
#include <string>

std::future<bool> submitForm(const std::string& form) {
    auto handle = [](const std::string& form) -> bool {
        std::cout << form << std::endl;
        return true;
    };

    std::packaged_task<bool(const std::string&)> task(handle);
    task.make_ready_at_thread_exit(form);
    return task.get_future();
}

int main() {
    auto check = submitForm("my form");
    if (check.get()) 
        std::cout << "Yes" << std::endl;
    else
        std::cout << "No" << std::endl;

    return 0;
}
