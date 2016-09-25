#include <atomic>


int main() {
    std::atomic<int> integer{ 0 };
    std::atomic<int> otherInteger{ 0 };

    // atomic
    integer++; 

    // non atomic
    otherInteger += integer++;

    return 0;
}
