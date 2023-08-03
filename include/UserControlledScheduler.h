#pragma once
#include <DeterministicThread.h>
#include <cstddef>
#include <Array>
#include <tuple>

namespace DeterministicConcurrency{

    template<size_t N>
    class UserControlledScheduler{

        struct emplace_t {};

        public:

        template <typename... Tuples>
        UserControlledScheduler(Tuples&&... tuples)
            : UserControlledScheduler{std::index_sequence_for<Tuples...>{},
                                static_cast<Tuples&&>(tuples)...} {}


        /*
        Will switch context allowing the threads with threadIndixes to proceed while stopping the scheduler from executing until all of them switchContext back or tock()
        */
        template<typename... Args>
        void switchContextTo(Args&&... threadIndixes){
            tick(std::forward<Args>(threadIndixes)...);
            wait(std::forward<Args>(threadIndixes)...);
        }

        template<typename... Args>
        void wait(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].wait_for_tock(), ...);
        }

        void waitAll(){
            for (auto& _thread : _threads)
                _thread.wait_for_tock();
        }

        template<typename... Args>
        void tick(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].tick(), ...);
        }

        void tickAll(){
            for (auto& _thread : _threads)
                _thread.tick();
        }

        template<typename... Args>
        void joinOn(Args&&... threadIndixes){
            static_assert(sizeof...(threadIndixes) <= N, "Too many args");
            (_threads[threadIndixes].join(), ...);
        }

        void joinAll(){
            for (auto& _thread : _threads)
                _thread.join();
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

        std::array<thread_context, N> _contexts;
        std::array<DeterministicThread, N> _threads;
    };
    
}
