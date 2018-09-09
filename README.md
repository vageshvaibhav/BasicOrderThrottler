# BasicOrderThrottler

1) SessionThrottle : is the central class responsible for throttling of messages
2) SessionThrottle uses Concrete Timer/Service classes for throttling functionality
3) For the throttled messages, priority_queue has been used to give PullOrder/Cancel Order requests priority. This is defined
in comparator function for the heap which can be modified to suit the requirements.
4) This has been developed and tested on MAC xcode IDE.
5) main.cpp contains some sample client code making use of throttler by placing various types of orders.
6) It's a sliding window throttler which compares the timestamp of the oldest message on heap at a given time on the basis 
of timestamp of that order and current timestamp to calculating throttle wait time.
7) Any excess messages are queued and throttling logic is applied on the queue as well.
