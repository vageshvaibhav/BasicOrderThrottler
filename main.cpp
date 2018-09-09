//
//  main.cpp
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#include <iostream>
#include "SimpleOrderRouter.h"
#include "session_throttle.h"
#include <condition_variable>

std::condition_variable cv;
std::mutex mtx;

//PlaceOrder is a simple test function for passing/sending orders to SessionThrottle via separate threads
void PlaceOrder(std::shared_ptr<SessionThrottle<SimpleOrderRouter>> simpleOrd,int totalNumOrders,int orderType)
{
    std::unique_lock<std::mutex> lock(mtx);
    for(int i = 0; i <totalNumOrders; i++)
    {
        std::string orderId ("Ord");
        std::shared_ptr<BaseOrder> prData;
        switch(orderType)
        {
            case RequestTypes::OnNewOrderRequest:
                orderId += "New";
                orderId += std::to_string(i);
                prData.reset(new NewOrder(RequestTypes::OnNewOrderRequest, orderId, 'B', 1000, 100, "ABC"));
                break;
            case RequestTypes::OnAmendOrderRequest:
                orderId += "Amend";
                orderId += std::to_string(i);
                prData.reset(new AmendOrder(RequestTypes::OnAmendOrderRequest,orderId, 1000, 100));
                break;
            case RequestTypes::OnPullOrderRequest:
                orderId += "Pull";
                orderId += std::to_string(i);
                prData.reset(new PullOrder(RequestTypes::OnPullOrderRequest, orderId)); //Alias for cancel order
                break;
            default:
                break;
        }
        simpleOrd->Throttle(prData); //Send message for throttling
    }

    cv.wait(lock); //Wait till all orders as per test case are sent. This coditional variable is used just for testing purpose
}

int main(int argc, const char * argv[]) {
    //Running simple test case for SessionThrottle with SimpleOrderRouter

    std::shared_ptr<SessionThrottle<SimpleOrderRouter>> simpleOrd(new SessionThrottle<SimpleOrderRouter>());
    int totalNumOrders = 1000, throttleInterval = 1000 /*ms*/ , maxOrdersInThrottleInterval= 100; //Simple test case data
    std::shared_ptr<session> s(new session(cv,3*totalNumOrders/*sending three types of orders in different threads*/));
    simpleOrd->SetThrottle(s, throttleInterval, maxOrdersInThrottleInterval); //Setting throttle parameters
    std::thread tNewOrderSend(PlaceOrder,simpleOrd,totalNumOrders,RequestTypes::OnNewOrderRequest);
    std::thread tAmendOrderSend(PlaceOrder,simpleOrd,totalNumOrders,RequestTypes::OnAmendOrderRequest);
    std::thread tPullOrderSend(PlaceOrder,simpleOrd,totalNumOrders,RequestTypes::OnPullOrderRequest);
    tNewOrderSend.join();
    tAmendOrderSend.join();
    tPullOrderSend.join();
    return 0;
}
