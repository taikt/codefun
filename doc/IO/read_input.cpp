#include <bits/stdc++.h>
using namespace std;
#define loop(i,a,b) for(int i=a; i<b; i++)
int main() {
    /*
    3
    1,2,4,6
    2,5,6,7
    3,7,8,9,10

    or
    3
    1 2 4 6
    2 5 6 7
    3 7 8 9 10
    */ 
#if 0   
    int N;
    scanf("%d\n",&N); 
    string token,st;
    loop(i,0,N) {
        getline(cin,st); // get a line: i.e 1,2,4,6
        
        istringstream ss(st);
        //while(getline(ss,token,',')) { // read each token separated by ',' in above line
        while(getline(ss,token,' ')) { // read each token separated by ',' in above line
            //printf("  %s", token.c_str());
            int num = stoi(token);
            printf("%d ",num);
        }
        printf("\n");
    }
#endif

    /*
    3
    messi 10
    ronaldo 12
    perse 8
    */
    int N;
    scanf("%d\n",&N); 
    string name,st;
    int value;
    /*
    loop(i,0,N) {
        getline(cin,st); // get a line: messi 10
        
        istringstream ss(st);
        ss >> name;
        ss >> value;
        printf("name=%s, value=%d\n", name.c_str(), value);
    }
    */
    /*
    3
    messi 10 kaka
    ronaldo 12 pachele
    perse 8 hunde
    */
    /*
    string name2;
    loop(i,0,N) {
        cin >> name >> value >> name2;
            printf("name=%s, value=%d, name2=%s\n", name.c_str(), value, name2.c_str());
    }
    */

    /*
    peter hunbe convit hoho
    */
   loop(i,0,N) {
       cin >> name;
       printf("name=%s\n",name.c_str());
   }

    return 0;
}
