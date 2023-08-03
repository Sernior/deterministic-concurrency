Tool made to test syncronization mechanisms in a deterministic way that does not rely on OS scheduler.
Tests should be deterministic and reproducible.


<a name="readme-top"></a>




<!-- PROJECT LOGO -->
<br />
<div align="center">

<h3 align="center">Deterministic Concurrency</h3>

  <p align="center">
    A simple C++ lib for testing syncronization mechanisms.
  </p>
</div>



<!-- TABLE OF CONTENTS -->
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



<!-- ABOUT THE PROJECT -->
## About The Project

Concurrency testing often poses challenges due to the non-deterministic nature of threads and synchronization mechanisms. The DeterministicConcurrency library aims to address these challenges by providing tools to create and manage deterministic concurrency scenarios, enabling precise and reliable testing.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- GETTING STARTED -->
## Getting Started

TODO

### Prerequisites

I build and tested this lib with C++20 but it should be usable for C++17 or more.

### Installation

This is an header only library but you can build the main.cpp which contains a simple test using:

   ```sh
   cmake . -B build -G Ninja
   ```

   ```sh
   cmake --build build
   ```

You can also generate the build files to build the tests with:
   ```sh
   cmake . -B build -DDC_COMPILE_TESTS=ON -G Ninja
   ```


<!-- USAGE EXAMPLES -->
## Usage

TODO

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- CONTRIBUTING -->
## Contributing

TODO

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- LICENSE -->
## License

Distributed under the MIT License. See LICENSE.txt for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Federico Abrignani - federignoli@hotmail.it

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ACKNOWLEDGMENTS -->
## Authors

* Federico Abrignani (Author)
* Paolo Di Giglio (Contributor)

<p align="right">(<a href="#readme-top">back to top</a>)</p>