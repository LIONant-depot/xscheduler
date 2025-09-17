# xscheduler: Advanced Coroutine-Enabled Job Scheduler in C++

`xscheduler` is a high-performance C++ library for managing concurrent jobs with coroutine support, designed for efficient task scheduling in multithreaded environments. It enables dependency graphs, priorities, and affinities, making it ideal for game engines, simulations, or any application needing lightweight parallelism.

## Key Features

* **Lockless Efficiency**: Uses lock-free queues and pools for low-contention threading. 
* **Job Properties**: Customizable affinity, priority, and complexity for optimized workload distribution. 
* **Coroutine Integration**: Seamless `co_await` and `co_yield` for asynchronous workflows without blocking. 
* **Dependency Triggers**: Build graphs with `trigger` for synchronized job completion. 
* **task_group for Batching**: Group tasks with `join()` for easy synchronization and foreach parallel processing. 
* **Modern C++20 Design**: Header-only, type-safe, and fast—perfect for performance-critical code. 
* **MIT License**: Free and open-source—use and modify as needed! 

## Dependencies

- [xcontainer](https://github.com/LIONant-depot/xcontainer): Required for lockless queues, pools, and functions.

## Code Example

Below is an example demonstrating basic job submission and async dependencies with a trigger.

```cpp
#include "xscheduler.h"

int main() {
    xscheduler::system Scheduler{4}; // 4 workers

    std::atomic<int> Count = 0;
    xscheduler::trigger<2> Trigger;

    // Define jobs
    struct MyJob : xscheduler::job<0> {
        std::atomic<int>& m_Count;
        MyJob(std::atomic<int>& Count) : m_Count(Count) {}
        void OnRun() noexcept override { ++m_Count; printf("Job done: %d\n", m_Count.load()); }
    };

    MyJob Job1(Count), Job2(Count);
    Trigger.JobWillNotifyMe(Job1);
    Trigger.JobWillNotifyMe(Job2);

    Scheduler.SubmitJob(Job1);
    Scheduler.SubmitJob(Job2);

    // Async job awaiting trigger
    Scheduler.SubmitLambda([&](xscheduler::job_base& This) -> xscheduler::async_handle {
        co_await Trigger; // Wait for dependencies
        printf("Async job complete!\n");
        ++Count;
        co_return;
    });

    Channel.join(); // Wait for all

    assert(Count.load() == 3);
    return 0;
}
```

## Usage

1. **Define Job Types**: Inherit from `job<N>` or `async_job<N>` for dependencies (N = max dependents).
2. **Implement OnRun**: For regular jobs, define work; for async, return `async_handle` with coroutines.
3. **Submit Jobs**: Use `SubmitJob` or channel's `Submit` for batched tasks.
4. **Handle Dependencies**: Use `trigger` for graphs: `JobWillNotifyMe` for waits, `AppendJobToBeTrigger` for notifications.
5. **Synchronize**: Call `channel.join()` or poll `isDone()` for completion.
6. **Parallel Foreach**: Use task_group's `ForeachLog` or `ForeachFlat` for container processing.

## Installation

1. Include `xscheduler.h` in your project.
2. Ensure C++20 support and link against the Standard Library.
3. Compile with dependencies from `xcontainer`.

## Contributing

Star, fork, and contribute to `xscheduler` on GitHub! 🚀 Submit issues or PRs to improve features or docs.
