# PA8: Solar System

## Team

Our team is composed of the following members:  
- Joshua Dahl
- Bryan Dedeurwaerder
- Jonathan Peters

# Arguments and Operation

The command to run the program (assuming that we compiled in a build directory one level below root) is:
```bash
./PA8 -c config.json
```

## Arguments
* -h, -?, --help - Shows a help message
* -c <file> - Sets the configuration file (relative to the resource directory)
* -v <file> - Sets the vertex shader (relative to the resource/shaders directory)
* -f <file> - Sets the fragment shader (relative to the resource/shaders directory)
* -m <file> - Sets the obj model (relative to the resource/models directory)
### Optional
* --resource-path <path> - Sets the resource directory, the directory where all of the program's resources can be found. [default=../]


## Operation
You can navigate the solar system by holding the right mouse button and dragging, or scrolling the mouse wheel to zoom in or out. The tab key can be pressed to cycle which celestial object the camera is focused on.

The Simulation control menu at the top of the program has options for controlling the speed of the simulation. It also has a toggle to switch between an actual view of the solar system and a scaled logarithmic view which makes it easier to see the various planets and moons.
If you need a reminder about this information it is available in the help menu at the top of the program.


# Dependencies, Building, and Running

## Dependency Instructions
For both of the operating systems to run this project installation of these three programs are required [GLEW](http://glew.sourceforge.net/), [GLM](http://glm.g-truc.net/0.9.7/index.html), and [SDL2](https://wiki.libsdl.org/Tutorials).

## Dear ImGui
The program relies on the Dear ImGui library. If git is being used this library is included as a submodule and may be downloaded by running:
```bash
git submodule init
git submodule update
```

If git is not being used the library will need to be download from: https://github.com/ocornut/imgui
The the code zip must be extracted into `PA8/thirdparty`.

## Assimp Model Loading

The program also relies on the Assimp library for model loading. See the assimp build install page for getting assimp installed.

[Assimp build install](https://github.com/assimp/assimp/blob/master/Build.md)

## CMake Instructions
The building of the project is done using CMake, installation with apt-get or brew may be necessary. A compiler capable of compiling c++17 code is required (any compiler shipped with a modern distribution of Linux should be sufficient).

```bash
mkdir build
cd build
cmake ..
make
```
