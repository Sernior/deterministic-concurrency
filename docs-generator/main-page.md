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
Controlling the flow of concurrency can be achieved by calling the methods of <span style="color: darkgoldenrod;">thread_context</span> and <span style="color: darkgoldenrod;">UserControlledScheduler</span>.

## Motivation

This library exists because of a specific need: <span style="color: darkgoldenrod;">testing</span> the correctness of my own <span style="color: darkgoldenrod;">synchronization primitives</span>. I required a straightforward and readable approach to define precise scheduling sequences for making meaningful assertions.
<br>
Having used this tool successfully, I found it invaluable for identifying and rectifying bugs and incorrect behaviors in my other projects. As a result of its effectiveness in improving code quality, I've chosen to release this library to the public. I hope that others can also find it useful and straightforward to use.

## DeterministicThread

Except for those users who wants to contribute to this library, standard users should not be concerned about this class. This definition of thread uses an internal definition of time shared with the <span style="color: darkgoldenrod;">UserControlledScheduler</span> to manage context switches.

## thread_context

Each <span style="color: darkgoldenrod;">DeterministicThread</span> has a <span style="color: darkgoldenrod;">thread_context</span>, the thread context is then passed to the thread function and contains methods for the <span style="color: darkgoldenrod;">DeterministicThread</span> to manage its flow and also to update its state.

Each <span style="color: darkgoldenrod;">thread_context</span> contains the state of the <span style="color: darkgoldenrod;">DeterministicThread</span> associated with it.

```cpp
    enum class thread_status_t{
        RUNNING,
        WAITING,
        NOT_STARTED,
        FINISHED,
        WAITING_EXTERNAL
    };
```

After the initial invocation of ```.proceed()``` or ```.switchContextTo()``` from the scheduler, the thread transitions from <span style="color: darkgoldenrod;">NOT_STARTED</span> to the <span style="color: darkgoldenrod;">RUNNING</span> state.<br> It remains in this state until one of the following conditions is met:

1. **Completion**: The thread completes its task, resulting in a transition to the <span style="color: darkgoldenrod;">FINISHED</span> state (also becoming joinable);
2. **Context Switch**: When the ```thread_context::switchContext()``` method is called, the thread's state transitions to <span style="color: darkgoldenrod;">WAITING</span> until the scheduler gives it permission to run again;
3. **External Lock**: If an external lock event occurs, the thread's state changes to <span style="color: darkgoldenrod;">WAITING_EXTERNAL</span> until the "external source of waiting" allows it to continue.

## UserControlledScheduler

The <span style="color: darkgoldenrod;">UserControlledScheduler</span> manages the creation and the flow of threads.<br>
It also creates each deterministic thread ad thread_context object through which the thread can control its flow and decide when to switch context.

Let`s see an example:
```cpp
static custom_syncronization_mechanism m;

using namespace DeterministicConcurrency;

thread_function0 (DeterministicConcurrency::thread_context* c) {
    std::cout << "Hello!\n";    // #0
    c->lock(&m);                // #1
    c->switchContext();         // #2
    std::cout << "name ";       // #3
    m.unlock();                 // #4
}

thread_function1 (DeterministicConcurrency::thread_context* c) {
    std::cout << "My ";         // #5
    c->lock(&m);                // #6
    c->switchContext();         // #7
    std::cout << "is ";         // #8
    m.unlock();                 // #9
}

int main(){
    auto thread0 = std::tuple{&thread_function0};
    auto thread1 = std::tuple{&thread_function1};

    auto sch = make_UserControlledScheduler(thread0, thread1);

    sch.switchContextTo(0);     // #1.1
    sch.proceed(1);
    sch.waitUntilAllThreadStatus<thread_status_t::WAITING_EXTERNAL>(1);

    sch.switchContextTo(0);     // #1.2
    sch.waitUntilAllThreadStatus<thread_status_t::WAITING>(1);

    sch.switchContextTo(1);
    sch.joinAll();
    
    std::cout << "Bob.";
}
```

### Explanation:

```cpp
    auto thread0 = std::tuple{&thread_function0};
    auto thread1 = std::tuple{&thread_function1};
    auto sch = make_UserControlledScheduler(thread0, thread1);
```
This scheduler is controlling <span style="color: darkgoldenrod;">2 determistic threads</span>. Deterministic threads are stored as elements in an array within the scheduler, so in this case we have ```thread0``` with index <span style="color: darkgoldenrod;">0</span> as it is the first argument given to ```make_UserControlledScheduler``` and ```thread1``` with index <span style="color: darkgoldenrod;">1</span>. We can use these indexes to control them. As stated these threads are lazy so they are just waiting for sch to do something here.

```cpp
    sch.switchContextTo(0);     // #1.1
```

This will allow ```thread0``` to proceed and stop the scheduler until thread0 uses switch context on its ```thread_context``` or until it finishes. So ```thread0``` will start executing perform <span style="color: darkgoldenrod;">#0</span> taking the lock on m at <span style="color: darkgoldenrod;">#1</span> and then execute <span style="color: darkgoldenrod;">#2</span> stopping itself and giving control back to the scheduler.<br>

At this point assuming ```m``` function similarly to a ```std::mutex```, it will not allow ```thread1``` to go past the lock; if we were to perform ```sch.switchContextTo(1)``` we would surely deadlock ourselves. So, instead, we can use ```proceed()``` which allows threads to proceed but without stopping the scheduler thread.

```cpp
    sch.proceed(1);
```

So ```thread1``` will perform <span style="color: darkgoldenrod;">#5</span> and then get stopped by <span style="color: darkgoldenrod;">#6</span> since the lock is already owned by ```thread0```.<br>
Just to make sure ```thread1``` is actually locked by ```m``` we can ask the scheduler to wait until ```thread1``` goes into ```thread_status_t::WAITING_EXTERNAL```.

```cpp
    sch.waitUntilAllThreadStatus<thread_status_t::WAITING_EXTERNAL>(1);
```

Right now ```thread0``` is in state WAITING at <span style="color: darkgoldenrod;">#2</span> and ```thread1``` is in state <span style="color: darkgoldenrod;">WAITING_EXTERNAL</span> at <span style="color: darkgoldenrod;">#6</span>. <br>
So let's make ```thread0``` continue its executing until it finishes.

```cpp
    sch.switchContextTo(0);     // #1.2
```

Since ```switchContextTo()``` unlike proceed waits for the thread or threads indexes given as parameters to either context switch back or finish, after this point we know thread0 is <span style="color: darkgoldenrod;">FINISHED</span> and so the mutex has been unlocked.<br>
Since we cannot be sure on when the OS scheduler decides to execute ```thread0``` or ```thread1``` we can not be sure at this point if ```thread1``` was already unlocked by ```m``` or not so so we must wait for it to reach its switchContext at <span style="color: darkgoldenrod;">#7</span>.

```cpp
    sch.waitUntilAllThreadStatus<thread_status_t::WAITING>(1);
    sch.switchContextTo(1);
    sch.joinAll();
    std::cout << "Bob.";
```

We will end up printing:

```
Hello!
My name is Bob.
```

and we successfully controlled the flow of our threads.

It is worth noting that on "external sources of waiting" we can't just directly call ```m.lock()``` or ```m.lock_shared()``` so I had to implement <span style="color: darkgoldenrod;">lock</span> and <span style="color: darkgoldenrod;">lock_shared</span>  as ```thread_context``` methods.
This is because thread contextes need to know that they could remain stuck on a lock and thus update the thread status to <span style="color: darkgoldenrod;">WAITING_EXTERNAL</span>.
