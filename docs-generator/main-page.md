# Documentation deterministic-concurrency

<em>F.Abrignani, P. Di Giglio, S. Martorana - federignoli@hotmail.it - Version 1.4.5, 14.08.2023</em>

<table style="border:none;border-collapse:collapse;">
    <tr>
        <td style="border:none;border-left:none;border-top:none;border-bottom:none;width: 80px;"> <img src="warning.png" style= "width:70px; height:70px; object-fit: cover; object-position: 100% 0;"/>
        </td>
        <td style="border-right:none;border-top: none;border-bottom: none;"> 
        Tool made to test syncronization mechanisms in a deterministic way that does not rely on OS scheduler.<br>
        Tests should be deterministic and reproducible.
        </td>
    </tr>
</table>

## Overview
Deterministic Concurrency, as the name suggests, aims to allow the user to create reproducible scheduling sequences so to allow deterministic asserts while testing concurrent algorithms.
Controlling the flow of concurrency can be achieved by calling the methods of ```thread_context``` and ```UserControlledScheduler```.

## Motivation
This library exists because of a specific need: testing the correctness of my own synchronization primitives. I required a straightforward and readable approach to define precise scheduling sequences for making meaningful assertions.
<br>
Having used this tool successfully, I found it invaluable for identifying and rectifying bugs and incorrect behaviors in my other projects. As a result of its effectiveness in improving code quality, I've chosen to release this library to the public. I hope that others can also find it useful and straightforward to use.

## DeterministicThread
Except for those users who wants to contribute to this library, standard users should not be concerned about this class. This definition of thread uses an internal definition of time shared with the ```UserControlledScheduler``` to manage context switches.

## thread_context
Each ```DeterministicThread``` has a ```thread_context```, the thread context is then passed to the thread function and contains methods for the ```DeterministicThread``` to manage its flow and also to update its state.

Each ```thread_context``` contains the state of the ```DeterministicThread``` associated with it.

```cpp
    enum class thread_status_t{
        RUNNING,
        WAITING,
        NOT_STARTED,
        FINISHED,
        WAITING_EXTERNAL
    };
```

After the initial invocation of ```.proceed()``` or ```.switchContextTo()``` from the scheduler, the thread transitions from ```NOT_STARTED``` to the ```RUNNING``` state.<br> It remains in this state until one of the following conditions is met:

1. **Completion**: The thread completes its task, resulting in a transition to the ```FINISHED``` state (also becoming joinable);
2. **Context Switch**: When the ```thread_context::switchContext()``` method is called, the thread's state transitions to ```WAITING``` until the scheduler gives it permission to run again;
3. **External Lock**: If an external lock event occurs, the thread's state changes to ```WAITING_EXTERNAL``` until the "external source of waiting" allows it to continue.

## UserControlledScheduler
The ```UserControlledScheduler``` manages the creation and the flow of threads.<br>
It also creates each deterministic thread ad thread_context object through which the thread can control its flow and decide when to switch context.

Let`s see an example:
```cpp
static custom_syncronization_mechanism m;
using namespace DeterministicConcurrency;
thread_function0(DeterministicConcurrency::thread_context* c){
    std::cout << "Hello!\n"; // #0
    c->lock(&m); // #1
    c->switchContext(); // #2
    std::cout << "name "; // #3
    m.unlock(); // #4
}
thread_function1(DeterministicConcurrency::thread_context* c){
    std::cout << "My "; // #5
    c->lock(&m); // #6
    c->switchContext(); // #7
    std::cout << "is "; // #8
    m.unlock(); // #9
}

int main(){
    auto thread0 = std::tuple{&thread_function0};
    auto thread1 = std::tuple{&thread_function1};
    auto sch = make_UserControlledScheduler(thread0, thread1);
    // this scheduler is controlling 2 deterministic threads.
    // Deterministic threads are stored as elements in an array within the scheduler,
    // so in this case we have thread0 with index 0 as it is the first argument given to
    // make_UserControlledScheduler and thread1 with index 1.
    // We can use these indexes to control them.
    // As stated these threads are lazy so they are just waiting for sch to do something here.


    sch.switchContextTo(0); 
    // this will allow thread 0 to proceed and stop the scheduler until thread0 uses switch context
    // on its thread_context or until it finishes.
    // So thread0 will start executing perform #0 taking the lock on m at #1 and then execute #2
    // stopping itself and giving control back to the scheduler

    // at this point assuming m function similarly to a std::mutex, so not allowing also
    // thread0 to go past the lock if we were to perform sch.switchContextTo(1) we would
    // surely deadlock ourselves. So insted we can use proceed() which allows threads to proceed
    // but without stopping the scheduler thread.
    sch.proceed(1);
    // so thread 1 will perform #5 and then get stopped by #6 since the lock is already owned by
    // thread0.

    // Just to make sure thread1 is actually locked by 'm' we can ask the scheduler to wait
    // until thread1 goes into thread_status_t::WAITING_EXTERNAL.
    sch.waitUntilAllThreadStatus<thread_status_t::WAITING_EXTERNAL>(1);

    // Right now thread 0 is in state WAITING at #2 and thread 1 is in state WAITING_EXTERNAL at #6.
    // So let`s make thread 0 continue its executing until it finishes.
    sch.switchContextTo(0); 
    // Since switchContextTo unlike proceed waits for the thread or threads indexes given as parameters
    // to either context switch back or finish, after this point we know thread 0 is FINISHED
    // and so the mutex has been unlocked.

    // Since we cannot be sure on when the OS scheduler decides to execute 0 or 1 
    // we can not be sure at this point if 1 was already unlock by 'm' or not so
    // so we must wait for it to reach its switchContext at #7

    sch.waitUntilAllThreadStatus<thread_status_t::WAITING>(1);

    sch.switchContextTo(1);

    sch.joinAll();

    std::cout << "Bob.";

}

```

We will end up printing
```
Hello!
My name is Bob.
```
and we successfully controlled the flow of our threads.<br>
It is worth noting that on "external sources of waiting" we can't just directly call ```m.lock()``` or ```m.lock_shared()``` so I had to implement ```lock``` and ```lock_shared``` as thread_context methods.
This is because thread contextes need to know that they could remain stuck on a lock and thus update the thread status to WAITING_EXTERNAL.
