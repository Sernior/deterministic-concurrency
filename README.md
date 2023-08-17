# Deterministic Concurrency

**Tool made to test syncronization mechanisms in a deterministic way that does not rely on OS scheduler.**

**Tests should be deterministic and reproducible.**

<a name="readme-top"></a>

<div align="center">
  <p align="center">
    A simple C++ lib for testing syncronization mechanisms.
  </p>
</div>


<!-- <details>
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
</details> -->

## About The Project

Concurrency testing often poses challenges due to the `non-deterministic` nature of threads and synchronization mechanisms. The `DeterministicConcurrency` library aims to address these challenges by providing tools to create and manage deterministic concurrency scenarios, enabling precise and reliable testing.<br />
While primarily designed for testing, the `UserControlledScheduler` can also serve as a framework to create synchronized thread pool-like structures for broader multithread applications.

The assumptions are:
  - `UserControlledScheduler` automatically creates a number of `std::threads` equal to the number of arguments you pass them; these `threads` are lazy and each one of them must be allowed to terminate by the `UserControlledScheduler`. 
  - To allow a `thread` to finish, the `UserControlledScheduler` must allow these `threads` to proceed until they are in a `joinable` state.

## Getting Started
### Prerequisites

I build and tested this lib with C++17.
If for some reason, on some compilers, it doesn`t work on C++17+ please email me or open an issue.
### Setup
This is an header only library but you can build the `main.cpp` which contains a simple test using:

   ```sh
   $ cmake . -B build -DDC_COMPILE_MAIN=ON -G Ninja
   ```

   ```sh
   $ cmake --build build
   ```

You can also generate the build files to build the tests with:
   ```sh
   $ cmake . -B build -DDC_COMPILE_TESTS=ON -G Ninja
   ```

### Installation

Using cmake you can include this lib using:
```cmake
include(FetchContent)

FetchContent_Declare(
  dc
  GIT_REPOSITORY https://github.com/Sernior/deterministic-concurrency.git
  GIT_TAG [TAG] #change with the tag you want to use
)

FetchContent_MakeAvailable(dc)
```

At this point you should be able to link the library simply using:

``` cmake
target_link_libraries(your_stuff.cpp deterministic_concurrency)
```


## Snippet
This is a snippet ready-to-use.
```cpp
#include <iostream>
#include <DeterministicConcurrency>

void your_function(DeterministicConcurrency::thread_context* c, uint32_t a, uint32_t b) {
    std::cout << "I'm writing " << a << "!\n";
    c->switchContext();
    std::cout << "I'm writing " << b << "!\n";
}

int main()
{
    auto thread_0 = std::tuple{&your_function, 0, 2};
    auto thread_1 = std::tuple{&your_function, 1, 3};
    
    auto sch = DeterministicConcurrency::make_UserControlledScheduler(
            thread_0, thread_1
        );

    sch.switchContextTo(1);
    sch.switchContextTo(0);
    sch.switchContextTo(0);
    sch.switchContextTo(1);

    sch.joinAll();
}
```

The output of the above will be:

```
I'm writing 1!
I'm writing 0!
I'm writing 2!
I'm writing 3!
```

## Contributing

If you encounter any issues or would like to suggest new features, please don't hesitate to open an issue or get in touch with me at federignoli@hotmail.it.<br />Contributions are also welcome! Feel free to open pull requests to the main repository and assign me as a reviewer – I'll be sure to review them. Your help is greatly appreciated!



## License

Distributed under the MIT License. See LICENSE.txt for more information.



## Contacts

Federico Abrignani - federignoli@hotmail.it



## Authors and Contributors

* Federico Abrignani (Author)
* Paolo Di Giglio (Contributor)
* Salvatore Martorana (Contributor)

