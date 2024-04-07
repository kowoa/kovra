# Kovra

A Vulkan renderer I use to learn how to program (graphically).

This project is a rewrite of my older Vulkan project, [vulkaning](https://github.com/kowoa/vulkaning), which I wrote in Rust.
The reason for this rewrite is it allows me to directly take advantage of C++'s larger software ecosystem.

<div align="center">
    <img src="skybox.gif" alt="Skybox Demo GIF"/>
</div>

## Getting Started

### Linux

Requirements:

- CMake 3.28 or later
- C++20 compiler
- Vulkan 1.3 or later
- Make for building and running the project

```shell
git clone --recurse-submodules https://github.com/kowoa/kovra.git
cd kovra
make run
```
