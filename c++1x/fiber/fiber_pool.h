#pragma once
#include <functional>
#include <future>

using task_fn = std::function<void()>;
using box_ptr = std::shared_ptr<acl::box2<task_fn>>;

class fiber_pool {
public:
    fiber_pool(int buf, int concurrency, int milliseconds, bool thr = false)
    : milliseconds_(milliseconds)
    {
        for (int i = 0; i < concurrency; i++) {
            std::shared_ptr<acl::box2<task_fn>> box;

            if (thr) {
                box = std::make_shared<acl::fiber_tbox2<task_fn>>();
            } else {
                box = std::make_shared<acl::fiber_sbox2<task_fn>>(buf);
            }

            boxes_.push_back(box);

            wg_.add(1);
            auto fb = go[this, box] {
                fiber_run(box);
            };

            fibers_.push_back(fb);
        }
    }

    ~fiber_pool() = default;

    template<class Fn, class ...Args>
    void exec(Fn&& fn, Args&&... args) {
        auto obj = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
        boxes_[next_++ % boxes_.size()]->push(obj, true);
    }

    void stop() {
        for (auto fb : fibers_) {
            acl_fiber_kill(fb);
        }

        wg_.wait();
    }

private:
    acl::wait_group wg_;
    int milliseconds_;
    size_t next_ = 0;
    std::vector<box_ptr> boxes_;
    std::vector<ACL_FIBER*> fibers_;

    void fiber_run(const std::shared_ptr<acl::box2<task_fn>>& box) {
        while (true) {
            task_fn t;
            if (box->pop(t, milliseconds_)) {
                t();
            } else if (acl::fiber::self_killed()) {
                break;
            }
        }

        wg_.done();
    }
};
