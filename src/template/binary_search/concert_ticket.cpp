#include "bits/stdc++.h"
using namespace std;

// https://cses.fi/problemset/task/1091
// https://usaco.guide/problems/cses-1091-concert-tickets/solution
/*
Input:
5 3
5 3 7 8 5
4 8 3
Output:
3
8
-1

5: number of tickets
3: number of customers
5 3 7 8 5: price of tickets
4 8 3: maximum pay by each customer

3: ticket price that first cuscomer buy (<= 4)
8: ticket price that second customer buy (<=8)
-1: third customer cannot buy any ticket (<=3, ticket with price 3 already bought)
*/
int n,m;
multiset<int> tickets;
void solve() {
    // for each customer, find ticket with maximum price that he can buy
    // store tickets in a multiset
    // find upper_bound of his maximum price on the set -> pos (ticket price > his maximum price)
    // --pos: return ticket with value <= maximum price

    // if finding upper_bound of maximum price returns beggining of set (means every element in set is larger than the maximum price)
    // return -1 (he cannot buy)
    cin >> n >> m;
    int h,t;
    for (int i = 0; i < n; i++) { // prices of each available ticket
        cin >> h;
        tickets.insert(h);
    }

    for (int i = 0; i < m; i++) { // maximum price of each customer
        cin >> t;
        auto it = tickets.upper_bound(t);
        if (it == tickets.begin()) {
            cout<<"-1 \n";
        } else {
            cout<<*(--it)<<"\n";
            tickets.erase(it);
        }
    }

    return;
}

int main() {
    freopen("input.txt","r",stdin);
    solve();

    return 0;
}