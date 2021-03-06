//Copyright 2008-2010 Thomas A Caswell
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
#include "particle_base.h"
#include "particle_track.h"
#include "track_box.h"
#include <typeinfo>
#include <iostream>
#include "exception.h"
#include "hash_shelf.h"

using namespace tracking;
using std::set;
using utilities::Ll_range_error;
using utilities::Null_field;
using std::logic_error;
using std::runtime_error;

/*
particle_track::particle_track(Wrapper_in * i_data, 
			       Wrapper_out* o_out, int i_ind, 
			       Track_box* i_track)
  :particle(i_data,  o_out,i_ind),next(NULL),prev(NULL),track(i_track)
  ,n_pos_link(NULL),p_pos_link(NULL){



}
*/
particle_track::particle_track(int i_ind,int frame)
  :particle_base(i_ind,frame),
   n_pos_link(NULL),
   p_pos_link(NULL),
   next_(NULL),
   prev_(NULL),
   track_(NULL) ,
   shelf_(NULL)
{

}

particle_track::particle_track(int i_ind,utilities::Tuplef pos,unsigned int frame)
  :particle_base(i_ind,pos,frame),
   n_pos_link(NULL),
   p_pos_link(NULL),
   next_(NULL),
   prev_(NULL),
   track_(NULL) ,
   shelf_(NULL)
{
}



void particle_track::print_t(int more)const{
  particle_base::print();
  if(more-->0&&next_!=NULL)
    next_->print_t(more);
  
}


void particle_track::set_next(particle_track* n_next){
  if(next_!=NULL)
    throw runtime_error("nuking the list");
  next_ = n_next;

  

}


void particle_track::set_prev(particle_track* n_prev){
  if(prev_!=NULL)
    throw runtime_error("nuking the list");
  prev_ = n_prev;
}


bool particle_track::step_forwards(int n, const particle_track* & dest)const{

  if(n==0)
  {
    dest = this;
    return true;
  }

  if (next_ == NULL)
  {
    dest = NULL;
    return false;
  }
    
  if(n==1)
  {
    dest = next_;
    return true;
  }


  if(n<0)
    throw runtime_error("negative forward step");
  else
  {
    //check for obo bugs
    return next_->step_forwards(--n,dest);
  }
  
}


bool particle_track::step_backwards(int n, const particle_track* & dest)const{

  if (next_ == NULL)
  {
    dest = NULL;
    return false;
  }

  if(n==1)
  {
    dest = prev_;
    return true;
  }
  if(n<1)
  {
    dest = this;
    return true;
  }
    
  
  //check for obo bugs
  return prev_->step_backwards(--n,dest);
}

bool particle_track::step_forwards(int n,  particle_track* & dest){

  
  if(n==0)
  {
    dest = this;
    return true;
  }
  
  if (next_ == NULL)
  {
    dest = NULL;
    return false;
  }

  if(n==1)
  {
    
    dest = next_;
    return true;
  }
  

  if(n<0)
    throw runtime_error("negative forward step");
  else
  {
    //check for obo bugs
    return next_->step_forwards(--n,dest);
  }
  
}


bool particle_track::step_backwards(int n, particle_track* & dest){

  if (prev_ == NULL)
  {
    dest = NULL;
    return false;
  }

  if(n==1)
  {
    dest = prev_;
    return true;
  }
  if(n<1)
  {
    dest = this;
    return true;
  }
    
  
  //check for obo bugs
  return prev_->step_backwards(--n,dest);
}


void particle_track::set_track(Track_box* i_track){
  if(track_!=NULL)
    throw runtime_error("moving between lists!");
  track_ = i_track;

}

Track_box* particle_track::get_track()const{
  if(track_ ==NULL)
  {
    throw Null_field("Particle does not have a track");   
  }
  return track_;
}


int particle_track::get_track_id()const{
  if (track_ ==NULL)
  {
    throw Null_field("Particle does not have a track");   
  }
  return track_->get_id();
}

particle_track::~particle_track(){
  //just to be safe
  delete n_pos_link;
  delete p_pos_link;
  p_pos_link = NULL;
  n_pos_link = NULL;
}

particle_track *  particle_track::reset_track(Track_box * i_track,int & count)
{
  track_ = i_track;
  ++count;
  if(next_!=NULL)
    return next_->reset_track(i_track, count);
  else
    return this;
  
}


float particle_track::distancesq_corrected(const particle_track* part_in)const
{

  if (shelf_ ==NULL)
    {
      throw runtime_error("shelf not defined");
      return 0;
    }

  return (
	  (position_ - shelf_->get_cum_forward_disp()) 
	  - (
	     (part_in->position_ )  - ((part_in->shelf_)->get_cum_forward_disp()))
	  ).magnitude_sqr();

}

const utilities::Tuplef particle_track::get_corrected_disp(const particle_track* part_in)const
{
  
  if (shelf_ ==NULL)
    throw runtime_error("shelf not defined");
  
  if(part_in==NULL)
    throw runtime_error("");
  
  
  
  return (
	  (
	     (part_in->position_ )  - ((part_in->shelf_)->get_cum_forward_disp()))
	  - (position_ - shelf_->get_cum_forward_disp()) 
	  );
}

void particle_track::clear_track_data(){
  next_ = NULL;
  prev_ = NULL;
  track_ = NULL;
}

bool particle_track::has_track()const
{
  if(track_ == NULL)
  {
    return false;
  }
  return true;
}

bool particle_track::has_next()const
{
  if(next_ == NULL)
  {
    return false;
  }
  return true;
}
  
const utilities::Tuplef particle_track::get_corrected_pos()const
{
  if (shelf_ ==NULL)
  {
    throw runtime_error("shelf not defined");
    return utilities::Tuplef();
  }
  return position_ - shelf_->get_cum_forward_disp();
}


void particle_track::clear_data_forward()
{
  particle_track * next_part = next_;
  next_ = NULL;
  if(next_part !=NULL)
  {
    next_part->prev_ = NULL;
    next_part->track_= NULL;
    next_part->clear_data_forward();
  }
  
}


void particle_track::clear_data_backward()
{
  particle_track * next_part = prev_;
  prev_ = NULL;
  if(next_part !=NULL)
  {
    next_part->next_ = NULL;
    next_part->track_= NULL;
    next_part->clear_data_backward();
  }
  
}

  
int particle_track::get_value(utilities::D_TYPE type,int & val) const
{
  if(v_type(type) != utilities::V_INT)
    throw runtime_error("particle_base: wrong V_TYPE");
 
  switch(type)
  {
  case utilities::D_NEXT_INDX:
    if(next_ !=NULL) val = next_->get_ind();
    else val = -1;
    break;
  case utilities::D_PREV_INDX:
    if(prev_ !=NULL)val = prev_->get_ind();
    else val = -1;
    break;
  case utilities::D_TRACKID:
    if(track_!= NULL) val = track_->get_id();
    else val = -1;
    break;
  default:
    particle_base::get_value(type,val);
  }
  
  return val;

}

  
float particle_track::get_value(utilities::D_TYPE type,float &val) const
{
  return particle_base::get_value(type,val);
}

std::complex<float> particle_track::get_value(utilities::D_TYPE type,
			      std::complex<float>& val) const
{
  return particle_base::get_value(type,val);
}

void particle_track::set_shelf(Hash_shelf * shelf)
{
  if(shelf_)
    throw logic_error("particle_track: particle already has shelf_ set");
  shelf_ = shelf;
}
