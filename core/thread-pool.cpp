
#include "thread-pool.h"
#include<assert.h>
#if defined(__linux__)
#include <sys/prctl.h>
#endif
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

uint32_t ThreadPool::taskCount() const
{
    std::lock_guard<std::mutex> lock(mTasksMutex);
    return mTasks.size();
}

void ThreadPool::spawn(int count) {
    std::unique_lock<std::mutex> lock(mWorkersMutex);
    mJoining.exchange(false);
    while (count-- > 0){
        mWorkers.emplace_back(std::bind(&ThreadPool::run, this,mWorkers.size()));
    }
}

void ThreadPool::join() {
    if(mJoining)return;
    mJoining.exchange(true);
    {
        std::lock_guard<std::mutex> lock(mTasksMutex);
        mCondition.notify_all();
    }
    std::unique_lock<std::mutex> locker(mWorkersMutex);
    for (auto &w : mWorkers)
        w.join();

    mWorkers.clear();
}

void ThreadPool::run(uint32_t index) {
#if defined(__linux__)
    prctl(PR_SET_NAME,(std::string("threadpool_")+std::to_string(index)).c_str(), 0, 0, 0);
#endif
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
