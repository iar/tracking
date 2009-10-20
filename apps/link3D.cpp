//Copyright 2009 Thomas A Caswell
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
// 
//If you modify this Program, or any covered work, by linking or
//combining it with IPP (or a modified version of that library),
//containing parts covered by the terms of End User License Agreement
//for the Intel(R) Software Development Products, the licensors of
//this Program grant you additional permission to convey the resulting
//work.
//
//If you modify this Program, or any covered work, by linking or
//combining it with FreeImage (or a modified version of that library),
//containing parts covered by the terms of End User License Agreement
//for FreeImage Public License, the licensors of
//this Program grant you additional permission to convey the resulting


#include <iostream>

#include "master_box_t.h"

#include "particle_base.h"
#include "hash_case.h"
#include "pair.h"
#include "wrapper_o_hdf.h"
#include "wrapper_i_hdf.h"
#include "filter.h"

#include "generic_wrapper_hdf.h"
#include "corr_gofr.h"
#include "gnuplot_i.hpp" //Gnuplot class handles POSIX-Pipe-communication with Gnuplot

using std::cout;
using std::endl;
using std::set;
using std::string;
using std::cerr;


using utilities::Wrapper_o_hdf;
using utilities::Wrapper_i_hdf;

using utilities::Pair;
using utilities::Filter_basic;
using utilities::Filter_trivial;
using utilities::D_TYPE;
using utilities::Generic_wrapper_hdf;


using tracking::master_box_t;
using tracking::particle;
using tracking::hash_case;
using tracking::Corr_gofr;

static string base_proc_path = "/home/tcaswell/colloids/processed/";

int main(int argc, const char * argv[])
{

  if(argc != 3)
  {
    cerr<< "wrong number of args args"<<endl;
    return 0;
  }
  
  
  string file_path = string(argv[1]);
  string file_name = string(argv[2]);
  

  string proc_file = base_proc_path + file_path + file_name + ".h5";
  string out_file = base_proc_path + file_path + "" + ".h5";
  cout<<"file to read in: "<<proc_file<<endl;
  cout<<"file that will be written to: "<<out_file<<endl;

  try
  {
   
    
    // set up data types to import form the input
    D_TYPE tmp[] = {utilities::D_XPOS,
		    utilities::D_YPOS,
		    utilities::D_DX,
		    utilities::D_DY,
		    utilities::D_I,
		    utilities::D_R2,
		    utilities::D_MULT,
		    utilities::D_E
		    };
    set<D_TYPE> data_types = set<D_TYPE>(tmp, tmp+8);

  
    // set up the input wrapper
    Wrapper_i_hdf wh(proc_file,data_types);
    

    // fill the master_box
    master_box_t box;
    Filter_basic filt(proc_file);
    //    Filter_trivial filt;
    box.init(wh,filt);
    

 
  }
  catch(const char * err){
    std::cerr<<"caught on error: ";
    std::cerr<<err<<endl;
  } 


  return 0;
  

}