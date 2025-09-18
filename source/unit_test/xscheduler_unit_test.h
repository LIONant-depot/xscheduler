#include <cassert>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>

#include "source/xscheduler.h"

namespace xscheduler::unit_test
{
    struct j0 : job<0>
    {
        std::atomic<int>& m_A;
        j0(std::atomic<int>& A) : job<0>(xscheduler::str_v<"j0">), m_A(A){}
        void OnRun() noexcept override
        {
            int a = ++m_A;
            printf("J0 Adding %d \n", a);
        }
    };

    struct j1 : job<1>
    {
        std::atomic<int>& m_A;
        j1(std::atomic<int>& A) : job<1>(xscheduler::str_v<"j1">), m_A(A){}
        void OnRun() noexcept override
        {
            int a = ++m_A;
            printf("J1 Adding %d \n", a);
        }
    };

    struct aj0 : async_job<0>
    { std::atomic<int>& m_A;
        aj0(std::atomic<int>& A) : async_job<0>(xscheduler::str_v<"aj0">), m_A(A) {}
        xscheduler::async_handle OnAsyncRun() noexcept override
        {
            int a = ++m_A;
            co_yield *this;
            printf("AJ0 Adding %d \n", a);
            co_return;
        }
    };

    struct aj1 : async_job<1>
    {
        std::atomic<int>& m_A;

        aj1(std::atomic<int>& A) : async_job<1>(xscheduler::str_v<"aj1">), m_A(A) {} xscheduler::async_handle OnAsyncRun() noexcept override
        {
            int a = ++m_A;
            co_yield *this;
            printf("AJ1 Adding %d \n", a);
            co_return;
        }
    };

