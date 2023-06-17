// priority queue
// Commonly in a queue, ordering will be FIFO
// sometimes, ordering needs priority, that doesn't care who went to the queue first
// but consider about priority of element
// underlying data structure is Binary Heap
// for Maxheap, the priority is based on maximum item (at root, parent > left_child, right_child)
// for MinHeap, the priority is based on minimum item (at root, parent < left_child, right_child)

// complexity of below operations is O(log(n))

// push(i): push an item to priority queue 
// pop(): remove the top element 
// top(): access the top item
// size(): return the size of queue
// empty(): check the stack is empty or not

// http://quiz.geeksforgeeks.org/binary-heap/
// https://en.cppreference.com/w/cpp/container/priority_queue

#include <cstdio>
#include <queue>
using namespace std;

int main()
{
#if 0  //MaxHeap  
    priority_queue<int> myq;
    
    myq.push(2);
    myq.push(5);
    
    printf("top element is %d\n", myq.top());
    printf("top element is %d\n", myq.top());
   
    myq.pop();
    
    printf("top element is %d\n", myq.top());
    
    myq.push(7);

    printf("size of queue %d\n", myq.size());

    printf("top element is %d\n", myq.top());
 
    myq.pop();

    printf("size of queue %d\n", myq.size());
#endif

#if 1 // MinHeap
    priority_queue<int, vector<int>, greater<int> > myq;
    
    myq.push(2);
    myq.push(5);
    
    printf("top element is %d\n", myq.top());
    printf("top element is %d\n", myq.top());
   
    myq.pop();
    
    printf("top element is %d\n", myq.top());
    
    myq.push(7);

    printf("size of queue %d\n", myq.size());

    printf("top element is %d\n", myq.top());
 
    myq.pop();

    printf("size of queue %d\n", myq.size());
#endif    
    return 0;
}