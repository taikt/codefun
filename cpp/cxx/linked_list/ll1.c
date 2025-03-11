// http://www.geeksforgeeks.org/data-structures/linked-list/
// https://www.youtube.com/watch?v=vcQIFT79_50&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P&index=5

#include <stdio.h>

typedef struct node_
{
	int data;
	struct node_ *next;
} node;

node *createnode(node* head, int data)
{
	//add node to last position 
	node* p = NULL;
	node* t = NULL;
	p = (node*) malloc(sizeof(node));
	
	p->data = data;
	p->next = NULL;
		
	if (head == NULL)
	return p;
	else
	{
		for (t=head; t->next != NULL; t=t->next)
		{};
	    t->next = p;
	}
	
	return head;
}

void showlist(node* head)
{
	node *p = head;
	while (p!= NULL)
	{
		printf("%d ",p->data);
		p = p->next;
	}
}

void freelist(node* head)
{
	node* p = head;
	while (p!= NULL)
	{
		p = p->next;
		free(p);
	}
	printf("HEAD:%d",head->data);
	free(head);
	//printf("HEAD:%d",head->data);
}

int main()
{
	node* head = NULL;
	head = createnode(head,10);
	head = createnode(head,20);
	head = createnode(head,30);
	showlist(head);
	freelist(head);
}
