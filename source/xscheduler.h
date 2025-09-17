#ifndef XCORE_SCHEDULER_H
#define XCORE_SCHEDULER_H
#pragma once

//
// standard library
//
#include <atomic>
#include <array>
#include <span>
#include <thread>
#include <functional>
#include <coroutine>
#include <cstdint>
#include <concepts>
#include <cassert>
#include <variant>
#include <format>
#include <condition_variable>

//
// Platform specific
// Simple keyword to show which functions are in the quantum world
//
#define xquatum
#ifdef _WIN32
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <processthreadsapi.h>
    #undef DELETE
#elif __linux__
    #include <pthread.h>
#elif __APPLE__
    #include <pthread.h>
#endif

//
// Dependencies
//
#include "../../dependencies/xcontainer/source/xcontainer_lockless_queue.h"
#include "../../dependencies/xcontainer/source/xcontainer_lockless_pool.h"
#include "../../dependencies/xcontainer/source/xcontainer_function.h"

//
// Predifinitions
//
namespace xscheduler
{
    class system;
}

//
// Core header files
//
#include "xscheduler_jobs.h"
#include "xscheduler_triggers.h"
#include "xscheduler_system.h"
#include "xscheduler_task_group.h"

//
// Implementations
//
#include "Implementation/xscheduler_system_inline.h"
#include "Implementation/xscheduler_jobs_inline.h"
#include "Implementation/xscheduler_triggers_inline.h"
#include "Implementation/xscheduler_task_group_inline.h"

#endif
