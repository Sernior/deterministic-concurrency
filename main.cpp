#include <iostream>
#include <tuple>
#include <DeterministicConcurrency>
#include <mutex>
#include <vector>
#include <boost/thread/thread_only.hpp>
#include <boost/thread/xtime.hpp>
//#include <boost/array.hpp>

struct thread_alarm
{
    thread_alarm(int secs) : m_secs(secs) { }
    void operator()()
    {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC_);
        xt.sec += m_secs;

        boost::thread::sleep(xt);

        std::cout << "alarm sounded..." << std::endl;
    }

    int m_secs;
};


int main()
{
    int secs = 55;
    std::cout << "setting alarm for 55 seconds..." << std::endl;
    thread_alarm alarm(secs);
    boost::thread thrd(alarm);
    thrd.join();
    return 0;
}