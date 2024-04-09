﻿## How-TO-Build  
use cmake:
```bash
git submodule update --init --recursive
cmake -S . -B build -G"MinGW Makefiles"
cmake --build build
```

Designate SDL2_ROOT TO CMAKE CACHE
```bash
cmake -S . -B cmake-build -DSDL2_ROOT=<your-sdl2-path>
```
Im's Use form MinGW Makefiles Has different SDL2 root  

## 3rd Libraries
|library|description|
|--|--|
|[SDL2](https://github.com/libsdl-org/SDL)| use to support create window, render context and basic input|
|[Vulkan](https://www.vulkan.org/)| GUP Renderer support Api|
|[VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)| GUP Memory Allocate|
|[stb_image](http://nothings.org/stb)| use to loading images|
|[dear-imgui](https://github.com/ocornut/imgui)| use to support GUI in editor|
|[glm](https://miniaud.io/)| Math Library|
|[tinygltf](https://github.com/syoyo/tinygltf)| use to Load Model|
|[entt](https://github.com/skypjack/entt)| use to ECS framework|
|[Yaml](https://yaml.org/)| use to Serialization data format|
|[yaml-cpp](https://github.com/jbeder/yaml-cpp)| use to yaml format library|
|[cereal](https://github.com/USCiLab/cereal)| Use to Serialization Library|
|[spdlog](https://github.com/gabime/spdlog)| Use to Output Log |

## Questions
This engine framwork Thanks for [Chero](https://github.com/TheCherno/Hazel)