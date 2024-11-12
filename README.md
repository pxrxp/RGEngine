# RivalGrounds


## Table of Contents
---------------------

* [Introduction](#introduction)
* [Cloning the Repository](#cloning-the-repository)
* [Prerequisites](#prerequisites)
* [Dependencies](#dependencies)
* [Building the Project](#building-the-project)
	+ [Build Commands](#build-commands)
	+ [Running Unit Tests](#running-unit-tests)
* [Running the Project](#running-the-project)
* [Project Structure](#project-structure)
* [Contributing](#contributing)
* [License](#license)

## Introduction
------------

**Welcome to RivalGrounds!**

A first-person shooter game built using the Entity-Component-System (ECS) architecture and OpenGL. *Currently in its early stages of development*, RivalGrounds aims to deliver a visceral and immersive gaming experience. This project aims to demonstrate the development process and share knowledge with others, while also creating a fun and engaging game.

 Stay tuned for updates, behind-the-scenes insights, and a glimpse into the world of RivalGrounds.

## Cloning the Repository
-------------------------

To clone the repository, run the following command in your terminal:

```bash
git clone --recurse-submodules https://github.com/VexilophragmaticZynglethorpean/RivalGrounds.git
```

This will clone the repository and initialize all submodules.

## Prerequisites
----------------

Before you begin, ensure you have the following installed:

- CMake
- A C++ compiler that supports C++20
- Python


## Dependencies
------------

This project depends on the following libraries:

* GLFW (for window and input management)
* GLAD (for OpenGL loading)
* GLM (for vector and matrix operations)

## Building the Project
---------------------
### Build Commands

Make sure you are in the project root directory.

* **For initial configuration:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=<build_type>
```
* **For all subsequent builds:**
```bash
cmake --build build
```
Replace `<build_type>` with one of the following options:

* `Debug`: For development and debugging, no optimizations.
* `Release`: For production, with optimizations enabled.
* `RelWithDebInfo`: For debugging optimized code, includes debug symbols.
* `MinSizeRel`: For minimizing binary size, with optimizations for size.

Note: If not specified, defaults to `MinSizeRel`

### Running Unit Tests

* Navigate to `build` directory and then:
```bash
ctest --rerun-failed --output-on-failure
```

Note: Make sure to set build type as `Debug`. Some functions from the source code are enabled only in Debug mode.

### Example Use Cases

* Build the project for your current operating system:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

* Build the project for Windows from Linux
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=<arch>-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=<arch>-w64-mingw32-g++ -DCMAKE_RC_COMPILER=<arch>-w64-mingw32-windres
cmake --build build
```
Replace `<arch>` with `i686` for x86, `x86_64` for x64

## Running the Project
----------------------
**Option 1**: Build the project yourself then:

To run the project, follow these steps:

1. Navigate to the `build` directory
2. Navigate to the `Release`, `Debug`, `MinSizeRel` or `RelWithDebInfo` directory
3. Run the `RivalGrounds` executable to launch the game

**Option 2**: Find the suitable executable for your operating system from the `Releases` section of the GitHub repository.

## Project Structure
-------------------

The project is organized using the Entity-Component-System (ECS) architecture. This means that the game logic is divided into three main components:

* **Entities**: Represent game objects, such as characters, enemies, and obstacles.
* **Components**: Represent the data and behavior of entities, such as position, velocity, and rendering.
* **Systems**: Represent the logic that operates on entities and their components, such as physics, rendering, and input handling.

## Contributing
------------

Contributions are welcome! If you'd like to contribute to the project, please fork the repository and submit a pull request with your changes.

## License
-------

This project is licensed under the [MIT License](LICENSE).
