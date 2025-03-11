// greedy method: AC
// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/4
#include <bits/stdc++.h>
#define db(x) cout<<#x<<": "<<x<<endl;
#define int int64_t
using namespace std;
void solve() {
	const int size=10;
	int n;
	cin>>n;
	vector<int> a(size);
	for(int &x:a) cin>>x;
	vector<int> v={1,5,10,50,100,500,1000,3000,6000,12000};
	int total=0;//tong full packets
	for(int i=0;i<size;i++) total+= v[i]*a[i];
	int cur=total;
	cur-=n;//luong con lai can xoa (bo qua n)
	for(int i=size-1; i>=0; i--){
		if (cur>0){
			//max cnt[i] <= a[i] that will remove cnt[i]*v[i]
			int t=cur/v[i];
			t=min(t,a[i]); // luong packet se xoa
			a[i]=a[i]-t;// so packet con lai ko xoa
			cur=cur-t*v[i];
		}
	}
	int ans=0;
	for(int &x:a) ans+=x;
	cout<<ans<<endl;
	for(auto &x:a){
		cout<<x<<" ";
	}
}

int32_t main() {
    solve();
    return 0;
}

/*
#include <bits/stdc++.h>
using namespace std;
vector<int> cnt(10);
vector<int> res(10, 0);
vector<int> value = {1, 5, 10, 50, 100, 500, 1000, 3000, 6000, 1200};
vector<int> can_reach;
int cntB = 0;
void solve(int x) {
	if(x <= 0)
		return;
	int index = -1;
	for(int i = 0; i < 10; ++i) {
		if(can_reach[i] == x) {
			for(int j = 0; j <= i; ++j) {
				res[j] = cnt[j];
			}
			return;
		}
		else if(can_reach[i] > x) {
			index = i;
			break;
		}
	}
	if(index == 0) {
		res[index] = x;
		cntB += res[index];
	}
	else 
	{
		res[index] = (x - index?can_reach[index - 1]:0 - 1) / value[index] + 1;
		cntB += res[index];
		solve(x - res[index] * value[index]);
	}
}
main() {
	int n;
	cin >> n;
	int sum = 0;
	for(int i = 0; i < 10; ++i) {
		cin >> cnt[i];
		sum += cnt[i] * value[i];
		can_reach.push_back(sum);
	}
	solve(n);
	cout << cntB << endl;
	for(int i : res)
		cout << i << " ";
}
*/