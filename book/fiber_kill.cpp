#include <stdio.h>
#include <stdlib.h>
#include <acl-lib/fiber/libfiber.hpp>

class fiber_killee : public acl::fiber {
public:
    fiber_killee() {}

private:
    ~fiber_killee() {}

protected:
    // @override
    void run() {
        while (true) {
            acl::fiber::delay(1000); // 因休眠而挂起
            if (acl::fiber::self_killed()) {  // 被其它协程通知退出
                printf("The fiber has been killed!\r\n");
                break;
            }
            printf("fiber_killee waikeup\r\n");
        }
        delete this;
    }
};

class fiber_killer : public acl::fiber {
public:
    fiber_killer(acl::fiber* fb) : fb_(fb) {}

private:
    acl::fiber* fb_;
    ~fiber_killer() {}

protected:
    // @override
    void run() {
        acl::fiber::delay(5000);
        printf("Begin to kill the killee\r\n");
        fb_->kill();  // 通知协程退出
        delete this;
    }
};

int main() {
    acl::fiber* killee = new fiber_killee;
    killee->start();

    acl::fiber* killer = new fiber_killer(killee);
    killer->start();

    acl::fiber::schedule();
    return 0;
}
