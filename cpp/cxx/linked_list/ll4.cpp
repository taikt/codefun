// 11.Aug.2017
// singly linked list using OOP

#include <stdio.h>
#include <stdlib.h>

class linkedList {
	struct Node {
		int data;
		Node* link;
	};

private:
	Node* head;

public:
	linkedList() {
		head = NULL;
	}

	void createNode(int data) {
		Node* tmp = new Node();
		tmp->data = data;
		tmp->link = NULL;

		Node* tmp2 = head;
		if (tmp2 == NULL)
			head = tmp;
		else {
			while (tmp2->link != NULL) {
				tmp2 = tmp2->link;
			}
			tmp2->link = tmp;
		}
	}

	void printList() {
		Node* tmp = head;
		while (tmp != NULL) {
			printf("%d\n",tmp->data);
			tmp = tmp->link;
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