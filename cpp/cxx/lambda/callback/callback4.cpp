// callback4.cpp - C++11 Lambda Callback

// To build:
//   g++ -std=c++11 callback4.cpp

// Situation:  A "Caller" class allows another class "Callee"
//             to connect to it via callback.  How to implement this?

// A C++11 lambda function can be used.

// Advantages: 
//   - Relatively direct and straightforward C++11.

// Disadvantages: 
//   - Requires decent C++11 support.

//   - Requires an understanding of lambdas and std::function.

// Overall, this is pretty workable.  It might be a close contender with
// other callback approaches like Rich Hickey's.

// (I use printf() because it is faster than cout << ...)
#include <stdio.h>

// This gets us std::function
#include <functional>

//------------------------------------------------------------------------
// Callback function.
typedef std::function<int(int)> CallbackFunction;

//------------------------------------------------------------------------
// "Caller" allows a callback to be connected.  It will call that callback.
class Caller
{
public:
    // Clients can connect their callback with this.
    void connectCallback(CallbackFunction cb)
    {
        m_cb = cb;
    }

    // Test the callback to make sure it works.
    void test()
    {
        printf("Caller::test() calling callback...\n");
        int i = m_cb(10);

        printf("Result (50): %d\n", i);
    }

private:
    // The callback provided by the client via connectCallback().
    CallbackFunction m_cb;
};

//------------------------------------------------------------------------
// "Callee" can provide a callback to Caller.
class Callee
{
public:
    Callee(int i) : m_i(i) { }

    // The callback function that Caller will call.
    int callbackFunction(int i)
    {
        printf("  Callee::callbackFunction() inside callback\n");
        return m_i * i; 
    }

private:
    // To prove "this" is indeed valid within callbackFunction().
    int m_i;
};

//------------------------------------------------------------------------

int main()
{
    Caller caller;
    Callee callee(5);

    // Connect the callback.  Like with the C-style function pointer and 
    // static function, we use a lambda to get back into the object.
    caller.connectCallback(
        [&callee](int i) { return callee.callbackFunction(i); });

    // Note how this is getting closer to the ultimate:
    //     caller.connectCallback(callee.callbackFunction);

    // Test the callback
    caller.test();

    return 0;
}

