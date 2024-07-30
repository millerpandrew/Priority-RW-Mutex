#ifndef PRIORITY_RW_MUTEX_H
#define PRIORITY_RW_MUTEX_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>

// this class uses a boost::shared_mutex along with a few different CVs to function as a write prioritizing RW mutex which allows for a priority write lock which can "cut" the low priority write lock
// both types of write locks supersede the read lock in priority 
class priority_rw_mutex {
public:

    priority_rw_mutex() = default;
    priority_rw_mutex(const priority_rw_mutex&) = delete;
    priority_rw_mutex& operator=(const priority_rw_mutex&) = delete;

    void priority_write_lock() {
        std::unique_lock<std::mutex> lock(priority_mutex);
        pending_priority_writes++;
        priority_cv.wait(lock, [&]() { return !is_writing && (num_reading == 0); });
        is_writing = true;
        pending_priority_writes--;
    }

    void priority_write_unlock() {
        std::lock_guard<std::mutex> lock(priority_mutex);
        is_writing = false;
        notify_next();
    }

    void low_priority_write_lock() {
        std::unique_lock<std::mutex> lock(priority_mutex);
        pending_low_priority_writes++;
        low_priority_cv.wait(lock, [&]() { return !is_writing && (num_reading == 0) && pending_priority_writes == 0; });
        is_writing = true;
        pending_low_priority_writes--;
    }

    void low_priority_write_unlock() {
        std::lock_guard<std::mutex> lock(priority_mutex);
        is_writing = false;
        notify_next();
    }

    void read_lock() {
        std::unique_lock<std::mutex> lock(priority_mutex);
        read_cv.wait(lock, [&]() { return !is_writing && pending_priority_writes == 0 && pending_low_priority_writes == 0; });
        num_reading++;
    }

    void read_unlock() {
        std::lock_guard<std::mutex> lock(priority_mutex);
        if (--num_reading == 0) {
            notify_next();
        }
    }

private:
    void notify_next() {
        if (pending_priority_writes) {
            priority_cv.notify_one();
        } else if (pending_low_priority_writes) {
            low_priority_cv.notify_one();
        } else {
            read_cv.notify_all();
        }
    }

    std::mutex priority_mutex;
    std::condition_variable priority_cv;
    std::condition_variable low_priority_cv;
    std::condition_variable read_cv;
    size_t pending_priority_writes = 0;
    size_t pending_low_priority_writes = 0;
    size_t num_reading = 0;
    bool is_writing = false;
};

#endif // PRIORITY_RW_MUTEX_H