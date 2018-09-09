//
//  session_throttle_inl.h
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#ifndef session_throttle_inl_h
#define session_throttle_inl_h


template <typename OrderRouterT>
SessionThrottle<OrderRouterT>::SessionThrottle() : m_retransmitTimer(nullptr)
{
}

template <typename OrderRouterT>
SessionThrottle<OrderRouterT>::~SessionThrottle()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    StopRetransmitTimer();
    m_session = nullptr;
    
    while (!m_messageQueue.empty())
    {
        m_messageQueue.pop();
    }
}

template <typename OrderRouterT>
inline void SessionThrottle<OrderRouterT>::SetThrottle(
                                                       std::shared_ptr<typename OrderRouterT::Traits::SessionT> session,
                                                       int64_t throttleInterval,
                                                       uint32_t throttleLimit)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_session = session;
    m_throttleLimit = throttleLimit;
    m_throttleWindow = throttleInterval;
    m_timerService.reset(new ConcreteTimerService());
}

template <typename OrderRouterT>
inline void SessionThrottle<OrderRouterT>::ResetThrottle()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    
    m_throttleLimit =  DEFAULT_THROTTLE_LIMIT;
    m_throttleWindow = DEFAULT_THROTTLE_WINDOW;
    
    m_times.clear();
    
    while (!m_messageQueue.empty())
    {
        m_messageQueue.pop();
    }
    
    m_timerService = new ConcreteTimerService();
}

template <typename OrderRouterT>
inline bool SessionThrottle<OrderRouterT>::Throttle(shared_ptr<BaseOrder> prData)
{
    if (0 == m_throttleLimit)
    {
        // throttle limit of 0 means throttling is turned off
        return false;
    }
    
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (!m_messageQueue.empty())
    {
        // Messages are currently being throttled so add it to the
        // message queue to be sent in order at a later time
        m_messageQueue.push(prData);
        //std::cout <<"Queuing message = " <<prData->id_;
        return true;
    }
    
    uint64_t waitTime = ThrottleMsg(false);
    
    if (0 == waitTime)
    {
        m_session->Send(prData); //Send it straight away
        return false;
    }
    
    //std::cout<< "Wait Time = " << waitTime << " for id = " <<prData->id_ <<"\n";
    // Message is throttled so add it to a message queue and attempt to
    // resend it later.
    m_messageQueue.push(prData);
    
    StartRetransmitTimer(waitTime);
    return true;
}

template <typename OrderRouterT>
inline uint64_t SessionThrottle<OrderRouterT>::ThrottleMsg(bool checkOnly)
{
    using boost::posix_time::ptime;
    using boost::posix_time::microsec_clock;
    
    uint32_t sz = m_times.size();
    assert(sz <= m_throttleLimit && "malformed throttle count");
    
    ptime now = microsec_clock::universal_time();
    
    if (sz >= m_throttleLimit)
    {
        uint64_t diff = (now - m_times.front()).total_milliseconds();
        if (m_throttleWindow > diff)
        {
            // Return a waitTime indicating the message was
            // throttled and the time in ms until we
            // can send next
            return (m_throttleWindow - diff);
        }
        
        if (!checkOnly)
        {
            // We can send so remove the oldest
            // sent message timestamp
            m_times.pop_front();
        }
    }
    if (!checkOnly)
    {
        m_times.push_back(now);
    }
    
    // Return zero indicating the message was not throttled
    return 0;
}

template <typename OrderRouterT>
inline void SessionThrottle<OrderRouterT>::StartRetransmitTimer(uint64_t waitTime)
{
    if(m_messageQueue.empty() || nullptr != m_retransmitTimer)
    {
        // We either have no messages to resend or a timer is
        // already scheduled
        return;
    }
    m_retransmitTimer = m_timerService->CreateTimer();
    //std::cout << "StartRetransmitTimer : wait time =" <<waitTime <<"\n";
    m_retransmitTimer->Set(
                           bind(&SessionThrottle<OrderRouterT>::DoRetransmit, this),
                           waitTime);
}

template <typename OrderRouterT>
inline void SessionThrottle<OrderRouterT>::StopRetransmitTimer()
{
    if (nullptr != m_retransmitTimer)
    {
        m_retransmitTimer->Cancel();
        m_timerService->DestroyTimer(m_retransmitTimer);
        m_retransmitTimer = nullptr;
    }
}

template <typename OrderRouterT>
inline void SessionThrottle<OrderRouterT>::DoRetransmit()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (m_messageQueue.empty())
    {
        return;
    }
    
    uint64_t waitTime = ThrottleMsg(false);
    if (waitTime > 0)
    {
        // We should not be throttled when the timeout fires,
        // but wait longer if the throttle tells us to
        StartRetransmitTimer(waitTime);
        return;
    }
    
    //Send the message. The throttle flag must be false since
    // we have already throttled the message
    std::shared_ptr<BaseOrder> prData = m_messageQueue.top();
    m_session->Send(prData);
    
    // pop the top message from priority_queue
    m_messageQueue.pop();
    
    if (m_messageQueue.empty())
    {
        StopRetransmitTimer();
        return;
    }
    
    // Calculate how long we should wait before sending the next message.
    // We should wait a minimum of 1 ms to avoid hitting the throttle
    waitTime = ThrottleMsg(true);
    if (0 == waitTime)
    {
        waitTime = 1;
    }
    
    StopRetransmitTimer();
    StartRetransmitTimer(waitTime);
}


#endif /* session_throttle_inl_h */
