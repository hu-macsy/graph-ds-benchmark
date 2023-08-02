#pragma once

#include <chrono>
#include <ctime>

namespace gdsb
{

template <typename TimePoint, typename Duration> class Timer
{
public:
    using DurationT = Duration;
    virtual void start() = 0;
    virtual void end() = 0;
    virtual Duration duration() const = 0;
    virtual ~Timer() = default;

protected:
    TimePoint m_start_point;
    TimePoint m_end_point;
};

//! Use to measure system time duration. This will include time the process is not executed by the
//! CPU.
class WallTimer : public Timer<std::chrono::high_resolution_clock::time_point, std::chrono::nanoseconds>
{
public:
    void start() override;
    void end() override;
    std::chrono::nanoseconds duration() const override;
};

class CPUTimer : public Timer<std::clock_t, std::chrono::nanoseconds>
{
public:
    void start() override;
    void end() override;
    std::chrono::nanoseconds duration() const override;
};

class ClockTimer : public Timer<timespec, std::chrono::nanoseconds>
{
public:
    ClockTimer(clockid_t clock_id);

    void start() override;
    void end() override;
    std::chrono::nanoseconds duration() const override;

private:
    clockid_t m_clock_id;
};

//! High-resolution per-process timer from the CPU.
class ProcessCPUTimer : public ClockTimer
{
public:
    ProcessCPUTimer()
    : ClockTimer(CLOCK_PROCESS_CPUTIME_ID)
    {
    }
};

class ThreadCPUTimer : public ClockTimer
{
public:
    ThreadCPUTimer()
    : ClockTimer(CLOCK_THREAD_CPUTIME_ID)
    {
    }
};

class RealTimeClockTimer : public ClockTimer
{
public:
    RealTimeClockTimer()
    : ClockTimer(CLOCK_MONOTONIC)
    {
    }
};

} // namespace gdsb
