#include <atomic>


class CustomMutex {
    public:
        void lock() {
            while (m_Locked.exchange(true));
        }

        void unlock() {
            m_Locked.store(false);
        }

    private:
        std::atomic<bool> m_Locked{ false };
};


int main() {
    return 0;
}
