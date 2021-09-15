#include <iostream>

#include "engine.h"
#include "arguments.h"


int main(int argc, char **argv) {
	// Parse the command line arguments
	Arguments args(argc, argv);
	if(!args.getCanContinue()) return 1;

	// Start an engine and run it then cleanup after
	Engine *engine = new Engine("Tutorial Window Name", 800, 600);
	if(!engine->Initialize(args)) {
		printf("The engine failed to start.\n");
		delete engine;
		engine = NULL;
		return 2;
	}

	engine->Run();

	delete engine;
	engine = NULL;
	return 0;
}
