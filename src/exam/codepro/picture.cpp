// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/6
// adhoc: AC
#include <bits/stdc++.h>
#define int int64_t
#define INF 20
#define db(v) cout<<#v<<" "<<v<<endl;
using namespace std;
void solve(){
	int n;
	cin>>n;
	vector<vector<int>> a(n,vector<int>(n));
	map<int,vector<int>> m;
	vector<bool> mark(10,false);
	for(int i=0;i<n;i++){
		string s;
		cin>>s;
		for(int j=0;j<n;j++) {
			a[i][j]=s[j]-'0';
			mark[a[i][j]]=true;
		}
	}
	vector<int> v={INF,INF,-INF,-INF};
	for (int i=0;i<=9;i++){
		m[i]=v;
	}
	
	for (int i=0;i<n;i++)
		for (int j=0;j<n;j++){
			//m[a[i][j]] => v[0], v[1], v[2], v[3]
			// v[0] v[1] hang cot cua left one
			// v[2] v[3] hang cot cua right one
			// i<v[0]: v[0]=i
			// j<v[1]: v[1]=j
			// i>v[2]: v[2]=i
			// j>v[3]: v[3]=j
			if (a[i][j]==0) continue;
			vector<int> v=m[a[i][j]];
			v[0]=min(i,v[0]);
			v[1]=min(j,v[1]);
			v[2]=max(i,v[2]);
			v[3]=max(j,v[3]);
			m[a[i][j]] = v;
		}
	
	for (int i=0;i<n;i++)
		for (int j=0;j<n;j++){
			//tim thay toa do [i,j] trong map cua index khac 
			int cur=a[i][j];
			if (cur==0) continue;
			for (int k=1;k<=9;k++){
				vector<int> v=m[k];
				if (k!= cur){
					if (i>=v[0] && i<=v[2] && j>=v[1] && j<=v[3]) {
						mark[cur]=false;
						break;
					}
				}
			}
		}
	int cnt=0;
	for(int i=1;i<=9;i++)
		if (mark[i]) cnt++;
	cout<<cnt;
}
int32_t main() {
	solve();
	return 0;
}

/*
#include <bits/stdc++.h>
using namespace std;
#define pii pair<int, int>
vector<bool> valid(10, 1);
vector<bool> has(10);
vector<int> topX(10, 11);
vector<int> topY(10, 11);
vector<int> downX(10);
vector<int> downY(10);
int main() {
	int n;
	cin >> n;
	vector<string> s(n);
	for(string &i : s) {
			cin >> i;
	}
	for(int i = 0; i < n; ++i) {
		for(int j = 0; j < n; ++j) {
			char c = s[i][j];
			topX[c - '0'] = min(topX[c - '0'], j);
			topY[c - '0'] = min(topY[c - '0'], i);
			downX[c - '0'] = max(downX[c - '0'], j);
			downY[c - '0'] = max(downY[c - '0'], i);
			has[c - '0'] = 1;
		}
	}
	for(int i = 1; i < 10; ++i) {
		if(has[i]) {
			//cout << i << " " << topY[i] << " " << topX[i] << " " << downY[i] << " " << downX[i] << endl;
			for(int j = topY[i]; j <= downY[i]; ++j) {
				for(int k = topX[i]; k <= downX[i]; ++k) {
					int d = s[j][k] - '0';
                    assert(d != 0);
					if(d != i)
						valid[d] = 0;
				}
			}
		}
	}
	int ans = 0;
	for(int i = 1; i < 10; ++i) {
		if(valid[i] && has[i])
			++ans;
	}
	// for(int i : has) cout << i << " ";
	// cout << endl;
	// for(int i : valid) cout << i << " ";
	// cout << endl;
	cout << ans;
	//for(string &i : s) cout << i;
	return 0;
}
*/