#include <iostream>

#include "engine.h"
#include "arguments.h"


int main(int argc, char **argv) {
	// Parse the command line arguments
	Arguments args(argc, argv);
	if(!args.getCanContinue()) return 1;

	// Start an engine
	Engine *engine = new Engine("Pinball Game", 1000, 1000);
	if(!engine->Initialize(args)) {
		printf("The engine failed to start.\n");
		delete engine;
		engine = NULL;
		return 2;
	}

	// Run it
	engine->Run();

	// Clean it up
	delete engine;
	engine = NULL;
	return 0;
}
