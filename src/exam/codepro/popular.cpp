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