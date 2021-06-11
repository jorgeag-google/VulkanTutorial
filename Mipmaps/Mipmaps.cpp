#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "MipmapApp.h"

int main(int argc, char* argv[]) {
	MipmapApp app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}