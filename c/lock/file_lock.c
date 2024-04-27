#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static int fcntl_lock(int fd) {
	struct flock lock;

	memset(&lock, 0, sizeof(lock));
	lock.l_type   = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start  = 0;
	lock.l_len    = 0;

	int ret = fcntl(fd, F_SETLKW, &lock);
	if (ret == -1) {
		printf("Lock %d error %s\r\n", fd, strerror(errno));
		return -1;
	}

	printf("%s: lock ok!\r\n", __FUNCTION__);
	return 0;
}

static int flock_lock(int fd) {
	int ret = flock(fd, LOCK_EX);
	if (ret == -1) {
		printf("flock %d error %s\r\n", fd, strerror(errno));
		return -1;
	}

	printf("%s: lock ok!\r\n", __FUNCTION__);
	return 0;
}

static void usage(const char *procname) {
	printf("usage: %s -k [use flock] -o [if open file\r\n", procname);
}

int main(int argc, char *argv[]) {
	const char *filename = "./dummy.lock";
	int ch, use_flock = 0, open_read = 0;

	while ((ch = getopt(argc, argv, "hko")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'k':
			use_flock = 1;
			break;
		case 'o':
			open_read = 1;
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
		if (flock_lock(fd) == -1) {
			close(fd);
			return 1;
		}
	} else {
		if (fcntl_lock(fd) == -1) {
			close(fd);
			return 1;
		}
	}

	int fd2 = -1;
	if (open_read) {
		//fd2 = open(filename, O_RDWR, 0600);
		fd2 = open(filename, O_RDONLY, 0600);
		if (fd2 == -1) {
			printf("Open %s error %s\r\n", filename, strerror(errno));
			close(fd);
			return 1;
		}
		printf("Open %s ok!\r\n", filename);
	}

	sleep(10);

	close(fd);
	close(fd2);
	return 0;
}
