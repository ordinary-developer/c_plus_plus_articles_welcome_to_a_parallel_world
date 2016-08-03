#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <list>
#include <limits>

class WarehouseEmpty { };

unsigned short c_SpecialItem = 
    std::numeric_limits<unsigned short>::max();

class Warehouse {
    public:
        void AcceptItem(unsigned short item) {
            _store.push_back(item);
        }

        unsigned short HandLastItem() {
            if (_store.empty()) 
                throw WarehouseEmpty();
            unsigned short item = _store.front();
            if (c_SpecialItem != item) 
                _store.pop_front();
            return item;
        }
    private:
        std::list<unsigned short> _store;
};

Warehouse g_FirstWarehouse;
Warehouse g_SecondWarehouse;
std::timed_mutex g_FirstMutex;
std::mutex g_SecondMutex;

int main() {
    auto supplier = []() {
        for (unsigned short i = 0, j = 0; i < 10 && j < 10;) {
            if (i < 10 && g_FirstMutex.try_lock()) {
                g_FirstWarehouse.AcceptItem(i);
                i++;
                g_FirstMutex.unlock();
            }
            if (j < 10 && g_SecondMutex.try_lock()) {
                g_SecondWarehouse.AcceptItem(j);
                j++;
                g_SecondMutex.unlock();
            }
            std::this_thread::yield();
        }
        g_FirstMutex.lock();
        g_SecondMutex.lock();
        g_FirstWarehouse.AcceptItem(c_SpecialItem);
        g_SecondWarehouse.AcceptItem(c_SpecialItem);
        g_FirstMutex.unlock();
        g_SecondMutex.unlock();
    };

    auto consumer = []() {
        while (true) {
            g_FirstMutex.lock();
            unsigned short item = 0;
            try {
                item = g_FirstWarehouse.HandLastItem();
            }
            catch (WarehouseEmpty&) {
                std::cout << "Warehouse is empty!\n";
            }
            g_FirstMutex.unlock();
            if (c_SpecialItem == item) 
                break;
            std::cout << "Got new item: " << item << "!\n";
            std::this_thread::sleep_for(std::chrono::seconds(4));
        }
    };

    auto impatientConsumer = []() {
        while (true) {
            unsigned short item = 0;
            if (g_FirstMutex.try_lock_for(std::chrono::seconds(2))) {
                try {
                    item = g_FirstWarehouse.HandLastItem();
                }
                catch (WarehouseEmpty&) {
                    std::cout << "The first warehouse is empty!\n";
                }
                g_FirstMutex.unlock();
            }
            else {
                std::cout << "First warehouse is alwasy busy!\n";
                g_SecondMutex.lock();
                try {
                    item = g_SecondWarehouse.HandLastItem();
                }
                catch (WarehouseEmpty&) {
                    std::cout << "The 2nd warehouseis empty!!!\n";
                }
                g_SecondMutex.unlock();
            }

            if (c_SpecialItem == item) 
                break;

            std::cout << "At last i got new item: " << item << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(4));
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
