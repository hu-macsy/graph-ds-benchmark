#pragma once

#include <gdsb/timer.h>

#include <algorithm>
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

//! Use this function to output parameters that shall be profiled.
template <typename T> void out(std::string name, T value) { std::cout << name << ": " << value << std::endl; }

//! Use this function to output ranges in yaml format.
template <typename It> void out(std::string name, It begin, It end, std::ostream& stream = std::cout)
{
    stream << name << ": [";

    if (begin != end)
    {
        std::for_each(begin, end - 1, [&](auto const& e) { stream << e << ", "; });
        stream << *(end - 1);
    }

    stream << "]" << std::endl;
}

//! Use this function to output ranges in yaml format.
template <typename It, typename F>
void out(std::string name, It begin, It end, F const&& access, std::ostream& stream = std::cout)
{
    stream << name << ": [";

    if (begin != end)
    {
        std::for_each(begin, end - 1,
                      [access = std::move(access), &stream](auto const& e) { stream << access(e) << ", "; });
        stream << access(*(end - 1));
    }

    stream << "]" << std::endl;
}

//! Returns the memory usage in KB for the calling process.
unsigned long long memory_usage_in_kb();

} // namespace gdsb
