# PA5: Assimp Model Loading

## Team

Our team is composed of the following members:  
- Joshua Dahl  
- Bryan Dedeurwaerder
- Jonathan Peters

# Arguments and keys

The command to run the program (assuming that we compiled in a build directory one level below root) is:
```bash
./PA5 -v vert.glsl -f frag.glsl -m box.obj
```

The slight variations in color of the box model are intended to provide a little bit of fake depth perception.
Several additional object files have been included for testing:
* Colored Cube: startingCube.obj
* UTAH Bunny: bunny.obj
* Stanford Teapot: teapot.obj
* 3D Printing Benchmark Boat: benchy.obj

## Arguments
* -h, -?, --help - Shows the help message
* -v <file> - Sets the vertex shader (relative to the resource/shaders directory)
* -f <file> - Sets the fragment shader (relative to the resource/shaders directory)
* -m <file> - Sets the obj model (relative to the resource/models directory)
### Optional
* --resource-path <path> - Sets the resource directory, the directory where all of the program's resources can be found. [default=../]


## Keys
The Showcase dropdown menu at the top of the application is also capable of preforming all of the following operations.  

* R - Pause the rotation of the Showcase.
* O - Pause the orbit of the Showcase.
* left click, L - Reverse the orbit of the Showcase.
* right click, F - Reverse the rotation of the Showcase.

Additionally, the orbit and rotation speed of the Showcase can be adjusted in the Showcase dropdown menu.


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
The the code zip must be extracted into `PA5/thirdparty`.

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
./PA5 -v vert.glsl -f frag.glsl  
```
