# PA1: Spinning Cube

# Arguments and keys

The command to run the program (assuming that we compiled in a build directory one level below root) is:  
  ./Tutorial -v vert.glsl -f frag.glsl  

## Arguments
* -h, -?, --help - Shows the help message
* -v <file> - Sets the vertex shader (relative to the resource/shader directory)
* -f <file> - Sets the fragment shader (relative to the resource/shader directory)
### Optional
* --resource-path <path> - Sets the resource directory, the directory where all of the program's resources can be found. [default=../]


## Keys
The cube menu at the top of the application is also capable of preforming all of the fellowing operations.  

* R - pause the rotation of the cube
* O - pause the orbit of the cube
* left click, L - reverse the orbit of the cube
* right click, F - reverse the rotation of the cube


# Dependencies, Building, and Running

## Dependency Instructions
For both of the operating systems to run this project installation of these three programs are required [GLEW](http://glew.sourceforge.net/), [GLM](http://glm.g-truc.net/0.9.7/index.html), and [SDL2](https://wiki.libsdl.org/Tutorials).

These programs should already be installed on the ECC computers.

### Ubuntu/Linux
```bash
sudo apt-get install libglew-dev libglm-dev libsdl2-dev
```

### Mac OSX
Installation of brew is suggested to easily install the libs. Ensure that the latest version of the Developer Tools is installed.
```bash
brew install glew glm sdl2
```

## Building and Running
To build this project there are two options. One is to use CMake which makes including new libraries easier, and handles new files added automatically to the src and include directory. CMake is a small new learning curve but makes things easier in the future.
The second option is to use the provided Makefile which is used as usual.

Running the make in a separate directory will allow easy cleanup of the build data, and an easy way to prevent unnecessary data to be added to the git repository.  

### CMake Instructions
The building of the project is done using CMake, installation with apt-get or brew may be necessary. Later use with CMake and Shader files will be require the copy of a directory where those files are stored (ex. shaders). To do this in the ```add_custom_target``` function place
```cmake
COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/shaders/ ${CMAKE_CURRENT_BINARY_DIR}/shaders
```

```bash
mkdir build
cd build
cmake ..
make
./Tutorial
```

### Makefile Instructions
The makefile works as expected and must be updated with new files added in.

```bash
mkdir build
cd build
cp ../makefile .
make
./Tutorial
```

## Netboot Ubuntu\Debian
The project should run on the network install of Debian in the ECC. To boot into this OS, restart the computer and use the arrow keys on boot to select Ubuntu\Debian from the startup list (currently its listed under CS135).
