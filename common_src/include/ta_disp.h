//Copyright 2012 Thomas A Caswell
//tcaswell@uchicago.edu
//http://jfi.uchicago.edu/~tcaswell
//
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, see <http://www.gnu.org/licenses>.
//
//Additional permission under GNU GPL version 3 section 7
//
//If you modify this Program, or any covered work, by linking or
//combining it with IPP (or a modified version of that library),
//containing parts covered by the terms of End User License Agreement
//for the Intel(R) Software Development Products, the licensors of
//this Program grant you additional permission to convey the resulting
//work.


#ifndef TRK_ACCUM_DISP
#define TRK_ACCUM_DISP

#include "track_accum.h"
#include "histogram.h"

namespace tracking
{

/**
   Track accumulator that computes the distribution of \f$P(\theta)\f$ and \f$P(r)\f$ for
   displacements.
 */
class TA_disp: public Trk_accumulator
{

public:
  // inherited form Trk_accumulator
  void add_disp(const utilities::Tuplef & displacement,unsigned steps);
  unsigned max_step()const {return max_step_;}
  
  /**
     Constructor
   */
  TA_disp(unsigned max_steps,
	     unsigned r_nbins,
             float max_r,
	     unsigned t_nbins);

  /**
     Destructor
   */
  ~TA_disp();
  
  /**
     Output function
  */
  void output_to_wrapper(utilities::Generic_wrapper & out,
			 const utilities::Md_store & md_store)const;
  
private:
  /**
     vector of Histogram objects, one for each time step, for
     squared displacement.
   */
  std::vector<utilities::Histogram> r_disp_;

  /**
     A vector of histograms for 
   */
  std::vector<utilities::Histogram> theta_;
  /**
     Maximum time step
   */
  unsigned max_step_;

};

}
#endif

