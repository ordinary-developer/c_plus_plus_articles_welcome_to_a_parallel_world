#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <string>
#include <memory>

std::condition_variable g_Bell;
std::condition_variable g_Door;

class ILogger {
    public:
        virtual void log(const std::string& message) = 0;
};

class Logger : public ILogger {
    public:
        Logger(std::mutex& mutex) : _mutex(mutex) { }

        virtual void log(const std::string& message) final override {
            std::lock_guard<std::mutex> lock(_mutex);
            std::cout << message << std::endl;
        }

    private:
        std::mutex& _mutex; 
};

class Manager {
    public:
        explicit Manager(std::shared_ptr<ILogger> logger) 
            : _logger(logger) { }

        void comeToWork() {
            _logger->log("[MANAGER]: {hey security, open the door!}");
            g_Bell.notify_one();

            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);
            g_Door.wait(lock);
        }

    private:
        std::shared_ptr<ILogger> _logger;
};

class Security {
    public:
        explicit Security(std::shared_ptr<ILogger> logger)
            : _logger(logger) { }

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
                    _logger->log("[SECURITY]: hello a great manager");
                }
            }
        }
    private:
        std::shared_ptr<ILogger> _logger;

        static bool _sectorClear;
        static std::mutex _sectorMutex;
};
bool Security::_sectorClear;
std::mutex Security::_sectorMutex; 

class Programmer {
    public:
        explicit Programmer(std::shared_ptr<ILogger> logger) 
            : _logger(logger) { }

        void workHard() {
            _logger->log("[PROGRAMMER]: let's write some code");
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

    private:
        std::shared_ptr<ILogger> _logger;
};

int main() {
    std::mutex mutex;
    std::shared_ptr<Logger> logger = std::make_shared<Logger>(mutex);

    Manager manager(logger);
    Programmer programmer(logger);
    Security security(logger);

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
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::thread managerThread(managerCall);

    managerThread.join();
    programmerThread.join();
    securityThread.join();

    return 0;
}
