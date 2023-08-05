#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

namespace DeterministicConcurrency{
    enum class tick_tock_t{
        TICK,
        TOCK
    };
    class thread_context {
    public:
        thread_context() noexcept : mutex_(), tick_tock(), tick_tock_v(tick_tock_t::TOCK) {}

        /*
        Allow the scheduler to proceed its execution
        */
        void tock() {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                tick_tock_v = tick_tock_t::TOCK;
            }
            tick_tock.notify_one();
        }

        /*
        Wait until the scheduler notify this thread
        */
        void wait_for_tick(){
            std::unique_lock<std::mutex> lock(mutex_);
            while (tick_tock_v == tick_tock_t::TOCK) // while it is tock the other thread is going
                tick_tock.wait(lock);
        }

        /*
        Notify the scheduler that this thread is ready to give it back the control and wait until the scheduler notify back
        */
        void switchContext(){
            tock();
            wait_for_tick();
        }

        std::mutex mutex_;
        std::condition_variable tick_tock;
        tick_tock_t tick_tock_v;
    };

    class DeterministicThread {
    public:
        template <typename Func, typename... Args>
        explicit DeterministicThread(thread_context* t ,Func&& func, Args&&... args)
            : _thread(std::forward<Func>(func), t, std::forward<Args>(args)...), _this_thread(t) {}

        void join() {
            _thread.join();
        }

        /*
        Allow the thread to proceed its execution
        */
        void tick() {
            {
                std::unique_lock<std::mutex> lock(_this_thread->mutex_);
                _this_thread->tick_tock_v = tick_tock_t::TICK;
            }
            _this_thread->tick_tock.notify_one();
        }

        /*
        Wait until the thread notify the scheduler
        */
        void wait_for_tock(){
            std::unique_lock<std::mutex> lock(_this_thread->mutex_);
            while (_this_thread->tick_tock_v == tick_tock_t::TICK)// while it is tick the other thread is going
                _this_thread->tick_tock.wait(lock);
        }

    private:
        thread_context* _this_thread;
        std::thread _thread;
    };
}
