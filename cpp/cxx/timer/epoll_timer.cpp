// Reference
// https://stackoverflow.com/questions/63241720/how-to-use-timerfd-properly
// https://github.com/csimmonds/periodic-threads/blob/master/timerfd.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#undef PDEBUG             	/* undef it, just in case */

#define TEST_DEBUG

#ifdef TEST_DEBUG
#  define PDEBUG(fmt, args...) 	printf("DEBUG: " fmt, ## args)
#else
#  define PDEBUG(fmt, args...) 	/* not debugging: nothing */
#endif

#define handle_error(msg) \
		do { perror(msg); exit(EXIT_FAILURE); } while (0)


static int timerfd_stop(int fd)
{
	struct itimerspec new_value;

	if (fd == -1)
		return -1;

	memset(&new_value, 0, sizeof(new_value));

	if (timerfd_settime(fd, 0, &new_value, NULL) == -1)
		handle_error("timerfd_settime disarm");

//	fprintf(stdout, "[%s] timerfd : %d\n", __func__, fd);

	return 0;
}

static int timerfd_start(int fd, time_t period_sec, long period_nsec)
{
	struct itimerspec mod_val;
	struct timespec now;
	if (fd < 0)
		return -1;

	if (timerfd_stop(fd) < 0)
		return -1;

	if (period_sec > 0 || period_nsec > 0) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		mod_val.it_value.tv_sec = now.tv_sec + period_sec;
		mod_val.it_value.tv_nsec = now.tv_nsec;
		mod_val.it_interval.tv_sec = period_sec;
		mod_val.it_interval.tv_nsec = period_nsec;

		if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &mod_val, NULL) < 0)
			return -1;
	}

	PDEBUG("[%s]: set timer period sec : %ld, period nsec : %ld\n", __func__, period_sec, period_nsec);

	return 0;
}


#define MAX_EVENTS 20

struct timer_info;

struct timer_info {
	int timerfd;
	time_t period_sec;
	long period_nsec;
	void *obj;
	void (*handle_timer)(void *obj, struct timer_info *info);
	char name[64];
};

static void handle_timer_can1(void *obj, struct timer_info *info)
{
	PDEBUG("[%s]: handled, obj : %p, info->name : %s\n", __func__, obj, info->name);
}

static void handle_timer_can2(void *obj, struct timer_info *info)
{
	PDEBUG("[%s]: handled, obj : %p, info->name : %s\n", __func__, obj, info->name);
}

static void handle_timer_can3(void *obj, struct timer_info *info)
{
	PDEBUG("[%s]: handled, obj : %p, info->name : %s\n", __func__, obj, info->name);
}

static struct timer_info timer_infos[] = {
	{
		.timerfd = -1,
		.period_sec = 0,
		.period_nsec = 100 * 1000 * 1000,  /* 100ms */
		.obj = NULL,
		.name = "TIMER_CAN_1",
		.handle_timer = handle_timer_can1,
	},
	{
		.timerfd = -1,
		.period_sec = 0,
		.period_nsec = 500000000, /* 500ms */
		.obj = NULL,
		.name = "TIMER_CAN_2",
		.handle_timer = handle_timer_can2,
	},
	{
		.timerfd = -1,
		.period_sec = 1,  /* 1 sec */
		.period_nsec = 500 * 1000 * 1000, /*  500ms     */
		.obj = NULL,
		.name = "TIMER_CAN_3",
		.handle_timer = handle_timer_can3,
	}
};

const static int timer_nr = sizeof(timer_infos) / sizeof(timer_infos[0]);


static int timer_info_create(void)
{
	struct epoll_event ev;
	int epollfd;
	int i;

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < timer_nr; i++) {
		struct timer_info *info = &timer_infos[i];

		if ((info->timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC)) == -1) {
			assert(0);
		}

		ev.events = EPOLLIN;
        ev.data.ptr = info;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, info->timerfd, &ev) == -1) {
			perror("epoll_ctl: listen_sock");
			exit(EXIT_FAILURE);
       }
	}
	
	return epollfd;
}


static void timer_info_start_all(void)
{
	int i;

	for (i = 0; i < timer_nr; i++) {
		struct timer_info *info = &timer_infos[i];
		if (info->timerfd == -1)
			continue;

		timerfd_start(info->timerfd, info->period_sec, info->period_nsec);
	}
}


static void timer_info_stop_all(void)
{
	int i;

	for (i = 0; i < timer_nr; i++) {
		struct timer_info *info = &timer_infos[i];
		if (info->timerfd == -1)
			continue;

		timerfd_stop(info->timerfd);
	}
}



static void handle_timer_epoll(struct timer_info *info)
{
	int nread;
	uint64_t exp;

	//lseek(info->timerfd, 0, SEEK_SET);
	while (((nread = read(info->timerfd, &exp, sizeof(exp))) == -1) && (errno == EINTR));
	if (nread <= 0) {
		fprintf(stderr, "[%s] error !! : errno: %d\n", __func__, errno);
		return;
	}
	
	PDEBUG("MSG TYPE : %s, length : %d, %ld\n", info->name, nread, exp);

	if (info->handle_timer) {
		PDEBUG("call handle_msg!!!\n");
		info->handle_timer(info->obj, info);
	}
}


int main(void)
{
	int epollfd;
	struct epoll_event events[MAX_EVENTS];

	epollfd = timer_info_create();

	int n;
	int nfds;

	sleep(3);
	timer_info_start_all();

	while (1) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 2000);

		if (nfds == 0) {
			printf("epoll timeout !!\n");
			continue;
		}
		
		if (nfds < 0) {
			printf("epoll error !!\n");
			continue;
		}

		for (n = 0; n < nfds; n++) {
			handle_timer_epoll(events[n].data.ptr);
		}
	}

	timer_info_stop_all();

	return 0;
}
