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

//standard includes
#include<iostream>
#include<fstream>
#include <exception>

//my includes
#include "wrapper.h"
#include "wrapper_i.h"
#include "wrapper_o.h"

#include "particle.h"
#include "params.h"


#include "hash_box.h"
#include "hash_shelf.h"
#include "track_box.h"
#include "master_box_t.h"
#include "hash_case.h"

#include "wrapper_i_matlab.h"
#include "params_matlab.h"


#include "wrapper_i_ning.h"
#include "params_ning.h"
#include "params_file.h"


#include "mex.h"

#include "matlab_utils.h"


#include "histogram.h"
#include "svector.h"

#include "array.h"

#include "generic_wrapper_matlab.h"
#include "generic_parameters_matlab.h"

#include "cell_matlab.h"

#include "coarse_grain_array.h"
#include "counted_vector.h"

using namespace tracking;
using std::exception;

using utilities::array_to_mat;
using utilities::vector_to_mat;
using utilities::Svector;
using utilities::Histogram;
using utilities::Array;
using utilities::Cell_matlab;
using utilities::Generic_wrapper_base;
using utilities::Generic_parameters_matlab;
using utilities::Coarse_grain_array;
using utilities::Counted_vector;



extern void _main();
void mexFunction( int nlhs, mxArray *plhs[], 
		  int nrhs, const mxArray* prhs[] ){

  if(nlhs!=2|| nrhs!=7){
    cout<<"Error, wrong number of arguments"<<endl;
    return;
  }
  try{

    
    utilities::Tuple dims;	
    dims[0] = (int)mxGetScalar(prhs[1]);
    dims[1] = (int)mxGetScalar(prhs[2]);
    int frames = (int)mxGetScalar(prhs[3]);

    double max_r = mxGetScalar(prhs[4]);

    int min_track = (int) mxGetScalar(prhs[5]);
    int correct_drift = (int) mxGetScalar(prhs[6]);

    int msd_steps = 50;
    
    //nonsense to get the map set up
    map<wrapper::p_vals, int> contents;
    //   wrapper::p_vals tmp[] = {wrapper::d_index,wrapper::d_xpos,
    // 			   wrapper::d_ypos, wrapper::d_zpos, 
    // 			   wrapper::d_frame};
    //   int tmp2[] = {0, 1, 2 ,3,4};
    wrapper::p_vals tmp[] = {wrapper::d_xpos,
			     wrapper::d_ypos, 
			     wrapper::d_frame};
    int tmp2[] = {0, 1, 2 };
  

    vector<wrapper::p_vals > tmp3(tmp, tmp+3);
    vector<wrapper::p_vals>::iterator it1 = tmp3.begin();

    vector<int> tmp4(tmp2, tmp2+3);
    vector<int>::iterator it2 = tmp4.begin();

    map<wrapper::p_vals, int>::iterator it3 = contents.begin();

    for( ;it2<tmp4.end() && it1<tmp3.end() ; it1++, it2++, it3++)
      contents.insert(it3,pair<wrapper::p_vals, int>(*it1, *it2));
    //end nonsense
    //there has to be a better way to do this
    

    // set up the wrappers
    vector<Generic_wrapper_base *> wrapper_vec(1);
    
    //     Generic_parameters_matlab arr_parm(frames,2,plhs);
    //     wrapper_vec[0] = arr_parm.make_wrapper();
    //     Generic_parameters_matlab arr_parm2(1,msd_steps,plhs+1);
    //     for(int j = 0;j<6; ++j)
    //     {
    //       arr_parm2.change_mxArray(plhs+j+1);
    //       wrapper_vec[1+j] = arr_parm2.make_wrapper();
    //     }
    //     //     Generic_parameters_matlab arr_parm3(2500,corr_steps,plhs+3);
    //     //     for(int j = 0;j<10; ++j)
    //     //     {
    //     //       arr_parm3.change_mxArray(plhs+j+7);
    //     //       wrapper_vec[7+j] = arr_parm3.make_wrapper();
    //     //     }
    //     //     Generic_parameters_matlab arr_parm4(502,2,plhs+17);
    //     //     wrapper_vec[17] = arr_parm4.make_wrapper();


  


    params_matlab p_in = params_matlab(prhs,contents);
    contents.insert(pair<wrapper::p_vals, int>(wrapper::d_trackid,3));
    
    params_matlab p_out = params_matlab(plhs,contents,mxGetM(*prhs),
 					contents.size());
     
    //    params_file p_out = params_file(mxGetM(*prhs),"new_output.txt",contents);


    // params_ning p_in = params_ning(3,100*1000,contents);

    //  contents.insert(pair<wrapper::p_vals, int>(wrapper::d_unqid,4));
    //  contents[wrapper::d_index] = 5;
    //  contents.insert(pair<wrapper::p_vals, int>(wrapper::d_trackid,3));
    //  params_matlab p_out = params_matlab(plhs,contents,mxGetM(*prhs),contents.size());
    //  params_file p_out = params_file(5,contents);
    //  master_box b = master_box(&p,&p,6);

    master_box_t<particle_track>bt(&p_in,&p_out);
    track_shelf tracks;
    
    cout<<"total number of particles is: "<<bt.size()<<endl;;
  
    hash_case s(bt,dims,(int) max_r+1,frames);
    cout<<"case built"<<endl;
    s.link(max_r,tracks);
    //     int yar = tracks.get_track_count();
    
    cout<<"linked: "<<endl;
    
    
    if(correct_drift == 1)
    {
      s.compute_mean_disp();
      cout<<"computed mean frame displacement"<<endl;
    }
    
    //     Array mean_frame_disp(frames);
    //     s.get_mean_disp(mean_frame_disp);
    //     mean_frame_disp.set_array(wrapper_vec[0]);
  
    // Compute msd


    // output tracks
    Cell_matlab test_cell2(tracks.get_track_count(),plhs+1);
    tracks.set_corrected_disp_to_cell(test_cell2);
    cout<<"c tracks"<<tracks.get_track_count()<<endl;


    Histogram hist1(frames,0,frames);
    tracks.track_length_histogram(hist1);
    
    Generic_parameters_matlab arr_parm4(frames + 2,2,plhs);
    wrapper_vec.at(0) = arr_parm4.make_wrapper();
    
    hist1.output_to_wrapper(wrapper_vec.at(0));
	

     
    Counted_vector md(msd_steps);
    Counted_vector msd(msd_steps);
    Counted_vector msd_sq(msd_steps);
    tracks.msd_corrected(md,msd,msd_sq);
    md.output_to_wrapper(wrapper_vec[1],wrapper_vec[2]);
    msd.output_to_wrapper(wrapper_vec[3],wrapper_vec[4]);
    msd_sq.output_to_wrapper(wrapper_vec[5],wrapper_vec[6]);

    
    cout<<"c msd: "<<endl;
    


    

    //     cout<<"trimmed"<<endl;

    
    


    for(int j = 0;j<wrapper_vec.size(); ++j)
    {
      delete wrapper_vec.at(j);
      wrapper_vec.at(j) = NULL;
    }

    
    

  }
  catch(const char * err){
    cout<<err<<endl;
  } 
  catch(exception & e)
  {
    cout<<e.what()<<endl;
  }
  catch(...){
    cout<<"uncaught error"<<endl;
  }



  return;
}
