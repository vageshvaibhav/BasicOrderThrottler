//
//  OrderDef.cpp
//  OrderThrottler
//
//  Created by Vagesh Vaibhav on 09/09/18.
//  Copyright © 2018 Vagesh Vaibhav. All rights reserved.
//

#include "OrderDef.hpp"

BaseOrder::BaseOrder(int reqType, string id) : m_requestType(reqType) ,m_id(id)
{
    m_timeStamp = microsec_clock::universal_time(); //Timestamp for priority_queue calculations
}

BaseOrder::~BaseOrder()
{
    
}

int BaseOrder::GetOrderType()
{
    return m_requestType;
}

boost::posix_time::ptime BaseOrder::GetTimeStamp()
{
    return m_timeStamp;
}

std::ostream& operator<<(std::ostream& os, const BaseOrder& bo)
{
    switch(bo.m_requestType)
    {
        case RequestTypes::OnNewOrderRequest:
            os <<" NewOrderRequest "<<"\n";
            break;
        case RequestTypes::OnAmendOrderRequest:
            cout <<" AmendOrderRequest "<<"\n";
            break;
        case RequestTypes::OnPullOrderRequest:
            cout <<" PullOrderRequest "<<"\n";
            break;
        default:
            cout <<" DefaultOrderRequest "<<"\n";
            break;
            
    }
    return os;
}


NewOrder::NewOrder(int reqType, string id, char side, int price, int size, string symbol) : BaseOrder(reqType,id) , m_side(side), m_price(price), m_size(size), m_symbol(symbol)
{
    
}

NewOrder::~NewOrder()
{
    
}

AmendOrder::AmendOrder(int reqType, string id, int price, int size) : BaseOrder(reqType,id) , m_price(price), m_size(size)
{
    
}
AmendOrder::~AmendOrder()
{
    
}

PullOrder::PullOrder(int reqType, string id) : BaseOrder(reqType,id)
{
    
}
PullOrder::~PullOrder()
{
    
}
