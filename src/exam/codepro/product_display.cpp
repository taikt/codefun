// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/1

int CalLargestBlock(int id_ignore){
	int max_block_size = 0;
	int cur_size = 1;
	int last=ID[0];
	for (int i = 1; i < N; i++){
		if (ID[i]==id_ignore) continue;
		if (ID[i] == last) cur_size++;
		else {
			cur_size = 1;
			last=ID[i];
		}
		
		if (max_block_size < cur_size) {
			max_block_size = cur_size;
		}
	}
	return max_block_size;
}

/*
#include<bits/stdc++.h>
#define db(x) cout<<#x<<"="<<x<<endl;
int CalLargestBlock(int id_ignore){
	vector<int> v;
	int max_block_size = 0;
	int cur_size = 1;
	for(int i=0;i<N;i++){
		if (id_ignore == ID[i]) continue;
		v.push_back(ID[i]);
	}
	int m=v.size();
	for (int i = 1; i < m; i++){
		if (v[i] == v[i - 1]) cur_size++;
		else cur_size = 1;
		
		if (max_block_size < cur_size) {
			max_block_size = cur_size;
		}
	}
	return max_block_size;
}
*/