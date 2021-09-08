#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>

class Arguments {
private:
	std::string vertexFilePath; //= "../shaders/vert.glsl";
	std::string fragmentFilePath; //= "../shaders/frag.glsl";
	std::string resourcePath = "../";

	// Variable tracking wether or not we can continue
	bool canContinue = true;
public:
	Arguments(int argc, char **argv);

	std::string getVertexFilePath() const { return vertexFilePath; }
	std::string getResourcePath() const { return resourcePath; }
	std::string getFragmentFilePath() const { return fragmentFilePath; }

	bool getCanContinue() const { return canContinue; }
};

#endif // ARGUMENTS_H
