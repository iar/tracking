//Copyright 2009,2010 Thomas A Caswell
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
//see website for additional permissions under GNU GPL version 3 section 7
#include<iostream>
#include<cmath>
#include <stdexcept>

#include "hash_case.h"
#include "hash_shelf.h"
#include "hash_box.h"
#include "track_shelf.h"
#include "track_box.h"
#include "particle_track.h"
#include "wrapper_o.h"
#include "corr.h"
#include "accum_sofq.h"
#include "accum_case.h"
#include "corr_case.h"
#include "md_store.h"
using tracking::hash_box;
using tracking::Hash_shelf;
using tracking::hash_case;
using tracking::particle;

using tracking::Corr_case;


using tracking::Track_shelf;
using tracking::Track_box;


using utilities::Wrapper_out;

using utilities::Tuplei;
using utilities::Tuplef;
using utilities::Accum_case;

using tracking::Corr;

using std::set;
using std::vector;
using std::cout;
using std::endl;
using std::ceil;
using std::list;
using std::map;

using std::runtime_error;
using std::logic_error;
using std::exception;
using std::invalid_argument;



// hash output chain
void hash_case::output_to_wrapper(Wrapper_out & wrapper) const
{


  wrapper.add_meta_data_root("dims",h_case_[0]->get_img_dims());
  wrapper.add_meta_data_root("number-of-planes",(int)h_case_.size());
    
  
  for(vector<Hash_shelf*>::const_iterator current_shelf= h_case_.begin();
      current_shelf!=h_case_.end();++current_shelf)				// loop over frames
  {
    (*current_shelf)->output_to_wrapper(wrapper);
  }



}
void Hash_shelf::output_to_wrapper(Wrapper_out & wrapper) const
{
  // cout<<"frame "<<plane_number_<<" contains ";
  
  // cout<<particle_count_<<" particles"<<endl;
  
  wrapper.open_group(plane_number_,particle_count_,max_part_indx_ + 1);
  // try
  // {
  //   wrapper.add_meta_data("z-position",z_offset_,false);
  // }
  // catch(exception & e)
  // {
  //   cout<<e.what()<<endl;
  // }

#ifdef TRACKING_FLG
  // add 
  wrapper.add_meta_data_comp("mean_forward_displacement",mean_forward_disp_);
  wrapper.add_meta_data_comp("cumulative_displacement",cumulative_disp_);
#endif //PYTYPE == 1


  if(md_store_)
    wrapper.add_meta_store(md_store_);
  
  for(vector<hash_box*>::const_iterator current_box = hash_.begin();
      current_box != hash_.end();++current_box)
  {
    hash_box * tmp_box = *current_box;
    if(tmp_box)
      (tmp_box)->output_to_wrapper(wrapper);
  }
  wrapper.close_group();
}

void hash_box::output_to_wrapper(Wrapper_out & wrapper) const
{

  for(vector<particle*>::const_iterator current_part = contents_.begin();
      current_part!=contents_.end();++current_part)
  {
    wrapper.set_all_values(*current_part);
  }
}




// track output chain
void Track_shelf::output_to_wrapper(Wrapper_out & wrapper) const
{

  list<Track_box*>::const_iterator myend =  tracks_.end();
  for(list<Track_box*>::const_iterator it = tracks_.begin();it!=myend;++it)
    (*it)->output_to_wrapper(wrapper);
  

}

void Track_box::output_to_wrapper(Wrapper_out & wrapper) const
{
  wrapper.open_group(id_,length_);
  // add meta data
  wrapper.add_meta_data("init_plane",t_first_->get_frame());
  

  particle_track * cur_part = t_first_;
  for(int j = 0; j< length_;++j)
  {
    wrapper.set_all_values(cur_part);
    cur_part = cur_part->get_next();
  }
  wrapper.close_group();
}
    





void hash_case::pass_fun_to_part(void(particle::*fun)())
{
  vector<Hash_shelf*>::iterator myend =  h_case_.end();
  for(vector<Hash_shelf*>::iterator it = h_case_.begin();
      it!=myend;++it)
  {
    (*it)->pass_fun_to_part(fun);
    
  }
}
void Hash_shelf::pass_fun_to_part(void(particle::*fun)())
{
  vector<hash_box*>::iterator myend =  hash_.end();
  for(vector<hash_box*>::iterator it = hash_.begin();
      it!=myend;++it)
  {
    hash_box *tmp_box= *it;
    if(tmp_box)
      (tmp_box)->pass_fun_to_part(fun);
  }
}
void hash_box::pass_fun_to_part(void(particle::*fun)())
{
  vector<particle*>::iterator myend = contents_.end();
  for(vector<particle*>::iterator it = contents_.begin();
      it!=myend;++it)
  {
    ((*it)->*fun)();
  }
}








