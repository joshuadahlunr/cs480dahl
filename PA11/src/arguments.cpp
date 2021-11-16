#include "arguments.h"

#include <iostream>
#include <fstream>

// Constructor parses the arguments
Arguments::Arguments(int argc, char **argv) {
	// If we weren't given any arguments, skip argument parsing
	if(argc == 1)
		canContinue = false;

	// Foreach argument
	for(int i = 1; i < argc && canContinue; i++) {
		std::string arg = argv[i];
		std::string file = arg.substr(2);

		// If the argument is a help command
		if(arg.substr(0, 2) == "-h" || arg.substr(0, 2) == "-?" || arg.substr(0, 6) == "--help") {
			std::cout << std::string(60, '-') << std::endl;
			std::cout << "Arguments" << std::endl;
			std::cout << std::string(60, '-') << std::endl;

			std::cout << "\t-h, -?, --help - Shows this help message" << std::endl;
			std::cout << "\t-c <file> - Sets the config file (relative to the resource directory)\n\t\t[default=config.json]" << std::endl;
			std::cout << "\t-v <file> - Sets the vertex shader (relative to the resource/shaders" << std::endl << "\t\tdirectory)\n\t\t[Can be ommited if specified in the top level of the config file\n\t\twith \"Vertex Shader File Path\"]" << std::endl;
			std::cout << "\t-f <file> - Sets the fragment shader (relative to the resource/shaders" << std::endl << "\t\tdirectory)\n\t\t[Can be ommited if specified in the top level of the config file\n\t\twith \"Fragment Shader File Path\"]" << std::endl;

			std::cout << "Optional" << std::endl;
			std::cout << "\t--resource-path <path> - Sets the resource directory, the directory" << std::endl << "\t\twhere all of the program's resources can be found. [default=../]" << std::endl;

			std::cout << std::string(60, '-') << std::endl;
			std::cout << "Keys" << std::endl;
			std::cout << std::string(60, '-') << std::endl;

			std::cout << "\tR - Pause the rotation of the showcase" << std::endl;
			std::cout << "\tO - Pause the orbit of the showcase" << std::endl;
			std::cout << "\tleft click, L - Reverse the orbit of the showcase" << std::endl;
			std::cout << "\tright click, F - Reverse the rotation of the showcase" << std::endl;
			std::cout << std::endl;

			std::cout << "The orbit and rotation speed of the showcase can be adjusted in the showcase menu." << std::endl;
			std::cerr << std::endl << std::endl;
		}

		// If the argument starts with "-vv"
		else if(arg.substr(0, 3) == "-vv") {
			// Check if the rest of the argument contains the file
			if(file.find(".glsl") != std::string::npos)
				perVertexVertexFilePath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				perVertexVertexFilePath = argv[i];
			}
		}

		// If the argument starts with "-vf"
		else if(arg.substr(0, 3) == "-vf") {
			// Check if the rest of the argument contains the file
			if(file.find(".glsl") != std::string::npos)
				perVertexFragmentFilePath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				perVertexFragmentFilePath = argv[i];
			}
		}

		// If the argument starts with "-v"
		else if(arg.substr(0, 3) == "-fv") {
			// Check if the rest of the argument contains the file
			if(file.find(".glsl") != std::string::npos)
				perFragmentVertexFilePath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				perFragmentVertexFilePath = argv[i];
			}
		}

		// If the argument starts with "-f"
		else if(arg.substr(0, 3) == "-ff") {
			// Check if the rest of the argument contains the file
			if(file.find(".glsl") != std::string::npos)
				perFragmentFragmentFilePath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				perFragmentFragmentFilePath = argv[i];
			}
		}

		// If the argument starts with -c
		else if(arg.substr(0, 2) == "-c") {
			// Check if the rest of the argument contains the file
			if(file.find(".json") != std::string::npos)
				configFilePath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				configFilePath = argv[i];
			}
		}

		// If the argument starts with "--resource-path"
		else if(arg.substr(0, 15) == "--resource-path") {
			i++;
			resourcePath = argv[i];
		}
	}

	// Make sure the config file exists and parse it
	canContinue &= !configFilePath.empty();
	if(canContinue) {
		// Open the config file
		std::ifstream configFile(resourcePath + configFilePath);
		if(!configFile) {
			std::cerr << "Failed to open config file `" << resourcePath + configFilePath << "`" << std::endl;
			canContinue = false;
		}

		// Parse the config file
		try{
			if(canContinue) configFile >> config;
		} catch(json::parse_error& e) {
			std::cerr << "Parsing the JSON config file failed: " << e.what() << std::endl;
			canContinue = false;
		}

		// Close the config file
		configFile.close();
	}

	// Attempt to load the file paths from the config file if they weren't specified on the command line
	if(perVertexVertexFilePath.empty() && config.contains("Per Vertex Vertex Shader File Path"))
		perVertexVertexFilePath = config["Per Vertex Vertex Shader File Path"];
	if(perVertexFragmentFilePath.empty() && config.contains("Per Vertex Fragment Shader File Path"))
		perVertexFragmentFilePath = config["Per Vertex Fragment Shader File Path"];
	if(perFragmentVertexFilePath.empty() && config.contains("Per Fragment Vertex Shader File Path"))
		perFragmentVertexFilePath = config["Per Fragment Vertex Shader File Path"];
	if(perFragmentFragmentFilePath.empty() && config.contains("Per Fragment Fragment Shader File Path"))
		perFragmentFragmentFilePath = config["Per Fragment Fragment Shader File Path"];

	// If we can't continue provide an error message
	canContinue &= !perVertexVertexFilePath.empty() && !perVertexFragmentFilePath.empty() && !perFragmentVertexFilePath.empty() && !perFragmentFragmentFilePath.empty();
	if(!canContinue) {
		std::cerr << "To run the program you must specify the vertex shader, fragment shader, (both per fragment and per vertex) and config file to load:" << std::endl;
		std::cerr << argv[0] << " -vv <file> -vf <file> -fv <file> -ff <file> -c <file>" << std::endl;
		std::cerr << std::endl << std::string(60, '-') << std::endl;
		std::cerr << "For more infromation display the help menu:" << std::endl;
		std::cerr << argv[0] << " -h" << std::endl;
	}
}
