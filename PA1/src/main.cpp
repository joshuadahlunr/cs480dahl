#include <iostream>

#include "engine.h"
#include "arguments.h"


int main(int argc, char **argv) {
	// Parse the command line arguments
	Arguments args(argc, argv);

	// Start an engine and run it then cleanup after
	Engine *engine = new Engine("Tutorial Window Name", 800, 600);
	if(!engine->Initialize(args.getVertexFilePath(), args.getFragmentFilePath())) {
		printf("The engine failed to start.\n");
		delete engine;
		engine = NULL;
		return 1;
	}

	engine->Run();

	delete engine;
	engine = NULL;
	return 0;
}
