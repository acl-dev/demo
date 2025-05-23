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

struct my_task {
    std::atomic_long res;
    std::vector<int> buff;
    time_t last = time(nullptr);
};

static void add_bat(acl::wait_group& wg, my_task* task, int i) {
    task->buff.push_back(i);
    time_t now = time(nullptr);

    printf("size=%zd, diff=%ld\n", task->buff.size(), now - task->last);

    for (auto& n : task->buff) {
        task->res += n;
        wg.done();
    }

    printf(">>>>diff=%ld<<<\r\n", now - task->last);
    task->last = now;
    task->buff.clear();
}

static void usage(const char *procname) {
    printf("usage: %s -h [help] -c fibers_count\r\n", procname);
}

int main(int argc, char *argv[]) {
    int ch, nfiber = 10, buf = 500;

    while ((ch = getopt(argc, argv, "hc:")) > 0) {
        switch (ch) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'c':
            nfiber = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    std::atomic_long result(0);

    std::shared_ptr<acl::fiber_pool> fibers(new acl::fiber_pool(nfiber, nfiber, -1, buf));
    acl::wait_group wg;

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
        for (int i = 0; i < 10; i++) {
            result++;
            ::usleep(1000);
        }

        wg.done();
    }).detach();

    //////////////////////////////////////////////////////////////////////////
    // Execute in the fibers of another thread.

    wg.add(1);
    std::thread([&wg, buf, nfiber] {
        std::shared_ptr<acl::fiber_pool> fbs
            (new acl::fiber_pool(nfiber, nfiber, -1, buf));
        acl::wait_group wg2;

        my_task task;
        task.last = time(nullptr);

        for (int i = 0; i < 12; i++) {
            wg2.add(1);
            fbs->exec(add_bat, std::ref(wg2), &task, 1);
        }

        go[&wg2, fbs] {
            wg2.wait();
            fbs->stop();
        };

        acl::fiber::schedule();
        printf("The result by add_bat is: %ld\r\n", task.res.load());
        ::sleep(1);
        wg.done();
    }).detach();

    //////////////////////////////////////////////////////////////////////////

    // Wait all fibers done.
    go[&wg, fibers] {
        wg.wait();
        fibers->stop();
    };

    struct timeval begin;
    gettimeofday(&begin, nullptr);

    acl::fiber::schedule();

    struct timeval end;
    gettimeofday(&end, nullptr);
    double tc = acl::stamp_sub(end, begin);

    printf("The result is %ld, time cost: %.2f ms\r\n", result.load(), tc);
    return 0;
}
