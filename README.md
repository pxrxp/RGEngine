# RivalGrounds


## Table of Contents
---------------------

* [Introduction](#introduction)
* [Cloning the Repository](#cloning-the-repository)
* [Dependencies](#dependencies)
* [Building the Project](#building-the-project)
	+ [Setting the Build Type](#setting-the-build-type)
	+ [Available Build Targets](#available-build-targets)
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


## Dependencies
------------

This project depends on the following libraries:

* GLFW (for window and input management)
* GLAD (for OpenGL loading)
* GLM (for vector and matrix operations)

## Building the Project
---------------------

### Setting the Build Type

To set the build type, use the following command:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=<build_type>
```
Replace `<build_type>` with one of the following options:

* `Debug`: Build the project with debug symbols and optimizations disabled.
* `Release`: Build the project with optimizations enabled and debug symbols disabled.

Note: If not specified, defaults to `Release`

### Available Build Targets

Once you have set the build type, you can build the project using the following command:
```bash
cmake --build build --target <target>
```
Replace `<target>` with one of the following options:

* `build`: Build the project for your current operating system.
* `build_all`: Build the project for all supported platforms, including:
	+ Win32 (32-bit Windows)
	+ Win64 (64-bit Windows)
	+ Linux (64-bit)

Note:
* The `build_all` target will build the project for all platforms in the same directory.
* MacOS is not supported currently.

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
cmake --build build --target build
```
* Build the project for all supported platforms:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target build_all
```


## Running the Project
----------------------
**Option 1**: Build the project yourself then:

To run the project, follow these steps:

1. Navigate to the `build` directory
2. Navigate to the `Release` or `Debug` directory
3. Navigate to the directory of target operating system
4. Run the `RivalGrounds` executable to launch the game

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

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT).

