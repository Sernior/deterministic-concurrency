/**
 * @file UserControlledScheduler.h
 * @author F. Abrignani (federignoli@hotmail.it)
 * @author P. Di Giglio
 * @author S. Martorana
 * @brief Contains the definition of the UserControlledScheduler.
 * @version 1.4.5
 * @date 2023-08-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <DeterministicConcurrency>
#include <cstddef>
#include <array>
#include <tuple>
#include <type_traits>
#include <chrono>
#include <thread>

namespace DeterministicConcurrency{

    /**
     * @brief A scheduler which allow to manage the flow of its managed threads.
     * 
     */
    template<size_t N>
    class UserControlledScheduler{

        struct emplace_t {};

        public:

        /// @private
        template <typename... Tuples>
        UserControlledScheduler(Tuples&&... tuples)
            : UserControlledScheduler{std::index_sequence_for<Tuples...>{},
                                static_cast<Tuples&&>(tuples)...} {}

        /**
         * @brief Wait until all of the threadIndexes threads have thread_status_v equal to S.
         * 
         * @tparam S : thread_status_t to wait until.
         * @param threadIndexes : Indexes of the threads to perform waitUntilAllThreadStatus() on.
         * 
         * #### Example:
         * Wait until threads 0, 1, 2 and 3 reach status WAITING:
         * \code{.cpp}
         * sch.waitUntilAllThreadStatus<thread_status_t::WAITING>(0,1,2,3);
         * \endcode
         */
        template<thread_status_t S, typename... Args>
        void waitUntilAllThreadStatus(Args&&... threadIndexes){
            for(size_t numThreads = 0; numThreads != sizeof...(threadIndexes);){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                numThreads = 0;
                ([&]{
                    if (getThreadStatus(threadIndexes) == S)
                        numThreads++;
                }(),...);
            }
            return;
        }

        /**
         * @brief Wait until lockable is owned.
         * 
         * @tparam BasicLockable 
         * @param lockable 
         * 
         * #### Example:
         * \code{.cpp}
         * std::mutex m;
         * sch.waitUntilLocked(&m);
         * \endcode
         */
        template<typename BasicLockable>
        static void waitUntilLocked(BasicLockable* lockable){
            while (lockable->try_lock()){
                lockable->unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        /**
         * @brief Wait until at least one of the threadIndexes threads have thread_status_v equal to S and return the index of the first thread who reached S.
         * 
         * @tparam S : The thread_status_t waitUntilOneThreadStatus will wait until.
         * @param threadIndexes : Indexes of the threads to perform waitUntilOneThreadStatus() on.
         * @return size_t : the index of the first thread who reached thread_status_t S.
         * 
         * #### Example:
         * \code{.cpp}
         * auto index = sch.waitUntilOneThreadStatus<thread_status_t::WAITING>(0,1,2,3);
         * \endcode
         */
        template<thread_status_t S, typename... Args>
        size_t waitUntilOneThreadStatus(Args&&... threadIndexes){
            size_t threadIndex = -1;
            for (;threadIndex == -1;){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ([&]{
                    if (getThreadStatus(threadIndexes) == S)
                        threadIndex = threadIndexes;
                }(),...);
            }
            return threadIndex;
        }

        /**
         * @brief Switch context allowing the threads with threadIndexes to proceed while stopping the scheduler from executing until all threads switchContext back.
         * 
         * @param threadIndexes : Indexes of the threads to perform switchContextTo() on.
         * 
         * #### Example:
         * example of switchContextTo():
         * \code{.cpp}
         * sch.switchContextTo(0, 1, 2, 3);
         * \endcode
         */
        template<typename... Args>
        void switchContextTo(Args&&... threadIndexes){
            ([&]{
                proceed(threadIndexes);
                wait(threadIndexes);
            }(),...);
        }

        /**
         * @brief Switch context allowing all the threads to proceed while stopping the scheduler from executing until all of the threads switchContext back.
         * 
         * #### Example:
         * \code{.cpp}
         * sch.switchContextAll();
         * \endcode
         */
        void switchContextAll(){
            switchContextAll(std::make_index_sequence<N>());
        }

        /**
         * @brief Perform a join on the threads with threadIndexes.
         * 
         * @param threadIndexes : Indexes of the threads to perform joinOn on.
         * 
         * #### Example:
         * \code{.cpp}
         * sch.joinOn(0, 1, 2, 3);
         * \endcode
         */
        template<typename... Args>
        void joinOn(Args&&... threadIndexes){
            static_assert(sizeof...(threadIndexes) <= N, "Too many args");
            (_threads[threadIndexes].join(), ...);
        }

        /**
         * @brief Perform a join on all threads.
         * 
         * #### Example:
         * \code{.cpp}
         * sch.joinAll();
         * \endcode
         */
        void joinAll(){
            for (auto& _thread : _threads)
                _thread.join();
        }

        /**
         * @brief Allow threadIndexes to continue while not stopping the scheduler thread.
         * 
         * @param threadIndexes : Indexes of the threads to perform proceed on.
         * 
         * #### Example:
         * \code{.cpp}
         * sch.proceed(0, 1, 2, 3);
         * \endcode
         */
        template<typename... Args>
        void proceed(Args&&... threadIndexes){
            static_assert(sizeof...(threadIndexes) <= N, "Too many args");
            (_threads[threadIndexes].tick(), ...);
        }

        /**
         * @brief Wait until the threads with threadIndexes go into WAITING status.
         * 
         * @param threadIndexes : Indexes of the threads to perform wait on.
         * 
         * #### Example:
         * \code{.cpp}
         * sch.wait(0, 1, 2, 3);
         * \endcode
         */
        template<typename... Args>
        void wait(Args&&... threadIndexes){
            static_assert(sizeof...(threadIndexes) <= N, "Too many args");
            (_threads[threadIndexes].wait_for_tock(), ...);
        }

        /**
         * @brief Get the Thread Status of the thread with threadIndex.
         * 
         * @param threadIndex : Obtain the thread_status of the thread identified by threadIndex.
         * @return thread_status_t : the status of the threadIndex-th thread.
         */
        thread_status_t getThreadStatus(size_t threadIndex){
            return _contexts[threadIndex].thread_status_v;
        }

        private:

        template <typename... Tuples>
        UserControlledScheduler(emplace_t, Tuples&&... tuples)
            : _threads{std::make_from_tuple<DeterministicThread>(tuples)...} {}

        template <typename... Tuples, std::size_t... Is>
        UserControlledScheduler(std::index_sequence<Is...>, Tuples&&... tuples)
            : UserControlledScheduler{
                emplace_t{}, std::tuple_cat(std::tuple{&std::get<Is>(_contexts)},
                                            static_cast<Tuples&&>(tuples))...} {}


        template <std::size_t... Is>
        void switchContextAll(std::index_sequence<Is...>){
            ([&]{
                proceed(Is);
                wait(Is);
            }(),...);
        }

        std::array<thread_context, N> _contexts;
        std::array<DeterministicThread, N> _threads;
    };

    /**
     * @brief Helper function to create an UserControlledScheduler.
     * 
     * @param tuples : tuples containing the function the threads have to performs followed by their arguments.
     * @return UserControlledScheduler.
     * 
     * #### Example:
     * example of `make_UserControlledScheduler()`:
     * \code{.cpp}
     * void f(thread_context* c, int a){}
     * void h(thread_context* c, string a){} 
     * auto thread0 = tuple{&f, 3};
     * auto thread1 = tuple{&f, "aaa"};
     * auto sch = make_UserControlledScheduler(thread0, thread1);
     * \endcode 
     */
    template<typename... Tuples>
    auto make_UserControlledScheduler(Tuples&&... tuples) {
        return UserControlledScheduler<sizeof...(Tuples)>(static_cast<Tuples&&>(tuples)...);
    }
    
}
