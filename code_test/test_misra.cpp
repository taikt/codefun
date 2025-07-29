#include <cstdint>

int example(int test, int result)
{
    if (test > 5) 
    {
        test--;
        result = test + result;  
    } 
    else if (test <= 5) 			
    {
        result = test - result;   
    } 

    return result;
}

int main()
{
    int test = 10;
    int result = 0;

    result = example(test, result);
    
    return result;
}