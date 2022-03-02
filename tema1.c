#include "genetic_algorithm.h"

int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	//number of cores
	int cores = 0;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &cores, argc, argv)) {
		return 0;
	}

	// the function that is going to initialize and start the threads
	paralel_solved(&objects, &object_count, &sack_capacity, &generations_count, &cores);

	free(objects);

	return 0;
}
