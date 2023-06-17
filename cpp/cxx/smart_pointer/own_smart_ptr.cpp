// https://www.codeproject.com/Articles/15351/Implementing-a-simple-smart-pointer-in-c
#include "stdio.h"
// taikt
class RefBase {
    int * ptr_test;
    public:
        RefBase()
            :ptr_test (new int) 
        {
            printf("init refbase\n");
        }
};

class Person : public RefBase
{
    int age;
    char* pName;

    public:
        Person(): pName(0),age(0)
        {
        }
        Person(char* pName, int age): pName(pName), age(age)
        {
        }
        ~Person()
        {
        }

        void Display()
        {
            printf("Name = %s Age = %d \n", pName, age);
        }
        void Shout()
        {
            printf("Ooooooooooooooooo");
        } 
};

class RC
{
    private:
    int count; // Reference count

    public:
    RC() {
        printf("init RC\n");
    }
    void AddRef()
    {
        // Increment the reference count
        count++;
    }

    int Release()
    {
        // Decrement the reference count and
        // return the reference count.
        return --count;
    }
};

template < typename T > class SP
{
private:
    T*    pData;       // pointer
    RC* reference; // Reference count

public:
    SP() : pData(0), reference(0) 
    {
        // Create a new reference 
        reference = new RC();
        // Increment the reference count
        reference->AddRef();
    }

    SP(T* pValue) : pData(pValue), reference(0)
    {
        // Create a new reference 
        reference = new RC();
        // Increment the reference count
        reference->AddRef();
    }

    SP(const SP<T>& sp) : pData(sp.pData), reference(sp.reference)
    {
        // Copy constructor
        // Copy the data and reference pointer
        // and increment the reference count
        reference->AddRef();
    }

    ~SP()
    {
        // Destructor
        // Decrement the reference count
        // if reference become zero delete the data
        if(reference->Release() == 0)
        {
            // should check if pData != NULL ?
            delete pData;
            delete reference;
        }
    }

    T& operator* ()
    {
        return *pData;
    }

    T* operator-> ()
    {
        return pData;
    }
    
    SP<T>& operator = (const SP<T>& sp)
    {
        // Assignment operator
        if (this != &sp) // Avoid self assignment
        {
            // Decrement the old reference count
            // if reference become zero delete the old data
            if(reference->Release() == 0)
            {
                delete pData;
                delete reference;
            }

            // Copy the data and reference pointer
            // and increment the reference count
            pData = sp.pData;
            reference = sp.reference;
            reference->AddRef();
        }
        return *this;
    }
};

int main()
{
    SP<Person> p(new Person("Scott", 25));
    //SP<Person> p;
    //Person q;
    printf("log 1\n");
    //p->Display();
#if 1
    {
        SP<Person> q = p;
        printf("log 2\n");
        q->Display();
        // Destructor of q will be called here..

        SP<Person> r;
        printf("log 3\n");
        r = p;
        r->Display();
        // Destructor of r will be called here..
    }
    p->Display();
#endif
    // Destructor of p will be called here 
    // and person pointer will be deleted
    //

    return 0;
}
