#pragma once
#include <DeterministicConcurrency>
#include <cstddef>
#include <array>
#include <tuple>

namespace DeterministicConcurrency{

    template<size_t N>
    class UserControlledScheduler{

        struct emplace_t {};

        public:
        template<typename... Args>
        void waitUntilWAITING_EXTERNAL(Args&&... args){
            size_t ret = 0;
            for(;ret == sizeof...(args);){
                ([&]{
                    if (getThreadStatus(args) == DeterministicConcurrency::tick_tock_t::WAITING_EXTERNAL)
                        ret = args;
                }(),...);
            }
            return;
        }

        template<typename... Args>
        size_t waitUntilWAITING(Args&&... args){
            size_t ret = -1;
            for (;ret == -1;){
                ([&]{
                    if (getThreadStatus(args) == DeterministicConcurrency::tick_tock_t::WAITING)
                        ret = args;
                }(),...);
            }
            return ret;
        }


        /*
        Construct a UserControlledScheduler controlling N threads
        */
        template <typename... Tuples>
        UserControlledScheduler(Tuples&&... tuples)
            : UserControlledScheduler{std::index_sequence_for<Tuples...>{},
                                static_cast<Tuples&&>(tuples)...} {}


        /*
        Switch context allowing the threads with threadIndixes to proceed while stopping the scheduler from executing until all of them switchContext back
        */
        template<typename... Args>
        void switchContextTo(Args&&... threadIndixes){
            tick(static_cast<decltype(threadIndixes)>(threadIndixes)...);
            wait(static_cast<decltype(threadIndixes)>(threadIndixes)...);
        }

        /*
        Switch context allowing all the threads to proceed while stopping the scheduler from executing while all of them switchContext back
        */
        void switchContextAll(){
            tickAll();
            waitAll();
        }

        /*
        Performa a join on the threads with threadIndixes
        */
        template<typename... Args>
        void joinOn(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].join(), ...);
        }

        /*
        Performa a join on all threads
        */
        void joinAll(){
            for (auto& _thread : _threads)
                _thread.join();
        }

        /*
        Tick threadIndixes threads, allowing them to continue if they were waiting for tick()
        */
        template<typename... Args>
        void tick(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].tick(), ...);
        }

        tick_tock_t getThreadStatus(size_t index){
            return _contexts[index].tick_tock_v;
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


        /*
        Tick all threads, allowing them to continue if they were waiting for tick()
        */
        void tickAll(){
            for (auto& _thread : _threads)
                _thread.tick();
        }

        /*
        Wait until the threads with threadIndixes tock()
        */
        template<typename... Args>
        void wait(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].wait_for_tock(), ...);
        }

        /*
        Wait until all threads tock()
        */
        void waitAll(){
            for (auto& _thread : _threads)
                _thread.wait_for_tock();
        }

        std::array<thread_context, N> _contexts;
        std::array<DeterministicThread, N> _threads;
    };

    template<typename... Tuples>
    auto make_UserControlledScheduler(Tuples&&... tuples) {
        return UserControlledScheduler<sizeof...(Tuples)>(static_cast<Tuples&&>(tuples)...);
    }
    
}
