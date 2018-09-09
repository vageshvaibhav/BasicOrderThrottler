//
//  concrete_timer.hpp
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#ifndef concrete_timer_hpp
#define concrete_timer_hpp

#include <stdio.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <limits>
#include "timer.h"
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;


class ConcreteTimerQueue
{
public:
    typedef uint64_t TimeType;
    
    struct TimerData {
        TimerData()
        : timeout(0)
        , index((size_t)(-1))
        {}
        
        Timer::Handler cbk;
        TimeType timeout;
        size_t index;
    };
    
    ConcreteTimerQueue();
    virtual ~ConcreteTimerQueue();
    
    // Add data object to the timer queue. Return true if this item is the
    // first item in the queue (lowest timeout value). O(log(n))
    // Timeout is absolute time in milliseconds (use GetTime()).
    bool Add(TimerData& data, TimeType timeout);
    // Remove data from the queue. O(log(n))
    void Remove(TimerData& data);
    // Change the timeout value of a data object in the queue. Return true if
    // this item is first in the queue. This has the same effect as a Remove
    // and an Add, but is faster. O(log(n))
    // Timeout is absolute time in milliseconds (use GetTime()).
    bool Change(TimerData& data, TimeType timeout);
    // Remove all expired timers in the queue and add them to the cbks list.
    // O(m*log(n)) where m = number of expired timers.
    void RemoveExpired(std::vector<Timer::Handler>& cbks);
    // Get the relative time (in milliseconds) of the next timer timeout.
    // Returns max if no timers will time out in under max amount of time.
    TimeType NextTimeout(TimeType max) const;
    // Get the current absolute time in milliseconds.
    TimeType GetTime() const;
    
private:
    ConcreteTimerQueue(const ConcreteTimerQueue&);
    void operator=(const ConcreteTimerQueue&);
    
    typedef std::vector<TimerData*> HeapType;
    
    void UpHeap(size_t i);
    void DownHeap(size_t i);
    void SwapHeapElements(size_t a, size_t b);
    
    HeapType m_Heap;
    
};

class ConcreteTimerService : public TimerService
{
public:
    ConcreteTimerService();
    virtual ~ConcreteTimerService();
    
    Timer* CreateTimer();
    void DestroyTimer(Timer* timer);
    
    // These are used by the timer object to interface with the timer queue.
    void AddTimer(ConcreteTimerQueue::TimerData& data, unsigned int timeout);
    void RemoveTimer(ConcreteTimerQueue::TimerData& data);
    void ChangeTimer(ConcreteTimerQueue::TimerData& data, unsigned int timeout);
    
private:
    ConcreteTimerService(const ConcreteTimerService&);
    void operator=(const ConcreteTimerService&);
    
    void Run();
    
    std::mutex m_Mutex;
    ConcreteTimerQueue m_Queue;
    std::thread m_Thread;
    atomic<bool> m_Shutdown;
};

template <class TimerService = ConcreteTimerService>
class ConcreteTimer : public Timer
{
public:
    ConcreteTimer(TimerService& service)
    : m_Service(service)
    {}
    
    virtual ~ConcreteTimer()
    {
        m_Service.RemoveTimer(m_Data);
    }
    
    void Set(Timer::Handler cbk, unsigned int timeout)
    {
        //std::cout << "Setting the call back in Timer, timeout =" << timeout <<",m_Data.index =" <<m_Data.index<< "\n";
        m_Service.RemoveTimer(m_Data);
        m_Data.cbk = cbk;
        m_Service.AddTimer(m_Data, timeout);
    }
    
    void Reset(unsigned int timeout)
    {
        m_Service.ChangeTimer(m_Data, timeout);
    }
    
    void Cancel()
    {
        m_Service.RemoveTimer(m_Data);
    }
    
private:
    ConcreteTimer(const ConcreteTimer&);
    void operator=(const ConcreteTimer&);
    
    TimerService& m_Service;
    ConcreteTimerQueue::TimerData m_Data;
};
#endif /* concrete_timer_hpp */
