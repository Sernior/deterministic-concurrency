/**
 * @file UserControlledScheduler.h
 * @author F. Abrignani (federignoli@hotmail.it)
 * @author P. Di Giglio
 * @author S. Martorana
 * @brief 
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
     * @brief #TODO
     * 
     * @tparam N 
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
         * @brief Wait until all of the threadIndixes threads have thread_status_v equal to S // we must disable the resolution for S == WAITING_EXTERNAL.#TODO
         * 
         * @tparam S 
         * @tparam Args 
         * @param threadIndixes param
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        template<thread_status_t S, typename... Args>
        void waitUntilAllThreadStatus(Args&&... threadIndixes){
            for(size_t numThreads = 0; numThreads != sizeof...(threadIndixes);){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                numThreads = 0;
                ([&]{
                    if (getThreadStatus(threadIndixes) == S)
                        numThreads++;
                }(),...);
            }
            return;
        }

        /**
         * @brief #TODO
         * 
         * @tparam BasicLockable 
         * @param lockable 
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        template<typename BasicLockable>
        void waitUntilLocked(BasicLockable* lockable){
            while (lockable->try_lock()){
                lockable->unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        /**
         * @brief Wait until at least one of the threadIndixes threads have thread_status_v equal to S and return the index of the first thread who reached S.#TODO
         * 
         * @tparam S 
         * @tparam Args 
         * @param threadIndixes 
         * @return size_t 
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        template<thread_status_t S, typename... Args>
        size_t waitUntilOneThreadStatus(Args&&... threadIndixes){
            size_t threadIndex = -1;
            for (;threadIndex == -1;){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ([&]{
                    if (getThreadStatus(threadIndixes) == S)
                        threadIndex = threadIndixes;
                }(),...);
            }
            return threadIndex;
        }

        /**
         * @brief Switch context allowing the threads with threadIndixes to proceed while stopping the scheduler from executing until all of them switchContext back.#TODO
         * 
         * @tparam Args : describe Args
         * @param threadIndixes : describe threadIndixes
         * 
         * example of switchContextTo:
         * \code{.cpp}
         * DeterministicConcurrency::UserControlledScheduler ) {
         *     //...do something
         *     sch.switchContextTo();
         *     //...do something
         * };
         * \endcode
         */
        template<typename... Args>
        void switchContextTo(Args&&... threadIndixes){
            ([&]{
                proceed(threadIndixes);
                wait(threadIndixes);
            }(),...);
        }

        /**
         * @brief Switch context allowing all the threads to proceed while stopping the scheduler from executing while all of them switchContext back.#TODO
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        void switchContextAll(){
            switchContextAll(std::make_index_sequence<N>());
        }

        /**
         * @brief Perform a join on the threads with threadIndixes.#TODO
         * 
         * @tparam Args 
         * @param threadIndixes 
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        template<typename... Args>
        void joinOn(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].join(), ...);
        }

        /**
         * @brief Perform a join on all threads.
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        void joinAll(){
            for (auto& _thread : _threads)
                _thread.join();
        }

        /**
         * @brief Tick threadIndixes threads, allowing them to continue if they were in WAITING status.#TODO
         * 
         * @tparam Args 
         * @param threadIndixes 
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        template<typename... Args>
        void proceed(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].tick(), ...);
        }

        /**
         * @brief Wait until the threads with threadIndixes go into WAITING status.#TODO
         * 
         * @tparam Args 
         * @param threadIndixes 
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode
         */
        template<typename... Args>
        void wait(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].wait_for_tock(), ...);
        }

        /**
         * @brief Get the Thread Status object. #TODO
         * 
         * @param threadIndixes 
         * @return thread_status_t
         * 
         * example:
         * \code{.cpp}
         * void my_function(my_namespace::my_class my_instance) {
         *     //...do something
         *     my_instance.my_method();
         *     //...do something
         * };
         * \endcode 
         */
        thread_status_t getThreadStatus(size_t threadIndixes){
            return _contexts[threadIndixes].thread_status_v;
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
     * @brief describe make_UserControlledScheduler#TODO
     * 
     * @tparam Tuples 
     * @param tuples 
     * @return UserControlledScheduler 
     * 
     * example of `make_UserControlledScheduler()`:
     * \code{.cpp}
     * #include <DeterministicConcurrency>
     * 
     * void thread_function(DeterministicConcurrency::thread_context* c, uint32_t a) {
     *      //...do something
     * }
     * 
     * void main() {
     *     //...do something
     *     auto thread_0 = std::tuple{&thread_function, 0};
     * 
     *     auto sch = DeterministicConcurrency::make_UserControlledScheduler(
     *         thread_0
     *     );
     *     //...do something
     *     sch.joinAll();
     * }
     *  
     * \endcode 
     */
    template<typename... Tuples>
    auto make_UserControlledScheduler(Tuples&&... tuples) {
        return UserControlledScheduler<sizeof...(Tuples)>(static_cast<Tuples&&>(tuples)...);
    }
    
}
