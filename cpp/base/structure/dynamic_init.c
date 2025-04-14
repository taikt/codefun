#include <stdio.h>

typedef struct tagInfo2 {
    int e;
    int f;
} INFO2;

typedef struct tagInfo1 {
    int a;
    short int b;
    short int c;
    int d;
    INFO2 st;
} INFO1;

// minh hoa padding
// rule: phan bo 1 dia chi cho 1 bien thi dia chi do chia het cho size of datatype cua bien
typedef struct tagLayout1 {
    unsigned int uiA; //UINT32
    unsigned char ubyB; //UINIT8
    unsigned short int usC; //UINT16
    unsigned char ubyD;
    unsigned int uiE;
    unsigned char ubyF;
    unsigned char uubyG[4];
} LAYOUT1;

static void printfInfo(const char *pName, INFO1 *pst)
{
    printf("%5s> @:%p a:%02X b:%02hX c:%02hX d:%02X st2.f:%02X\n",
           pName,pst, pst->a, pst->b, pst->c, pst->d, pst->st.f);
}

static void dynamicStructInit(void)
{
    INFO1 sta, stb;
    printf("sizes> INFO2:%d INFO1:%d sta:%d LAYOUT1:%d\n",sizeof(INFO2), sizeof(INFO1), sizeof(sta),sizeof(LAYOUT1));

    memset(&sta, 0, sizeof(INFO1));
    sta.a = 0x10;
    sta.d = 0x20;
    sta.st.f = 0x30;

    printfInfo("sta",&sta);

    memset(&stb, 0, sizeof(INFO1));
    stb.b = 0x40;
    stb.c = 0x50;
    printfInfo("stb",&stb);
}
int main() {

    dynamicStructInit();

    return 0;
}
