#include <algorithm>
#include <stdexcept>
#include <limits>
#include <fwdpp/diploid.hh>
#include "sample.hpp"

namespace fwdpy {
  void get_sh_details( const std::vector<std::pair<double,std::string> > & sample,
		       const singlepop_t::mcont_t & mutations,
		       const singlepop_t::mcount_t & mcounts,
		       const size_t & twoN, const unsigned & gen,
		       std::vector<double> * s,
		       std::vector<double> * h,
		       std::vector<double> * p,
		       std::vector<double> * a,
		       std::vector<decltype(KTfwd::mutation_base::xtra)> * l)
  {
    for( const auto & site : sample )
      {
	auto mitr = std::find_if(mutations.begin(),mutations.end(),[&site]( const singlepop_t::mutation_t & m ) {
	     	    return site.first==m.pos;
	  });
	s->push_back(mitr->s);
	h->push_back(mitr->h);
	p->push_back(double(mcounts[std::distance(mutations.begin(),mitr)])/double(twoN));
	a->push_back(double(gen-mitr->g)); //mutation age--this is correct b/c of def'n of 'gen' in the pop objects!
	l->push_back(mitr->xtra); //This is the 'label' assigned to a mutation -- See Regions.pyx for details.
      }
  }
  
  void get_sh( const std::vector<std::pair<double,std::string> > & samples,
	       const singlepop_t * pop,
	       std::vector<double> * s,
	       std::vector<double> * h,
	       std::vector<double> * p,
	       std::vector<double> * a,
	       std::vector<decltype(KTfwd::mutation_base::xtra)> * l)
  {
    get_sh_details(samples,
		   pop->mutations,
		   pop->mcounts,
		   2*pop->diploids.size(),
		   pop->generation,
		   s,h,p,a,l);
  }

  void get_sh( const std::vector<std::pair<double,std::string> > & samples,
	       const metapop_t * pop,
	       std::vector<double> * s,
	       std::vector<double> * h,
	       std::vector<double> * p,
	       std::vector<double> * a,
	       std::vector<decltype(KTfwd::mutation_base::xtra)> * l)
  {
    unsigned ttlN=0;
    for(auto itr = pop->diploids.begin();itr!=pop->diploids.end();++itr)
      {
	ttlN+=itr->size();
      }
    get_sh_details(samples,
		   pop->mutations,
		   pop->mcounts,
		   ttlN,
		   pop->generation,
		   s,h,p,a,l);
  }
}

