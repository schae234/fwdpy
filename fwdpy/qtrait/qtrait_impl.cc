/* 
   Models of quantitative traits
   Trait values are additive over 1, 1+hs, 1+2s, where s is a Gaussian deviate

   The infinitely-many sites stuff is an Cython/fwdpp-based re-implementation of the 
   code used to generate preliminary data for R01GM115564.
*/

#include <types.hpp>
#include <fwdpy_internal.hpp>
#include <qtrait_rules.hpp>
#include <fwdpp/diploid.hh>
#include <fwdpp/experimental/sample_diploid.hpp>
#include <fwdpp/sugar/singlepop.hpp>
#include <fwdpp/sugar/popgenmut.hpp>

#include <qtrait_details.hpp>
#include <thread>
#include <algorithm>
#include <memory>
#include <limits>

#include <gsl/gsl_statistics_double.h>

//Libsequence
#include <Sequence/PolySIM.hpp>
//Note: requires libseq >= 1.8.5!!!!
#include <Sequence/PolyTableSlice.hpp>

using namespace std;
using namespace Sequence;

using poptype = fwdpy::singlepop_t;

namespace fwdpy
{
  namespace qtrait
  {
    void evolve_qtraits_t( GSLrng_t * rng, std::vector<std::shared_ptr<singlepop_t> > * pops,
			   const unsigned * Nvector,
			   const size_t Nvector_length,
			   const double mu_neutral,
			   const double mu_selected,
			   const double littler,
			   const double f,
			   const double sigmaE,
			   const double optimum,
			   const double VS,
			   const int track,			   
			   const fwdpy::internal::region_manager * rm)
    {
      const KTfwd::extensions::discrete_mut_model m(rm->nb,rm->ne,rm->nw,rm->sb,rm->se,rm->sw,rm->callbacks);
      auto recmap = KTfwd::extensions::discrete_rec_model(rm->rb,rm->rw,rm->rw);
      std::vector<GSLrng_t> rngs;
      std::vector<qtrait_model_rules> rules;
      for(unsigned i=0;i<pops->size();++i)
	{
	  //Give each thread a new RNG + seed
	  rngs.emplace_back(GSLrng_t(gsl_rng_get(rng->get())) );
	  rules.emplace_back(qtrait_model_rules(sigmaE,optimum,VS,*std::max_element(Nvector,Nvector+Nvector_length)));
	}
      std::vector<std::thread> threads(pops->size());
      for(unsigned i=0;i<pops->size();++i)
	{
	  threads[i]=std::thread(fwdpy::qtrait::qtrait_sim_details_t<qtrait_model_rules>,
				 rngs[i].get(),
				 pops->operator[](i).get(),
				 Nvector,Nvector_length,
				 mu_neutral,mu_selected,littler,f,sigmaE,optimum,track,
				 std::cref(m),std::cref(recmap),
				 std::ref(rules[i]));
	}
      for(unsigned i=0;i<threads.size();++i) threads[i].join();
    }

    //Get properties out from the population
    std::map<string,double> qtrait_pop_props( const fwdpy::singlepop_t * pop )
    {
      std::vector<double> VG,VE,wbar,esize;
      
      //Get data from the diploids
      for( auto itr = pop->diploids.cbegin() ; itr != pop->diploids.cend() ; ++itr )
	{
	  VG.push_back( itr->g );
	  VE.push_back( itr->e );
	  wbar.push_back( itr->w );
	}
      
      //Get data from the mutations
      for( auto itr = pop->mutations.cbegin() ; itr != pop->mutations.cend() ; ++itr )
	{
	  esize.push_back( itr->s );
	}
      
      //Find the "leading factor"
      double twoN = 2.*double(pop->diploids.size());
      auto itr = std::max_element(pop->mutations.cbegin(),pop->mutations.cend(),
				  [&twoN]( const poptype::mutation_t & m1,
					   const poptype::mutation_t & m2 ) {
				    double p1 = double(m1.n)/twoN,p2=double(m2.n)/twoN;
				    return p1*(1.-p1)*std::pow(m1.s,2.) < p2*(1.-p2)*std::pow(m2.s,2.);
				  });
      
      double mvexpl = std::numeric_limits<double>::quiet_NaN(),leading_e=std::numeric_limits<double>::quiet_NaN(),leading_f=std::numeric_limits<double>::quiet_NaN();
      if(itr != pop->mutations.end())
	{
	  mvexpl = 2.*(double(itr->n)/twoN)*(1.-(double(itr->n)/twoN))*std::pow(itr->s,2.);
	  leading_e = itr->s;
	  leading_f = double(itr->n)/twoN;
	}
      map<string,double> rv;
      rv["VG"] = gsl_stats_variance(&VG[0],1,VG.size());
      rv["VE"] = gsl_stats_variance(&VE[0],1,VE.size());
      rv["H2"] = rv["VG"]/(rv["VG"]+rv["VE"]);
      rv["wbar"] = gsl_stats_mean(&wbar[0],1,wbar.size());
      rv["max_expl"] = mvexpl;
      rv["leading_e"] = leading_e;
      rv["leading_q"] = leading_f;
      return rv;
    }

    map<string,vector<double> > get_qtrait_traj(const singlepop_t *pop,const unsigned minsojourn,const double minfreq)
    {
      std::vector<double> pos,freq,s;
      std::vector<double> generations;
      /*
	Key is origin, (pos,s), trajectories
      */
      //using trajtype = std::map< std::pair<unsigned,std::pair<double,double> >, std::vector<double> >;
      for( poptype::trajtype::const_iterator itr = pop->trajectories.begin() ;
	   itr != pop->trajectories.end() ; ++itr )
	{
	  double maxfreq = *std::max_element(itr->second.cbegin(),itr->second.cend());
	  if( itr->second.size() >= minsojourn && maxfreq >= minfreq )
	    {
	      std::vector<unsigned> times(itr->second.size());
	      unsigned itime = itr->first.first;
	      std::generate(times.begin(),times.end(),[&itime]{ return itime++; });
	      generations.insert(generations.end(),times.begin(),times.end());
	      std::fill_n(std::back_inserter(pos),itr->second.size(),itr->first.second.first);
	      std::fill_n(std::back_inserter(s),itr->second.size(),itr->first.second.second);
	      std::copy(itr->second.begin(),itr->second.end(),std::back_inserter(freq));
	    }
	}
      map<string,vector<double>> rv;
      rv["pos"]=std::move(pos);
      rv["freq"]=std::move(freq);
      rv["generation"]=std::move(generations);
      rv["esize"]=std::move(s);
      return rv;
    }

    map<string,vector<double> > qtrait_esize_freq(const singlepop_t * pop)
    {
      double twoN = 2.*double(pop->diploids.size());
      std::vector<double> esize,freq;
      for( const auto & __m : pop->mutations )
	{
	  esize.push_back(__m.s);
	  freq.push_back(double(__m.n)/twoN);
	}
      map<string,vector<double>>rv;
      rv["esize"]=std::move(esize);
      rv["freq"]=std::move(freq);
      return rv;
    }
  } //ns qtrait
} //ns fwdpy

  
