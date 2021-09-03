#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>

class Arguments {
private:
	std::string vertexFilePath; //= "../shaders/vert.glsl";
	std::string fragmentFilePath; //= "../shaders/frag.glsl";
	std::string resourcePath = "../";

public:
	Arguments(int argc, char **argv);

	std::string getVertexFilePath() const { return vertexFilePath; }
	std::string getResourcePath() const { return resourcePath; }
	std::string getFragmentFilePath() const { return fragmentFilePath; }
};

#endif // ARGUMENTS_H
