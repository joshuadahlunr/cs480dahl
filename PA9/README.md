# PA9: Lighting

## Extra Credit

- NA

## Team

Our team is composed of the following members:  
- Joshua Dahl
- Bryan Dedeurwaerder
- Jonathan Peters

# Arguments and Operation

The command to run the program (assuming that we compiled in a build directory one level below root) is:
```bash
./PA9 -c config.json
```

## Arguments
* -h, -?, --help - Shows a help message
* -c <file> - Sets the configuration file (relative to the resource directory)
* -vv <file> - Sets the per vertex, vertex shader (relative to the resource/shaders directory)
* -vf <file> - Sets the per vertex, fragment shader (relative to the resource/shaders directory)
* -fv <file> - Sets the per fragment, vertex shader (relative to the resource/shaders directory)
* -ff <file> - Sets the per fragment, fragment shader (relative to the resource/shaders directory)
### Optional
* --resource-path <path> - Sets the resource directory, the directory where all of the program's resources can be found. [default=../]


## Operation
- Holding right click will let you rotate the camera
- WASD to move the sphere around.
- Spacebar toggles between per fragment and per vertex shaders
- GUI interface to control spotlight settings


# Dependencies, Building, and Running

## Dependency Instructions
To run this project installation of these three programs are required [GLEW](http://glew.sourceforge.net/), [GLM](http://glm.g-truc.net/0.9.7/index.html), and [SDL2](https://wiki.libsdl.org/Tutorials).

A CMAKE version of at least 3.8 is required to build the project.

## Dear ImGui and ReactPhysics3D
The program relies on the Dear ImGui library. If git is being used these libraries are included as submodules and may be downloaded by running:
```bash
git submodule init
git submodule update
```

If git is not being used the two libraries will need to be download from: https://github.com/ocornut/imgui  and: https://github.com/DanielChappuis/reactphysics3d respectively.
The the code zip for both must then be extracted into `PA9/thirdparty` (The symbolic-links must be overwritten).

## Assimp Model Loading

The program also relies on the Assimp library for model loading. See the assimp build install page for getting assimp installed.

[Assimp build install](https://github.com/assimp/assimp/blob/master/Build.md)

## CMake Instructions
The building of the project is done using CMake (at least version 3.8), installation with apt-get or brew may be necessary. A compiler capable of compiling c++17 code is required (any compiler shipped with a modern distribution of Linux should be sufficient).

```bash
mkdir build
cd build
cmake ..
make
```
