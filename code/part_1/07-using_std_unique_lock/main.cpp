#include <thread>
#include <mutex>
#include <chrono>
#include <list>
#include <limits>
#include <list>
#include <iostream>
#include <string>

class Logger {
    public:
        void log(const std::string& message) {
            std::lock_guard<std::mutex> lock(_mutex);
            std::cout << message << std::endl;
        }
    private:
        std::mutex _mutex;
};

class WarehouseEmptyException { };

unsigned short SPECIAL_ITEM = std::numeric_limits<unsigned short>::max();

class Warehouse {
    public:
        void acceptItem(unsigned short item) {
            _store.push_back(item);
        }

        unsigned short takeLastItem() {
            if (_store.empty()) 
                throw WarehouseEmptyException();

            unsigned short item = _store.front();
            if (SPECIAL_ITEM != item)
                _store.pop_front();

            return item;
        }
    private:
        std::list<unsigned short> _store;
};

Warehouse g_firstWarehouse;
Warehouse g_secondWarehouse;
std::timed_mutex g_firstMutex;
std::mutex g_secondMutex;

int main() {
    Logger logger;

    auto supplier = [&logger]() {
        for (unsigned short i = 0, j = 0; i < 10 && j < 10;) {
            if (i < 10 && g_firstMutex.try_lock()) {
                logger.log("[SUPPLIER]: {putting data} {" + std::to_string(i) + "}" 
                           + " {1-st warehouse}");

                g_firstWarehouse.acceptItem(i);
                ++i;
                g_firstMutex.unlock();
            }
            if (j < 10 && g_secondMutex.try_lock()) {
                logger.log("[SUPPLIER]: {putting data} {" + std::to_string(j) + "}" 
                           + " {2-nd warehouse}");

                g_secondWarehouse.acceptItem(j);
                ++j;
                g_secondMutex.unlock();
            }

            std::this_thread::yield();
        }

        std::lock_guard<std::timed_mutex> firstGuard(g_firstMutex);
        std::lock_guard<std::mutex> secondGuard(g_secondMutex);
        logger.log("[SUPPLIER]: {putting data} {" + std::to_string(SPECIAL_ITEM) + "}" 
                   + " {1-st warehouse}");
        logger.log("[SUPPLIER]: {putting data} {" + std::to_string(SPECIAL_ITEM) + "}" 
                   + " {2-nd warehouse}");
        g_firstWarehouse.acceptItem(SPECIAL_ITEM);
        g_secondWarehouse.acceptItem(SPECIAL_ITEM);
    };

    auto consumer = [&logger]() {
        while (true) {
            unsigned short item = 0;

            try {
                std::lock_guard<std::timed_mutex> guard(g_firstMutex);
                item = g_firstWarehouse.takeLastItem();
                logger.log("[CONSUMER]: {taking data } {" + std::to_string(item) + "}"
                           + " {1-st warehouse}");
            }
            catch (WarehouseEmptyException&) {
                logger.log("[CONSUMER]: {1-st warehouse} {empty}");
            }

            if (SPECIAL_ITEM == item) 
                break;
        };
    };

    auto impatientConsumer = [&logger]() {
        while (true) {
            unsigned short item = 0;
            std::unique_lock<std::timed_mutex> firstGuard(
                                                g_firstMutex,
                                                std::defer_lock);

            if (firstGuard.try_lock_for(std::chrono::seconds(2))) {
                try {
                    std::lock_guard<std::timed_mutex> guard(
                                                       g_firstMutex, 
                                                       std::adopt_lock);
                    item = g_firstWarehouse.takeLastItem();
                    logger.log("[IMPATIENT CONSUMER]: {taking data } {" 
                               + std::to_string(item) + "}"
                               + " {1-st warehouse}");
                }
                catch (WarehouseEmptyException&) {
                    logger.log("[IMPATIENT CONSUMER]: {1-st warehouse} {empty}");
                }

                firstGuard.unlock();
            }
            else {
                logger.log("[IMPATIENT CONSUMER]: {1-st warehouse}"
                           + std::string("{busy}"));

                try {
                    std::unique_lock<std::mutex> guard(g_secondMutex);
                    logger.log("[IMPATIENT CONSUMER]: {taking data } {" 
                               + std::to_string(item) + "}"
                               + " {2-nd warehouse}");

                }
                catch (WarehouseEmptyException&) {
                    logger.log("[IMPATIENT CONSUMER]: {2-nd warehouse} {empty}");
                }
            }

            if (SPECIAL_ITEM == item) 
                break;
        }

    };

    std::thread supplierThread(supplier);
    std::thread consumerThread(consumer);
    std::thread impatientConsumerThread(impatientConsumer);
    supplierThread.join();
    consumerThread.join();
    impatientConsumerThread.join();

    return 0;
}
