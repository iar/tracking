//Copyright 2010,2012 Thomas A Caswell
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



#include "md_store.h"
#include "enum_utils.h"


#include <string>
#include <stdexcept>
#include <list>
#include <iostream>
#include <sstream>
#include <limits.h>

#include <unistd.h>
#include <errno.h>


using utilities::Md_store;


using std::string;
using std::vector;
using std::list;

using std::cerr;
using std::cout;
using std::endl;

using std::logic_error;
using std::runtime_error;

//taken from
//http://www.codeguru.com/forum/showthread.php?t=231054
// the third parameter of from_string() should be 
// one of std::hex, std::dec or std::oct
template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

template <class T>
std::string to_string(T in)
{
  std::ostringstream o;
  o<<in;
  return o.str();
}


Md_store::Md_store(){}

		     

void Md_store::add_element(const std::string & key,const std::string & type, const std::string & value)
{
  if(contains_key(key))
    throw runtime_error("can't add a second copy of a key");
  

  int * tmpi_p;
  int tmpi;
  unsigned int * tmpui_p;
  unsigned int tmpui;
  float * tmpf_p;
  float tmpf;
  std::string * tmps_p;
  
  bool * tmpb_p;
  

  
  V_TYPE itype = str2VT_s(type);
  
  
  switch(itype)
  {
  case V_UINT:
    if(!from_string<unsigned int> (tmpui,value,std::dec))
      throw runtime_error("failed to parse");
    tmpui_p = new unsigned int(tmpui);
    entries_.push_back(Md_element(key,itype,(void*) tmpui_p));    
  break;
  
 case V_INT:
   if(!from_string<int> (tmpi,value,std::dec))
     throw runtime_error("failed to parse");
   tmpi_p = new  int(tmpi);

   entries_.push_back(Md_element(key,itype,(void*) tmpi_p));    
   break;
  case V_FLOAT:
    if(!from_string<float> (tmpf,value,std::dec))
      throw runtime_error("failed to parse");
    tmpf_p = new float(tmpf);
    entries_.push_back(Md_element(key,itype,(void*) tmpf_p));    
    break;
    
  case V_STRING:
  case V_TIME:
  case V_GUID:
    tmps_p = new std::string(value);
    entries_.push_back(Md_element(key,itype,(void*) tmps_p));    
    break;
  case V_BOOL:

    if(value.compare("on" )==0||value.compare("true")==0)
      tmpb_p = new bool(true);
    else
      tmpb_p = new bool(false);
    entries_.push_back(Md_element(key,itype,(void*) tmpb_p));    
    break;
  case V_ERROR:
  case V_COMPLEX:

    throw logic_error("type " + type + " not supported");
    
  }
  

  
  

}

void Md_store::add_element(const char * key,const char * type, const char * value)
{  
  add_element(string(key),string(type),string(value));
  
}

void Md_store::add_element(const char * key,float val)
{
  float * tmpf_p = new float(val);
  entries_.push_back(Md_element(key,utilities::V_FLOAT,(void * ) tmpf_p));
}

void Md_store::add_element(const char * key,int val)
{
  int * tmpi_p = new int(val);
  entries_.push_back(Md_element(key,utilities::V_INT,(void *)tmpi_p));
}


void Md_store::add_element(const char * key,unsigned int val)
{
  unsigned  int * tmpi_p = new unsigned int(val);
  entries_.push_back(Md_element(key,utilities::V_UINT,(void * )tmpi_p));
}

void Md_store::add_element(const char * key,bool val)
{
  bool * tmpb_p = new bool(val);
  entries_.push_back(Md_element(key,utilities::V_BOOL,(void * )tmpb_p));
}


void Md_store::add_element(const char * key,const char * val)
{
  std::string * tmps_p = new std::string(val);
  
  entries_.push_back(Md_element(key,utilities::V_STRING,(void * ) tmps_p));
}



void Md_store::add_element(const char * key,const string & val)
{
  std::string * tmps_p = new std::string(val);
  
  entries_.push_back(Md_element(key,utilities::V_STRING,(void * ) tmps_p));
}


