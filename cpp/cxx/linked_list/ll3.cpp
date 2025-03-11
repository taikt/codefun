// 6-Aug.2017

// https://www.youtube.com/watch?v=vcQIFT79_50   => must check
// https://stackoverflow.com/questions/22141477/simple-linked-list-c


#include <stdio.h>
#include <stdlib.h>

typedef struct _Node {
	int data;
	_Node * link;
} Node;

Node* createNode1(Node* head, int data) {
	// create a node on Heap
	Node* temp = new Node();
	(*temp).data = data; // temp->data = data
	(*temp).link = NULL; // temp->link = NULL;

	if (head == NULL)
		return temp;
	else {
		Node* a = head;
		while ((*a).link != NULL) {
			a = (*a).link;
		}
		(*a).link = temp;
		return head;
	}
}

void createNode2(Node** head, int data) {
	// create a node on Heap
	Node* temp = new Node();
	(*temp).data = data; // temp->data = data
	(*temp).link = NULL; // temp->link = NULL;

	if ((*head) == NULL)
		(*head) = temp;
	else {
		//Node** a = head;
		//Node* b = *a;
		Node* b = *head;
		while ((*b).link != NULL) {
			b = (*b).link;
		}
		(*b).link = temp;
	}
}

void printList(Node* head) {
	Node* a = head;

	while (a != NULL) {
		printf("%d \n",(*a).data);
		a = (*a).link;
	}
}

int main() {

	Node* A; // A points to root node, A is a pointer variable on stack memory,
	// other nodes of linked list including root node stay on Heap memory
	// value of A is fixed, always store address of root node.

	A = NULL;
	A = createNode1(A,2);
	A = createNode1(A,3);
	printf("first list\n");
	printList(A);


	Node* B;
	B = NULL;
	createNode2(&B,4);
	createNode2(&B,6);
	createNode2(&B,9);
	createNode2(&B,10);
	createNode2(&B,11);
	printf("\nsecond list\n");
	printList(B);


}
