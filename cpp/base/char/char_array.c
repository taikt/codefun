#include <stdio.h>

struct table {
	char name[20];
	char value[100];
};

struct table tb;

void copy(char* name, char* value) {
	if (!strcmp(name,"test")) {
		strcpy(tb.value,value);
		printf("%x\n",tb.value[2]);
	}
}



int main() {
	char test_value[100];
	unsigned int buff[1024];

	//char buff[1000];

	printf("size of test_value %d\n",sizeof(test_value));
	printf("size of buff %d\n",sizeof(buff));

	memset(test_value,0,sizeof(test_value));
	memset(buff,0,sizeof(buff));

	strcpy(buff,"abadfxxddd");
	

	memcpy(test_value,buff,3);

	printf("test value %s\n", test_value);
	
	strcpy(tb.name,"test");
	strcpy(tb.value,test_value);
	copy("test",tb.value);

}