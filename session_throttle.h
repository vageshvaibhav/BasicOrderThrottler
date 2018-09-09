//
//  session_throttle.h
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#ifndef session_throttle_h
#define session_throttle_h

#define DEFAULT_THROTTLE_WINDOW 1000
#define DEFAULT_THROTTLE_LIMIT 100

#include "concrete_timer.hpp"
#include <queue>
#include <list>
#include "OrderDef.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>


struct comparator
{
    bool operator()(std::shared_ptr<BaseOrder>&lhs,std::shared_ptr<BaseOrder>&rhs)
    {
        //Give priority to Pull request/ Cancel order first
        //If orders are of Cancel/Pull request type, give priority to earlier received order
        return (lhs->GetOrderType() < rhs->GetOrderType()
                ||
                (lhs->GetOrderType() == rhs->GetOrderType()
                 &&  (lhs->GetTimeStamp() > rhs->GetTimeStamp())));  //Higher timestamp means recieved later
    }
};

template<typename OrderRouterT>
class SessionThrottle
{
public:
    SessionThrottle();
    ~SessionThrottle();
    
    void SetThrottle(
                     std::shared_ptr<typename OrderRouterT::Traits::SessionT> session,
                     int64_t throttleInterval,
                     uint32_t throttleLimit);
    void ResetThrottle();
    bool Throttle(std::shared_ptr<BaseOrder> prData);
    void DoRetransmit();
private:
    
    // Check throttle and return wait time if message is throttled
    uint64_t ThrottleMsg(bool checkOnly);
    
    void StartRetransmitTimer(uint64_t waitTime);
    void StopRetransmitTimer();
    
    
    std::mutex m_mutex;
    std::unique_ptr<TimerService> m_timerService;
    std::shared_ptr<typename OrderRouterT::Traits::SessionT> m_session;
    
    std::list <boost::posix_time::ptime> m_times;
    
    // A default value of zero indicates no throttling
    uint32_t m_throttleLimit =  DEFAULT_THROTTLE_LIMIT;
    uint64_t m_throttleWindow = DEFAULT_THROTTLE_WINDOW;
    
    //Priority Queue for giving priority to PullRequest/Cancel Order
    std::priority_queue<std::shared_ptr<BaseOrder>,vector<std::shared_ptr<BaseOrder>>,comparator> m_messageQueue;
    Timer* m_retransmitTimer;
};

#include "session_throttle_inl.h"

#endif /* session_throttle_h */
