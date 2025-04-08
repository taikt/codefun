/*
24.Aug.2017
Implement stack in C++
*/
#include <stdio.h>

struct Node {
	int data;
};

class _stack {

public:
	_stack (int size) {
		buff = new Node[size];
		curr = 0;
	}

	void push(int data) {
		buff[curr].data = data;
		curr++;
	}

	void pop() {
		curr--;
	}

	int top() {
		return buff[curr-1].data;
	}

	int size() {
		return curr;
	}

private:
	Node* buff;
	int curr;
};

int main() {
	_stack st(10);
	st.push(2);
	st.push(3);
	st.push(4);
	printf("top is %d\n",st.top());
	st.pop();	
	printf("top is %d\n",st.top());

	return 0;
}