#include <bits/stdc++.h>
using namespace std;

#define FOR(i,a,b) for(int i=a; i<b; i++)
#define N 8

class Solution {
public:
    int repeatedArithmethicShift(int x, int count) {
        FOR(i,0,count) {
            x>>=1;
            cout<<"shift right:"<<i<<"\n";
            cout<<bitset<N>(x)<<"["<<x<<"]"<<"\n";
        }
        return x;
    }
    // TODO: no support for logical shift in c++?
    #if 0
    int repeatedLogicalShift(int x, int count) {
        FOR(i,0,count) {
            x = x >>> 1;
            cout<<"shift right:"<<i<<"\n";
            cout<<bitset<8>(x)<<"["<<x<<"]"<<"\n";
        }
        return x;
    }
    #endif // 0

    // mask: set 1 at position p (count from rightmost with 0), others are 0
    // 0000 1000, p=3
    int setOne(int p) {
        int x=1<<p; // 0000 1000
        return x;
    }
    // mask: all zeros, followed by p ones.
    // 0000 0111
    int allZeroFollowedbyPones(int p) {
        int x = setOne(p)-1; // 0000 0111
        return x;
    }

    // mask: all ones, followed by p zeros.
    // 1111 1000
    int allOnesFollowedbyPzeros(int p) {
        int x = ~(allZeroFollowedbyPones(p)); // 1111 1000
        return x;
    }

    // mask: middle ones: p ones, followed by q zero
    // 0000 0111 => << q
    // 0001 1100
    int middleOnes(int p,int q) {
        int x = allZeroFollowedbyPones(p); // 0000 0111
        return x<<q; // 0001 1100
    }

    // clears rightmost p bits of N
    // 1101 1101 => 1101 1000 (clear 3 rightmost bits)
    // method: create mask: 1111 1000
    // n &= mask;
    // out: 1101 1000
    int clearRightmostPbits(int p,int n) {
        int mask = allOnesFollowedbyPzeros(p); // 1111 1000
        return n&mask;
    }

    // insert P ones to right of N
    // N=0110 1011, p=3 => 0110 1111
    // method: create mask: 0000 0111
    // n |=mask;
    int insertPonestoRight(int p,int n) {
        int mask = allZeroFollowedbyPones(p); //0000 0111
        return (n|=mask);
    }
};


int main() {
    //freopen("","r",stdin);
    freopen("bits.out","w",stdout);
    //int x=-75; // check negative x=-75
    //cout<<"x="<<bitset<N>(x)<<"["<<x<<"]"<<"\n";
    Solution a;
    //a.repeatedArithmethicShift(x,8);
    //a.repeatedLogicalShift(x,8);
    //a.setOne(3);
    //a.allZeroFollowedbyPones(3);
    //a.allOnesFollowedbyPzeros(3);
    //a.clearRightmostPbits(3,75);
    //cout<<"N="<<bitset<N>(75)<<", clear rightmost p bits="<<bitset<N>(a.clearRightmostPbits(3,75))<<"\n";
    //cout<<"N="<<bitset<N>(75)<<", inst P ones To Right="<<bitset<N>(a.insertPonestoRight(3,75))<<"\n";
    cout<<bitset<N>(a.middleOnes(3,2));

    return 0;
}
