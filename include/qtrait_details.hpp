#ifndef __FWDPY_QTRAIT_DETAILS_HPP__
#define __FWDPY_QTRAIT_DETAILS_HPP__

#include "types.hpp"
#include <fwdpp/extensions/regions.hpp>

namespace fwpdy {
namespace qtrait {
  template<typename rules>
  void qtrait_sim_details_t( fwdpy::singlepop_t * pop,
			     gsl_rng * rng,
			     const std::vector<int> & Nvector,
			     const double & neutral,
			     const double & selected,
			     const double & recrate,
			     const double & sigmaE,
			     const double & f,
			     const double optimum ,
			     const bool track,  //do we want to track the trajectories of all mutations?
			     const KTfwd::extensions::discrete_mut_model & m,
			     const KTfwd::extensions::discrete_rec_model & recmap,
			     rules & model_rules)   
  {
    const unsigned simlen = Nvector.size()-1;
    
    const double mu_tot = neutral + selected;
    
    std::function<double(void)> recpos = std::bind(&KTfwd::extensions::discrete_rec_model::operator(),&recmap,rng);
    for( unsigned g = 0 ; g < simlen ; ++g, ++pop->generation )
      {
	KTfwd::experimental::sample_diploid(rng,
					    &pop->gametes,  
					    &pop->diploids, 
					    &pop->mutations,
					    unsigned(Nvector[g]),
					    unsigned(Nvector[g+1]),
					    mu_tot,
					    std::bind(&KTfwd::extensions::discrete_mut_model::make_mut<decltype(pop->mut_lookup)>,&m,rng,neutral,selected,pop->generation,&pop->mut_lookup),
					    std::bind(KTfwd::genetics101(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
						      std::ref(pop->neutral),std::ref(pop->selected),
						      &pop->gametes,
						      recrate,
						      rng,
						      recpos),
					    std::bind(KTfwd::insert_at_end<typename fwdpy::singlepop_t::mutation_t,typename fwdpy::singlepop_t::mlist_t>,std::placeholders::_1,std::placeholders::_2),
					    std::bind(KTfwd::insert_at_end<typename fwdpy::singlepop_t::gamete_t,typename fwdpy::singlepop_t::glist_t>,std::placeholders::_1,std::placeholders::_2),
					    //We use an empty fitness fxn here b/c the rules policies keep track of it separately.
					    []( typename fwdpy::singlepop_t::dipvector_t::const_iterator & ) { return 0.; },
					    KTfwd::remove_nothing(),
					    f,
					    model_rules);
	KTfwd::remove_lost(&pop->mutations,&pop->mut_lookup);
	//This being put here ignores any mutation existing for only 1 generation
	if(track) pop->updateTraj();
	assert(KTfwd::check_sum(pop->gametes,2*Nvector[g+1]));
      }
    //Update population's size variable to be the current pop size
    pop->N = pop->diploids.size();
  }
}
} //namespace

#endif
