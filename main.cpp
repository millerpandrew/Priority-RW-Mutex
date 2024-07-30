#include <stdio.h>
#include "PriorityRWMutex.h"


void reader(priority_rw_mutex& prio_lock, int id) {
    prio_lock.read_lock();
    printf("Reader %d acquired read lock\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    printf("Reader %d released read lock\n", id);
    prio_lock.read_unlock();
}

void high_priority_writer(priority_rw_mutex& prio_lock, int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    prio_lock.priority_write_lock();
    printf("High-priority writer %d acquired write lock\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    printf("High-priority writer %d released write lock\n", id);
    prio_lock.priority_write_unlock();
}

void low_priority_writer(priority_rw_mutex& prio_lock, int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    prio_lock.low_priority_write_lock();
    printf("Low-priority writer %d acquired write lock\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    printf("Low-priority writer %d released write lock\n", id);
    prio_lock.low_priority_write_unlock();
}


int main() {
    priority_rw_mutex prio_lock;
    std::vector<std::thread> threads;

    

    // Create multiple high-priority writer threads
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(low_priority_writer, std::ref(prio_lock), i);
    }

    // Create multiple low-priority writer threads
    for (int i = 1; i <= 1; ++i) {
        threads.emplace_back(high_priority_writer, std::ref(prio_lock), i);
    }

    // Create multiple reader threads
    for (int i = 1; i <= 5; ++i) {
        threads.emplace_back(reader, std::ref(prio_lock), i);
        if(i == 3){
           std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
        }
    }
    

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}