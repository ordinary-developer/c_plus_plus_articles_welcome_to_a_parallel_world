#include <thread>
#include <mutex>
#include <chrono>
#include <list>
#include <limits>
#include <iostream>

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
std::mutex logMutex;

int main() {
    auto supplier = []() {
        for (unsigned short i = 0, j = 0; i < 10 && j < 10;) {
            if (i < 10 && g_firstMutex.try_lock()) {
                g_firstWarehouse.acceptItem(i);
                logMutex.lock();
                std::cout << "[SUPPLIER]=> {supplying data} {" << i << "} "
                          << "{warehouse} {1-st}"
                          << std::endl;
                logMutex.unlock();
                
                ++i;
                g_firstMutex.unlock();
            }
            if (j < 10 && g_secondMutex.try_lock()) {
                g_secondWarehouse.acceptItem(j);
                logMutex.lock();
                std::cout << "[SUPPLIER]=> {supplying data} {" << j << "} " 
                          << "{warehouse} {2-nd}"
                          << std::endl;
                logMutex.unlock();

                ++j;
                g_secondMutex.unlock();
            }
            std::this_thread::yield();
        }
        g_firstMutex.lock();
        g_secondMutex.lock();
        logMutex.lock();
        g_firstWarehouse.acceptItem(SPECIAL_ITEM);
        g_secondWarehouse.acceptItem(SPECIAL_ITEM);
        std::cout << "[SUPPLIER]=> {supplying data} {" << SPECIAL_ITEM << "} " 
                  << "{warehouse} {1-st}"
                  << std::endl;
        std::cout << "[SUPPLIER]=> {supplying data} {" << SPECIAL_ITEM << "} " 
                  << "{warehouse} {2-nd}"
                  << std::endl;

        g_firstMutex.unlock();
        g_secondMutex.unlock();
        logMutex.unlock();
    };

    auto consumer = []() {
        while (true) {
            g_firstMutex.lock();

            unsigned short item = 0;
            try {
                item = g_firstWarehouse.takeLastItem();

                logMutex.lock();
                std::cout << "[CONSUMER]=> {getting data} {" << item << "}" 
                          << "{warehouse} {1-st}" << std::endl;
                logMutex.unlock();
            }
            catch (WarehouseEmptyException&) {
                logMutex.lock();
                std::cout << "[CONSUMER]=> {exception} {warehouse} {1-st} {empty}"
                          << std::endl;
                logMutex.unlock();
            }

            g_firstMutex.unlock();

            if (SPECIAL_ITEM == item) 
                break;
        }
    };

    auto impatientConsumer = []() {
        while (true) {
            unsigned short item = 0;
            if (g_firstMutex.try_lock_for(std::chrono::seconds(2))) {
                try {
                    item = g_firstWarehouse.takeLastItem();

                    logMutex.lock();
                    std::cout << "[IMPATIENT_CONSUMER]=> {getting data} {" << item << "} "
                              << "{warehouse} {1-st}" << std::endl;
                    logMutex.unlock();
                }
                catch (WarehouseEmptyException&) {
                    logMutex.lock();
                    std::cout << "[IMPATIENT CONSUMER]=> {exception} {warehouse} {1-st} {empty}"
                              << std::endl;
                    logMutex.unlock();
                }

                g_firstMutex.unlock();
            }
            else {
                logMutex.lock();
                std::cout << "[IMPATIENT_CONSUMER]=> {warehouse} {1-st} { busy}"
                          << std::endl;
                logMutex.unlock();
                
                g_secondMutex.lock();
                try {
                    item = g_secondWarehouse.takeLastItem();

                    logMutex.lock();
                    std::cout << "[IMPATIENT_CONSUMER]=> {getting data} {" << item << "} "
                              << "{warehouse} {2-nd}" << std::endl;
                    logMutex.unlock();
                }
                catch (WarehouseEmptyException&) {
                    logMutex.lock();
                    std::cout << "[IMPATIENT_CONSUMER]=> {exception} {warehouse} {2-nd} {empty}"
                              << std::endl;
                    logMutex.unlock();
                }

                g_secondMutex.unlock();
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
