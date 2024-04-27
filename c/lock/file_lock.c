#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

static int fcntl_lock(int fd, int nowait) {
	struct flock lock;
	lock.l_type   = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start  = 0;
	lock.l_len    = 0;

	int ret = fcntl(fd, nowait ? F_SETLK : F_SETLKW, &lock);
	if (ret == -1) {
		printf("%s: lock %d error %s\r\n", __FUNCTION__, fd, strerror(errno));
		return -1;
	}

	printf("%s: lock ok!\r\n", __FUNCTION__);
	return 0;
}

static int flock_lock(int fd, int nowait) {
	int ret = flock(fd, nowait ? (LOCK_EX | LOCK_NB) : LOCK_EX);
	if (ret == -1) {
		printf("%s: flock %d error %s\r\n", __FUNCTION__, fd, strerror(errno));
		return -1;
	}

	printf("%s: lock ok!\r\n", __FUNCTION__);
	return 0;
}

static void usage(const char *procname) {
	printf("usage: %s -h [help]\r\n"
		" -f [use flock]\r\n"
		" -b [if using blocking lock]\r\n"
		" -o [if opening file again\r\n"
		" -c [if closing the second opened file]\r\n", procname);
}

int main(int argc, char *argv[]) {
	const char *filename = "./dummy.lock";
	int ch, use_flock = 0, nowait = 1, open_second = 0, close_second = 0;

	while ((ch = getopt(argc, argv, "hfboc")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'b':
			nowait = 0;
			break;
		case 'f':
			use_flock = 1;
			break;
		case 'o':
			open_second = 1;
			break;
		case 'c':
			close_second = 1;
			break;
		default:
			break;
		}
	}

	int fd = open(filename, O_RDWR | O_CREAT, 0600);
	if (fd == -1) {
		printf("Open %s error %s\r\n", filename, strerror(errno));
		return 1;
	}

	if (use_flock) {
		if (flock_lock(fd, nowait) == -1) {
			close(fd);
			return 1;
		}
	} else {
		if (fcntl_lock(fd, nowait) == -1) {
			close(fd);
			return 1;
		}
	}

	if (open_second) {
		int fd2 = open(filename, O_RDONLY, 0600);
		if (fd2 == -1) {
			printf("Open %s error %s\r\n", filename, strerror(errno));
			close(fd);
			return 1;
		}
		printf("Open %s ok again!\r\n", filename);

		if (close_second) {
			close(fd2);
			printf("Close the second opened\r\n");
		}
	}

	sleep(60);
	close(fd);
	return 0;
}
