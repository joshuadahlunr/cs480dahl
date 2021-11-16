#include <iostream>

#include "application.h"
#include "arguments.h"


int main(int argc, char **argv) {
	// Parse the command line arguments
	Arguments args(argc, argv);
	if(!args.getCanContinue()) return 1;

	// Start an engine
	Application *engine = new Application("Pinball Game", 1000, 1000);
	if(!engine->initialize(args)) {
		printf("The engine failed to start.\n");
		delete engine;
		engine = NULL;
		return 2;
	}

	// run it
	engine->run();

	// Clean it up
	delete engine;
	engine = NULL;
	return 0;
}
