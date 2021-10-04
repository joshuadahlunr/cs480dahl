#include "arguments.h"

#include <iostream>

Arguments::Arguments(int argc, char **argv){
	// If we weren't given any arguments, skip argument parsing
	if(argc == 1)
		canContinue = false;

	// Foreach argument
	for(int i = 1; i < argc && canContinue; i++){
		std::string arg = argv[i];
		std::string file = arg.substr(2);

		// If the argument is a help command
		if(arg.substr(0, 2) == "-h" || arg.substr(0, 2) == "-?" || arg.substr(0, 6) == "--help"){
			std::cout << std::string(60, '-') << std::endl;
			std::cout << "Arguments" << std::endl;
			std::cout << std::string(60, '-') << std::endl;

			std::cout << "\t-h, -?, --help - Shows this help message" << std::endl;
			std::cout << "\t-v <file> - Sets the vertex shader (relative to the resource/shaders" << std::endl << "\t\tdirectory)" << std::endl;
			std::cout << "\t-f <file> - Sets the fragment shader (relative to the resource/shaders" << std::endl << "\t\tdirectory)" << std::endl;
			std::cout << "\t-m <file> - Sets the obj model (relative to the resource/models" << std::endl << "\t\tdirectory)" << std::endl;
			std::cout << "\t-c <file> - Sets the config file" << std::endl;

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

		// If the argument starts with "-v"
		else if(arg.substr(0, 2) == "-v"){
			// Check if the rest of the argument contains the file
			if(file.find(".glsl") != std::string::npos)
				vertexFilePath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				vertexFilePath = argv[i];
			}
		}

		// If the argument starts with "-f"
		else if(arg.substr(0, 2) == "-f"){
			// Check if the rest of the argument contains the file
			if(file.find(".glsl") != std::string::npos)
				fragmentFilePath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				fragmentFilePath = argv[i];
			}
		}

		// If the argument starts with -m
		else if(arg.substr(0, 2) == "-m"){
			// Check if the rest of the argument contains the file
			if(file.find(".obj") != std::string::npos)
				showcaseModelPath = file;
			// And if it doesn't, the next argument does
			else {
				i++;
				showcaseModelPath = argv[i];
			}
		}

		// If the argument starts with -c
		else if(arg.substr(0, 2) == "-c"){
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
		else if(arg.substr(0, 15) == "--resource-path"){
			i++;
			resourcePath = argv[i];
		}
	}

	// State that we can't continue if either the vertex or fragment shader isn't specified
	canContinue = !vertexFilePath.empty() && !fragmentFilePath.empty() && !showcaseModelPath.empty();// && !configFilePath.empty();

	// If we can't continue provide an error message
	if(!canContinue){
		std::cerr << "To run the program you must specify the vertex shader, fragment shader, model, and config file to load:" << std::endl;
		std::cerr << argv[0] << " -v <file> -f <file> -m <file> -c <file>" << std::endl;
		std::cerr << std::endl << std::string(60, '-') << std::endl;
		std::cerr << "For more infromation display the help menu:" << std::endl;
		std::cerr << argv[0] << " -h" << std::endl;
	}
}
