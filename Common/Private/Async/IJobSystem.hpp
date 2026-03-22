#pragma once

#include <functional>

namespace cp
{
    /**
    * @brief Function type for jobs submitted to the JobSystem.
    */
    using JobFunction = std::function<void(void)>;

    /**
    * @brief Abstract interface for job system implementations.
    * 
    * This interface allows for dependency injection and mocking in tests.
    * The primary implementation is the singleton JobSystem class.
    */
    class IJobSystem
    {
    public:
        virtual ~IJobSystem() = default;

        /**
        * @brief Enqueues a job for asynchronous execution.
        * 
        * @param _job The function to execute asynchronously.
        */
        virtual void Enqueue(const JobFunction& _job) = 0;

        /**
        * @brief Checks if there are jobs still being processed.
        * 
        * @return true if jobs are pending or executing, false otherwise.
        */
        [[nodiscard]] virtual bool IsBusy() const = 0;

        /**
        * @brief Blocks until all enqueued jobs are completed.
        */
        virtual void Wait() = 0;
    };
}
