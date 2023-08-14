#include <iostream>
#include <tuple>
#include <DeterministicConcurrency>
#include <mutex>
#include <vector>
 
static std::mutex ctrl_mutex;

void f(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << a;
    c->lock(&ctrl_mutex);
    c->switchContext();
    ctrl_mutex.unlock();
    std::cout << b;
}

void h(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << b;
    c->lock(&ctrl_mutex);
    c->switchContext();
    ctrl_mutex.unlock();
    std::cout << a;
}

void g(DeterministicConcurrency::thread_context* c ,int a){

    c->switchContext();
    std::cout << a;
}
static std::vector<uint32_t> priorityVector = {0, 1, 2, 3, 4};
static std::vector<uint32_t> input_Vector = {0, 1, 2, 3, 4};
static std::vector<uint32_t> fair_input;

void test_custom_mutex (DeterministicConcurrency::thread_context* c, uint32_t a) {
    c->lock(&ctrl_mutex);
    c->switchContext();
    fair_input.push_back(a);
    ctrl_mutex.unlock();
}


int main()
{
    auto sch2 = DeterministicConcurrency::make_UserControlledScheduler(std::tuple{&g, 0}, std::tuple{&g, 1}, std::tuple{&g, 2}, std::tuple{&g, 3});
    sch2.switchContextAll();
    sch2.switchContextAll();
    sch2.joinAll();

    auto thread_0 = std::tuple{&test_custom_mutex, input_Vector[0]};
    auto thread_1 = std::tuple{&test_custom_mutex, input_Vector[1]};
    auto thread_2 = std::tuple{&test_custom_mutex, input_Vector[2]};
    auto thread_3 = std::tuple{&test_custom_mutex, input_Vector[3]};
    auto thread_4 = std::tuple{&test_custom_mutex, input_Vector[4]};
    auto sch1 = DeterministicConcurrency::make_UserControlledScheduler(
        thread_0, thread_1, thread_2, thread_3, thread_4
    );

    sch1.switchContextTo(0);
    sch1.proceed(1);
    sch1.waitUntilLocked(&ctrl_mutex);
    sch1.waitUntilAllThreadStatus<DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL>(1);
    sch1.switchContextTo(0);
    auto index1 = sch1.waitUntilOneThreadStatus<DeterministicConcurrency::thread_status_t::WAITING>(1);
    sch1.proceed(2);
    sch1.waitUntilLocked(&ctrl_mutex);
    sch1.waitUntilAllThreadStatus<DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL>(2);
    sch1.switchContextTo(index1);
    index1 = sch1.waitUntilOneThreadStatus<DeterministicConcurrency::thread_status_t::WAITING>(2);
    sch1.proceed(3);   
    sch1.waitUntilLocked(&ctrl_mutex);
    sch1.waitUntilAllThreadStatus<DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL>(3);
    sch1.switchContextTo(index1);
    index1 = sch1.waitUntilOneThreadStatus<DeterministicConcurrency::thread_status_t::WAITING>(3);
    sch1.proceed(4); 
    sch1.waitUntilLocked(&ctrl_mutex);
    sch1.waitUntilAllThreadStatus<DeterministicConcurrency::thread_status_t::WAITING_EXTERNAL>(4);
    sch1.switchContextTo(index1);
    index1 = sch1.waitUntilOneThreadStatus<DeterministicConcurrency::thread_status_t::WAITING>(4);
    sch1.switchContextTo(index1);
    sch1.joinAll();
    std::cout << " fair_input:" << ' '; // Congratulations you just controlled the flow of a std::mutex
    for (int i : fair_input)
        std::cout << i << ' ';
    return 0;
}