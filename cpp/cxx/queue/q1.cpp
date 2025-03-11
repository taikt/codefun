// operations of queue (FIFO-First In First Out)
// push(i): push an item to queue
// pop(): remove an item from queue
// front(): access the top item
// size(): return the size of queue
// empty(): check the stack is empty or not

#include <cstdio>
#include <queue>
using namespace std;

int main()
{
    queue<int> myq;
    
    myq.push(2);
    myq.push(5);
    
    printf("first element is %d\n", myq.front()); // 2
    printf("first element is %d\n", myq.front()); // 2
   
    myq.pop(); 
    
    printf("first element is %d\n", myq.front()); // 5
    
    myq.push(7); 

    printf("size of queue %d\n", myq.size()); // 2

    printf("first element is %d\n", myq.front()); // 5
 
    myq.pop(); 

    printf("size of queue %d\n", myq.size()); // 1
    
    return 0;
}