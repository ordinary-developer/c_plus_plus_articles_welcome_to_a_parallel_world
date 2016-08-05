#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <string>

std::condition_variable g_Bell;
std::condition_variable_any g_Door;

class Manager {
    public:
        void comeToWork() {
            std::cout << "Hey security, please open the door!\n";
            g_Bell.notify_one();

            std::mutex mutex;
            mutex.lock();
            g_Door.wait(mutex);
            mutex.unlock();
        }
};

class Security {
    public:
        static bool sectorClear() {
            std::lock_guard<std::mutex> lock(_sectorMutex);
            return _sectorClear;
        }

        void notifyFellows() {
            std::lock_guard<std::mutex> lock(_sectorMutex);
            _sectorClear = false;
        }

        void workHard() {
            _sectorClear = true;
            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);

            while (true) {
                if (std::cv_status::timeout ==
                       g_Bell.wait_for(lock, std::chrono::seconds(5)))
                {
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                }
                else {
                    notifyFellows();
                    g_Door.notify_one();
                    std::cout << "Hello great manager" << std::endl;
                }
            }
        }
    private:
        static bool _sectorClear;
        static std::mutex _sectorMutex;
};
bool Security::_sectorClear;
std::mutex Security::_sectorMutex; 

class Programmer {
    public:
        void workHard() {
            std::cout << "Let's write some code\n";
            int i = 0;
            while (true) {
                i++;
                i--;
            }
        }

        void playStarcraft() {
            while(Security::sectorClear());
            workHard();
        }
};

int main() {
    Manager manager;
    Programmer programmer;
    Security security;

    auto managerCall = [&manager]() {
        manager.comeToWork();
    };
    auto programmerCall = [&programmer]() {
        programmer.playStarcraft();
    };
    auto securityCall = [&security]() {
        security.workHard();
    };
    
    std::thread securityThread(securityCall);
    std::thread programmerThread(programmerCall);
    std::this_thread::sleep_for(std::chrono::minutes(1));
    std::thread managerThread(managerCall);

    managerThread.join();
    programmerThread.join();
    securityThread.join();

    return 0;
}
