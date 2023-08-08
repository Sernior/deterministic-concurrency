#pragma once
#include <DeterministicConcurrency>
#include <cstddef>
#include <array>
#include <tuple>
#include <chrono>
#include <thread>

namespace DeterministicConcurrency{

    template<size_t N>
    class UserControlledScheduler{

        struct emplace_t {};

        public:

        /*
        Construct a UserControlledScheduler controlling N threads
        */
        template <typename... Tuples>
        UserControlledScheduler(Tuples&&... tuples)
            : UserControlledScheduler{std::index_sequence_for<Tuples...>{},
                                static_cast<Tuples&&>(tuples)...} {}

        /*
        Wait until all of the threadIndixes threads have thread_status_v equal to S
        */
        template<thread_status_t S, typename... Args>
        void waitUntilAllThreadStatus(Args&&... threadIndixes){
            size_t numThreadsWaitingExternal = 0;
            for(;numThreadsWaitingExternal != sizeof...(threadIndixes);){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ([&]{
                    if (getThreadStatus(threadIndixes) == S)
                        numThreadsWaitingExternal++;
                }(),...);
            }
            return;
        }

        /*
        Wait until at least one of the threadIndixes threads have thread_status_v equal to S and return the index of the first thread who reached S
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

        /*
        Switch context allowing the threads with threadIndixes to proceed while stopping the scheduler from executing until all of them switchContext back
        */
        template<typename... Args>
        void switchContextTo(Args&&... threadIndixes){
            ([&]{
                proceed(threadIndixes);
                wait(threadIndixes);
            }(),...);
        }

        /*
        Switch context allowing all the threads to proceed while stopping the scheduler from executing while all of them switchContext back
        */
        void switchContextAll(){
            switchContextAll(std::make_index_sequence<N>());
        }

        /*
        Perform a join on the threads with threadIndixes
        */
        template<typename... Args>
        void joinOn(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].join(), ...);
        }

        /*
        Perform a join on all threads
        */
        void joinAll(){
            for (auto& _thread : _threads)
                _thread.join();
        }

        /*
        Tick threadIndixes threads, allowing them to continue if they were in WAITING status
        */
        template<typename... Args>
        void proceed(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].tick(), ...);
        }

        /*
        Wait until the threads with threadIndixes go into WAITING status
        */
        template<typename... Args>
        void wait(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].wait_for_tock(), ...);
        }

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

    template<typename... Tuples>
    auto make_UserControlledScheduler(Tuples&&... tuples) {
        return UserControlledScheduler<sizeof...(Tuples)>(static_cast<Tuples&&>(tuples)...);
    }
    
}
