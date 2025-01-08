#include <unistd.h>
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

#include "fiber_pool2.h"

static void add(acl::wait_group& wg, std::atomic_long& result, int i) {
    result += i;;
    printf("Thread-%ld, fiber-%d: add i=%d, result=%ld\n",
        acl::thread::self(), acl::fiber::self(), i, result.load());
    wg.done();
}

static void dec(acl::wait_group& wg, std::atomic_long& result, int i) {
    result -= i;
    printf("Thread-%ld, fiber-%d: dec i=%d, result=%ld\n",
        acl::thread::self(), acl::fiber::self(), i, result.load());
    wg.done();
}

static void say(acl::wait_group& wg, std::string data) {
    printf("Thread-%ld, fiber-%d: say %s\n",
        acl::thread::self(), acl::fiber::self(), data.c_str());
    wg.done();
}

static void fmt_print(acl::wait_group& wg, const char* fmt, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    printf("Thread-%ld, fiber-%d print: %s\r\n",
        acl::thread::self(), acl::fiber::self(), buf);
    wg.done();
}

static void usage(const char *procname) {
    printf("usage: %s -h [help] -c fibers_count -t timeout\r\n", procname);
}

int main(int argc, char *argv[]) {
    int ch, nfiber = 10, buf = 500, timeout = -1, merge_len = 1;

    while ((ch = getopt(argc, argv, "hc:t:")) > 0) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 'c':
                nfiber = atoi(optarg);
                break;
            case 'm':
                merge_len = atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            default:
                usage(argv[0]);
                return 1;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    std::atomic_long result(0);

    std::shared_ptr<fiber_pool2> fibers
        (new fiber_pool2(buf, nfiber, timeout, merge_len, true));
    acl::wait_group wg;

    go[&wg, fibers] {
        wg.wait();
        fibers->stop();
    };

    //////////////////////////////////////////////////////////////////////////
    // Execute in the currecnt thread.

    for (int i = 0; i < 10; i++) {
        wg.add(1);
        fibers->exec(add, std::ref(wg), std::ref(result), i);
    }

    for (int i = 0; i < 10; i++) {
        wg.add(1);
        fibers->exec([&wg, &result, i] {
            dec(wg, result, i);
        });
    }

    wg.add(1);
    fibers->exec(say, std::ref(wg), "hello");

    const char* s = "zsxxsz";

    wg.add(1);
    fibers->exec(fmt_print, std::ref(wg), "You're welcome, %s!", s);

    //////////////////////////////////////////////////////////////////////////

    wg.add(1);
    std::thread([&wg, &result] {
        for (int i = 0; i < 100; i++) {
            result++;
            ::usleep(1000);
        }

        wg.done();
    }).detach();

    //////////////////////////////////////////////////////////////////////////
    // Execute in the current thread's fibers and put by another thread.

    wg.add(1);
    std::thread([&wg, &result, fibers] {
        for (int i = 0; i < 10; i++) {
            wg.add(1);
            fibers->exec(add, std::ref(wg), std::ref(result), i);
        }

        wg.done();
    }).detach();

    //////////////////////////////////////////////////////////////////////////
    // Execute in the fibers of another thread.

    wg.add(1);
    std::thread([&wg, &result, buf, nfiber, timeout, merge_len] {
        std::shared_ptr<fiber_pool2> fbs
            (new fiber_pool2(buf, nfiber, timeout, merge_len));
        acl::wait_group wg2;

        go[&wg2, fbs] {
            wg2.wait();
            fbs->stop();
        };

        for (int i = 0; i < 10; i++) {
            wg2.add(1);
            fbs->exec(add, std::ref(wg2), std::ref(result), i);
        }

        acl::fiber::schedule();
        wg.done();
    }).detach();

    //////////////////////////////////////////////////////////////////////////

    struct timeval begin;
    gettimeofday(&begin, nullptr);

    acl::fiber::schedule();

    struct timeval end;
    gettimeofday(&end, nullptr);
    double tc = acl::stamp_sub(end, begin);

    printf("The result is %ld, time cost: %.2f ms\r\n", result.load(), tc);
    return 0;
}
