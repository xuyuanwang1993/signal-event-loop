#ifndef TIMERESCAPER_H
#define TIMERESCAPER_H
#include "log/aimy-log.h"
#include "core/object.h"
namespace aimy {
template < class ChronoTimeType=std::chrono::milliseconds>
class TimerEscaper{
public:
    explicit TimerEscaper(const std::string &_msg="default",const std::string &_name="default",int64_t _warnning_time=0):msg(_msg),name(_name),warnning_time(_warnning_time)
    {
        if(typeid (ChronoTimeType)==typeid (std::chrono::milliseconds))
        {
            units="ms";
        }
        else if(typeid (ChronoTimeType)==typeid (std::chrono::microseconds))
        {
            units="us";
        }
        else if(typeid (ChronoTimeType)==typeid (std::chrono::seconds))
        {
            units="sec";
        }
        else if(typeid (ChronoTimeType)==typeid (std::chrono::nanoseconds))
        {
            units="ns";
        }
        else {
            throw std::invalid_argument("only support ns->s");
        }
#ifdef DEBUG
        AIMY_DEBUG("%s %s start tick!",name.c_str(),msg.c_str());
#endif
        start_time=Timer::getTimeNow<ChronoTimeType>();
    }

    void restart()
    {
        start_time=Timer::getTimeNow<ChronoTimeType>();
    }

    int64_t escaped()const
    {
        return Timer::getTimeNow<ChronoTimeType>()-start_time;
    }

    ~TimerEscaper()
    {
        auto escaped_time=escaped();
        if(escaped_time>warnning_time)
        {
            AIMY_ERROR("%s overflow [%ld>%ld]%s msg[%s]",name.c_str(),escaped_time,warnning_time,units.c_str(),msg.c_str());
        }
        else {
#ifdef DEBUG
        AIMY_ERROR(" %s cost [%ld]%s msg[%s]",name.c_str(),escaped_time,units.c_str(),msg.c_str());
#endif
        }
    }
private:
    const std::string msg;
    const std::string name;
    const int64_t warnning_time;
    std::string units;
    int64_t start_time;
};
}
#endif // TIMERESCAPER_H
