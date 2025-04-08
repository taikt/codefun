// operations of stack (LIFO-Last In First Out)
// push(i): push an item to stack
// pop(): remove an item from stack
// top(): access the top item
// size(): return the size of stack
// empty(): check the stack is empty or not

#include <cstdio>
#include <stack>
using namespace std;

int main() {

	stack<int> st;

	st.push(2);	
	st.push(3);
	st.push(4);
	printf("size of stack %d\n", st.size()); // 3
	printf("the top element is %d\n",st.top());	// 4
	printf("the top element is %d\n",st.top()); // 4
	printf("pop out an element\n");
	st.pop();
	
	printf("size of stack %d\n", st.size()); // 2
	printf("the top element is %d\n",st.top()); // 3

}
