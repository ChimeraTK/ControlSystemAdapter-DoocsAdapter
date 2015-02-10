#ifndef __dice__
#define __dice__





#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/generator_iterator.hpp>



// Define a random number generator and initialize it with a reproducible seed.
boost::minstd_rand generator(42);

// Define a uniform random number distribution of integer values between 1 and 6 inclusive.
typedef boost::uniform_int<> 												distribution_type;
typedef boost::variate_generator< boost::minstd_rand&, distribution_type > 	gen_type;

gen_type 								die_gen(generator, distribution_type(1, 6));

/**
 * A simple pseudo-random int number generator iterator with a values range of <1, 6>
 * (a kinda dice)
 * for aiding the unit tests.
 */
boost::generator_iterator<gen_type>		die(&die_gen);





#endif /* __dice__ */

