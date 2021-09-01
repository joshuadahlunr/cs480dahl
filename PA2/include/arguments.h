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

	std::string getVertexFilePath() const;
	std::string getFragmentFilePath() const;
	std::string getResourcePath() const;
};

#endif // ARGUMENTS_H
