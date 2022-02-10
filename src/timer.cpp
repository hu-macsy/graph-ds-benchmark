#include <gdsb/timer.h>

namespace gdsb
{

void WallTimer::start() { m_start_point = std::chrono::high_resolution_clock::now(); }

void WallTimer::end() { m_end_point = std::chrono::high_resolution_clock::now(); }

std::chrono::nanoseconds WallTimer::duration() const
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(m_end_point - m_start_point);
}

void CPUTimer::start() { m_start_point = std::clock(); }

void CPUTimer::end() { m_end_point = std::clock(); }

std::chrono::nanoseconds CPUTimer::duration() const
{
    std::chrono::nanoseconds ns(1000000000 * (m_end_point - m_start_point) / CLOCKS_PER_SEC);
    return ns;
}

ClockTimer::ClockTimer(clockid_t clock_id)
: m_clock_id(clock_id){

};

void ClockTimer::start() { clock_gettime(m_clock_id, &m_start_point); }

void ClockTimer::end() { clock_gettime(m_clock_id, &m_end_point); }

std::chrono::nanoseconds ClockTimer::duration() const
{
    std::chrono::nanoseconds ns(m_end_point.tv_nsec - m_start_point.tv_nsec);
    return ns;
}

} // namespace gdsb
