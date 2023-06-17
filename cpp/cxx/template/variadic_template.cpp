#if 0
// https://www.geeksforgeeks.org/variadic-function-templates-c/

// C++ program to demonstrate working of
// Variadic function Template
#include <iostream>
using namespace std;

// To handle base case of below recursive
// Variadic function Template
void func()
{
    cout << "empty function and "
            "I am called at last.\n" ;
}

// Variadic function Template that takes
// variable number of arguments and prints
// all of them.
// Types: template parameter pack
// var2: function parameter pack
// use typename... or class...
//template <typename T, typename... Types>
template <typename T, class... Types>
void func(T var1, Types... var2)
{
    cout << var1 << endl ;

    func(var2...) ;
}

// Driver code
int main()
{
    func(1, 2, 3.14, "Pass me any "
              "number of arguments",
                  "I will print\n");

    return 0;
}

#endif

#if 0
#include <iostream>

using namespace std;

#include <iostream>

template <typename A>
A myMax(A a,A b)      //this is an overload, not specialization
{
   if (a>b)
      return a;
   else
      return b;
}

template <typename A, typename ... Args>
A myMax(A a, A b, Args ... args)
{
   return myMax(myMax(a,b),args...);
}

int main()
{
   std::cout<<myMax(1,5,2,9,20);
}

#endif


#include <iostream>
#include <string>

template<class L, class... R> class My_class;

template<class L>
class My_class<L>
{
public:

protected:
  L get()
  {
    return val;
  }

  void set(const L new_val)
  {
    val = new_val;
  }

private:
  L val;
};

template<class L, class... R>
class My_class : public My_class<L>, public My_class<R...>
{
public:
  template<class T>
  T Get()
  {
    return this->My_class<T>::get();
  }

  template<class T>
  void Set(const T new_val)
  {
    this->My_class<T>::set(new_val);
  }
};

int main(int, char**)
{
  My_class<int, double, std::string> c;
  c.Set<int>(4);
  c.Set<double>(12.5);
  c.Set<std::string>("Hello World");

  std::cout << "int: " << c.Get<int>() << "\n";
  std::cout << "double: " << c.Get<double>() << "\n";
  std::cout << "string: " << c.Get<std::string>() << std::endl;

  return 0;
}