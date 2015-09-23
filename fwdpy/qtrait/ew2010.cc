/*
  Reference: www.pnas.org/cgi/doi/10.1073/pnas.0906182107
*/
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <vector>
#include <map>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <types.hpp>

using namespace std;

namespace
{
  map<double,double> ew2010_effects_details(gsl_rng * r, const fwdpy::singlepop_t::mlist_t & mutations, const double fourN, const double tau, const double sigma)
  {
    map<double,double> rv;
    for( auto itr = mutations.cbegin() ; itr != mutations.cend() ; ++itr )
      {
	if(!itr->neutral)
	  {
	    if(rv.find(itr->pos) != rv.end())
	      {
		throw runtime_error("multiple mutations at same position");
	      }
	    double d = (gsl_rng_uniform(r) < 0.5) ? -1. : 1.;
	    rv[itr->pos] = d*pow(fourN*std::fabs(itr->s),tau)*(1. + gsl_ran_gaussian_ziggurat(r,sigma));
	  }
      }
    return rv;
  }
}

namespace fwdpy
{
  namespace qtrait
  {
    //returns a list of trait values for each diploid
    vector<double> ew2010_traits_cpp(GSLrng_t * rng,
				     const fwdpy::singlepop_t * pop,
				     const double tau,
				     const double sigma)
    {
      auto effects = ew2010_effects_details(rng->get(),pop->mutations,double(4.*pop->diploids.size()),tau,sigma);
      vector<double> rv;
      const auto sum_lambda = [&effects](const double & sum, const fwdpy::singlepop_t::gamete_t::mutation_list_type_iterator & mitr) {
	if(effects.find(mitr->pos) == effects.end())
	  {
	    throw runtime_error("diploid contains a mutation at an unknown position");
	  }
	return sum + effects[mitr->pos];
      };
      for_each(pop->diploids.cbegin(),pop->diploids.cend(),[&rv,&sum_lambda]( const fwdpy::singlepop_t::diploid_t & dip )
	       {
		 double traitvalue = accumulate(dip.first->smutations.begin(),
						dip.first->smutations.end(),0.,
						sum_lambda) +
		   accumulate(dip.second->smutations.begin(),
			      dip.second->smutations.end(),0.,
			      sum_lambda);
		 rv.push_back(traitvalue);
	       });
      return rv;
    }
  }
}
