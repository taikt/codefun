#include <sddio.h>

int main()
{

    unsigned char a;
    // 8bit, bit 7| bit 6 | bit 5... |bit 0
    // min 0000 0000 (0), max 1111 1111 (255)
    // tat ca bit bieu dien gia tri, khong co bit dau

    signed char a;
    // 8bit, bit 7| bit 6 | bit 5... |bit 0
    // bit 7 la bit dau: 1 => - (value = -2^7=-128), 0 => +)
    // min -128, max +127
    // 1/2 gia tri duong (128): 0 -> 127
    // 1/2 gia tri am trong value range (128)
    // -1 -> -128


    return 0;
}
