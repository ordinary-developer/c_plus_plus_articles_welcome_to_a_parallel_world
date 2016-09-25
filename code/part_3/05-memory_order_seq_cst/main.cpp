#include <future>
#include <atomic>
#include <vector>
#include <cassert>


int main() {
    size_t nonAtomic = 0;
    std::atomic<bool> flagA{ false };
    std::atomic<bool> flagB{ false };

    std::vector<std::future<void>> futures;
    futures.emplace_back(
        std::async(std::launch::deferred,
                   [&nonAtomic, &flagA]() -> void 
                     {
                       nonAtomic++;
                       flagA.store(true);
                     }
                  )
    );

    futures.emplace_back(
        std::async(std::launch::deferred,
                   [&nonAtomic, &flagB]() -> void 
                     {
                       nonAtomic++;
                       flagB.store(true);
                     }
                  )
    );

    futures.emplace_back(
        std::async(std::launch::deferred,
                   [&nonAtomic, &flagA, &flagB]() -> void 
                     {
                       while (!flagA.load())
                         ;
                       if (flagB.load())
                         assert(2 == nonAtomic);
                       else 
                         assert(0 != nonAtomic);
                     }
                  )
    );

    futures.emplace_back(
        std::async(std::launch::deferred,
                   [&nonAtomic, &flagA, &flagB]() -> void 
                     {
                       while (!flagB.load())
                         ;
                       if (flagA.load())
                         assert(2 == nonAtomic);
                       else 
                         assert(0 != nonAtomic);
                     }
                  )
    );


    for (auto& future: futures)
        future.get();

    return 0;
}

