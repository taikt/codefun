/* https://www.tutorialspoint.com/cplusplus/cpp_overloading.htm
*   Binary Operators Overloadin
*   ++ and -- Operators Overloading
*   Assignment Operators Overloading
*   Function call () Operator Overloading
*   Subscripting [] Operator Overloading
*   Class Member Access Operator -> Overloading
*   TODO
*   Relational Operators Overloading
*   Input/Output Operators Overloading
*
*/

#include <stdio.h>
using namespace std;

class Box {
    public:
        Box() {
            printf("object is created\n");
        }
        
        ~Box() {
            printf("object is removed\n");
        }

        Box (int a, int b):
            a(a),b(b) {
                printf("object is created with parameters\n");
            }

        void print(int i) {
            printf("i=%d\n",i);
        }
        
        void print(char *c) {
            printf("c=%s\n",c);
        }

        int getVolume(void) {
            return a*b;
        }
    
        // https://www.tutorialspoint.com/cplusplus/binary_operators_overloading.htm    
        Box operator+(const Box& x) {
            /*
            Box box;
            box.a = this->a + x.a;
            box.b = this->b + x.b;
            return box;
            */
            return Box(a+x.a, b+x.b);
        }
    
        // https://www.tutorialspoint.com/cplusplus/increment_decrement_operators_overloading.htm
        // prefix ++, i.e ++pd
        Box operator++() {
            a++;
            b++;
            return Box(a,b);
        }
        
        // posfix ++, i.e pd++
        Box operator++(int) {
            a += 2;
            b++;
            return Box(a,b);
        }

        // https://www.tutorialspoint.com/cplusplus/assignment_operators_overloading.htm
        // https://stackoverflow.com/questions/1734628/copy-constructor-and-operator-overload-in-c-is-a-common-function-possible
        // https://www.geeksforgeeks.org/copy-constructor-in-cpp/
        void operator=(const Box& x) {
            a = x.a;
            b = x.b;
        }

        // https://www.tutorialspoint.com/cplusplus/function_call_operator_overloading.htm
        // operator() overloading
        Box operator() (int x, int y, int z, int t) {
            return Box(x+y,z+t);
        }

    public:
        int a;
        int b;        
};

int main() {
    Box pd1(2,3);
    Box pd2(4,5);

    Box pd3;
    // https://www.tutorialspoint.com/cplusplus/binary_operators_overloading.htm
    pd3 = pd1 + pd2;

    pd3.print("hello");

    printf("a=%d, b=%d, volume=%d\n",pd3.a, pd3.b, pd3.getVolume());
    // https://www.tutorialspoint.com/cplusplus/increment_decrement_operators_overloading.htm
    pd3++;  // operator++(int) declared for postfix
    //++pd3; // operator+() declared for prefix
    printf("a=%d, b=%d, volume=%d\n",pd3.a, pd3.b, pd3.getVolume());
    
    // https://www.tutorialspoint.com/cplusplus/assignment_operators_overloading.htm
    pd1 = pd3;
    printf("a=%d, b=%d, volume=%d\n",pd1.a, pd1.b, pd1.getVolume());
   
    // https://www.tutorialspoint.com/cplusplus/function_call_operator_overloading.htm 
    // operator() overload
    Box pd4;
    pd4 = pd1(1,2,3,4);
    printf("a=%d, b=%d, volume=%d\n",pd4.a, pd4.b, pd4.getVolume());
    printf("a=%d, b=%d, volume=%d\n",pd1.a, pd1.b, pd1.getVolume());

    return 0;
}
