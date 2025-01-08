#pragma once
#include <functional>
#include <future>
#include <vector>

using task_fn = std::function<void()>;
using box_ptr = std::shared_ptr<acl::box2<task_fn>>;

class fiber_pool2 {
public:
    fiber_pool2(int buf, int concurrency, int ms, size_t merge_len, bool thr = false)
    : buf_(buf)
    , ms_(ms)
    , merge_len_(merge_len)
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

    ~fiber_pool2() = default;

    template<class Fn, class ...Args>
    void exec(Fn&& fn, Args&&... args) {
        auto obj = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
	next_ %= boxes_.size();
        boxes_[next_]->push(obj, true);
        if (buf_ > 0 && boxes_[next_++]->size() >= (size_t) buf_) {
            acl::fiber::yield();
        }
    }

    void stop() {
        for (auto fb : fibers_) {
            acl_fiber_kill(fb);
        }

        wg_.wait();
    }

private:
    acl::wait_group wg_;
    int buf_;
    int ms_;
    size_t merge_len_ = 0;
    size_t next_ = 0;
    std::vector<box_ptr> boxes_;
    std::vector<ACL_FIBER*> fibers_;

    void fiber_run(const std::shared_ptr<acl::box2<task_fn>>& box) {
        int ms = ms_;
        std::vector<task_fn> tasks;

        while (true) {
            task_fn t;
            if (box->pop(t, ms)) {
                tasks.emplace_back(std::move(t));
                if (tasks.size() < merge_len_) {
                    ms = 0;
                    continue;
                }
            } else if (acl::fiber::self_killed()) {
                break;
            } else if (acl::last_error() == EAGAIN) {
                if (tasks.empty()) {
                    ms = ms_;
                    continue;
                }
            }

            for (auto& task : tasks) {
                task();
            }

            tasks.clear();
            ms = ms_;
        }

        wg_.done();
    }
};
