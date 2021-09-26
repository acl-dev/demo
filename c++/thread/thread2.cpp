#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class mythread : public acl::thread {
public:
    mythread(acl::tbox<acl::thread>& box) : box_(box) {}
    ~mythread(void) {}

protected:
    // @override
    void* run(void) {
        box_.push(this);
        return NULL;
    }

private:
    acl::tbox<acl::thread>& box_;
};

int main(void) {
    acl::tbox<acl::thread> box;
    acl::thread* thread = new mythread(box);
    thread->set_detachable(true);
    thread->start();

    for (int i = 0; i < 1000000; i++) {
        acl::thread* thr = box.pop();
        thr->set_detachable(true);
        thr->start();
        usleep(10000);
    }

    thread->wait();
    delete thread;
    printf("ok\r\n");
    pause();
    return 0;
}
