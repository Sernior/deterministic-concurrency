#include <iostream>
#include <tuple>
#include <DeterministicConcurrency>
 
void f(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << a;
    c->switchContext();
    std::cout << b;
}

void h(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << b;
    c->switchContext();
    std::cout << a;
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