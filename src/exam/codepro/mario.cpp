#include <bits/stdc++.h>
using namespace std;

int main(){
	int ans = -1;

	int n;			//	Input function
    cin >> n;
	//	Write the code
    long long eat_odd = 0;
    long long eat_even = 0;
    for(int i = 0; i < n; ++i) {
        int x;
        cin >> x;
        eat_odd = max(eat_odd, eat_even + x);
        eat_even = max(eat_even, eat_odd - x);
    } 
	
	
	cout << max(eat_even, eat_odd) << endl;	//	Output answer
	return 0;
}
