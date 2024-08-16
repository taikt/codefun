// greedy method: AC
#include <bits/stdc++.h>
#define int int64_t
#define db(x) cout<<#x<<":"<<x<<endl;
using namespace std;
void solve(){
	int n;
	cin>>n;
	vector<int> a(n);
	for(auto &x: a) cin>>x;
	int up=0,down=0,sum=a[0],i=0;
	
	while(i<n){
		while(a[i]>=a[i+1] && i<n-1){
			i++;
		}
		down=i;
		//db(down);
		while(a[i]<a[i+1] && i<n-1){
			i++;
		}
		up=i;
		//db(up);
		sum=sum-a[down];
		sum=sum+a[up];
		i++;
	}
	cout<<sum;
}

int32_t main() {
	solve();
	return 0;
}
/*
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
*/
