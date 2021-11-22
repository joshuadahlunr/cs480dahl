#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>
#include <json.h>
using json = nlohmann::json;

// Class holding command line arguments and parsed json
class Arguments {
private:
	std::string resourcePath = "../";
	std::string configFilePath = "config.json";

	std::string perFragmentVertexFilePath;
	std::string perFragmentFragmentFilePath;
	std::string perVertexVertexFilePath;
	std::string perVertexFragmentFilePath;

	json config;

	// Variable tracking whether or not we can continue
	bool canContinue = true;
public:
	// Constructor parses the arguments
	Arguments() = default;
	Arguments(int argc, char **argv);
	Arguments(Arguments&) = default;

	std::string getResourcePath() const { return resourcePath; }
	std::string getConfigFilePath() const { return configFilePath; }

	std::string getPerFragmentVertexFilePath() const { return perFragmentVertexFilePath; }
	std::string getPerFragmentFragmentFilePath() const { return perFragmentFragmentFilePath; }
	std::string getPerVertexVertexFilePath() const { return perVertexVertexFilePath; }
	std::string getPerVertexFragmentFilePath() const { return perVertexFragmentFilePath; }

	json getConfig() const { return config; }

	bool getCanContinue() const { return canContinue; }
};

#endif // ARGUMENTS_H