void Md_store::print() const
{
  for(unsigned int j = 0; j<entries_.size();++j)
  {
    V_TYPE type = entries_[j].type;
    
    cout<<'('<<entries_[j].key<<','<<VT2str_s(type)<<',';
    switch(type)
    {
    case V_UINT:
      cout<<*(unsigned int *) entries_[j].value;
      break;
    case V_INT:
      cout<< *(int *) entries_[j].value;
        break;
      case V_FLOAT:
        cout<<* (float *) entries_[j].value;
        break;
      case V_BOOL:
        cout<<* (bool *) entries_[j].value;    
        break;
      case V_STRING:
      case V_TIME:
      case V_GUID:
        cout<<* (std::string *) entries_[j].value;
        break;
      case V_ERROR:
      case V_COMPLEX:

        break;
      }


    cout<<')'<<endl;

  }
  
}

  
Md_store::~Md_store()
{
}

int Md_store::get_key_index(const string& key) const
{
  int max = entries_.size();
  for(int j = 0;j<max;++j)
    if(entries_[j].key.compare(key) == 0)
      return j;
  throw logic_error("Md_store::does not contain key: " + key);
}


bool Md_store::contains_key(const string& key) const
{
  unsigned int tmp;
  return contains_key(key,tmp);
  
}


bool Md_store::contains_key(const string& key,unsigned int & indx) const
{
  unsigned int max = entries_.size();
  for(indx = 0;indx<max;++indx)
    if(entries_[indx].key.compare(key) ==0)
      return true;
  
  ++indx;
  
  return false;
}


float Md_store::get_value(int j,float & val)const
{
  if(entries_[j].type == utilities::V_FLOAT)
  {
    val = *(float *) entries_[j].value;
    return val;
  }
  
  else
    throw logic_error("Md_store::get_value, expect pram of type: float, found type: " + VT2str_s(entries_[j].type) + ": " + entries_[j].key );
}


int Md_store::get_value(int j,int & val)const
{
  V_TYPE type = entries_[j].type;
  // deal with if the signedness matches
  if(type == utilities::V_INT)
  {
    val = * (int * ) entries_[j].value;
    return val;
    
  }
  // deal with if the signs don't match
  else if(type  == utilities::V_UINT)
  {
    unsigned int tmp_ui = *(unsigned int *)entries_[j].value;
    
    if(tmp_ui>INT_MAX)
      throw runtime_error("Md_store :: trying to cast an unsigned int to an int that is too large: "
			  + VT2str_s(entries_[j].type) + ": " + entries_[j].key);
    val = tmp_ui;
    return val;
    
  }
  // deal with if the class is wrong
  else
    throw logic_error("Md_store::get_value, expect pram of type: int, found type: " 
		      + VT2str_s(entries_[j].type) + ": " + entries_[j].key );
}


unsigned int Md_store::get_value(int j,unsigned int & val)const
{
  V_TYPE type = entries_[j].type;
  if(type == utilities::V_UINT)
  {
    val =  *(unsigned int *)entries_[j].value;
    return val;
  }
  // deal with if the signs don't match
  else if(type  == utilities::V_INT)
  {
    int tmp_i  =  *(int *)entries_[j].value;
    if(tmp_i<0)
      throw runtime_error("Md_store :: trying to cast a negative int to an unsigned int: "+ 
                          VT2str_s(entries_[j].type) + ": " + entries_[j].key );
    val = tmp_i;
    return val;
    
  }
  
  else
    throw logic_error("Md_store::get_value, expect pram of type: int, found type: " + VT2str_s(entries_[j].type) + ": " + entries_[j].key );
}



string Md_store::get_value(int j,string & val)const
{
  V_TYPE val_type = entries_[j].type;
  

  if(val_type == utilities::V_STRING ||
     val_type == utilities::V_GUID ||
     val_type == utilities::V_TIME)
  {
    val = *(string * ) entries_[j].value;
    return val;
  }
  else
    throw logic_error("Md_store::get_value, expect pram of type: string, found type: " + VT2str_s(entries_[j].type) + ": " + entries_[j].key );
}


