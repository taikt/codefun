#include <bits/stdc++.h>
using namespace std;
#define debug(x) cout << #x << ": " << x << endl;
int N;//the number of test cases
int B;//number system
string S;//first integer
string D;//second integer
unordered_map<char, int> char_to_int;
unordered_map<int, char> int_to_char;
void InputData(){
	cin >> B >> S >> D;
}

string add(string &s1, string &s2) {
	int d = 0;
	string ans = "";
	int i = s1.length() - 1;
	int j = s2.length() - 1;
	while(1) {
		if(i < 0 && j < 0)
			break;
		int k = 0;
		k += i < 0 ? 0 :  char_to_int[s1[i]];
		k += j < 0 ? 0 :  char_to_int[s2[j]];
		ans += int_to_char[(k+d) % B];
		d = (k + d) / B;
		--i;
		--j;
	}
	while(d) {
		ans += int_to_char[d%B];
		d /= B;
	}
	reverse(ans.begin(), ans.end());
	return ans;
}
string multi(string &s,char c) {
	int d = 0;
	string ans = "";
	for(int i = s.length() - 1; i >= 0; --i) {
		int k = char_to_int[c] * char_to_int[s[i]];
		ans += int_to_char[(k + d) % B];
		d = (k + d) / B;
	}
	while(d) {
		ans += int_to_char[d%B];
		d /= B;
	}
	reverse(ans.begin(), ans.end());
	return ans;
}
void solve() {
    if(S == "0" || D == "0") {
        cout << '0' << endl;
        return;
    }
	string ans;
	bool sign1 = 0;
	bool sign2 = 0;
	if(S[0] == '-') {
		S.erase(S.begin());
		sign1 = 1;
	}
	if(D[0] == '-') {
		D.erase(D.begin());
		sign2 = 1;
	}
	int n = S.size();
	int m = D.size();
	string addition = "";
	for(int i = n - 1; i >= 0; --i) {
		string k = multi(D, (char)S[i]);
		k += addition;
		ans = add(ans, k);
		addition += '0';
	}
	if(sign1 != sign2) cout << '-';
	//valid(ans);
	cout << ans << endl;
}
int main(){

	string s = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int i = 0;
	for(char c : s) {
		char_to_int[c] = i;
		int_to_char[i] = c;
		i++;
	}
	//B = 10;
	//cout << add("1", "2");
	scanf("%d", &N);
	for(i = 0; i < N; i++){
		InputData();//input function
		//	write the code
		solve();
	}
	return 0;
}
