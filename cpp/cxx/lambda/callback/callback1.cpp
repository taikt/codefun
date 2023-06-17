// callback1.cpp - Interface Class Callback

// Situation:  A "Caller" class allows another class "Callee"
//             to connect to it via callback.  How to implement this?

// One solution is to provide an interface class that the callee can 
// derive from.  The callee implements that interface.  A pointer to
// the callee can be sent to the caller.

// Advantages: 
//   - Handles multiple callbacks well.  Just add functions to the
//     interface class.

//   - Fairly straightforward C++.

// Disadvantages: 
//   - Name of function is dictated by interface class so there can be 
//     naming clashes.  Using a prefix can mitigate this.  In this example,
//     I used "cbi" which comes from the name of the interface class,
//     CallbackInterface.

//   - Can only derive once, so "Callee" can only connect to one "Caller"
//     that uses a specific interface class.  In cases where one callee 
//     might connect to more than one caller, this technique is pretty much
//     useless.

// Overall, this is a really great callback approach, especially if your
// interface needs more than one callback function between the two classes.
// An example might be implementing the Observer pattern on a complex
// problem domain object.  Instead of having just a single notification
// callback for any change made, it might be more efficient to have
// multiple callbacks for specific fields or groups of fields in the object.

// (I use printf() because it is faster than cout << ...)
#include <stdio.h>

//------------------------------------------------------------------------
// Abstract Base Class
// Those who want to provide a callback must derive from this class and
// provide an implementation of cbiCallbackFunction().
class CallbackInterface
{
public:
    // The prefix "cbi" is to prevent naming clashes.
    virtual int cbiCallbackFunction(int) = 0;
};

//------------------------------------------------------------------------
// "Caller" allows a callback to be connected.  It will call that callback.
class Caller
{
public:
    // Clients can connect their callback with this
    void connectCallback(CallbackInterface *cb)
    {
        m_cb = cb;
    }

    // Test the callback to make sure it works.
    void test()
    {
        printf("Caller::test() calling callback...\n");
        int i = m_cb -> cbiCallbackFunction(10);

        printf("Result (20): %d\n", i);
    }

private:
    // The callback provided by the client via connectCallback().
    CallbackInterface *m_cb;
};

//------------------------------------------------------------------------
// "Callee" can provide a callback to Caller.
class Callee : public CallbackInterface
{
public:
    // The callback function that Caller will call.
    int cbiCallbackFunction(int i)  
    { 
        printf("  Callee::cbiCallbackFunction() inside callback\n");
        return 2 * i; 
    }
};

//------------------------------------------------------------------------

int main()
{
    Caller caller;
    Callee callee;

    // Connect the callback
    caller.connectCallback(&callee);

    // Test the callback
    caller.test();

    return 0;
}