bool Md_store::get_value(int j,bool & val)const
{
  if(entries_[j].type == utilities::V_BOOL)
  {
    val = *(bool *)entries_[j].value; ;
    return val;
    
  }
  else
    throw logic_error("Md_store::get_value, expect pram of type: float, found type: " + VT2str_s(entries_[j].type) + ": " + entries_[j].key );
}


template <class T>
T Md_store::get_value(const string& key,T & val)const
{
  int j = get_key_index(key);
  return get_value(j,val);
  
}

template int Md_store::get_value(const string& key,int & val)const;
template unsigned int Md_store::get_value(const string& key,unsigned int & val)const;
template float Md_store::get_value(const string& key,float & val)const;
template string Md_store::get_value(const string& key,string & val)const;
template bool Md_store::get_value(const string& key,bool & val)const;


template <class T>
void Md_store::set_value(const string& key,const T & val,bool over_write)
{
  if(over_write)
    remove_key(key);
  add_element(key.data(),val);
}

template void Md_store::set_value(const string& key,const int & val,bool);
template void Md_store::set_value(const string& key,const unsigned int & val,bool);
template void Md_store::set_value(const string& key,const float & val,bool);
template void Md_store::set_value(const string& key,const string & val,bool);
template void Md_store::set_value(const string& key,const bool & val,bool);

bool Md_store::remove_key(const std::string & key)
{

  vector<Md_element>::iterator it_end = entries_.end();
  for(vector<Md_element>::iterator it = entries_.begin();
      it != it_end;++it)
  {
    if((*it).key.compare(key) ==0)
    {
      entries_.erase(it);
      return true;
    }
  }
  return false;
  

}



void Md_store::add_elements(const Md_store * in)
{
  vector<Md_element>::const_iterator it_end = in->entries_.end();
  
  for(vector<Md_element>::const_iterator it = in->entries_.begin();
      it != it_end;++it)
    entries_.push_back(*it);
  
}


Md_store::Md_element::~Md_element()
{

  switch(type)
  {
  case V_UINT:
    delete (unsigned int *) value;
    break;
  
  case V_INT:
    delete (int *) value;
    break;
  case V_FLOAT:
    delete (float *) value;
    break;
  case V_BOOL:
    delete (bool *) value;    
    break;
    
  case V_STRING:
  case V_TIME:
  case V_GUID:
    delete (std::string *) value;
    break;
  case V_ERROR:
  case V_COMPLEX:

    break;
  }
      
}
Md_store::Md_element::Md_element(const  Md_element &in)
{
  type = in.type;
  key = in.key;


  switch(type)
  {
  case V_UINT:
    value =(void *) new unsigned int(* (unsigned int *) in.value);
    break;
  
  case V_INT:
    value =(void *) new int(* (int *) in.value);

    break;
  case V_FLOAT:
    value =(void *) new float(* (float *) in.value);
    break;
  case V_BOOL:
    value =(void *) new bool(* (bool *) in.value);
    break;
    
  case V_STRING:
  case V_TIME:
  case V_GUID:
    value =(void *) new string(* (string *) in.value);
    break;
  case V_ERROR:
  case V_COMPLEX:
    break;
  }
  
}


Md_store::Md_element & Md_store::Md_element::operator =(const Md_element &other)
{
  

  if(this != &other)
  {
    type = other.type;
    key = other.key;
    void * tmp;
    
    switch(type)
    {
    case V_UINT:
      tmp =(void *) new unsigned int(* (unsigned int *) other.value);
      delete (unsigned int *) value;
      value = tmp;
    
      break;
  
    case V_INT:
      tmp =(void *) new int(* (int *) other.value);
      delete (int *) value;
      value = tmp;
      break;
    case V_FLOAT:
      tmp =(void *) new float(* (float *) other.value);
      delete (float *) value;
      value = tmp;
      break;
    case V_BOOL:
      tmp =(void *) new bool(* (bool *) other.value);
      delete (bool*)value;
      value = tmp;
      break;
      
    case V_STRING:
    case V_TIME:
    case V_GUID:
      tmp =(void *) new string(* (string *) other.value);
      delete (string*)value;
      value = tmp;
      break;
    case V_ERROR:
    case V_COMPLEX:
      break;
    }
      

  }
  

  return *this;
  
}
