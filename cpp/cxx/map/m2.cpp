#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <cstring>

using namespace std;

void findDuplicate(char st[]) {
	unordered_map<int,bool> map;
	for (int i=0; i<strlen(st); i++) {
		if (map[st[i]] == 1) // duplicate
			printf("found duplicate: %c\n",st[i]);

		map[st[i]] = 1;
	}
}

int main() {
	
	char st[]="abcxybtc";
	findDuplicate(st);

	return 0;
}