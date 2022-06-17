
#include "thread-pool.h"
#include<assert.h>
namespace aimy {

ThreadPool::ThreadPool(Object *_parent):parent(_parent)
{
#ifdef DEBUG
    assert(_parent!=nullptr);
#endif
}

ThreadPool::~ThreadPool() { join(); }

int ThreadPool::count() const {
    std::unique_lock<std::mutex> lock(mWorkersMutex);
    return int(mWorkers.size());
}

void ThreadPool::spawn(int count) {
    std::unique_lock<std::mutex> lock(mWorkersMutex);
    mJoining = false;
    while (count-- > 0)
        mWorkers.emplace_back(std::bind(&ThreadPool::run, this));
}

void ThreadPool::join() {
    std::unique_lock<std::mutex> lock(mWorkersMutex);
    mJoining = true;
    mCondition.notify_all();

    for (auto &w : mWorkers)
        w.join();

    mWorkers.clear();
}

void ThreadPool::run() {
    while (runOne()) {
    }
}

bool ThreadPool::runOne() {
    if (auto task = dequeue()) {
        task();
        return true;
    }
    return false;
}

std::function<void()> ThreadPool::dequeue() {
    std::unique_lock<std::mutex> lock(mTasksMutex);
    while (true) {
        if (!mTasks.empty()) {
            if (mTasks.top().time <= clock_t::now()) {
                auto func = std::move(mTasks.top().func);
                mTasks.pop();
                return func;
            }

            if (mJoining)
                break;

            mCondition.wait_until(lock, mTasks.top().time);
        } else {
            if (mJoining)
                break;

            mCondition.wait(lock);
        }
    }
    return nullptr;
}
} // namespace micagent
