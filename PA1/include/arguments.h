#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>

class Arguments {
private:
	std::string vertexFilePath; //= "../shaders/vert.glsl";
	std::string fragmentFilePath; //= "../shaders/frag.glsl";

public:
	Arguments(int argc, char **argv);

	std::string getVertexFilePath();
	std::string getFragmentFilePath();
};

#endif // ARGUMENTS_H
