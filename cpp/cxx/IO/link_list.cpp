#include <bits/stdc++.h>
using namespace std;
#define FOR(i,a,b) for(int i=a; i<b; i++)
/**
 * Definition for singly-linked list.
 */

/*
 struct ListNode {
      int val;
      ListNode *next;
      ListNode(int x) : val(x), next(NULL) {}
  };
*/


template <typename T>
struct ListNode {
      T val;
      ListNode *next;
      ListNode(T x) : val(x), next(NULL) {}
};


#if 1
// utility function that converts contents of std::vector to linked list
template <typename T>
ListNode<T>* vectorToList(vector<T> numbers){
    if(numbers.size() <= 0){
        return nullptr;
    }
    ListNode<T>* head = new ListNode<T>(numbers[0]);
    ListNode<T>* runner = head;
    for (int i = 1; i < numbers.size(); i++ ){
        ListNode<T>* temp = new ListNode<T>(numbers[i]);
        runner->next = temp;
        runner = runner->next;
    }
    return head;
}
#endif // 0


class Solution {
public:
    void printList(ListNode<double>* l) {
        ListNode<double> *temp = l;

        while (temp != nullptr) {
            cout<<temp->val<<" ";
            temp = temp->next;
        }
        cout<<"\n";
    }
};

int main() {
    freopen("link_list.inp","r",stdin);
    freopen("link_list.out","w",stdout);
    /*
    ListNode* l=nullptr;
    int val;
    string st;
    getline(cin,st);
    stringstream str(st);

    while(str>>val) {
        //scanf("%d",&val);
        ListNode* node = new ListNode(val);
        if (l == nullptr) {
            l = node;
        } else {
            ListNode *temp = l;
            while (temp->next != nullptr) temp = temp->next;
            temp->next = node;
        }
    }
    */
    vector<double> l1 = {1.2,7,3,6,5,4,2};

    ListNode<double>* l;
    l = vectorToList(l1);
    Solution su;
    su.printList(l);

    return 0;
}
