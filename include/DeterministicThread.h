#pragma once
#include <DeterministicConcurrency>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <tuple>

namespace DeterministicConcurrency{
    enum class thread_status_t{
        RUNNING,
        WAITING,
        NOT_STARTED,
        FINISHED,
        WAITING_EXTERNAL
    };

    class DeterministicThread;
    class thread_context {
    public:
        thread_context() noexcept : control_mutex(), tick_tock(), thread_status_v(thread_status_t::NOT_STARTED) {}

        /*
        Notify the scheduler that this thread is ready to give it back the control and wait until the scheduler notify back
        */
        void switchContext(){
            tock();
            wait_for_tick();
        }

        template<typename BasicLockable>
        void lock(BasicLockable* lockable){

            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL;
            }

            lockable->lock();
            
            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::RUNNING;
            }

        }

        template<typename BasicLockable>
        void lock_shared(BasicLockable* lockable){

            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL;
            }

            lockable->lock_shared();
            
            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::RUNNING;
            }

        }

        template<typename BasicLockable, typename... Args>
        void lock(BasicLockable* lockable, Args&&... args){

            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL;
            }

            lockable->lock(std::forward<Args>(args)...);
            
            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::RUNNING;
            }

        }

        private:

        friend class DeterministicThread;

        template<size_t N>
        friend class UserControlledScheduler;
        /*
        Wait until the scheduler switch context to this thread
        */
        void start(){
            std::unique_lock<std::mutex> lock(control_mutex);
            while (thread_status_v == thread_status_t::NOT_STARTED)
                tick_tock.wait(lock);
        }

        /*
        Notify the scheduler that this thread has finished not allowing the scheduler anymore to switch context to this thread
        */
        void finish(){
            {
                std::unique_lock<std::mutex> lock(control_mutex);
                thread_status_v = thread_status_t::FINISHED;
            }
            tick_tock.notify_one();
        }
        
        /*
        Allow the scheduler to proceed its execution
        */
        void tock() {
            {
                std::unique_lock<std::mutex> lock(control_mutex);
                thread_status_v = thread_status_t::WAITING;
            }
            tick_tock.notify_one();
        }

        /*
        Wait until the scheduler notify this thread
        */
        void wait_for_tick(){
            std::unique_lock<std::mutex> lock(control_mutex);
            while (thread_status_v == thread_status_t::WAITING)
                tick_tock.wait(lock);
        }

        std::condition_variable tick_tock;
        volatile thread_status_t thread_status_v;
        std::mutex control_mutex;
    };

    class DeterministicThread {
    public:
        template <typename Func, typename... Args>
        explicit DeterministicThread(thread_context* t ,Func&& func, Args&&... args)
            : _thread([function = std::forward<Func>(func), t, tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                t->start();
                std::apply(function, std::tuple_cat(std::make_tuple(t), tuple));
                t->finish();
            })
            , _this_thread(t) {}

        void join() {
            _thread.join();
        }

        /*
        Allow the thread to proceed its execution
        */
        void tick() {
            {
                std::unique_lock<std::mutex> lock(_this_thread->control_mutex);
                if (_this_thread->thread_status_v == thread_status_t::FINISHED)return;
                _this_thread->thread_status_v = thread_status_t::RUNNING;
            }
            _this_thread->tick_tock.notify_one();
        }

        /*
        Wait until the thread notify the scheduler
        */
        void wait_for_tock(){
            std::unique_lock<std::mutex> lock(_this_thread->control_mutex);
            while (_this_thread->thread_status_v == thread_status_t::RUNNING)
                _this_thread->tick_tock.wait(lock);
        }

    private:
        thread_context* _this_thread;
        std::thread _thread;
    };
}
