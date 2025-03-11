#include <bits/stdc++.h>
using namespace std;
#include <cstdio>

const int N=5e5+10;
int n,x,a[N];

int main(){
	freopen("out","w",stdout);
	freopen("in","r",stdin);

	scanf("%d %d",&n,&x);
	for(int w,i=1;i<=n;++i)scanf("%d",&w),++a[w];
	for(int i=1;i<x;++i){
		if(a[i]%(i+1)!=0){puts("No");return 0;}
		a[i+1]+=a[i]/(i+1);
	}
	puts("Yes");return 0;
}