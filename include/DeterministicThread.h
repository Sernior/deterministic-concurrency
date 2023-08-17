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

        /**
         * @brief Notify the scheduler that this thread is ready to give it back the control and wait until the scheduler notify back.
         * 
         * \code{.cpp}
         * void my_function(DeterministicConcurrency::thread_context* c) {
         *     //...do something
         *     c->switchContext();
         *     //...do something
         * };
         * \endcode
         */
        void switchContext(){
            tock();             
            wait_for_tick();    
        }

        /**
         * @brief #TODO
         * 
         * @tparam BasicLockable 
         * @tparam Args 
         * @param lockable 
         * @param args 
         */
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

        /**
         * @brief #TODO
         * 
         * @tparam BasicLockable 
         * @tparam Args 
         * @param lockable 
         * @param args 
         */
        template<typename BasicLockable, typename... Args>
        void lock_shared(BasicLockable* lockable, Args&&... args){

            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL;
            }

            lockable->lock_shared(std::forward<Args>(args)...);
            
            {
            std::lock_guard<std::mutex> lock(control_mutex);
            thread_status_v = DeterministicConcurrency::thread_status_t::RUNNING;
            }

        }

        private:

        /**
         * @brief #TODO
         * 
         */
        friend class DeterministicThread;

        /**
         * @brief #TODO
         * 
         * @tparam N 
         */
        template<size_t N>
        friend class UserControlledScheduler;

        /**
         * @brief Wait until the scheduler switch context to this thread.
         */
        void start(){
            std::unique_lock<std::mutex> lock(control_mutex);
            while (thread_status_v == thread_status_t::NOT_STARTED)
                tick_tock.wait(lock);
        }

        /**
         * @brief Notify the scheduler that this thread has finished not allowing the scheduler anymore to switch context to this thread.
         */
        void finish(){
            {
                std::unique_lock<std::mutex> lock(control_mutex);
                thread_status_v = thread_status_t::FINISHED;
            }
            tick_tock.notify_one();
        }
        
        /**
         * @brief Allow the scheduler to proceed its execution.
         */
        void tock() {
            {
                std::unique_lock<std::mutex> lock(control_mutex);
                thread_status_v = thread_status_t::WAITING;
            }
            tick_tock.notify_one();
        }

        /**
         * @brief Wait until the scheduler notify this thread.
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
        /**
         * @brief Construct a new Deterministic Thread object. #TODO
         * 
         * @tparam Func 
         * @tparam Args 
         * @param t 
         * @param func 
         * @param args 
         */
        template <typename Func, typename... Args>
        explicit DeterministicThread(thread_context* t ,Func&& func, Args&&... args)
            : _thread([function = std::forward<Func>(func), t, tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                t->start();
                std::apply(function, std::tuple_cat(std::make_tuple(t), tuple));
                t->finish();
            })
            , _this_thread(t) {}

        /**
         * @brief #TODO
         * 
         */
        void join() {
            _thread.join();
        }

        /**
         * @brief Allow the thread to proceed its execution
         */
        void tick() {
            {
                std::unique_lock<std::mutex> lock(_this_thread->control_mutex);
                if (_this_thread->thread_status_v == thread_status_t::FINISHED)return;
                _this_thread->thread_status_v = thread_status_t::RUNNING;
            }
            _this_thread->tick_tock.notify_one();
        }

        /**
         * @brief Wait until the thread notify the scheduler
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
