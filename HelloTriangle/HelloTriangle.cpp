#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "TriangleApp.h"

/*
 * Entry point for the application. Since all the logic
 * is on the TriangleApp class, here we only create an object
 * of that class and call his run method (which contains the main loop)
 */
int main(int argc, char* argv[]) {
	TriangleApp app;

	try {
		app.run(); // This is where the actual run start
	} catch (const std::exception& e) {
		// This is the only catch on the project
		// so, you can put a break point here to debug
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}