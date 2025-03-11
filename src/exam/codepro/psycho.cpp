// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/7
// two pointer method: AC
#include<bits/stdc++.h>
using namespace std;
#define int int64_t

void solve(){
	int n;
	cin>>n;
	vector<int> a(n);
	for(auto&x: a) cin>>x;
	int i=0,j=n-1,l=0,r=0,sum=INT64_MAX;
	while(i<j){
		if (abs(a[i]+a[j])<sum) {
			sum = abs(a[i]+a[j]);
			l=i;r=j;
		}
		if (a[i]+a[j]<0) i++;
		else if (a[i]+a[j]>0) j--;
		else {
			cout<<i<<" "<<j<<endl;
			return;
		}
	}
	cout<<l<<" "<<r<<endl;
}

int32_t main(){
	solve();
	return 0;
}

/*
#include <bits/stdc++.h>
using namespace std;

main() {
	int n;
	cin >> n;
	vector<int> tem(n);
	for(int &x : tem) cin >> x;
	int res1, res2;
	int first = 0, last = n - 1;
	int diff = 2e9 + 5;
	while(first != last) {
		if(diff > abs(tem[first] + tem[last])) {
			res1 = first;
			res2 = last;
			diff = abs(tem[first] + tem[last]);
		}
		if(abs(tem[first]) > abs(tem[last]))
			first++;
		else
			last--;
	}
	cout << res1 << " " << res2 << endl;
}
*/