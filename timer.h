//
//  timer.h
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#ifndef timer_h
#define timer_h

#include <functional>

class Timer
{
public:
    typedef std::function<void ()> Handler;
    
public:
    virtual ~Timer(){}
    virtual void Set(Timer::Handler handler, unsigned int interval) = 0;
    virtual void Reset(unsigned int interval) = 0;
    virtual void Cancel() = 0;
};

class TimerService
{
public:
    virtual ~TimerService(){}
    virtual Timer* CreateTimer() = 0;
    virtual void DestroyTimer(Timer* timer) = 0;
};

#endif /* timer_h */
