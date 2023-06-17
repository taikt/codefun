typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;


static void fillArray(uint8_t *arr, uint8_t size, uint8_t startVal)
{
    for (int i=0; i< size; i++) {
        arr[i] = startVal;
        startVal++;
    }
}

static void displayMemory(const char* st, uint8_t* arr, uint8_t size)
{
    printf("\n%s @>%p:",st,arr);
    for (int i=size-1; i>=0; i--)
        printf("%02X, ",arr[i]);
    printf("\n");
}

int main()
{
    uint8_t array[24];
    uint16_t *p; // 2 byte 04X, 4 chu so hexa, 2 chu so tao 1 byte
    uint32_t *q; // 4 byte 08X
    uint8_t *r; // 1 byte 02X

    fillArray(array,sizeof(array),0x01);
    displayMemory("fill", array,sizeof(array)); // truyen 1 String literal

    p = (uint16_t*)&array[0];
    //p = &array[0];
    q = (uint32_t*)&array[4];
    r = (uint8_t*)&array[8];

    printf("*p = 0x%04X\n",*p);
    *p = *p+2;
    printf("*p = 0x%04X\n",*p);
    printf("*q = 0x%08X\n",*q);
    //printf("*p = 0x%02X\n",(uint32_t)*r);
    printf("*r = 0x%02X\n",*r);

    // test pointer navigation
    // read 0x08070605 using p and r
    printf("\ntest pointer navigation\n");
    printf("navigate from p: 0x%08X\n", *((uint32_t*)p +1));
    printf("navigate from p: 0x%08X\n", *(uint32_t*)(p +2));
    printf("navigate from p: 0x%08X\n", *(p +2));
    printf("navigate from p: 0x%08X\n", *(uint32_t*)((uint8_t*)p + 4));
    printf("navigate from r: 0x%08X\n", *((uint32_t*)r -1));

    // test pointer indexing
    // base[offset]
    // exp1[exp2] = *(exp1+exp2) = *(exp2+exp1) = exp2[exp1] ==> i.e *((int*)k + 4) = ((int*)k)[4]
    // p[i] = *(p+i) = *(i+p) = i[p]
    printf("\ntest pointer indexing\n");
    displayMemory("fill", array,sizeof(array));
    p = (uint16_t*)&array[0];
    printf("*p = 0x%04X\n",*p);
    printf("*(p+2)=0x%04X 0x%04X 0x%04X 0x%04X 0x%04X\n",*(p+2),p[2],(p+1)[1],*(2+p),2[p]);

    // p[-i] = *(p-i)
    p = (uint16_t*)&array[4];
    printf("*p = 0x%04X\n",*p);
    printf("*(p-1) = 0x%04X 0x%04X\n",*(p-1), p[-1]);


    return 0;
}

