#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "MultisamplingApp.h"

int main(int argc, char* argv[]) {
	MultisamplingApp app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}