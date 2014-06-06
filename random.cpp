#include<ctime>
#include<boost/random/random_number_generator.hpp>
#include<boost/random/uniform_int.hpp>
#include<boost/random/mersenne_twister.hpp>
using namespace boost::random;
using namespace boost;

static mt19937 prng;

void randinit (void) {
prng.seed(time(NULL));
}

int randint (int max) {
uniform_int<> u(0,max -1);
return u(prng);
}
