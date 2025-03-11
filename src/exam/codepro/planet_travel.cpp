// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/6%3Fembed
// AC: monostack
#include<bits/stdc++.h>
using namespace std;
vector<int> a;
int N;
void solve() {
	int ans=N-1; // cac cap ke nhau
	// day vao stack giam dan
	// 6 3 3 4 5 2
	// => 6 3 4 => pop 3: push 4 (ans++)
	// 6 4 5 => pop 4: push 5 (ans++)
	// 6 5 2
	// ans = 5 +2 = 7
	stack<int> s;
	for(int i=0;i<N;i++){
		while(!s.empty() && a[s.top()]<a[i]){
			s.pop();
			if (!s.empty()) ans++; // 6 3 9, chi tinh 1 ket qua vi da khoi tao N-1 cho truong hop ke nhau
		}
		if (s.empty() || a[s.top()]>a[i]) s.push(i);
	}
	cout<<ans<<endl;
}
int main() {
	cin>>N;
	a.resize(N);
	for(auto &x:a) cin>>x;
	solve();
	return 0;
}