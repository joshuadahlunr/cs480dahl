#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>
#include <json.h>
using json = nlohmann::json;

class Arguments {
private:
	std::string resourcePath = "../";
	std::string configFilePath = "config.json";

	std::string vertexFilePath;
	std::string fragmentFilePath;
	std::string showcaseModelPath;

	json config;

	// Variable tracking whether or not we can continue
	bool canContinue = true;
public:
	Arguments(int argc, char **argv);

	std::string getResourcePath() const { return resourcePath; }

	std::string getVertexFilePath() const { return vertexFilePath; }
	std::string getFragmentFilePath() const { return fragmentFilePath; }
	std::string getShowcaseModelPath() const { return showcaseModelPath; }
	std::string getConfigFilePath() const { return configFilePath; }

	json getConfig() const { return config; }

	bool getCanContinue() const { return canContinue; }
};

#endif // ARGUMENTS_H
