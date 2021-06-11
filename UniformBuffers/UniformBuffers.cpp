#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "UniformBufferApp.h"

int main(int argc, char* argv[]) {
	UniformBufferApp app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}