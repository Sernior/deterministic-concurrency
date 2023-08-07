#include <iostream>
#include <tuple>
#include <DeterministicConcurrency>
#include <mutex>
 
static std::mutex ctrl_mutex;

void f(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << a;
    c->tryLock([]{ctrl_mutex.lock();});
    c->switchContext();
    ctrl_mutex.unlock();
    std::cout << b;
}

void h(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << b;
    c->tryLock([]{ctrl_mutex.lock();});
    c->switchContext();
    ctrl_mutex.unlock();
    std::cout << a;
}


int main()
{
    auto sch = DeterministicConcurrency::make_UserControlledScheduler(std::tuple{&f, 0, 1}, std::tuple{&h, 3, 2}, std::tuple{&h, 4, 5}, std::tuple{&h, 6, 7});
    sch.switchContextTo(0);
    sch.tick(1,2,3);
    sch.waitUntilWAITING_EXTERNAL(1,2,3);
    sch.switchContextTo(0);
    auto index = sch.waitUntilWAITING(1,2,3);
    sch.switchContextTo(index);
    index = sch.waitUntilWAITING(1,2,3);
    sch.switchContextTo(index);
    index = sch.waitUntilWAITING(1,2,3);
    sch.switchContextTo(index);
    sch.joinAll();
    return 0;
}