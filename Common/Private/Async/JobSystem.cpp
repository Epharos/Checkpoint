#include "JobSystem.hpp"

namespace cp
{
    namespace
    {
        void Pool(std::condition_variable& _conditionVariable)
        {
            _conditionVariable.notify_one();
            std::this_thread::yield();
        }
    }

    JobSystem& JobSystem::GetInstance()
    {
        static JobSystem instance(std::thread::hardware_concurrency() / 2);
        return instance;
    }

    void JobSystem::Initialize(uint32_t _workerThreadCount)
    {
        GetInstance();
    }

    void JobSystem::Shutdown()
    {
        JobSystem& instance = GetInstance();
        instance.shouldShutdown.store(true);
        instance.Wait();
        instance.workerThreadCondition.notify_all();
    }

    JobSystem::JobSystem(uint32_t _workerThreadCount)
        : workerThreadCount(std::max(1u, _workerThreadCount))
    {
        totalJobDequeued.store(0);

        for (uint32_t threadId = 0 ; threadId < workerThreadCount ; ++threadId)
        {
            workerThreads.emplace_back([this] {
                JobFunction currentJob;

                for (;;)
                {
                    if (shouldShutdown.load())
                    {
                        break;
                    }

                    if (jobPool.PopFront(currentJob))
                    {
                        currentJob();
                        totalJobDequeued.fetch_add(1);

                        if (!IsBusy())
                        {
                            completionCondition.notify_all();
                        }

                        continue;
                    }

                    std::unique_lock<std::mutex> lock(workerThreadMutex);
                    workerThreadCondition.wait(lock, [this] {
                        return shouldShutdown.load() || !jobPool.IsEmpty();
                    });
                }
            });
        }
    }

    JobSystem::~JobSystem()
    {
        shouldShutdown.store(true);
        workerThreadCondition.notify_all();

        for (auto& thread : workerThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    void JobSystem::Enqueue(const JobFunction& _job)
    {
        ++totalJobEnqueued;

        while (!jobPool.PushBack(_job)) Pool(workerThreadCondition);

        workerThreadCondition.notify_one();
    }

    bool JobSystem::IsBusy() const
    {
        return totalJobDequeued.load() < totalJobEnqueued;
    }

    void JobSystem::Wait()
    {
        std::unique_lock<std::mutex> lock(workerThreadMutex);
        completionCondition.wait(lock, [this] {
            return !IsBusy();
        });
    }
}
