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
#include "hash_shelf.h"
#include <exception>
#include <stdexcept> // out_of_range exception

#include<iostream>

using namespace tracking;
using std::list;

//Constructor.
/*
   Constructs a hash table from the particles contained in
   the master box.  The imagesize is the size in pixel of
   the initial data set.  This is use in initialization and 
   as a reference later.  ppb is the pixels per box, a single
   box in the hash table is ppb on a side.  the hash boxes
   are always square.  If the image size is is not a multiple
   of ppb the edges of the grid will hang off of the image.
*/

// hash_shelf::hash_shelf(master_box & mb,unsigned int imsz1, 
// 		       unsigned int imsz2, unsigned int PPB):  ppb(PPB),
// 							       plane_number(0)
// {
//   img_dims.push_back(imsz1);
//   img_dims.push_back(imsz2);
//   init2();
  
//   for(unsigned int j = 0; j<mb.get_size();j++)
//     {
//       particle_base* p = mb.get_particle(j);
//       push(p);
//     }
//     }




void hash_shelf::push(particle_base * p){
  try{
    (hash.at(hash_function(p))).push(p);
  }
  catch (std::exception& e)    {
    cout << e.what() << endl;
    std::cout<<hash_function(p)<<"\t"<<hash.size()<<std::endl;
    std::cout<<p->get_value(wrapper::d_ypos)<<"\t"<<p->get_value(wrapper::d_xpos)<<std::endl;
    p->print();
    throw;

  }
  catch(...){
    std::cout<<"the problem is here"<<std::endl;

    return;
  }    
//   cout<<"particle: "<<endl;
//   p->print();
//   cout<<"hash_box: "<<hash_function(p)<<endl;
}

hash_shelf::hash_shelf(unsigned int imsz1, 
		       unsigned int imsz2, unsigned int PPB,
		       int i_frame):  ppb(PPB), plane_number(i_frame)
{
  img_dims.push_back(imsz1);
  img_dims.push_back(imsz2);
  init2();
}

hash_shelf::hash_shelf(vector<int> imgsz, unsigned int ippb, int i_frame):
  img_dims(imgsz),  ppb(ippb),  plane_number(i_frame)
{  
  init2();
}


// void hash_shelf::init(unsigned int X, unsigned int Y){
//   sz1 = X;
//   sz2 = Y;
//   hash.reserve(sz1*sz2);
//   for(unsigned int j = 0; j<sz1*sz2;j++)
//     hash.push_back(hash_box());
// }

void hash_shelf::init2(){
  hash_dims.clear();
  hash.clear();
  for(vector<int>::iterator it = img_dims.begin();
      it<img_dims.end(); it++)
    hash_dims.push_back((*it)%ppb==0?(*it)/ppb:((*it)/ppb)+1);
  // tac 2009-03-11
  // hash_dims.push_back((*it)%ppb==0?(*it)/ppb:(1+(*it)/ppb)+1);


  int tmp_prod = 1;
  for(vector<int>::iterator it = hash_dims.begin();
      it<hash_dims.end(); it++)
    tmp_prod*=(*it);
  


  hash.reserve(tmp_prod);
  for(int j = 0; j<tmp_prod;j++)
    hash.push_back(hash_box());
}


void hash_shelf::print(){
  
  for(int j=0; j<hash_dims[0];j++){
    for(int i=0; i<hash_dims[1];i++){
      cout<<(hash.at(j*hash_dims[0] + i)).get_size() <<"\t";
    }
    cout<<endl;
  }
      

}


  ///appends the region of (2*range +1) on a side from
  ///the hash table centered on the box (n,m)  deals
  ///with boxes with in range of the edges by truncating
  ///the volume returned.  Functions that use this need
  ///to keep track of this them selves
void hash_shelf::get_region( int n, int m,
			     hash_box* box,int range){
  
  if(n<0||m<0||range<0||box==NULL)
    return;


   int j_bot = (((n-range)>=0)?(n-range):0);
   int j_top = ((n+range)<((int)hash_dims[0]-1)
		?(n+range+1):
		(hash_dims[0]-1));
   int k_bot = ((m-range)>0?(m-range):0);
   int k_top = ((m+range)<((int)hash_dims[1]-1)?(m+range+1):(hash_dims[1]-1));
	    
//     cout<<"n: "<<n<<"\t"
//         <<"m: "<<m<<"\t"
//         <<j_bot<<"\t"
//         <<j_top<<"\t"
//         <<k_bot<<"\t"
//         <<k_top<<"\t"<<endl;



     for( int j = j_bot; j<=j_top;j++){
       for( int k = k_bot; k<=k_top;k++){

	 box->append(&(hash.at(j+hash_dims[0]*k)));
	 //       cout<<j<<"\t"<<k<<endl;
	 //       cout<<"appending box at: "<<j+hash_dims[0]*k<<endl;

       }
     }
     
     //   cout<<"+++++++++++++++"<<endl;
   


     
}



void hash_shelf::get_region( int n, 
			     hash_box* box,int range){
  get_region(n%hash_dims[0], n/hash_dims[0], box, range);

}

void hash_shelf::get_region( particle_base* n,
		   hash_box* box, int range){
//   cout<<"+++++++++++++++"<<endl;
//   n->print();
  get_region(hash_function(n),box,range);

  }

  /**
     Remakes the hash table using the same particles using  new
     particle per box count
     @param PPB
     new particle per box value
   */
void hash_shelf::rehash(unsigned int PPB){
  list<particle_track*> *tmp = shelf_to_list();

  ppb = PPB;
  //rebuilds the hash table
  init2();
  for(list<particle_track*>::iterator it = tmp->begin(); it!=tmp->end(); it++)
      push(*it);
  
  delete tmp;

}

list<particle_track*> * hash_shelf::shelf_to_list(){
  list<particle_track*>* tmp = new list<particle_track*>;
  for(vector<hash_box>::iterator it = hash.begin(); it<hash.end(); it++)
    {
      
      for(vector<particle_base*>::iterator it2 = (*it).begin();
	  it2<(*it).end(); it2++)
	{
	  tmp->push_back(static_cast<particle_track*>(*it2));
	}
    }

  return tmp;
}

int hash_shelf::img_area(){
  int tmp = 1;
  for(int j = 0; j<img_dims.size();++j)
    tmp*=img_dims[j];
  cout<<tmp<<endl;
  return tmp;

}
