#pragma once
#include <functional>
#include <utility>
#include <future>
#include <vector>

template<class task_fn>
class fiber_box {
public:
    fiber_box(acl::box2<task_fn>* box2) : box(box2) {}
    ~fiber_box() { delete box; }

    acl::box2<task_fn> *box = nullptr;
    int  idx  = -1;
    int  idle = -1;
};

template<class task_fn>
class fiber_pool {
public:
    fiber_pool(size_t min, size_t max, int buf, int ms, size_t merge_len = 0)
    : buf_(buf)
    , ms_(ms)
    , merge_len_(merge_len)
    {
        assert(max >= min && min > 0);
        box_min_    = min;
        box_max_    = max;
        boxes_      = new fiber_box<task_fn>* [max];
        boxes_idle_ = new fiber_box<task_fn>* [max];

        fiber_create(min);
    }

    fiber_pool(const fiber_pool&) = delete;
    fiber_pool& operator=(const fiber_pool&) = delete;

    ~fiber_pool() {
        for (size_t i = 0; i < box_count_; i++) {
            delete boxes_[i];
        }

        delete []boxes_;
        delete []boxes_idle_;
    }

    template<class Fn, class ...Args>
    void exec(Fn&& fn, Args&&... args) {
        auto obj = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
        fiber_box<task_fn>* box;
        if (box_idle_ > 0) {
            box = boxes_idle_[next_idle_++ % box_idle_];
        } else {
            box = boxes_[next_box_++ % box_count_];
        }

        box->box->push(obj, true);
        if (buf_ > 0 && box->box->size() >= (size_t) buf_) {
            acl::fiber::yield();
        }
    }

    template<class Fn, class ...Args>
    void exec2(Fn&& fn, Args&&... args) {
        auto obj = [fn = std::forward<Fn>(fn), tuple_args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            return std::apply([&fn](auto&&... args) {
                return std::invoke(fn, std::forward<decltype(args)>(args)...);
            }, tuple_args);
        };

        fiber_box<task_fn>* box;
        if (box_idle_ > 0) {
            box = boxes_idle_[next_idle_++ % box_idle_];
        } else {
            box = boxes_[next_box_++ % box_count_];
        }

        box->box->push(obj, true);
        if (buf_ > 0 && box->box->size() >= (size_t) buf_) {
            acl::fiber::yield();
        }
    }

    template<class Fn, class ...Args>
    void exec3(Fn&& fn, Args&&... args) {
        auto obj = [fn = std::forward<Fn>(fn), tuple_args = std::make_tuple(std::forward<Args>(args)...)]
           (auto&&... params) mutable {
                return std::apply([&fn, &params...](auto&&... args) {
                    return std::invoke(fn, std::forward<decltype(params)>(params)..., std::forward<decltype(args)>(args)...);
                }, tuple_args);
        };

        fiber_box<task_fn>* box;
        if (box_idle_ > 0) {
            box = boxes_idle_[next_idle_++ % box_idle_];
        } else {
            box = boxes_[next_box_++ % box_count_];
        }

        box->box->push(obj, true);
        if (buf_ > 0 && box->box->size() >= (size_t) buf_) {
            acl::fiber::yield();
        }
    }

    template<class Fn, class ...Args>
    void exec4(Fn&& fn, Args&&... args) {
        auto obj = [fn = std::forward<Fn>(fn), tuple_args = std::make_tuple(std::forward<Args>(args)...)](bool param) mutable {
            return std::apply([&fn, &param](auto&&... args) {
                return std::invoke(fn, param, std::forward<decltype(args)>(args)...);
            }, tuple_args);
        };

        fiber_box<task_fn>* box;
        if (box_idle_ > 0) {
            box = boxes_idle_[next_idle_++ % box_idle_];
        } else {
            box = boxes_[next_box_++ % box_count_];
        }

        box->box->push(obj, true);
        if (buf_ > 0 && box->box->size() >= (size_t) buf_) {
            acl::fiber::yield();
        }
    }

    void stop() {
        for (auto fb : fibers_) {
            acl_fiber_kill(fb);
        }

        wg_.wait();
    }

    void status() {
        printf("min=%zd, max=%zd, count=%zd, idle=%zd\r\n",
                box_min_, box_max_, box_count_, box_idle_);
    }

private:
    acl::wait_group wg_;
    int    buf_;
    int    ms_;
    size_t merge_len_ = 0;

    size_t box_min_   = 0;
    size_t box_max_   = 0;

    size_t box_count_ = 0;
    size_t box_idle_  = 0;
    size_t next_box_  = 0;
    size_t next_idle_ = 0;

    fiber_box<task_fn> **boxes_;
    fiber_box<task_fn> **boxes_idle_;
    std::vector<ACL_FIBER*> fibers_;

    void fiber_create(size_t count) {
        for (size_t i = 0; i < count; i++) {
            acl::box2<task_fn>* box2;
            box2 = new acl::fiber_sbox2<task_fn>(buf_);

            auto* fbox = new fiber_box<task_fn>(box2);

            boxes_[box_count_] = fbox;
            fbox->idx = box_count_++;

            boxes_idle_[box_idle_] = fbox;
            fbox->idle = box_idle_++;

            wg_.add(1);

            auto fb = go[this, fbox] {
                fiber_run(fbox);

                if (box_count_-- > 1) {
                    boxes_[fbox->idx] = boxes_[box_count_];
                    boxes_[fbox->idx]->idx = fbox->idx;
                    boxes_[box_count_] = nullptr;
                } else {
                    assert(box_count_ == 0);
                    boxes_[box_count_] = nullptr;
                }

                if (fbox->idle >= 0) {
                    if (box_idle_-- > 1) {
                        boxes_idle_[fbox->idle] = boxes_idle_[box_idle_];
                        boxes_idle_[fbox->idle]->idle = fbox->idle;
                        boxes_idle_[box_idle_] = nullptr;
                    } else {
                        assert(box_idle_ == 0);
                        assert(boxes_idle_[0] == fbox);
                        boxes_idle_[0] = nullptr;
                    }
                }

                delete fbox;
                wg_.done();
            };

            fibers_.push_back(fb);
        }
    }

    void fiber_run(fiber_box<task_fn>* fbox) {
        int ms = ms_;
        std::vector<task_fn> tasks;

        while (true) {
            task_fn t;

            if (fbox->box->pop(t, ms)) {
                tasks.emplace_back(std::move(t));
                if (tasks.size() < merge_len_) {
                    ms = 0;
                    continue;
                }
            } else if (acl::fiber::self_killed()) {
                break;
            } else if (acl::last_error() == EAGAIN) {
                if (tasks.empty()) {
                    if (box_count_ > box_min_) {
                        break;
                    }

                    ms = ms_;
                    continue;
                }
            }

            if (fbox->idle >= 0) {
                if (box_idle_-- > 1) {
                    boxes_idle_[fbox->idle] = boxes_idle_[box_idle_];
                    boxes_idle_[fbox->idle]->idle = fbox->idle;
                    boxes_idle_[box_idle_] = nullptr;
                } else {
                    assert(box_idle_ == 0);
                    assert(boxes_idle_[0] == fbox);
                    boxes_idle_[0] = nullptr;
                }

                fbox->idle = -1;
            }

            if (box_idle_ == 0 && box_count_ < box_max_) {
                fiber_create(1);
            }

            for (auto& task : tasks) {
                task();
            }

            assert(box_idle_ < box_count_);

            fbox->idle = box_idle_;
            boxes_idle_[box_idle_++] = fbox;

            tasks.clear();
            ms = ms_;
        }
    }
};
