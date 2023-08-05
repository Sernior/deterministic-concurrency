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
  GIT_TAG v1.0.0
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
```
The first parameter must always be a pointer to DeterministicConcurrency::thread_context.<br /><br />
Now define your user controlled scheduler:
```
DeterministicConcurrency::UserControlledScheduler<2> sch{std::tuple{&f, 0, 1}, std::tuple{&h, 3, 2}};
```
This defines a scheduler which control 2 threads.<br />
The elements in the tuple after &f and &h are the arguments that will be passed to **`f`** and **`h`**.<br />
These threads will execute **`f`** and **`h`**.<br />
**`f`** will immediately **`wait_for_tick`** and same for **`h`**.<br />
What threads are within the **`UserControlledScheduler`** is elements in a std::array so you can think their indexes are their threadIDs.<br />
For instance in this context you can do:
```
sch.switchContextTo(1);
sch.switchContextTo(0);
sch.switchContextTo(0);
sch.switchContextTo(1);
```
And you will end up printing "2013".

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

<p align="right">(<a href="#readme-top">back to top</a>)</p>