/*
 *  Ref: libusb
 *
 *
 *
 * */

#include <stdio.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG
#define log(fmt,...) \
    do { \
        char str[100]; \
        sprintf(str,fmt,## __VA_ARGS__); \
        fprintf(stderr,"[%s] [%s:%d] %s\n",__FILE__,__func__,__LINE__,str); \
    } while (0)
#else
#define log(...)
#endif


struct usbi_os_backend {
    const char * name;
    int (*init) (int a);
    int (*exit) ();
    int (*open) (int b);
    void (*close) ();
};

int init_func (int a) {
    log("a=%d",a);
}

int exit_func () {
    log("");
}

int open_func (int a) {
    log("a=%d",a);
}

void close_func () {
    log("");
}

int main() {
    const struct usbi_os_backend usbi_backend = {
        .init = init_func,
        .exit = exit_func,
        .open = open_func,
        .close = close_func,
    };
    
    usbi_backend.init(2);
    usbi_backend.open(3);
    usbi_backend.close();
}
