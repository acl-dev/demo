#include <cstdio>
#include <thread>
#include <memory>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/libfiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

static const int fibers = 2, count = 5;

int main() {
    acl::fiber_sbox<int> box;

    for (int i = 0; i < fibers; i++) {
        go[&box] {
            for (int j = 0; j < count; j++) {
                int *n = box.pop();
                if (n) {
                    printf("fiber-%d got int* %d from box\r\n", acl::fiber::self(), *n);
                    delete n;
                    acl::fiber::yield();
                } else {
                    break;
                }
            }

        };
    }

    for (int i = 0; i < fibers; i++) {
        go[&box] {
            for (int j = 0; j < count; j++) {
                int *n = new int(j);
                box.push(n);
            }
        };
    }

    acl::fiber_sbox2<int> box2;

    for (int i = 0; i < fibers; i++) {
        go[&box2] {
            for (int j = 0; j < count; j++) {
                int n;
                if (box2.pop(n)) {
                    printf("fiber-%d got int %d from box2\r\n", acl::fiber::self(), n);
                    acl::fiber::yield();
                } else {
                    break;
                }
            }
        };
    }

    for (int i = 0; i < fibers; i++) {
        go[&box2] {
            for (int j = 0; j < count; j++) {
                box2.push(j);
            }
        };
    }

    std::shared_ptr<acl::fiber_sbox2<int*>> box3(new acl::fiber_sbox2<int*>);

    for (int i = 0; i < fibers; i++) {
        go[box3] {
            for (int j = 0; j < count; j++) {
                int *n;
                if (box3->pop(n)) {
                    printf("fiber-%d got int* %d %p from box3\r\n", acl::fiber::self(), *n, n);
                    delete n;
                    acl::fiber::yield();
                } else {
                    break;
                }
            }
        };
    }

    for (int i = 0; i < fibers; i++) {
        go[box3] {
            for (int j = 0; j < count; j++) {
                int *n = new int(j);
                printf("fiber-%d push %d %p to box3\r\n", acl::fiber::self(), *n, n);
                box3->push(n);
            }
        };
    }

    acl::fiber::schedule();
    return 0;
}
