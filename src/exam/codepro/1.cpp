#include <iostream>
#include <bits/stdc++.h>
using namespace std;

typedef struct st
{
	int id;
	int prior;
};


void Input_Packet(st& p)
{
	cin >> p.id >> p.prior; // read packet_no, prior_level
}

#define SUCCESS				(0)
#define ERROR_PUT_PACKET	(-1)
#define ERROR_BUF_EMPTY		(-2)




int main(void)
{
	int N;

	cin >> N;	// read number of packet
	
	vector<st> p;
	for (int i = 0; i < N; i++)
	{
		st packet;
		Input_Packet(packet);
		p.push_back(packet);
	}
	sort(p.begin(), p.end(), [](st a, st b) {
		if(a.prior == b.prior)
			return a.id < b.id;
		return a.prior < b.prior;
	});
	// print packet processing order 
	for(auto i : p) {
		cout << i.id << " ";
	}

	return 0;
}
