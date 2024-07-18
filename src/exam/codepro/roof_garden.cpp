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