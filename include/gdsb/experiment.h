#pragma once

#include <gdsb/timer.h>

#include <iostream>
#include <string>

namespace gdsb
{

template <typename F> std::chrono::milliseconds benchmark(F&& f)
{
    WallTimer timer;

    timer.start();
    while (!f())
    {
    };
    timer.end();

    std::chrono::nanoseconds const duration = timer.duration();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

template <typename F, typename DS> std::chrono::milliseconds benchmark(F&& f, DS& ds)
{
    WallTimer timer;

    timer.start();
    while (!f(ds))
    {
    };
    timer.end();

    std::chrono::nanoseconds const duration = timer.duration();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

template <typename F, class DS, class E> std::chrono::milliseconds benchmark(F&& f, DS& input, E& edges)
{
    WallTimer timer;

    timer.start();
    while (!f(input, edges))
    {
    };
    timer.end();

    std::chrono::nanoseconds const duration = timer.duration();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

//! Use this function to ouput parameters that shall be profiled.
template <typename T> void out(std::string name, T value) { std::cout << name << ": " << value << std::endl; }

//! Returns the memory usage in KB for the calling process.
unsigned long long memory_usage_in_kb();

} // namespace gdsb
