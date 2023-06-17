// 11.Aug.2017
// doubly linked list using OOP

#include <stdio.h>
#include <stdlib.h>

class linkedList {
	struct Node {
		int data;
		Node* pre;
		Node* next;
	};

private:
	Node* head;

public:
	linkedList() {
		head = NULL;
	}

	// insert new node at tail
	void createNode(int data) {
		Node* tmp = new Node();
		tmp->data = data;
		tmp->next = NULL;
		tmp->pre = NULL;

		Node* tmp2 = head;
		if (tmp2 == NULL)
			head = tmp;
		else {
			while (tmp2->next != NULL) {
				tmp2 = tmp2->next;
			}

			tmp2->next = tmp;
			tmp->pre = tmp2;
		}
	}

	void printList() {
		Node* tmp = head;
		while (tmp != NULL) {
			printf("%d\n",tmp->data);
			tmp = tmp->next;
		}
	}
};

int main() {
	linkedList ll;
	ll.createNode(2);
	ll.createNode(3);
	ll.createNode(5);

	ll.printList();

	return 0;
}