#include "Random.h"
#include <time.h>

using namespace Imzadi;

Random::Random()
{
	this->generator.seed(this->randomDevice());
}

/*virtual*/ Random::~Random()
{
}

void Random::SetSeed(int seed)
{
	this->generator.seed(seed);
}

void Random::SetSeedUsingTime()
{
	this->generator.seed(time(nullptr));
}

int Random::InRange(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(this->generator);
}

double Random::InRange(double min, double max)
{
	std::uniform_real_distribution<double> distribution(min, max);
	return distribution(this->generator);
}

bool Random::CoinFlip()
{
	std::uniform_int_distribution<> distribution(1, 1000);
	return distribution(this->generator) > 500;
}