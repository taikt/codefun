// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/12
// prefix sum + permutation: AC
#include <bits/stdc++.h>
#define db(x) cout<<#x<<"="<<x<<endl;
using namespace std;
vector<int> a;
int n,m;
void solve(){
	// xay dung bang prefix
	// prefix[x][i]: so phan tu x tu vi tri 0 toi vi tri i
	vector<vector<int>> prefix(10, vector<int>(n,0));
	prefix[a[0]][0]=1;
	for (int i=1;i<=m;i++){ // i chi so product hien tai
		if (a[0]==i) prefix[i][0]=1;
		for(int j=1;j<n;j++){
			if (a[j]==i){
				prefix[i][j]=prefix[i][j-1]+1;
			} else prefix[i][j]=prefix[i][j-1];
		}
	}
	//db(prefix[1][0]);
	vector<int> v;
	int ret=1e9;
	for(int i=1;i<=m;i++) v.push_back(i);
	do{
		int start=0, end=-1;
		int cnt=0;
		for (int i=0;i<m;i++){
			// xu ly moi v[i]
			// input: 1 2 2 2 1 2 1
			// tong so 1 la 3, tong so 2 la 4
			// vd (1 2)
			// 1 1 1 2 2 2 2
			// ans = tong so 1 - so luong 1 trong doan [0,2]
			//     + tong so 2 - so luong 2 trong doan [3,6]
			end+=prefix[v[i]][n-1];
			int beg=0;
			if (a[start]==v[i]) beg=1;
			cnt+=prefix[v[i]][n-1] - (prefix[v[i]][end]-prefix[v[i]][start]+beg); // tong so v[i] - so luong v[i] trong doan [start,end]
			start=end+1;
		}
		ret = min(ret,cnt);
	}while(next_permutation(v.begin(),v.end()));
	cout<<ret<<endl;
}
int main() {
	cin>>n>>m;
	a.resize(n);
	for(auto &x:a) cin>>x;
	solve();
	return 0;
}