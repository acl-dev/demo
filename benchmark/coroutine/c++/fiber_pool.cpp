#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <atomic>
#include <thread>

#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.h>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

#include "../../../c++1x/fiber/fiber_pool.h"

static void add(acl::wait_group& wg, std::atomic_long& result, int i) {
    result += i;;
    wg.done();
}

static void usage(const char *procname) {
    printf("usage: %s -h [help]\r\n"
        " -L min\r\n"
        " -H max\r\n"
        " -b buf\r\n"
        " -n count\r\n"
        " -t wating timeout[in milliseconds]\r\n"
        " -m merge_len\r\n"
        " -S [use_in different threads]\r\n", procname);
}

int main(int argc, char *argv[]) {
    int ch, buf = 500, timeout = -1;
    size_t max = 20, min = 10;
    size_t merge_len = 10;
    long long count = 1000;
    bool thread_safe = false;

    while ((ch = getopt(argc, argv, "hb:L:H:n:t:m:S")) > 0) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 'b':
                buf = atoi(optarg);
                break;
            case 'L':
                min = (size_t) atoi(optarg);
                break;
            case 'H':
                max = (size_t) atoi(optarg);
                break;
            case 'n':
                count = atoll(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            case 'm':
                merge_len = (size_t) atoi(optarg);
                break;
            case 'S':
                thread_safe = true;
                break;
            default:
                usage(argv[0]);
                return 1;
        }
    }

    if (min > max) {
        max = min;
    }

    //////////////////////////////////////////////////////////////////////////

    std::atomic_long result(0);

    std::shared_ptr<fiber_pool> fibers
        (new fiber_pool(min, max, buf, timeout, merge_len, thread_safe));
    acl::wait_group wg;

    struct timeval begin;
    gettimeofday(&begin, nullptr);

    go[&wg, fibers, count, &result, &begin] {
        printf("Wait for all tasks ...\r\n");
        wg.wait();
        printf("All tasks finished!\r\n");

        struct timeval end;
        gettimeofday(&end, nullptr);
        double tc = acl::stamp_sub(end, begin);
        double speed = (count * 1000) / (tc > 0 ? tc : 0.001);

        printf("The result is %ld, time cost: %.2f ms, speed: %.2f qps\r\n",
            result.load(), tc, speed);
        fibers->stop();
    };

    wg.add(1);
    go[fibers, &wg, &result, count] {
        printf("Begin add tasks ...\r\n");
        for (long long i = 0; i < count; i++) {
            wg.add(1);
            fibers->exec(add, std::ref(wg), std::ref(result), 1);
        }
        printf("Add tasks finished!\r\n");
	wg.done();
    };

    acl::fiber::schedule();
    printf("Fiber schedule finish!\r\n");
    return 0;
}