    // Test basic lambda job submission
    void TestBasicJob(system& System)
    {
        std::atomic<int> BasicJobCount = 0;

        System.SubmitLambda(xscheduler::str_v<"LambdaJob">, [&]()
        {
            printf("TestBasicJob!\n");
            BasicJobCount.fetch_add(1, std::memory_order_release);
        });

        // Wait for job to complete
        while (BasicJobCount.load() != 1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        printf("TestBasicJob - Successful\n");
    }

    // Test async job with co_yield
    void TestAsyncJob(system& System)
    {
        std::atomic<int> BasicJobCount = 0;

        System.SubmitLambda(xscheduler::str_v<"LambdaJob">, [&](xscheduler::job_base& This) -> xscheduler::async_handle
        {
            printf("TestAsyncJob + 1!\n");
            BasicJobCount.fetch_add(1, std::memory_order_relaxed);
            co_yield This; // Suspend

            printf("TestAsyncJob + 2!\n");
            BasicJobCount.fetch_add(1, std::memory_order_relaxed);
            co_yield This; // Suspend again

            printf("TestAsyncJob + 3!\n");
            BasicJobCount.fetch_add(1, std::memory_order_relaxed);
            co_return;
        });

        // Wait for completion
        while (BasicJobCount.load() != 3)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        printf("TestAsyncJob - Successful\n");
    }

    // Test dependency graph with trigger
    void TestDependencyGraph(system& System)
    {
        std::atomic<int> DepJobCount = 0;

        trigger<2> Trigger(xscheduler::str_v<"TestDependencyGraph::Trigger">);
        j0 Dep1(DepJobCount), Dep2(DepJobCount);
        j1 RootJob(DepJobCount);

        Trigger.JobWillNotifyMe(RootJob);
        Trigger.AppendJobToBeTrigger(Dep1);
        Trigger.AppendJobToBeTrigger(Dep2);

        System.SubmitJob(RootJob);

        // Wait for completion
        while (DepJobCount.load() != 3)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        printf("TestDependencyGraph - Successful\n");
    }

    // Test dependency graph with trigger and coroutines 
    void TestDependencyGraphASync(system& System)
    {
        std::atomic<int> DepJobCount = 0;

        trigger<2> Trigger(xscheduler::str_v<"TestDependencyGraphASync::Trigger">);
        aj0 Dep1(DepJobCount), Dep2(DepJobCount);
        aj1 RootJob(DepJobCount);

        Trigger.JobWillNotifyMe(RootJob);
        Trigger.AppendJobToBeTrigger(Dep1);
        Trigger.AppendJobToBeTrigger(Dep2);

        System.SubmitJob(RootJob);

        // Wait for completion
        while (DepJobCount.load() != 3)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        printf("TestDependencyGraphASync - Successful\n");
    }

    // Test async job with co_await on trigger
    void TestAsyncWithDependencies(system& System)
    {
        std::atomic<int> DepJobCount = 0;

        trigger<1> Trigger(xscheduler::str_v<"TestAsyncWithDependencies::Trigger">);
        j1 Dep1(DepJobCount);
        j1 Dep2(DepJobCount);
        Dep1.setupDefinition(job_definition::make<complexity::LIGHT, priority::NORMAL, affinity::ANY>());

        Trigger.JobWillNotifyMe(Dep1);
        Trigger.JobWillNotifyMe(Dep2);

        System.SubmitLambda(xscheduler::str_v<"LambdaJob">, [&]{ printf("Lambda Add\n"); DepJobCount.fetch_add(1, std::memory_order_relaxed); });

        System.SubmitLambda(xscheduler::str_v<"LambdaJobAsync">, [&](job_base& This) -> async_handle
        {
            co_await Trigger; // Wait for Dep1
            printf("ASynJob Add\n");
            DepJobCount.fetch_add(1, std::memory_order_relaxed);
            co_yield This; // Suspend until when?
            printf("ASynJob Add\n");
            DepJobCount.fetch_add(1, std::memory_order_relaxed);
            co_return;
        });

        System.SubmitJob(Dep1);
        System.SubmitJob(Dep2);

        // Wait for completion
        while (DepJobCount.load() != 5)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        printf("TestAsyncWithDependencies - Successful\n");
    }

    // Test the channel
    void TestBasicChannel(system& System)
    {
        // Submit a job to ensure system is active
        task_group Channel{ xscheduler::str_v<"TestBasicChannel">, System};

        std::atomic<int> BasicJobCount = 0;
        for (int i=0; i<1000; ++i)
        {
            Channel.Submit([&]
            {
                ++BasicJobCount;
                if (BasicJobCount.load()%100 == 0 )
                {
                    printf("Doing job %d, by worker %d\n", BasicJobCount.load(), xscheduler::thread_id_v);
                }
            });
        }

        Channel.join();

        assert(BasicJobCount == 1000);

        printf("TestBasicChannel - Successful\n");
    }

    void TestForeachLog(system& System)
    {
        task_group Channel{ xscheduler::str_v<"TestForeachLog">, System};
        std::vector<int> Vec(1000);
        std::atomic<int> Sum = 0;
        Channel.ForeachLog(Vec, 4, 10, [&Sum](std::span<int> View)
        {
            for (int& Val : View) Sum.fetch_add(++Val);
        });
        Channel.join();
        int Expected = 0;
        for (int Val : Vec) Expected += Val;
        assert(Sum.load() == Expected && Sum.load() == 1000);
        printf("TestForeachLog - Successful\n");
    }

    void TestForeachFlat(system& System)
    {
        task_group Channel{ xscheduler::str_v<"TestForeachFlat">, System };
        std::vector<int> Vec(1000);
        std::atomic<int> Sum = 0;
        Channel.ForeachFlat(Vec, 100, [&Sum](std::span<int> View)
        {
            for (int& Val : View) Sum.fetch_add(++Val);
        });
        Channel.join();
        int Expected = 0;
        for (int Val : Vec) Expected += Val;
        assert(Sum.load() == Expected && Sum.load() == 1000);
        printf("TestForeachFlat - Successful");
    }

    // Main test function
    void RunTests()
    {
        system System{4}; // 4 workers for testing
        if constexpr (true) TestBasicJob(System);
        if constexpr (true) TestAsyncJob(System);
        if constexpr (true) TestDependencyGraph(System);
        if constexpr (true) TestDependencyGraphASync(System);
        if constexpr (true) TestAsyncWithDependencies(System);
        if constexpr (true) TestBasicChannel(System);
        if constexpr (true) TestForeachLog(System);
        if constexpr (true) TestForeachFlat(System);

        std::cout << "**************** All tests passed! **************" << std::endl;
    }
}