void hash_case::pass_fun_to_part(void(particle::*fun)()const)const
{
  vector<Hash_shelf*>::const_iterator myend =  h_case_.end();
  for(vector<Hash_shelf*>::const_iterator it = h_case_.begin();
      it!=myend;++it)
  {
    (*it)->pass_fun_to_part(fun);
    
  }
}
void Hash_shelf::pass_fun_to_part(void(particle::*fun)()const)const
{
  vector<hash_box*>::const_iterator myend =  hash_.end();
  for(vector<hash_box*>::const_iterator it = hash_.begin();
      it!=myend;++it)
  {
    hash_box *tmp_box = *it;
    if(tmp_box)
      (tmp_box)->pass_fun_to_part(fun);
  }
}
void hash_box::pass_fun_to_part(void(particle::*fun)()const)const
{
  vector<particle*>::const_iterator myend = contents_.end();
  for(vector<particle*>::const_iterator it = contents_.begin();
      it!=myend;++it)
  {
    ((*it)->*fun)();
  }
}






void hash_case::pass_fun_to_shelf(void(Hash_shelf::*fun)())
{
  vector<Hash_shelf*>::iterator myend =  h_case_.end();
  for(vector<Hash_shelf*>::iterator it = h_case_.begin();
      it!=myend;++it)
  {
    ((*it)->*fun)();
    
  }
}
void hash_case::pass_fun_to_shelf(void(Hash_shelf::*fun)()const)const
{
  vector<Hash_shelf*>::const_iterator myend =  h_case_.end();
  for(vector<Hash_shelf*>::const_iterator it = h_case_.begin();
      it!=myend;++it)
  {
    ((*it)->*fun)();
    
  }
}

void hash_case::compute_accum(Accum_case & in )const
{
  if(in.size() != h_case_.size())
    throw std::invalid_argument("accumulator and case size do not match");

  for(unsigned int j = 0; j<in.size();++j)
    h_case_[j]->compute_accum(*in[j]);
}


void hash_case::compute_accum(Accumulator & in)const
{
  vector<Hash_shelf*>::const_iterator end_it = h_case_.end();
  for(vector<Hash_shelf*>::const_iterator shelf_it = h_case_.begin();
      shelf_it!= end_it;++shelf_it)
    (*shelf_it)->compute_accum(in);
}



void Hash_shelf::compute_accum(Accumulator & in) const
{
  vector<hash_box*>::const_iterator end_it = hash_.end();
  for(vector<hash_box*>::const_iterator box_it = hash_.begin();
      box_it != end_it; ++box_it)
  {
    hash_box *tmp_box = *box_it;
    if(tmp_box)
      (tmp_box)->compute_accum(in);
  }
  
}

void hash_box::compute_accum(Accumulator & in)const
{
  vector<particle*>::const_iterator end_it = contents_.end();
  for(vector<particle*>::const_iterator part_it = contents_.begin();
      part_it != end_it;++part_it)
    in.add_particle(*part_it);
  

}





void hash_case::fill_in_neighborhood() 
{
  
  for(vector<Hash_shelf*>::iterator shelf_it = h_case_.begin();
      shelf_it!= h_case_.end();++shelf_it)
  {
    (*shelf_it)->fill_in_neighborhood();
  }
}

///@todo change this to use not hash_box objects as containers
void Hash_shelf::fill_in_neighborhood()
{
  //  cout<<"particle_count_"<<particle_count_<<endl;
      
  int buffer = (int)ceil(particle::get_max_range()/upb_);
  if(buffer<1)
  {
    buffer = 1;
  }

  list<particle*> current_box;
  list<particle*> current_region;
  
  for(unsigned int j = 0; j<hash_.size(); ++j)
  {
    current_region.clear();
    current_box.clear();
    
    if(hash_[j]==NULL)
      continue;
    
    hash_[j]->box_to_list(current_box);
    if(current_box.empty())
      continue;
  
    
    get_region(j,current_region,buffer);

    for(list<particle*>::iterator box_part = current_box.begin();
	box_part != current_box.end();++box_part)
    {
      particle* box_part_ptr = *box_part;
      for(list<particle*>::const_iterator region_part = (current_region.begin());
	  region_part!= current_region.end();++region_part)
      {
	const particle* region_part_ptr = *region_part;
	box_part_ptr->add_to_neighborhood(region_part_ptr);
	
      }
      box_part_ptr->sort_neighborhood();
      
    }
  }
  
  
  
}

