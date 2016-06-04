#ifndef TTCHAT_SEMAHPOR_H
#define TTCHAT_SEMAHPOR_H

#include <mutex>
#include <condition_variable>

class MySemaphore {
public:
    MySemaphore (int count_ = 0)
            : count(count_) {}

    inline void notify()
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }

    inline void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);

        while(count == 0){
            cv.wait(lock);
        }
        count--;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};


#endif //TTCHAT_SEMAHPOR_H
