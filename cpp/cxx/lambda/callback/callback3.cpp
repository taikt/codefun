// callback3.cpp - Function Pointer Callback

// Situation:  A "Caller" class allows another class "Callee"
//             to connect to it via callback.  How to implement this?

// This version uses a C-style function pointer.

// Advantages: 
//   - Compatible with C-style callbacks.

// Disadvantages: 
//   - Slightly complicated C.

//   - Clunky.  Two callback functions required on the receiving end.
//     Have to pass the "this" pointer.  It's very "C".

// Overall, the C-style callback approach is perfect for dealing with
// callbacks in existing C APIs.  For anything else, it's a bit too clunky.

// (I use printf() because it is faster than cout << ...)
#include <stdio.h>

//------------------------------------------------------------------------
// Callback function pointer.
typedef int(*CallbackFunctionPtr)(void*, int);

//------------------------------------------------------------------------
// "Caller" allows a callback to be connected.  It will call that callback.
class Caller
{
public:
    // Clients can connect their callback with this.  They can provide
    // an extra pointer value which will be included when they are called.
    void connectCallback(CallbackFunctionPtr cb, void *p)
    {
        m_cb = cb;
        m_p = p;
    }

    // Test the callback to make sure it works.
    void test()
    {
        printf("Caller::test() calling callback...\n");
        int i = m_cb(m_p, 10);

        printf("Result (20): %d\n", i);
    }

private:
    // The callback provided by the client via connectCallback().
    CallbackFunctionPtr m_cb;
    // The additional pointer they provided (it's "this").
    void *m_p;
};

//------------------------------------------------------------------------
// "Callee" can provide a callback to Caller.
class Callee
{
public:
    // This static function is the real callback function.  It's compatible
    // with the C-style CallbackFunctionPtr.  The extra void* is used to
    // get back into the real object of this class type.
    static int staticCallbackFunction(void *p, int i)
    {
        // Get back into the class by treating p as the "this" pointer.
        ((Callee *)p) -> callbackFunction(i);
    }

    // The callback function that Caller will call via 
    // staticCallbackFunction() above.
    int callbackFunction(int i)  
    {
        printf("  Inside callback\n");
        return 2 * i; 
    }
};

//------------------------------------------------------------------------

int main()
{
    Caller caller;
    Callee callee;

    // Connect the callback.  Send the "this" pointer for callee as the 
    // void* parameter.
    caller.connectCallback(Callee::staticCallbackFunction, &callee);

    // Test the callback
    caller.test();

    return 0;
}

