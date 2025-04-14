// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/14
// Priority queue: AC
#include <bits/stdc++.h>
using namespace std;
int n,m;
map<string,int> order;
map<string,int> score;
void solve(){
	priority_queue<tuple<int,int,string>> q; //max heap
	for (auto &[x,p]:score){
		q.push({p,-order[x],x}); // phan tu 1: score, phan tu 2: order, phan tu 3: name
		// sap xep max heap theo score truoc, order thu 2 (dao dau gia tri vi uu tien order nho)
	}
	int cnt=0;
	while(!q.empty() && cnt<3){
		auto cur=q.top();
		q.pop();
		cout<<get<2>(cur)<<" "<<get<0>(cur)<<endl;
		cnt++;
	}
}
int main() {
	cin>>n;
	for(int i=0;i<n;i++){
		string x;
		cin>>x;
		order[x]=i;
	}
	cin>>m;
	for(int i=0;i<m;i++){
		string x;
		int p;
		cin>>x>>p;
		if (order.count(x)){
			score[x]+=p;
		}
	}
	solve();
	return 0;
}

/*
// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/14
// sort: AC
#include <bits/stdc++.h>
using namespace std;
int n,m;
map<string,int> order;
map<string,int> score;
void solve(){
	vector<pair<int,string>> v; 
	for (auto &[x,p]:score){
		v.push_back({p,x});
	}
	sort(v.begin(),v.end(),[&](pair<int,string>&i, pair<int,string>&j){
		if (i.first==j.first){
			return (order[i.second]<order[j.second]);
		}
		return (i.first>j.first);
	});
	for(int i=0;i<3;i++)
		cout<<v[i].second<<" "<<v[i].first<<endl;
	
}
int main() {
	cin>>n;
	for(int i=0;i<n;i++){
		string x;
		cin>>x;
		order[x]=i;
	}
	cin>>m;
	for(int i=0;i<m;i++){
		string x;
		int p;
		cin>>x>>p;
		if (order.count(x)){
			score[x]+=p;
		}
	}
	solve();
	return 0;
}
*/

/*
#include <bits/stdc++.h>
using namespace std;
void test_case() {
	unordered_map<string, int> name_to_id;  
	unordered_map<int, string> id_to_name;
	int n;
	cin >> n;
	for(int i = 0; i < n; ++i) {
		string name;
		cin >> name;
		name_to_id[name] = i;
		id_to_name[i] = name;
	}
	int m;
	cin >> m;
	vector<int> point(n);
	for(int i = 0; i < m; ++i) {
		string name;
		int p;
		cin >> name >> p;
		if(!name_to_id.count(name)) // Nếu tên ứng viên không hợp lệ thì bỏ qua
			continue;
		int id = name_to_id[name];
		point[id] += p;
	}
	vector<int> id(n);
	iota(id.begin(), id.end(), 0);
	sort(id.begin(), id.end(), [&](int x, int y) {
		if(point[x] == point[y])
			return x < y;	// Nếu 2 người có điểm số bằng nhau, chọn người có id thấp hơn
		return point[x] > point[y];
	});
	for(int i = 0; i < 3; ++i) {
		cout << id_to_name[id[i]] << " " << point[id[i]] << endl;
	}
}
int main() {
	test_case();
	return 0;
}
*/