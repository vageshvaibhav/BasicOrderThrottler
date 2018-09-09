//
//  concrete_timer.cpp
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#include "concrete_timer.hpp"
#include <sys/time.h>
#include <chrono>
#include <iostream>


using namespace std;


ConcreteTimerQueue::ConcreteTimerQueue() {}

ConcreteTimerQueue::~ConcreteTimerQueue() {}

bool ConcreteTimerQueue::Add(TimerData& data, TimeType timeout)
{
    data.timeout = timeout;
    data.index = m_Heap.size();
    m_Heap.push_back(&data);
    //std::cout << "ConcreteTimerQueue added, m_heap size = " <<m_Heap.size()<<"\n";
    UpHeap(m_Heap.size()-1);
    return data.index == 0;
}

void ConcreteTimerQueue::Remove(TimerData& data)
{
    //std::cout << "ConcreteTimerQueue first Remove, m_heap size = " <<m_Heap.size()<<"\n";
    if (data.index == (size_t)(-1)) {
        return;
    }
    
    size_t idx = data.index;
    m_Heap[idx] = m_Heap.back();
    m_Heap[idx]->index = idx;
    m_Heap.pop_back();
    //std::cout << "ConcreteTimerQueue Remove, m_heap size = " <<m_Heap.size()<<"\n";
    DownHeap(idx);
    data.index = (size_t)(-1);
}

bool ConcreteTimerQueue::Change(TimerData& data, TimeType timeout)
{
    if (data.index == (size_t)(-1)) {
        return false;
    }
    
    size_t idx = data.index;
    if (data.timeout < timeout) {
        data.timeout = timeout;
        DownHeap(idx);
    } else if (data.timeout > timeout) {
        data.timeout = timeout;
        UpHeap(idx);
        if (data.index == 0) {
            return true;
        }
    }
    return false;
}

void ConcreteTimerQueue::RemoveExpired(vector<Timer::Handler>& cbks)
{
    //std::cout << "Remove Expired , m_heap size = " <<m_Heap.size()<<"\n";
    TimeType now = GetTime();
    for (;;) {
        if (m_Heap.size() == 0) {
            break;
        }
        
        TimerData& data = *m_Heap[0];
        if (data.timeout <= now) {
            cbks.push_back(data.cbk);
            Remove(data);
        } else {
            break;
        }
    }
}

ConcreteTimerQueue::TimeType ConcreteTimerQueue::NextTimeout(TimeType max) const
{
    if (m_Heap.size() == 0) {
        return max;
    } else {
        TimeType now = GetTime();
        if (m_Heap[0]->timeout <= now) {
            return 0;
        } else {
            return m_Heap[0]->timeout - now;
        }
    }
}

void ConcreteTimerQueue::UpHeap(size_t i)
{
    for (;;) {
        if (i == 0) {
            break;
        }
        size_t parent = (i - 1) >> 1;
        if (m_Heap[i]->timeout < m_Heap[parent]->timeout) {
            SwapHeapElements(i, parent);
            i = parent;
        } else {
            break;
        }
    }
}

void ConcreteTimerQueue::DownHeap(size_t i)
{
    for (;;) {
        size_t left = (i << 1) + 1;
        size_t right = left + 1;
        size_t min = i;
        if ((left < m_Heap.size()) &&
            (m_Heap[left]->timeout < m_Heap[min]->timeout)) {
            min = left;
        }
        if ((right < m_Heap.size()) &&
            (m_Heap[right]->timeout < m_Heap[min]->timeout)) {
            min = right;
        }
        if (min == i) {
            break;
        }
        SwapHeapElements(i, min);
        i = min;
    }
}

void ConcreteTimerQueue::SwapHeapElements(size_t a, size_t b)
{
    swap(m_Heap[a], m_Heap[b]);
    m_Heap[a]->index = a;
    m_Heap[b]->index = b;
}

ConcreteTimerQueue::TimeType ConcreteTimerQueue::GetTime() const
{
    timeval tv;
    gettimeofday(&tv, NULL);
    TimeType t = tv.tv_sec * 1000;
    t += tv.tv_usec / 1000;
    return t;
}



ConcreteTimerService::ConcreteTimerService()
: m_Shutdown(false)
{
    m_Thread = std::thread(&ConcreteTimerService::Run, this);
}

ConcreteTimerService::~ConcreteTimerService()
{
    m_Shutdown = true;
    m_Thread.join();
}

Timer* ConcreteTimerService::CreateTimer()
{
    return new ConcreteTimer<ConcreteTimerService>(*this);
}

void ConcreteTimerService::DestroyTimer(Timer* timer)
{
    delete static_cast<ConcreteTimer<ConcreteTimerService>*>(timer);
}

void ConcreteTimerService::AddTimer(ConcreteTimerQueue::TimerData& data, unsigned int timeout)
{
    ConcreteTimerQueue::TimeType now = m_Queue.GetTime();
    std::unique_lock<std::mutex> lock(m_Mutex);
    //std::cout << "Adding  to timer , data timeout ="<<data.timeout <<" index = "<<data.index << "now + timeout =" << (now + timeout)<< "\n" ;
    if (m_Queue.Add(data, now + timeout)) {
    }
}

void ConcreteTimerService::RemoveTimer(ConcreteTimerQueue::TimerData& data)
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    //std::cout << "Removing existing timer , data timeout ="<<data.timeout <<" index = "<<data.index<< "\n";
    m_Queue.Remove(data);
}

void ConcreteTimerService::ChangeTimer(ConcreteTimerQueue::TimerData& data, unsigned int timeout)
{
    ConcreteTimerQueue::TimeType now = m_Queue.GetTime();
    std::unique_lock<std::mutex> lock(m_Mutex);
    if (m_Queue.Change(data, now + timeout)) {
    }
}

void ConcreteTimerService::Run()
{
    vector<Timer::Handler> callbacks;
    while (!m_Shutdown)
    {
        
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Queue.RemoveExpired(callbacks);
        }
        vector<Timer::Handler>::const_iterator iter = callbacks.begin();
        vector<Timer::Handler>::const_iterator end = callbacks.end();
        for (; iter != end; ++iter)
        {
            //std::cout<<"callbacks.size=" << callbacks.size() <<"\n";
            (*iter)();
        }
        callbacks.clear();
        
        ConcreteTimerQueue::TimeType delay(1000); 
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            delay = m_Queue.NextTimeout(delay);
        }
        
        std::this_thread::sleep_for(
                                      std::chrono::milliseconds(delay));
    }
}


