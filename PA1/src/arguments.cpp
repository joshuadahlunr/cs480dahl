#include "arguments.h"

#include <iostream>

Arguments::Arguments(int argc, char **argv){
	// Foreach argument
	for(int i = 1; i < argc; i++){
		std::string arg = argv[i];
		std::string file = arg.substr(2);

		// If the argument starts with "-v"
		if(arg.substr(0, 2) == "-v"){
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
	}
}

std::string Arguments::getVertexFilePath(){
	return vertexFilePath;
}

std::string Arguments::getFragmentFilePath(){
	return fragmentFilePath;
}
