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


#include "part_def.h"
#ifdef TRACKING_FLG
// this code only works for 2D
#if DIM_COUNT == 2
#include <stdexcept>

#include "corr_pair_diffusion.h"
#include "particle_track.h"
#include "generic_wrapper.h"
#include "enum_utils.h"
#include "md_store.h"



using std::runtime_error;
using std::vector;
using std::string;

using utilities::Generic_wrapper;
using utilities::Tuplef;
using utilities::Md_store;
using utilities::format_name;

using tracking::Corr_pair_diffusion;
using tracking::particle;
Corr_pair_diffusion::Corr_pair_diffusion(unsigned int n_dbins,
                                         float max_disp,
                                         unsigned int n_rbins,
                                         float min_r,
                                         float max_r,
                                         unsigned int min_t,
                                         unsigned int t_stride,
                                         unsigned int t_step_count):
  data_prll_(t_step_count,vector<utilities::Histogram>(n_rbins,utilities::Histogram(n_dbins,-max_disp,max_disp))),
  data_perp_(t_step_count,vector<utilities::Histogram>(n_rbins,utilities::Histogram(n_dbins,-max_disp,max_disp))),
  min_r_(min_r),
  max_r_(max_r),
  n_rbins_(n_rbins),
  min_r_square_(min_r*min_r),
  max_r_square_(max_r*max_r),
  min_t_(min_t),
  t_stride_(t_stride),
  t_step_count_(t_step_count)
{
  

}

  


void Corr_pair_diffusion::compute(const particle * p_in,const vector<const particle*> & nhood)
{

  unsigned int max_j = nhood.size();
  Tuplef pos_in = p_in->get_position();
  
  for(unsigned int j = 0; j<max_j;++j)
  {
    // pick out the next particle from the neighborhood 
    const particle* p_nhood= nhood[j];
    //    if(p_in == nhood[j])
    if(p_in == p_nhood)
      continue;
    
    Tuplef pos_nhood = p_nhood->get_position();
    
    // make sure we only do each pair once.
    if( pos_nhood[0] < pos_in[0])
      continue;
    

    float tmp_d = p_in->distancesq(p_nhood);
    if(tmp_d<max_r_square_ && tmp_d > min_r_square_)
    {
      // set up variables
      Tuplef prll_director = pos_nhood - pos_in;
      prll_director.make_unit();
      Tuplef perp_director = Tuplef(-prll_director[1],prll_director[0]);
      
      unsigned int r_ind = (unsigned int) (n_rbins_ * (sqrt(tmp_d) - min_r_)/(max_r_-min_r_));
      
      const particle * p_in_nxt=NULL;
      const particle * p_nhood_nxt=NULL;
            
      // get first set of displacements
      if(!(p_in->step_forwards(min_t_,p_in_nxt) && 
           p_nhood->step_forwards(min_t_,p_nhood_nxt)))
        continue;
      for(unsigned int t = 0; t<t_step_count_;++t)
      {
        Tuplef rel_disp = (p_nhood->get_corrected_disp(p_nhood_nxt) -
                           p_in->get_corrected_disp(p_in_nxt));
        
        data_prll_.at(t).at(r_ind).add_data_point(prll_director.dot(rel_disp));
        data_perp_.at(t).at(r_ind).add_data_point(perp_director.dot(rel_disp));

        // get the new next objects
        if(!(p_in_nxt->step_forwards(t_stride_,p_in_nxt) && 
             p_nhood_nxt->step_forwards(t_stride_,p_nhood_nxt)))
          // if either track does not go that far, break out of this loop
          break;
        
      }
    }
  }
}

                                           

void Corr_pair_diffusion::out_to_wrapper(Generic_wrapper & wrapper_out,
			       const std::string & g_name,
			       const utilities::Md_store * md_store)const
{
  bool opened_wrapper = false;


  if(!wrapper_out.is_open())
  {
    wrapper_out.open_wrapper();
    opened_wrapper = true;
    
  }
 
  wrapper_out.open_group(g_name);

  if(md_store)
    wrapper_out.add_meta_data(md_store);
  // shove in MD stuff 
  wrapper_out.add_meta_data("min_r",min_r_);
  wrapper_out.add_meta_data("max_r",max_r_);
  wrapper_out.add_meta_data("n_rbins",n_rbins_);

  wrapper_out.add_meta_data("min_t",min_t_);
  wrapper_out.add_meta_data("t_stride",t_stride_);
  wrapper_out.add_meta_data("t_step_count",t_step_count_);
  wrapper_out.close_group();    // close the group (has to do with poor state choice in my design)

  std::string t_group_name = g_name + "/time_step";

  std::string edge_name = "edges";
  std::string count_name = "count";
  
  // loop over 
  for( unsigned int j = 0; j < t_step_count_; ++j) {
    Md_store time_level_md(); // shove relevant stuff into unsigned int
    unsigned int t = min_t_ + j*t_stride_;
    std::string local_t_group_name = format_name(t_group_name,j);
    wrapper_out.open_group(local_t_group_name);
    std::cout<<local_t_group_name<<std::endl;
    
    wrapper_out.add_meta_data("time_lag",t);
    wrapper_out.close_group(); 
    for (unsigned int k = 0; k < n_rbins_; ++k)
    {
      std::string rd_perp_name = format_name(local_t_group_name + "/perp_rbin",k);
      std::string rd_prll_name = format_name(local_t_group_name + "/prll_rbin",k);
       
      Md_store group_md;
      group_md.set_value("r_start",min_r_ + ((max_r_ - min_r_)/n_rbins_)*k);
      group_md.set_value("r_end",min_r_ + ((max_r_ - min_r_)/n_rbins_)*(k+1));

      data_prll_[j][k].output_to_wrapper(&wrapper_out,rd_prll_name,count_name,edge_name,&group_md);
      data_perp_[j][k].output_to_wrapper(&wrapper_out,rd_perp_name,count_name,edge_name,&group_md);

      


      
    }
    
    
  }
  
  


  if(opened_wrapper)
    wrapper_out.close_wrapper();
  
}

#endif 
#endif 
