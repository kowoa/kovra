# Kovra

A Vulkan renderer I use to learn how to program (graphically).

This project is a rewrite of my older Vulkan project, [vulkaning](https://github.com/kowoa/vulkaning), which I wrote in Rust.
The reason for this rewrite is it allows me to directly take advantage of C++'s larger software ecosystem.

<div align="center">
    <img src="skybox.gif" alt="Skybox Demo GIF"/>
</div>

## Features

- [x] Basic rendering pipeline
- [x] Basic material system
- [x] Depth testing
- [x] Loading and rendering 3D models
- [x] Camera controls (arcball)
- [x] Cubemapped skybox
- [x] Frustum culling
- [x] Mipmapping
- [x] Multisample anti-aliasing (MSAA)
- [ ] Metallic-roughness workflow
- [ ] Specular-glossiness workflow
- [ ] Image-based lighting (IBL)
- [x] Albedo mapping
- [ ] Normal mapping
- [ ] Ambient Occlusion (AO) mapping
- [ ] Shadow mapping
- [ ] Height/displacement mapping
- [ ] PBR material inspector
- [x] Profiling stats
- [ ] GPU buffer visualizer
- [ ] Skeletal animation

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
