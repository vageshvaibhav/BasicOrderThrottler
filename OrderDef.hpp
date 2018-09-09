//
//  OrderDef.hpp
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#ifndef OrderDef_hpp
#define OrderDef_hpp

#include <stdio.h>
#include <string>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;

enum RequestTypes
{
    OnNewOrderRequest = 0,
    OnAmendOrderRequest = 1,
    OnPullOrderRequest = 2
};


class BaseOrder
{
    public:
    int m_requestType;
    std::string m_id;
    boost::posix_time::ptime m_timeStamp;
    
    public:
    BaseOrder(int reqType, string id);
    virtual ~BaseOrder();

    int GetOrderType();

    boost::posix_time::ptime GetTimeStamp();
    
    friend std::ostream& operator<<(std::ostream& os, const BaseOrder& bo);
    
};

class NewOrder : public BaseOrder
{
    char m_side; //'B' for Buy 'S' for Sell
    int m_price;
    int m_size;
    string m_symbol;
    
    public :
    NewOrder(int reqType, string id, char side, int price, int size, string symbol);
    
    virtual ~NewOrder();
};

class AmendOrder : public BaseOrder
{
    int m_price;
    int m_size;
    public:
    AmendOrder(int reqType, string id, int price, int size);
    virtual ~AmendOrder();
};

class PullOrder : public BaseOrder //Cancel order
{
    public :
    PullOrder(int reqType, string id);
    virtual ~PullOrder();
    
};

#endif /* OrderDef_hpp */
