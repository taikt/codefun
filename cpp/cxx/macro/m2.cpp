//https://gcc.gnu.org/onlinedocs/cpp/Macros.html

// Token-pasting operator(##)
// https://msdn.microsoft.com/en-us/library/09dwwt6y.aspx

#include <stdio.h>
#define paster( n ) printf( "token" #n " = %d", token##n )
int token9 = 9;

int main()
{
   paster(9);
}