/**
 * @file DeterministicThread.h
 * @author F. Abrignani (federignoli@hotmail.it)
 * @author P. Di Giglio
 * @author S. Martorana
 * @brief Contains the definition of DeterministicThread and thread_context.
 * @version 1.4.5
 * @date 2023-08-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <DeterministicConcurrency>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <tuple>

namespace DeterministicConcurrency{
    /**
     * @brief Enum describing the possible states of a thread.
     * 
     */
    enum class thread_status_t{
        RUNNING,
        WAITING,
        NOT_STARTED,
        FINISHED,
        WAITING_EXTERNAL
    };

    class DeterministicThread;

    /**
     * @brief Provide the thread with basic functionalities.
     * 
     * #### Example:
     * 
     * Use case of `thread_context`:
     * \code{.cpp}
     * #include <mutex>
     * 
     * static std::mutex m;
     * 
     * void my_function(DeterministicConcurrency::thread_context* c) {
     *     //...do something
     *     c->lock(&m);
     *     //...do something         // ...critical section
     *     c->switchContext();       // ...
     *     //...do something         // ...
     *     m.unlock();
     *     //...do something
     * };
     * \endcode
     * 
     * #### Explanation:
     * 
     * - `lock(&m)` lock the `thread_context` on the `m` mutex;
     * 
     * - `switchContext()` switch the context back to the scheduler;
     * 
     * - `unlock()` unlock the `m` mutex.
     */
    class thread_context {
    public:
        thread_context() noexcept : control_mutex(), tick_tock(), thread_status_v(thread_status_t::NOT_STARTED) {}

        /**
         * @brief Notify the scheduler that this thread is ready to give it back the control and wait until the scheduler notify back.
         * 
         * #### Example:
         * 
         * Example of `switchContext()`:
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
         * @brief Lock \p lockable and update the current \p thread_status_v of the current `deterministic thread`.
         * 
         * @param lockable : a lockable object like a mutex.
         * @param args : arguments that will be forwarded to the .lock().
         * 
         * #### Example:
         * 
         * Example of `lock()`:
         * \code{.cpp}
         * #include <mutex>
         * 
         * static std::mutex m;
         * 
         * void my_function(DeterministicConcurrency::thread_context* c) {
         *     //...do something
         *     c->lock(&m);
         *     //...critical section
         *     m.unlock();
         *     //...do something
         * };
         * \endcode
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
         * @brief Lock \p lockable in shared mode and update the current \p thread_status_v of the current `deterministic thread`.
         * 
         * @param lockable : a lockable object like a mutex.
         * @param args : arguments that will be forwarded to the .lock_shared().
         * 
         * #### Example:
         * 
         * Example of `lock_shared()`:
         * \code{.cpp}
         * #include <mutex>
         * 
         * static std::mutex m;
         * 
         * void my_function(DeterministicConcurrency::thread_context* c) {
         *     //...do something
         *     c->lock_shared(&m);
         *     //...critical section
         *     m.unlock_shared();
         *     //...do something
         * };
         * \endcode
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

        /// @brief 
        /// @private
        friend class DeterministicThread;

        /// @brief 
        /// @tparam N 
        /// @private
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

    /**
     * @brief A thread controlled by the UserControlledScheduler
     * @private
     */
    class DeterministicThread {
    public:
        /// @private
        template <typename Func, typename... Args>
        explicit DeterministicThread(thread_context* t ,Func&& func, Args&&... args)
            : _thread([function = std::forward<Func>(func), t, tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                t->start();
                std::apply(function, std::tuple_cat(std::make_tuple(t), tuple));
                t->finish();
            })
            , _this_thread(t) {}

        /**
         * @brief Join this thread
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
