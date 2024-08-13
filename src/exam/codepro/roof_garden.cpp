#include <bits/stdc++.h>
using namespace std;

int N;//Number of buildings
int H[80011];//Height of buildings

void Input_Data(void){
	cin >> N;
	for (int i = 0; i < N; i++){
		cin >> H[i];
	}
	H[N]=1e9;
}

int main(){
	long long ans = 0;
	Input_Data();		//	Input function
	//	Write the code
	stack<int> q;
	map<int,int> mask;
	for (int i=0;i<=N;i++){
		while(q.size()>0 && H[i]>=H[q.top()]){
			mask[q.top()]=i;
			q.pop();
		}
		q.push(i);
	}
	for(int i=0;i<N;i++){
		if (mask.count(i)){
			ans+=mask[i]-i-1;
			//cout<<i<<"->"<<mask[i]<<endl;
		}
	}
	
	cout << ans << endl;	//	Output right answer
	return 0;
}

/*
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

*/
