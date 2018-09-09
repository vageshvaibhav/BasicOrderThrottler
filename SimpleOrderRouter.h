//
//  SimpleOrderRouter.h
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#ifndef SimpleOrderRouter_h
#define SimpleOrderRouter_h
#include <boost/date_time/posix_time/posix_time.hpp>
#include "OrderDef.hpp"
#include <condition_variable>



struct session{
    int totalSend;
    int totalOrdersToBeSent;
    condition_variable& cv;
    session(condition_variable& cv,int totalOrd):totalSend(0),cv(cv),totalOrdersToBeSent(totalOrd)
    {
        
    }
    //Placeholder for actual Order Send call
     void Send(std::shared_ptr<BaseOrder> baseOrder){
         std::cout << "Sending order number = " << baseOrder->m_id <<", time= "<< boost::posix_time::microsec_clock::universal_time() <<", total sent =" <<++totalSend<<",  Order type = " << *baseOrder<<std::endl ;
         if(totalOrdersToBeSent == totalSend)
         {
             // This is a simple order router, Any complex order router can be created to utilize order throttler interface
             //Here notifying the global conditional variable from main for testing purpose to finish the process
             cv.notify_all();
         }
    }
};

struct SimpleOrderRouter
{
    struct Traits
    {
        typedef session SessionT;
    };
    
};

#endif /* SimpleOrderRouter_h */
