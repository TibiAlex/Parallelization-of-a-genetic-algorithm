#include "genetic_algorithm.h"

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *cores, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4)
	{
		fprintf(stderr, "Usage:\n\t./tema1_par in_file generations_count core_count\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		return 0;
	}


	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2)
	{
		fclose(fp);
		return 0;
	}

	if (*object_count % 10)
	{
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *)calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i)
	{
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2)
		{
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int)strtol(argv[2], NULL, 10);
	*cores = (int)strtol(argv[3], NULL, 10);

	if (*generations_count == 0)
	{
		free(tmp_objects);

		return 0;
	}

	if (*cores == 0) 
	{
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i)
	{
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i)
	{
		for (int j = 0; j < generation[i].chromosome_length; ++j)
		{
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, threadparam param, int sack_capacity)
{
	int weight;
	int profit;

	for (int i = param.start; i < param.end; ++i)
	{
	 	weight = 0;
	 	profit = 0;

		for (int j = 0; j < param.current_generation[i].chromosome_length; ++j)
		{
			if (param.current_generation[i].chromosomes[j])
			{
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		param.current_generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	int i;
	individual *first = (individual *)a;
	individual *second = (individual *)b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0)
	{
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i)
		{
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0)
		{
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) 
	{
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step)
		{
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} 
	else 
	{
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step)
		{
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step)
	{
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i)
	{
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

void *run_genetic_algorithm(void *arg)
{ 
	threadparam param = *(threadparam*)arg;
	int count, cursor;
	individual *tmp = NULL;

	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = param.start; i < param.end; ++i)
	{
		param.current_generation[i].fitness = 0;
		param.current_generation[i].chromosomes = (int *)calloc(param.object_count, sizeof(int));
		param.current_generation[i].chromosomes[i] = 1;
		param.current_generation[i].index = i;
		param.current_generation[i].chromosome_length = param.object_count;

		param.next_generation[i].fitness = 0;
		param.next_generation[i].chromosomes = (int *)calloc(param.object_count, sizeof(int));
		param.next_generation[i].index = i;
		param.next_generation[i].chromosome_length = param.object_count;
	}
	pthread_barrier_wait(param.barrier);

	// iterate for each generation
	for (int k = 0; k < param.generations_count; ++k)
	{
		cursor = 0;
		// compute fitness and sort by it
	 	compute_fitness_function(param.objects, param, param.sack_capacity);
		pthread_barrier_wait(param.barrier);

		qsort(param.current_generation + param.start, param.end - param.start, sizeof(individual), cmpfunc);
		pthread_barrier_wait(param.barrier);

		if (param.id == 0)
		{
			//merge sort pentru portiunile de genratii deja sortate crescator
			for(int t = 0; t < param.cores - 1; t++) {
				int start = (t + 1) * (double)param.object_count / param.cores;
        		int end = min((t + 2) * (double)param.object_count / param.cores, param.object_count);
				individual vec[end];
				int i = 0, j = start, l = 0;
				while (i < start && j < end)
				{
					if (cmpfunc(param.current_generation + i, param.current_generation + j) < 0)
					{
						vec[l++] = param.current_generation[i++];
					} 
					else 
					{
						vec[l++] = param.current_generation[j++];
					}
				}
				// Store remaining elements of first array
				while (i < start)
					vec[l++] = param.current_generation[i++];
				// Store remaining elements of second array
				while (j < end)
					vec[l++] = param.current_generation[j++];
				for(int i = 0; i < end; i++) {
					param.current_generation[i] = vec[i];
				}
			}
		}
		pthread_barrier_wait(param.barrier);

		// keep first 30% children (elite children selection)
		count = param.object_count * 3 / 10;
		int start = param.id * (double)count / param.cores;
        int end = min((param.id + 1) * (double)count / param.cores, count);
		for (int i = start; i < end; ++i)
		{
			copy_individual(param.current_generation + i, param.next_generation + i);
		}
		cursor = count;

		//mutate first 20% children with the first version of bit string mutation
		count = param.object_count * 2 / 10;
		start = param.id * (double)count / param.cores;
        end = min((param.id + 1) * (double)count / param.cores, count);
		for (int i = start; i < end; ++i)
		{
			copy_individual(param.current_generation + i, param.next_generation + cursor + i);
			mutate_bit_string_1(param.next_generation + cursor + i, k);
		}
		cursor += count;

		// mutate next 20% children with the second version of bit string mutation
		count = param.object_count * 2 / 10;
		start = param.id * (double)count / param.cores;
    	end = min((param.id + 1) * (double)count / param.cores, count);
		for (int i = start; i < end; ++i)
		{
			copy_individual(param.current_generation + i + count, param.next_generation + cursor + i);
			mutate_bit_string_2(param.next_generation + cursor + i, k);
		}
		cursor += count;

		//crossover first 30% parents with one-point crossover
		//(if there is an odd number of parents, the last one is kept as such)
		count = param.object_count * 3 / 10;
		if (count % 2 == 1)
		{
			copy_individual(param.current_generation + param.object_count - 1, param.next_generation + cursor + count - 1);
			count--;
		}

		start = param.id * (double)count / param.cores;
        end = min((param.id + 1) * (double)count / param.cores, count);
		for (int i = start; i < end; i+=2)
		{
			if(param.id == param.cores - 1 && i == end - 1)
			{
				continue;
			}
			if(i % 2 == 0)
			{
				crossover(param.current_generation + i, param.next_generation + cursor + i, k);
			}
		}
		pthread_barrier_wait(param.barrier);

		// switch to new generation
		tmp = param.current_generation;
		param.current_generation = param.next_generation;
		param.next_generation = tmp;
		pthread_barrier_wait(param.barrier);

		for (int i = param.start; i < param.end; ++i)
		{
			param.current_generation[i].index = i;
		}
		pthread_barrier_wait(param.barrier);

		if (k % 5 == 0 && param.id == 0)
		{
			print_best_fitness(param.current_generation);
		}
		pthread_barrier_wait(param.barrier);
	}

	compute_fitness_function(param.objects, param, param.sack_capacity);
	pthread_barrier_wait(param.barrier);

	qsort(param.current_generation + param.start, param.end - param.start, sizeof(individual), cmpfunc);
	pthread_barrier_wait(param.barrier);

	if (param.id == 0)
	{
		for(int t = 0; t < param.cores - 1; t++) {
			int start = (t + 1) * (double)param.object_count / param.cores;
			int end = min((t + 2) * (double)param.object_count / param.cores, param.object_count);
			individual vec[end];
			int i = 0, j = start, l = 0;
			while (i < start && j < end)
			{
				if (cmpfunc(param.current_generation + i, param.current_generation + j) < 0)
				{
					vec[l++] = param.current_generation[i++];
				} 
				else 
				{
					vec[l++] = param.current_generation[j++];
				}
			}
			// Store remaining elements of first array
			while (i < start)
				vec[l++] = param.current_generation[i++];
			// Store remaining elements of second array
			while (j < end)
				vec[l++] = param.current_generation[j++];
			for(int i = 0; i < end; i++) {
				param.current_generation[i] = vec[i];
			}
		}
		print_best_fitness(param.current_generation);
	}

	pthread_exit(NULL);
}

void paralel_solved(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *cores) 
{
	pthread_t threads[*cores];
	threadparam ids[*cores];
	int r;
  	long id;
  	void *status;

	individual *current_generation = (individual *)calloc(*object_count, sizeof(individual));
	individual *next_generation = (individual *)calloc(*object_count, sizeof(individual));

	//declare and initializethe barrier
	pthread_barrier_t barrier;
	if (pthread_barrier_init(&barrier, NULL, *cores) != 0)
	{
        printf("\n barrier init has failed\n");
    }

	//start the threads
	for (id = 0; id < *cores; id++)
	{
        int start = id * (double)*object_count / *cores;
        int end = min((id + 1) * (double)*object_count / *cores, *object_count);
        threadparam param;
        param.start = start;
        param.end = end;
		param.current_generation = current_generation;
		param.next_generation = next_generation;
		param.barrier = &barrier;
		param.id = id;
		param.cores = *cores;
		param.object_count = *object_count;
		param.generations_count = *generations_count;
		param.sack_capacity = *sack_capacity;
		param.objects = *objects;
		ids[id] = param;
		r = pthread_create(&threads[id], NULL, run_genetic_algorithm, &ids[id]);
		if (r)
		{
	  		printf("Eroare la crearea thread-ului %ld\n", id);
	  		exit(-1);
		}
  	}

	//join the threads
	for (id = 0; id < *cores; id++) 
	{
		r = pthread_join(threads[id], &status);

		if (r)
		{
	  		printf("Eroare la asteptarea thread-ului %ld\n", id);
	  		exit(-1);
		}
  	}

	// free resources for old generation
	free_generation(current_generation);
	free_generation(next_generation);

	// free resources
	free(current_generation);
	free(next_generation);

	pthread_barrier_destroy(&barrier);
}
