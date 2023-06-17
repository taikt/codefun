/*  Ref:
 *  libusb/libusb/io.c:2240 handle_events
 *  libusb/libusb/hotplug.c: usbi_hotplug_notification
 *  libusb/hotplug.c: usbi_hotplug_match_cb
 *  libusb/hotplug.c: libusb_hotplug_register_callback
 * */

#include <stdlib.h>
#include <stdio.h>
#include "list.h"

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



struct list_head hotplug_msgs;
struct list_head hotplug_cbs;
enum hotplug_flags {
    NEEDS_FREE = 0,
    DEVICE_ARRIVED,
    DEVICE_LEFT=4,
};

struct hotplug_message {
    int event;
    char content[20];
    struct list_head list;
};

typedef int (*hotplug_callback_fn) (void* user_data);

struct hotplug_callback {
    int flag;
    hotplug_callback_fn cb;
    void* user_data;
    struct list_head list;
};

int hotplug_register_callback(int flag, hotplug_callback_fn cb_fn, void* user_data) {
    struct hotplug_callback *new_callback;

    new_callback = calloc(1, sizeof(*new_callback));
    new_callback->flag = flag;
    new_callback->cb = cb_fn;
    new_callback->user_data = user_data;

    list_add(&new_callback->list, &hotplug_cbs);
}

void hotplug_match_cb(int event, struct hotplug_callback *hotplug_cb) {
    if ((hotplug_cb->flag != event)) return 0;

    return hotplug_cb->cb(hotplug_cb->user_data);
}


void hotplug_match(int event) {
    struct hotplug_callback *hotplug_cb, *next;
    
    list_for_each_entry_safe(hotplug_cb, next, &hotplug_cbs, list, struct hotplug_callback) {
        if (hotplug_cb->flag & NEEDS_FREE) {
            /* process deregistration in usbi_hotplug_deregister() */
            continue;
        }    

        hotplug_match_cb(event, hotplug_cb);
        
        /* 
         * do this if callback fb is handled fail
         */
#if 0
        list_del(&hotplug_cb->list);

        free(hotplug_cb);
#endif
    }
}

int hotplug_device_cb(void* user_data) {
    log("usb attach handle");
    //printf("usb attach handle\n");
}

int hotplug_device_cb_detach(void* user_data) {
    log("usb deattach handle");
    //printf("usb deattach handle\n");
}

int main() {
    int i;

    list_init(&hotplug_msgs); 
    list_init(&hotplug_cbs); 
    
    hotplug_register_callback(DEVICE_ARRIVED, hotplug_device_cb, NULL);
    hotplug_register_callback(DEVICE_LEFT, hotplug_device_cb_detach, NULL);
    
    //usb_connect, msg.event = DEVICE_ARRIVED
    //usb_diconnect, msg.event = DEVICE_LEFT
    // create random messages, which is assummly raised from HW detector
    for (i=0; i<10; i++) {
        struct hotplug_message *message = calloc(1, sizeof(*message));
        message->event = i;
        sprintf(message->content,"test%d",i);

        list_add_tail(&message->list, &hotplug_msgs);
    }

    // pull data
     /* process the hotplug messages, if any */
    while (!list_empty(&hotplug_msgs)) {
        struct hotplug_message *message = list_first_entry(&hotplug_msgs, struct hotplug_message, list);
        
        log("message: event=%d, content=%s",message->event, message->content);
        if (message->event == DEVICE_ARRIVED) log("message: usb connect\n");
        else if (message->event == DEVICE_LEFT) log("messgage: usb disconnect\n");
        else log("message: nothing\n");

        hotplug_match(message->event);

        list_del(&message->list);
        free(message);
    }


    return 0;
}
