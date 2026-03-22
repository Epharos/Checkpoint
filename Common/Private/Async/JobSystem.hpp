#pragma once

#include <condition_variable>
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>

#include "IJobSystem.hpp"
#include "../Containers/ThreadSafeStaticRingBuffer.hpp"

namespace cp
{
    class ILogger;

    /**
    * @brief Thread-safe job system using singleton pattern.
    * 
    * The JobSystem provides a global thread pool for executing asynchronous tasks.
    * It uses a fixed-size queue (256 jobs) and creates worker threads based on CPU cores.
    * 
    * All public methods are thread-safe and can be called from multiple threads simultaneously.
    * The singleton instance is lazily initialized in a thread-safe manner (C++11+).
    * 
    * Limitations:
    * - Fixed queue size: Maximum 256 jobs can be enqueued at once
    * - FIFO execution: No job priorities supported
    * - No cancellation: Jobs cannot be canceled once enqueued
    * - Active polling: Wait() uses busy-waiting which is CPU-intensive
    * - Detached threads: Worker threads cannot be joined
    *
    * TODO:
    * - Dependencies between jobs (DAG)
    * - Work stealing (per worker thread queue)
    * - Naming worker thread and ability to assign jobs to specific threads
    * - Profiling jobs
    * - Future and promises
    * - Priority queues
    * - Job cancellation
    */
    class JobSystem : public IJobSystem
    {
    public:
        /**
        * @brief Gets the singleton instance of the JobSystem.
        * 
        * Thread-safe lazy initialization.
        * 
        * @return Reference to the global JobSystem instance.
        */
        static JobSystem& GetInstance();

        JobSystem(const JobSystem&) = delete;
        JobSystem& operator=(const JobSystem&) = delete;
        JobSystem(JobSystem&&) = delete;
        JobSystem& operator=(JobSystem&&) = delete;

        /**
        * @brief Enqueues a job for asynchronous execution.
        * 
        * Blocks if queue is full (256 jobs) until space is available.
        * Thread-safe: Can be called from multiple threads.
        * 
        * @param _job Function to execute asynchronously.
        */
        void Enqueue(const JobFunction& _job) override;

        /**
        * @brief Checks if there are jobs still being processed.
        * 
        * Thread-safe: Can be called from multiple threads.
        * 
        * @return true if jobs are pending or executing, false otherwise.
        */
        [[nodiscard]] bool IsBusy() const override;

        /**
        * @brief Blocks until all enqueued jobs are completed.
        *
        * Thread-safe: Can be called from multiple threads.
        */
        void Wait() override;

        /**
        * @brief Gets the number of worker threads.
        * 
        * @return The number of worker threads.
        */
        [[nodiscard]] uint32_t GetWorkerCount() const { return workerThreadCount; }

        /**
        * @brief Initializes the JobSystem instance (optional).
        * 
        * Calling this is optional; GetInstance() will auto-initialize.
        * 
        * @param _workerThreadCount Number of worker threads (default: hardware_concurrency/2).
        */
        static void Initialize(uint32_t _workerThreadCount);

        /**
        * @brief Shuts down the job system.
        * 
        * Should be called before program exit for clean shutdown.
        * After shutdown, the JobSystem cannot be restarted.
        */
        static void Shutdown();

    private:
        explicit JobSystem(uint32_t _workerThreadCount = std::thread::hardware_concurrency() / 2);
        ~JobSystem() override;

    private:
        ThreadSafeStaticRingBuffer<JobFunction, 256> jobPool;
        std::vector<std::thread> workerThreads;

        std::condition_variable workerThreadCondition;
        std::condition_variable completionCondition;
        std::mutex workerThreadMutex;
        std::mutex completionMutex;

        size_t totalJobEnqueued = 0;
        std::atomic<size_t> totalJobDequeued;

        uint32_t workerThreadCount;

        std::atomic<bool> shouldShutdown{false};
    };
}
