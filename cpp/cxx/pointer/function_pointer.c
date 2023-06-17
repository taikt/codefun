#include <stdio.h>

// get_double and get_double2 are developed by company A
// get_double2 is a updated version of get_double and has same prototype with get_double

int get_double(int a) {
    return 2*a;
}

int get_double2(int a) {
    // change some thing new for better performance..
    // we only care unsigned value of a
    if (a <= 0) return 0;
    else
        return 2*a;
}

int* test2(int* a, int b) {

}

// API named 'API_in_lib_from_3rd_party' was developed by 3rd party B
// which should be consistent, no change code over time.
// this API should be independent from code change by different customers such as company A
// this API can be stored in a lib, which was supported by 3rd party B.
void API_in_lib_from_3rd_party(int x, int (*pfn)(int)) {
    printf("%d\n",pfn(x));
}

int main() {
    int (*pfn1) (int); // declare a func pointer

    // int *pfn1(int); // fail

    // pfn1 point to get_double
    pfn1 = &get_double; // or func_pointer = get_double

    // pfn2 point to test2
    int* (*pfn2) (int*,int) = &test2;

    printf("value=%d\n",(*pfn1)(2));
    printf("value=%d\n", pfn1(2)); // should use

    // note: *pfn1 = pfn1 = &get_double = get_double
    //printf("address of get_double=0x%08x, value of pfn1=0x%08x, 0x%08x\n", &get_double, pfn1, *pfn1);
    printf("address of get_double=%p,%p;  value of pfn1=%p, %p\n", &get_double, get_double, pfn1, *pfn1);

    // company A use the API named 'API_in_lib_from_3rd_party' from 3rd party B
    // pass a function pointer to the API
    API_in_lib_from_3rd_party(5,get_double);

    // sometimes later, when company A updates get_double to get_double2 for better performance
    // and want to use same API from 3rd party B without request for changing 3rd party side's code.
    // just pass a function pointer to the API again.
    // of course, if we(company A) request 3rd party B for changing code of lib, they may request us for additional fees and many confusing problems!!.
    API_in_lib_from_3rd_party(5,get_double2);

    return 0;
}
