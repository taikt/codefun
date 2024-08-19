// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/13
#include <bits/stdc++.h>
using namespace std;
void test_case() {
	string S, C;
	cin >> S >> C;
	stack<char> first, second;
	for(char x : S) {
		first.push(x);  // Khởi tạo con trỏ nằm cuối, các ký tự nằm hết trong stack đầu
	}
	for(auto& x : C) {
		if(x == 'L') {
			if(first.empty())
				continue;
			char c = first.top();  // Thêm ký tự đầu stack 1 sang stack 2
			first.pop();
			second.push(c);
		}
		else if(x == 'R') {
			if(second.empty()) 
				continue;
			char c = second.top(); // Thêm ký tự đầu stack 2 sang stack 1
			second.pop();
			first.push(c);
		}
		else if(x == 'B') {
			if(first.empty())
				continue;
			first.pop(); // Xóa ký tự đầu stack 1
		}
		else {
			first.push(x); // Thêm ký tự vào đầu stack 1
		}
	}
	string ans = "";
	while(!first.empty()) {
		ans += first.top();
		first.pop();
	}
	reverse(ans.begin(), ans.end());  // Khi lấy ký tự từ stack 1 ra, thứ tự bị đảo ngược, ta cần đảo lại chuỗi
	while(!second.empty()) {
		ans += second.top();
		second.pop();
	}
	cout << ans << endl;
}
int main() {
	test_case();
	return 0;
}