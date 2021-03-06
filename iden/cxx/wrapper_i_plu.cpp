//Copyright 2008,2009 Thomas A Caswell
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

#include<iostream>
#include <stdexcept>
#include "ipp.h"

#include "wrapper_i_plu.h"
#include "part_def.h"

using iden::Wrapper_i_plu;

using utilities::Tuple;

using std::cerr;
using std::cout;
using std::endl;

using std::vector;
using std::set;
using std::runtime_error;


using utilities::Data_map;
using utilities::D_TYPE;
using utilities::D_INDEX;
using utilities::D_XPOS;
using utilities::D_YPOS;
using utilities::D_FRAME;
using utilities::D_I;
using utilities::D_R2;
using utilities::D_E;
using utilities::D_NEXT_INDX;
using utilities::D_PREV_INDX;
using utilities::D_DX;
using utilities::D_DY;
using utilities::D_ORGX;
using utilities::D_ORGY;
using utilities::D_ZPOS;
using utilities::D_TRACKID;
using utilities::D_S_ORDER_PARAMETER ;
using utilities::D_MULT;
using utilities::D_SENTRY;
using utilities::V_ERROR;
using utilities::V_UINT;
using utilities::V_INT;
using utilities::V_FLOAT;
using utilities::V_COMPLEX;
using utilities::V_STRING;
using utilities::V_BOOL;
using utilities::V_TIME;
using utilities::V_GUID;
    

  
void Wrapper_i_plu::add_frame_data(Ipp32f data[][9], int frame,int num)
{
  if(data_.at(frame) !=NULL)
    throw runtime_error("already data at this frame");
  
  data_.at(frame) = data;
  frame_count_.at(frame) = num;
  count_+=num;
  
}

void Wrapper_i_plu::print()const
{
  cout<<"cotains: "<<count_<<endl;
  cout<<"frames: "<<data_.size()<<endl;
}


int Wrapper_i_plu::get_value(int& out,int ind,D_TYPE type, int frame) const
{
  if(V_INT!=v_type(type))
  {
    cerr<<"got "<<type<<" which is a "<<v_type(type)<<" need "<<V_INT<<endl;
    throw runtime_error("wrapper_i_plu: wrong data type, expect int");
  }
  
  if(type == D_FRAME)
  {
    out = frame;
  }
  else
  {
    //  out = (int) (*(*(data_.at(frame) + ind*cols) + data_map_(type)));
    out = (int)(data_.at(frame)[ind][data_map_(type)]);
  }
  
  return out;
}


float Wrapper_i_plu::get_value(float& out,int ind,D_TYPE type, int frame) const
{
  if(V_FLOAT!=v_type(type))
  {
    cerr<<"got "<<type<<" which is a "<<v_type(type)<<" need "<<V_FLOAT<<endl;
    throw runtime_error("wrapper_i_plu: wrong data type, expect float");
  }
  
  //  out = (float)(*(*(data_.at(frame) + ind*cols) + data_map_(type)));
  out = (float)(data_.at(frame)[ind][data_map_(type)]);
  return out;
  
}

Wrapper_i_plu::~Wrapper_i_plu()
{
  for(vector<Ipp32f (*)[9]>::iterator it = data_.begin();
      it!=data_.end();++it)
  {
    delete [] *it;
  }
}


Wrapper_i_plu::Wrapper_i_plu(int frames,Tuple<float,2> dims):data_(frames,NULL),frame_count_(frames,0),count_ (0),dims_(dims)
{

  
    // set the size of the md_store
  set_Md_store_size(frames);

  data_map_.set_lookup(D_XPOS,1);
  data_map_.set_lookup(D_YPOS,2);
  data_map_.set_lookup(D_DX  ,3);
  data_map_.set_lookup(D_DY  ,4);
  data_map_.set_lookup(D_I   ,5);
  data_map_.set_lookup(D_R2  ,6);
  data_map_.set_lookup(D_MULT,7);
  data_map_.set_lookup(D_E   ,8);
 
  D_TYPE tmp[] = {D_XPOS,
		  D_YPOS,
		  D_DX,
		  D_DY,
		  D_I,
		  D_R2,
		  D_MULT,
		  D_E,
		  D_FRAME};
  data_types_ = set<D_TYPE>(tmp, tmp+9);
}


Wrapper_i_plu::Wrapper_i_plu(int frames):data_(frames,NULL),frame_count_(frames,0),count_ (0)
{
  set_Md_store_size(frames);
  
  data_map_.set_lookup(D_XPOS,1);
  data_map_.set_lookup(D_YPOS,2);
  data_map_.set_lookup(D_DX  ,3);
  data_map_.set_lookup(D_DY  ,4);
  data_map_.set_lookup(D_I   ,5);
  data_map_.set_lookup(D_R2  ,6);
  data_map_.set_lookup(D_MULT,7);
  data_map_.set_lookup(D_E   ,8);
 
  D_TYPE tmp[] = {D_XPOS,
		  D_YPOS,
		  D_DX,
		  D_DY,
		  D_I,
		  D_R2,
		  D_MULT,
		  D_E,
		  D_FRAME};
  data_types_ = set<D_TYPE>(tmp, tmp+9);
}

void Wrapper_i_plu::set_dims(const Tuple<float,2>& dims)
{
  dims_ = dims;
}


int Wrapper_i_plu::get_num_entries(unsigned int j) const{
  return frame_count_.at(j);
}
int Wrapper_i_plu::get_num_entries() const{
  return count_;
}

  
bool Wrapper_i_plu::contains_type(utilities::D_TYPE in) const
{
  set<D_TYPE>::iterator it = data_types_.find(in);
  return it != data_types_.end();
}

Tuple<float,2> Wrapper_i_plu::get_dims() const
{
  return Tuple<float,2>(dims_);
}
