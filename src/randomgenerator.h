#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <cstdint>

class RandomGenerator {

	uint64_t _seed;

public:

	uint64_t index;

	RandomGenerator(uint64_t seed);

	/**
	 * Generates a decimal pseudo random number between 0 and 1
	 */
	double get();

	/**
	 * Generates a decimal pseudo random number between min and max
	 * @param min minum retornable value
	 * @param max maximum retornable value
	 * @param rand_sign if true, the returned value sign will be positive or negative at 50/50 chance
	 */
	double random(double min, double max, bool rand_sign = false);

	/**
	 * Generates an integer pseudo random number in the range [min, max]
	 * @param min minum retornable value
	 * @param max maximum retornable value
	 * @param rand_sign if true, the returned value sign will be positive or negative at 50/50 chance
	 */
	int int_random(int min, int max, bool rand_sign = false);

	uint64_t get_seed() { return this->_seed; }

};

#endif
