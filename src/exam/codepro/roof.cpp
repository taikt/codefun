#include<bits/stdc++.h>

using namespace std;
#define ll long long 
int N;//Number of buildings
int H[80010];//Height of buildings
vector<long long> max_right(80010);
void Input_Data(void){
	cin >> N;
	for (int i = 0; i < N; i++){
		cin >> H[i];
		max_right[i] = i;
	}
}

int main(){
	long long ans = 0;
	Input_Data();		//	Input function
	//vector<long long> right(N);
	//int max_right = -1;
	// for(int i : max_right) 
	// 	cout << i << " ";
	// cout << endl;
	for(ll i = N - 2; i >= 0; --i) {
		ll j = i + 1;
		while(1) {
			if(H[i] <= H[j]) {
				ans += (ll)(j - i - 1);
				max_right[i] = j;
				break;
			}
			else {
				if(j == max_right[j]) {
					ans += (ll)(N - i - 1);
					break;
				}
				j = max_right[j];
			}
		}
	}
	// for(int i : max_right) 
	// 	cout << i << " ";
	//	Write the code
	
	
	cout << ans << endl;	//	Output right answer
	return 0;
}
