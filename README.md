# Babylon Native - Doom Fire Demo

<p align="center">
  <a>
    <img src="https://github.com/SergioRZMasson/BabylonNativeDoomFireDemo/blob/main/Documentation/Sample.gif?raw=true" width="490">
  </a>
</p>

This is a demo showing how some computationally intensive tasks can be offloaded to C++ in a Babylon Native project. To illustrate that, we use multiple [Babylon.js Thin instances](https://doc.babylonjs.com/features/featuresDeepDive/mesh/copies/thinInstances), each instances has its color updated every frame by the [Doom Fire Algorithm](https://github.com/filipedeschamps/doom-fire-algorithm). The algorithm itself is pretty simple and it is just an excuse for use to update a large number of data every frame. The repo has two executables, in **DoomFireJS** we perform the fire update logic in JS, and in  **DoomFireNative** we move that logic to be done in C++. 

The goal for this project is to demonstrate the differences in performance by using JS vc C++ in a Babylon Native project. 

## Getting started

This repo uses npm to download the babylon.js dependencies, and CMake to create the platform specific project. It is required to run npm install before CMake since it will try to copy the JavaScript files from the babylon.js packages into the generated solution. 

### Mac
```
npm install
mkdir build
cd build
cmake .. -G "Xcode"
```

### Windows
```
npm install
mkdir build
cd build
cmake .. -G "Visual Studio 16 2022"
```

### Linux

#### Install CMake:

Babylon Native requires a cmake version higher than the one installed using default ```apt-get install cmake```. It is required that a higher cmake version is installed manually. For instructions on how do install cmake manually please follow the official documentation:

[Install CMake](https://cmake.org/install/)

#### Install dependecies:

```
sudo apt-get install libxi-dev libxcursor-dev libxinerama-dev libglfw3-dev libgl1-mesa-dev libcurl4-openssl-dev clang-9 libc++-9-dev libc++abi-9-dev lld-9 ninja-build
```

#### Install V8:

```
sudo apt-get install libv8-dev
```

#### Build:
```
npm install
mkdir build
cd build
cmake -G Ninja -D NAPI_JAVASCRIPT_ENGINE=V8
ninja
```
