#include <iostream>
#include <DeterministicThread.h>
#include <UserControlledScheduler.h>
#include <tuple>
 
void f(DeterministicConcurrency::thread_context* c ,int a, int b){
    c->wait_for_tick();
    std::cout << a;
    c->switchContext();
    std::cout << b;
    c->tock();
}

void h(DeterministicConcurrency::thread_context* c ,int a, int b){
    c->wait_for_tick();
    std::cout << b;
    c->switchContext();
    std::cout << a;
    c->tock();
}

int main()
{
    DeterministicConcurrency::UserControlledScheduler<2> sch{std::tuple{&f, 0, 1}, std::tuple{&h, 3, 2}}; // 2 0 1 3
    sch.switchContextTo(1);
    sch.switchContextTo(0);
    sch.switchContextTo(0);
    sch.switchContextTo(1);
    sch.joinAll();
    return 0;
}