#ifndef FWDPY_GWAS_GENOTYPE_MATRIX_HPP
#define FWDPY_GWAS_GENOTYPE_MATRIX_HPP

#include <vector>
#include <algorithm>
#include "types.hpp"
#include "sampler_additive_variance.hpp"

namespace fwdpy
{
  namespace gwas
  {
    struct genotype_matrix
    /*!
      Intended to be coerced to 2D numpy matrix
    */
    {
      std::vector<double> G,E,neutral,causative;
      std::size_t N,n_neutral,n_causative;
      genotype_matrix( std::vector<double> && G_,
		       std::vector<double> && E_,
		       std::vector<double> && n_,
		       std::vector<double> && c_,
		       std::size_t N_,
		       std::size_t nn_,
		       std::size_t ns_) : G(std::move(G_)),
					  E(std::move(E_)),
					  neutral(std::move(n_)),
					  causative(std::move(c_)),
					  N(N_),
					  n_neutral(nn_),
					  n_causative(ns_)
      {
      }
      genotype_matrix()
      {
      }
      genotype_matrix(const genotype_matrix &) = default;
    };

    genotype_matrix make_geno_matrix( const singlepop_t * pop, bool maf = true )
    //! Quick and dirty for ADL & Ted.
    {
      std::vector<std::size_t> neut_indexes,causative_indexes;
      for(std::size_t i=0;i<pop->mcounts.size();++i)
	{
	  if(pop->mcounts[i] && pop->mcounts[i] < 2*pop->diploids.size())
	    {
	      if(pop->mutations[i].neutral)
		{
		  neut_indexes.push_back(i);
		}
	      else
		{
		  causative_indexes.push_back(i);
		}
	    }
	}
      //Use GSL matrixes
      gsl_matrix_ptr_t gn(gsl_matrix_alloc(pop->diploids.size(),neut_indexes.size())),
	gc(gsl_matrix_alloc(pop->diploids.size(),causative_indexes.size()));
      //Fill the matrices, etc.
      std::size_t row = 0;
      std::vector<double> G,E;
      G.reserve(pop->diploids.size());
      E.reserve(pop->diploids.size());
      for(const auto & dip : pop->diploids)
	{
	  G.push_back(dip.g);
	  E.push_back(dip.e);
	  //Fill selected matrix
	  for( auto k : pop->gametes[dip.first].smutations )
	    {
	      auto i = std::find(causative_indexes.begin(),
				 causative_indexes.end(),k);
	      std::size_t col = std::distance(causative_indexes.begin(),i);
	      auto m = gsl_matrix_ptr(gc.get(),row,col);
	      *m += 1.0;	      
	    }
	  for( auto k : pop->gametes[dip.second].smutations )
	    {
	      auto i = std::find(causative_indexes.begin(),
				 causative_indexes.end(),k);
	      std::size_t col = std::distance(causative_indexes.begin(),i);
	      auto m = gsl_matrix_ptr(gc.get(),row,col);
	      *m += 1.0;
	    }
	  //Fill neutral matrix
	  for( auto k : pop->gametes[dip.first].mutations )
	    {
	      auto i = std::find(neut_indexes.begin(),
				 neut_indexes.end(),k);
	      std::size_t col = std::distance(neut_indexes.begin(),i);
	      auto m = gsl_matrix_ptr(gn.get(),row,col);
	      *m += 1.0;	      
	    }
	  for( auto k : pop->gametes[dip.second].mutations )
	    {
	      auto i = std::find(neut_indexes.begin(),
				 neut_indexes.end(),k);
	      std::size_t col = std::distance(neut_indexes.begin(),i);
	      auto m = gsl_matrix_ptr(gn.get(),row,col);
	      *m += 1.0;
	    }
	  ++row;
	}
      return genotype_matrix(std::move(G),std::move(E),
			     std::vector<double>(gn->data,gn->data+(gn->size1*gn->size2)),
			     std::vector<double>(gc->data,gc->data+(gc->size1*gc->size2)),
			     pop->diploids.size(),
			     neut_indexes.size(),
			     causative_indexes.size());
    }
  }
}

#endif
