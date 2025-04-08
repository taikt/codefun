#include <bits/stdc++.h>
using namespace std;

int main() {
#if 0
    printf("max heap\n");
    priority_queue<int> max_pq; // default max_heap
    max_pq.push(1);
    max_pq.push(2);
    max_pq.push(1);

    while (!max_pq.empty()) {
        int top = max_pq.top(); max_pq.pop();
        printf("%d\n", top);
    }

    printf("min heap\n");
    priority_queue<int, vector<int>, greater<int>> min_pq; // min_heap
    min_pq.push(1);
    min_pq.push(2);
    min_pq.push(1);

    while (!min_pq.empty()) {
        int top = min_pq.top(); min_pq.pop();
        printf("%d\n", top);
    }

    printf("min heap - pair element\n");
    typedef pair<int,int> pii; // so sanh thanh phan dau tien truoc, neu bang nhau, so sanh thanh phan thu 2
    priority_queue<pii, vector<pii>, greater<pii>> min_pq2; // min_heap
    min_pq2.push({2,4});
    min_pq2.push({2,9});
    min_pq2.push({2,5});

    while (!min_pq2.empty()) {
        auto [a,b] = min_pq2.top(); min_pq2.pop();
        printf("%d %d\n", a, b);
    }
#endif
    // banana => 2
    // cherry => 5
    // grape => 1
    // apple => 4
    // peach => 5
    // => out
    // cherry 5
    // peach 5
    // apple 4
    // banana 2
    // grape 1
    int N, value;
    string fruit;
    cin >> N;
    map<string,int> m;
    vector<string> fruits;
    for (int i = 0; i< N; i++) {
        cin >> fruit >> value;
        m[fruit] = value;
        fruits.push_back(fruit);
    }
    auto comp = [&](string& st1, string& st2) {
        if (m[st1] < m[st2]) return true;
        else if (m[st1] > m[st2]) return false;
        else {
            return st1 > st2;
        }
    };

    priority_queue<string, vector<string>, decltype(comp)> pq(comp);
    for (auto&x: fruits) {
        pq.push(x);
    }
    cout<<"output"<<endl;
    while (!pq.empty()) {
        string tt = pq.top();
        pq.pop();
        cout<<tt<<" "<<m[tt]<<"\n";
    }

    return 0;
}