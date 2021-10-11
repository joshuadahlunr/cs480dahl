#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>

class Arguments {
private:
	std::string resourcePath = "../";

	std::string vertexFilePath;
	std::string fragmentFilePath;

	std::string showcaseModelPath;

	// Variable tracking whether or not we can continue
	bool canContinue = true;
public:
	Arguments(int argc, char **argv);

	std::string getResourcePath() const { return resourcePath; }

	std::string getVertexFilePath() const { return vertexFilePath; }
	std::string getFragmentFilePath() const { return fragmentFilePath; }

	std::string getShowcaseModelPath() const { return showcaseModelPath; }

	bool getCanContinue() const { return canContinue; }
};

#endif // ARGUMENTS_H
