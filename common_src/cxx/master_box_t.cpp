//Copyright 2009-2011 Thomas A Caswell
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
//combining it with MATLAB (or a modified version of that library),
//containing parts covered by the terms of MATLAB User License, the
//licensors of this Program grant you additional permission to convey
//the resulting work.


#include "master_box_t.h"
#include "accumulator.h"
#include "particle_track.h"
#include "md_store.h"

#include "wrapper_i.h"
#include "filter.h"


using namespace tracking;
using std::endl;
using std::cout;
using tracking::Accumulator;
using std::vector;

using utilities::Md_store;
using utilities::Filter;



Master_box::Master_box()
  :in_wrapper_(NULL),own_wrapper_(false){
  

}



void Master_box::init(const utilities::Wrapper_in & w_in, utilities::Filter & filt )
{
  if(in_wrapper_!=NULL){
    std::cout<<"can't re-initialize"<<std::endl;
    return;
  }
  own_wrapper_ = false;
  
  in_wrapper_ = &w_in;
  
  filt.set_wrapper(in_wrapper_);
  
  priv_init(&filt);
  std::cout<<"finished init of box"<<std::endl;
  
}


void Master_box::priv_init(utilities::Filter const * const filt)
{
  
  
  data_types = in_wrapper_->get_data_types();
  

  
  particle::intialize_wrapper_in(in_wrapper_);
  

  int total_entries= in_wrapper_->get_num_entries();
  cout<<"box: total entries "<<total_entries<<endl;
  
  particle_vec_.reserve(total_entries);
  

  int num_frames = in_wrapper_->get_num_frames();
  
  std::cout<<"number of frames: "<<num_frames<<std::endl;
  
  
  for(int k = 0;k<num_frames;++k)
  {
    int num_entries= in_wrapper_->get_num_entries(k);
    //    std::cout<<"adding: "<<num_entries<<" to frame: "<<k<< std::endl;
    //  int count = 0;
    
    for(int j = 0; j<num_entries; ++j)
    {
      if((*filt)(j,k))
      {
	particle_vec_.push_back( new particle(j,k));
	//	++count;
      }
    }
    //    std::cout<<"added: "<<count<<" total count: "<<particle_vec_.size()<<std::endl;
    
  }
}



void Master_box::print(){
  for(unsigned int j = 0; j<particle_vec_.size();++j)
    particle_vec_.at(j)->print();
}


Master_box::~Master_box(){
  //deletes the particles it made
  for(unsigned int j = 0; j<particle_vec_.size();++j)
  {
    delete particle_vec_.at(j);
  }
  //deletes the wrapper objects
  particle_base::clear_wrapper_in();
  

  if(own_wrapper_)
  {
    delete in_wrapper_;
  }
  
  
//   std::cout<<"mb dead"<<std::endl;
}


void Master_box::compute_accum(Accumulator & in)const
{
  vector<particle*>::const_iterator end_it = particle_vec_.end();
  for(vector<particle*>::const_iterator part_it = particle_vec_.begin();
      part_it != end_it;++part_it)
    in.add_particle(*part_it);
}

const Md_store * Master_box::get_Md_store(unsigned int j) const
{
  return in_wrapper_->get_Md_store(j);
}

  
