// callback5.cpp - Template Functor Callback (Rich Hickey)

// Situation:  A "Caller" class allows another class "Callee"
//             to connect to it via callback.  How to implement this?

// This version uses Rich Hickey's template functor approach.

// Advantages: 
//     - Simple.

// Disadvantages: 
//     - CBFunctor1wRet<int, int> is a bit clunky.  Especially when compared
//       to std::function<int(int)>.

// Overall, Rich Hickey's still seems better than even the new std::function
// and lambda approach available in C++11.  Key differences:

//   typedef std::function<int(int)> CallbackFunction;
//   typedef CBFunctor1wRet<int, int> CallbackFunction;

// Ok, the std::function typedef is much more elegant than the clunky
// CBFunctor1wRet.

//   caller.connectCallback([&callee](int i) { return callee.foo(i); });
//   caller.connectCallback(makeFunctor(callee, &Callee::foo));

// makeFunctor() wins over the lambda function, though.  The lambda function
// is far more impenetrable than CBFunctor1wRet is clunky.  So, I think
// Rich Hickey's wins by a slim margin.

// Hmmmm.  If this is a functor...  Can we combine the two?  Maybe, but we
// would need to experiment with a newer version of Rich Hickey's callback
// that doesn't need the pointer argument to differentiate between versions
// of makeFunctor.  Need to find that for callback experiment #6.

// (I use printf() because it is faster than cout << ...)
#include <stdio.h>

#include "callback.h"

//------------------------------------------------------------------------
// Callback function.
typedef CBFunctor1wRet<int, int> CallbackFunction;

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
    // To prove "this" is indeed valid within foo().
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
        makeFunctor((CallbackFunction*)0, callee, &Callee::callbackFunction));

    // There are available newer versions of this that use "member 
    // templates" and therefore don't need the first parameter to 
    // makeFunctor() to figure out what's going on.  
    // That gives:
    //     caller.connectCallback(makeFunctor(callee, &Callee::foo));

    // Note how this is getting closer to the ultimate:
    //     caller.connectCallback(callee.foo);

    // Test the callback
    caller.test();

    return 0;
}

