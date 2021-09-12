# PA1: Spinning Cube

# Arguments and keys

The command to run the program (assuming that we compiled in a build directory one level below root) is:
```bash
./PA3 -v vert.glsl -f frag.glsl  
```

## Arguments
* -h, -?, --help - Shows the help message
* -v <file> - Sets the vertex shader (relative to the resource/shader directory)
* -f <file> - Sets the fragment shader (relative to the resource/shader directory)
### Optional
* --resource-path <path> - Sets the resource directory, the directory where all of the program's resources can be found. [default=../]


## Keys
The Planet dropdown menu at the top of the application is also capable of preforming all of the following operations.  

* R - Pause the rotation of the planet.
* O - Pause the orbit of the planet.
* left click, L - Reverse the orbit of the planet.
* right click, F - Reverse the rotation of the planet.

Additionally, the orbit and rotation speed of the planet can be adjusted in the planet dropdown menu.
Likewise the scale of the moon can be adjusted in the moon dropdown menu.


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
The the code zip must be extracted into `PA3/thirdparty`.

## CMake Instructions
The building of the project is done using CMake, installation with apt-get or brew may be necessary. A compiler capable of compiling c++17 code is required (any compiler shipped with a modern distribution of Linux should be sufficient).

```bash
mkdir build
cd build
cmake ..
make
./PA3 -v vert.glsl -f frag.glsl  
```
