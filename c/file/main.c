#define	_GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include <liburing.h>

static double stamp_sub(const struct timeval *from, const struct timeval *sub)
{
	struct timeval res;

	memcpy(&res, from, sizeof(struct timeval));

	res.tv_usec -= sub->tv_usec;
	if (res.tv_usec < 0) {
		--res.tv_sec;
		res.tv_usec += 1000000;
	}

	res.tv_sec -= sub->tv_sec;

	return (res.tv_sec * 1000.0 + res.tv_usec / 1000.0);
}

//////////////////////////////////////////////////////////////////////////////

static int __open_wflags = O_WRONLY | O_APPEND | O_CREAT;
static int __open_rflags = O_RDONLY;

//////////////////////////////////////////////////////////////////////////////

static void sys_write(const char *path, size_t count)
{
	int    fd = open(path, __open_wflags, 0600), ret;
	char   buf[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	size_t i;

	if (fd < 0) {
		printf("%s: open %s error %s\r\n", __FUNCTION__, path, strerror(errno));
		exit(1);
	}

	printf("%s: open %s ok, fd=%d\r\n", __FUNCTION__, path, fd);

	for (i = 0; i < count; i++) {
		if (write(fd, &buf[i % 10], 1) <= 0) {
			printf("write to %s error %s\r\n", path, strerror(errno));
			exit(1);
		}
		if (i < 10) {
			printf("%s: write char=%c\r\n", __FUNCTION__, buf[i % 10]);
		}
	}

	ret = write(fd, "\r\n", 2);
	if (ret <= 0) {
		printf("write CRLF error %s\r\n", strerror(errno));
	}

	ret = close(fd);
	printf("close %s %s, fd=%d\r\n", path, ret == 0 ? "ok" : "error", fd);
}

static void sys_read(const char *path, size_t count)
{
	int    fd = open(path, __open_rflags, 0600), ret;
	char   buf[1];
	size_t i;

	if (fd < 0) {
		printf("%s: open %s error %s\r\n", __FUNCTION__, path, strerror(errno));
		exit(1);
	}

	printf("%s: open %s ok, fd=%d\r\n", __FUNCTION__, path, fd);

	for (i = 0; i < count; i++) {
		ret = read(fd, buf, 1);
		if (ret == -1) {
			printf("read from %s error %s\r\n", path, strerror(errno));
			exit(1);
		} else if (ret == 0) {
			break;
		}
		if (i < 10) {
			printf("%s: char[%zd]=%c\r\n", __FUNCTION__, i, buf[0]);
		}
	}

	close(fd);
}

//////////////////////////////////////////////////////////////////////////////

static void uring_write(const char *path, size_t count)
{
	int    fd = open(path, __open_wflags, 0600), ret;
	size_t i;
	char   buf[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	if (fd < 0) {
		printf("%s: open %s error %s\r\n", __FUNCTION__, path, strerror(errno));
		exit(1);
	}

	printf("%s: open %s ok, fd=%d\r\n", __FUNCTION__, path, fd);

	struct io_uring_params params;
	struct io_uring uring;

	memset(&params, 0, sizeof(params));
	ret = io_uring_queue_init_params(100, &uring, &params);

	if (ret < 0) {
		printf("init io_uring error=%s\r\n", strerror(errno));
		exit(1);
	}

	struct io_uring_sqe *sqe;
	struct io_uring_cqe *cqe;

	for (i = 0; i < count; i++) {
		sqe = io_uring_get_sqe(&uring);
		io_uring_prep_write(sqe, fd, &buf[i % 10], 1, i);
		io_uring_submit(&uring);
		ret = io_uring_wait_cqes(&uring, &cqe, 1, NULL, NULL);
		if (ret < 0) {
			printf("io_uring_wait_cqe error=%s\r\n", strerror(-ret));
			exit(1);
		}

		io_uring_cqe_seen(&uring, cqe);
		if (i < 10) {
			printf("%s: write char=%c\r\n", __FUNCTION__, buf[i % 10]);
		}
	}

	ret = write(fd, "\r\n", 2);
	if (ret <= 0) {
		printf("write CRLF error %s\r\n", strerror(errno));
		exit(1);
	}

	ret = close(fd);
	printf("close %s %s, fd=%d\r\n", path, ret == 0 ? "ok" : "error", fd);
	io_uring_queue_exit(&uring);
}

static void uring_read(const char *path, size_t count)
{
	int    fd = open(path, __open_rflags, 0600), ret;
	size_t i;
	char   buf[1];

	if (fd < 0) {
		printf("%s: open %s error %s\r\n", __FUNCTION__, path, strerror(errno));
		exit(1);
	}

	printf("%s: read open %s ok, fd=%d\r\n", __FUNCTION__, path, fd);

	struct io_uring_params params;
	struct io_uring uring;

	memset(&params, 0, sizeof(params));
	ret = io_uring_queue_init_params(100, &uring, &params);

	if (ret < 0) {
		printf("init io_uring error=%s\r\n", strerror(errno));
		exit(1);
	}

	struct io_uring_sqe *sqe;
	struct io_uring_cqe *cqe;

	for (i = 0; i < count; i++) {
		sqe = io_uring_get_sqe(&uring);
		io_uring_prep_read(sqe, fd, buf, 1, i);
		io_uring_submit(&uring);
		ret = io_uring_wait_cqes(&uring, &cqe, 1, NULL, NULL);
		if (ret < 0) {
			printf("io_uring_wait_ceq error=%s\r\n", strerror(-ret));
			exit(1);
		}
		if (i < 10) {
			printf("%s: char[%zd]=%c\r\n", __FUNCTION__, i, buf[0]);
		}
		io_uring_cqe_seen(&uring, cqe);
	}

	close(fd);
	printf("close fd=%d\r\n", fd);
	io_uring_queue_exit(&uring);
}

//////////////////////////////////////////////////////////////////////////////

static void test_write(const char *filepath, size_t count)
{
	struct timeval begin, end;
	double tc, speed;

	//////////////////////////////////////////////////////////////////////

	gettimeofday(&begin, NULL);

	uring_write(filepath, count);

	gettimeofday(&end, NULL);

	tc    = stamp_sub(&end, &begin);
	speed = (count * 1000) / (tc > 0 ? tc : 0.001);

	printf("uring write, total write=%ld, cost=%.2f ms, speed=%.2f\r\n",
		count, tc, speed);

	//////////////////////////////////////////////////////////////////////

	printf("-------------------------------------------------------\r\n");

	gettimeofday(&begin, NULL);

	sys_write(filepath, count);

	gettimeofday(&end, NULL);

	tc = stamp_sub(&end, &begin);
	speed = (count * 1000) / (tc > 0 ? tc : 0.001);

	printf("sys write, total write=%ld, cost=%.2f ms, speed=%.2f\r\n",
		count, tc, speed);
}

//////////////////////////////////////////////////////////////////////////////

static void test_read(const char *filepath, size_t count)
{
	struct timeval begin, end;
	double tc, speed;

	//////////////////////////////////////////////////////////////////////

	gettimeofday(&begin, NULL);

	uring_read(filepath, count);

	gettimeofday(&end, NULL);

	tc = stamp_sub(&end, &begin);
	speed = (count * 1000) / (tc > 0 ? tc : 0.001);

	printf("uring read, total read=%ld, cost=%.2f ms, speed=%.2f\r\n",
		count, tc, speed);

	//////////////////////////////////////////////////////////////////////

	printf("-------------------------------------------------------\r\n");

	gettimeofday(&begin, NULL);

	sys_read(filepath, count);

	gettimeofday(&end, NULL);

	tc = stamp_sub(&end, &begin);
	speed = (count * 1000) / (tc > 0 ? tc : 0.001);

	printf("sys read, total read=%ld, cost=%.2f ms, speed=%.2f\r\n",
		count, tc, speed);
}

//////////////////////////////////////////////////////////////////////////////

static void usage(const char *proc)
{
	printf("usage: %s -h [help]\r\n"
		"  -f filepath\r\n"
		"  -n size[default: 1024]\r\n"
		, proc);
}

int main(int argc, char *argv[])
{
	int  ch, count = 10000;
	char path[256];

	snprintf(path, sizeof(path), "file.txt");

	while ((ch = getopt(argc, argv, "hf:n:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'f':
			snprintf(path, sizeof(path), "%s", optarg);
			break;
		case 'n':
			count = atoi(optarg);
			break;
		default:
			break;
		}
	}

	test_write(path, (size_t) count);

	printf("========================================================\r\n");

	test_read(path, (size_t) count);

	return 0;
}
