Tool made to test syncronization mechanisms in a deterministic way that does not rely on OS scheduler.
Tests should be deterministic and reproducible.


<a name="readme-top"></a>



<br />
<div align="center">

<h3 align="center">Deterministic Concurrency</h3>

  <p align="center">
    A simple C++ lib for testing syncronization mechanisms.
  </p>
</div>


<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Authors</a></li>
  </ol>
</details>

## About The Project

Concurrency testing often poses challenges due to the non-deterministic nature of threads and synchronization mechanisms. The DeterministicConcurrency library aims to address these challenges by providing tools to create and manage deterministic concurrency scenarios, enabling precise and reliable testing.<br />
While primarily designed for testing, the User-Controlled Scheduler can also serve as a framework to create synchronized thread pool-like structures for broader multithread applications.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Getting Started

This is an header only library but you can build the main.cpp which contains a simple test using:

   ```sh
   cmake . -B build -DDC_COMPILE_MAIN=ON -G Ninja
   ```

   ```sh
   cmake --build build
   ```

You can also generate the build files to build the tests with:
   ```sh
   cmake . -B build -DDC_COMPILE_TESTS=ON -G Ninja
   ```


### Prerequisites

I build and tested this lib with C++17.
If for some reason, on some compilers, it doesn`t work on C++17+ please email me or open an issue.

### Installation

Using cmake you can include this lib using:
```
include(FetchContent)

FetchContent_Declare(
  dc
  GIT_REPOSITORY https://github.com/Sernior/deterministic-concurrency.git
  GIT_TAG [TAG] #change with the tag you want to use
)

FetchContent_MakeAvailable(dc)
```

At this point you should be able to link the library simply using:
```
target_link_libraries(your_stuff.cpp deterministic_concurrency)
```

## Usage

Include the library:
```
#include <DeterministicConcurrency>
```
Define the functions you want the threads to execute:
```
void f(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << a;
    c->switchContext(); // wait until the scheduler switch context to this thread
    std::cout << b;
}

void h(DeterministicConcurrency::thread_context* c ,int a, int b){
    std::cout << b;
    c->switchContext();
    std::cout << a;
}
```
The first parameter must always be a pointer to DeterministicConcurrency::thread_context.<br /><br />
Now define your user controlled scheduler:
```
DeterministicConcurrency::UserControlledScheduler<2> sch{std::tuple{&f, 0, 1}, std::tuple{&h, 3, 2}};
```
This defines a scheduler which control 2 threads.<br />
The elements in the tuple after &f and &h are the arguments that will be passed to **`f`** and **`h`**.<br />
These threads will wait until they have been given permission from the scheduler to execute **`f`** and **`h`** (using switchContextTo).<br />
**`UserControlledScheduler::switchContextTo(threadIndexes...)`** and **`thread_context::switchContext()`** will block the caller thread until the context is given back.<br />
What threads are within the **`UserControlledScheduler`** is elements in a std::array; you can think their indexes as their threadIDs.<br /><br />
For instance in this example you can do:
```
sch.switchContextTo(1);
sch.switchContextTo(0);
sch.switchContextTo(0);
sch.switchContextTo(1);
```
And you will end up printing "2013".<br /><br />
The "main" thread (the one which defines the scheduler) and the threads synchronize through a system of tick() and tock() (my own definition of time).<br />
The scheduler can switch context to a specific thread **`sch.switchContextTo(1)`** allowing it to continue (you could also switch context to multiple threads to simulate real parallelism **`sch.switchContextTo(0,1)`**); and it can also use **`sch.joinOn(indexes...)`** to wait until threads finish.<br />

<p align="right">(<a href="#readme-top">back to top</a>)</p>


## Contributing

If you encounter any issues or would like to suggest new features, please don't hesitate to open an issue or get in touch with me at federignoli@hotmail.it.<br />Contributions are also welcome! Feel free to open pull requests to the main repository and assign me as a reviewer – I'll be sure to review them. Your help is greatly appreciated!

<p align="right">(<a href="#readme-top">back to top</a>)</p>


## License

Distributed under the MIT License. See LICENSE.txt for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


## Contact

Federico Abrignani - federignoli@hotmail.it

<p align="right">(<a href="#readme-top">back to top</a>)</p>


## Authors

* Federico Abrignani (Author)
* Paolo Di Giglio (Contributor)
* Salvatore Martorana (Contributor)

<p align="right">(<a href="#readme-top">back to top</a>)</p>