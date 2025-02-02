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

static void add(acl::wait_group* wg, std::atomic_long* result, int i) {
    *result += i;;
    wg->done();
}

static void usage(const char *procname) {
    printf("usage: %s -h [help]\r\n"
        " -L min\r\n"
        " -H max\r\n"
        " -b buf\r\n"
        " -n count\r\n"
        " -t wating timeout[in milliseconds]\r\n"
        " -S [shared fibers]\r\n"
        " -m merge_len\r\n", procname);
}

int main(int argc, char *argv[]) {
    int ch, buf = 500, timeout = -1;
    size_t max = 20, min = 10;
    size_t merge_len = 10;
    long long count = 1000;
    bool shared = false;

    while ((ch = getopt(argc, argv, "hb:L:H:n:t:S")) > 0) {
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
            case 'S':
                shared = true;
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

    using task_fn = std::function<void(void)>;

    std::shared_ptr<std::atomic_long> result(new std::atomic_long(0));

    if (shared) {
        // Avoid crash on Mac when using shared fibers mode.
        go[] {};
    }

    std::shared_ptr<fiber_pool<task_fn>> fibers
        (new fiber_pool<task_fn>(min, max, buf, timeout, merge_len, shared));

    std::shared_ptr<acl::wait_group> wg(new acl::wait_group);

    acl::wait_group* w = wg.get();
    w->add(1);
    std::atomic_long* res = result.get();

    go[fibers, w, res, count] {
        printf("Begin add tasks ...\r\n");
        for (long long i = 0; i < count; i++) {
            w->add(1);
            fibers->exec(add, w, res, 1);
        }
        printf("Add tasks finished!\r\n");
        w->done();
    };

    go[w, fibers, count, res] {
        struct timeval begin;
        gettimeofday(&begin, nullptr);

        printf("Wait for all tasks ...\r\n");
        w->wait();
        printf("All tasks finished!\r\n");

        struct timeval end;
        gettimeofday(&end, nullptr);
        double tc = acl::stamp_sub(end, begin);
        double speed = (count * 1000) / (tc > 0 ? tc : 0.001);

        printf("The result is %ld, time cost: %.2f ms, speed: %.2f qps\r\n",
            res->load(), tc, speed);
        fibers->stop();
    };

    acl::fiber::schedule();
    printf("Fiber schedule finish!\r\n");
    return 0;
}
