
#include "randomgenerator.h"

#include <cmath>


RandomGenerator::RandomGenerator(uint64_t seed) :
	_seed(seed),
	index(seed)
{

}


double RandomGenerator::get() {

	/**
	 * This algorithm is definitely not pseudo random but it is random
	 * enough for our purpose
	 */

	auto i = this->index++;

	auto out = sin(i) + sin(i / 3) + sin(i / 5) + sin(i / 7);
	out /= 4;
	if (out < 0) {
		out += 1;
	}
	
	return out;

}


double RandomGenerator::random(double min, double max, bool rand_sign) {

	double rnd_factor = this->get();
	//cout << "max: " << real_max << " | rnd_factor: " << rnd_factor << endl;
	double num = min + rnd_factor * (max - min);
	
	if (rand_sign && this->get() > 0.5) {
		num = -num;
	}
	
	return num;
}

int RandomGenerator::int_random(int min, int max, bool rand_sign) {

	//int res = (int) this->random(min, max + 1, rand_sign, false);
	//cout << "[" << min << ", " << max << "] --> " << res << endl;
	

	int res = (int) random(0, max - min + 1, false);
	//cout << "[" << min << ", " << max << "] --> " << res << endl;
	res += min;

	if (rand_sign && this->get() > 0.5) {
		res = -res;
	}
	/*
	std::stringstream logg;
	logg << "R" << count_r++ << ": "<< res << endl;
	log(logg.str());
	*/
	return res;

}