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
//work.
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "boost/date_time/posix_time/posix_time.hpp"


#include "iden.h"
#include "wrapper_i_plu.h"

#include "image1.h"
#include "data_proc1.h"

#include "ipp.h"


#include "tuple.h"

#include "image_base.h"

#include "mm_md_parser.h"
#include "md_store.h"
using std::string;


using std::cout;
using std::cerr;
using std::endl;


using tracking::hash_case;

using iden::Iden;
using iden::Image2D;

using iden::Wrapper_i_plu;
using utilities::Tuple;


using std::runtime_error;
using std::logic_error;

using utilities::Md_store;
using iden::Mm_md_parser;

using namespace boost::posix_time;
using namespace boost::gregorian;

Wrapper_i_plu * Iden::fill_wrapper(unsigned int frames,unsigned int start)
{
  
  if(!img_src_)
    throw runtime_error("Iden: did not provide a image source");
  if(!parser_)
    throw runtime_error("Iden: did not provide a meta data parser");
  
  // get parameters out of the md_store object
  float hwhm,thresh,top_cut,a,b;
  int feature_rad,dilation_rad, mask_rad;

  try
  {
    params_.get_value("threshold",thresh);
    params_.get_value("p_rad",feature_rad);
    params_.get_value("hwhm",hwhm);
    params_.get_value("d_rad",dilation_rad);
    params_.get_value("mask_rad",mask_rad);
    params_.get_value("top_cut",top_cut);
  }
  catch(logic_error & e)
  {
    std::cerr<<"Iden::error parsing the parameters"<<std::endl;
    std::cerr<<e.what()<<std::endl;
    throw;
    
  }
  
  if(params_.contains_key("a"))
    params_.get_value("a",a);
  else
    a = -1;
  if(params_.contains_key("b"))
    params_.get_value("b",b);
  else
    b = .2;



  Wrapper_i_plu * wrapper = NULL;
  unsigned int img_frames = img_src_->get_frame_count();

  if(start > img_frames)
    throw runtime_error("Iden: start is larger than the number of frames in image");
  
  if(start < 0)
    throw runtime_error("Iden: start is negetive, wtf");
  

  if(frames == 0)
  {
    frames = img_frames-start;
  }
  else if(start + frames > img_frames)
  {
    cout<<"start "<<start<<endl;
    cout<<"frames "<<frames<<endl;
    cout<<"img_frames"<<img_frames<<endl;
    
    throw runtime_error("Iden: asking for more frames than the stack has");
  }
  
  img_src_->select_plane(start);
  

  // image dimensions
  Tuple<unsigned int,2> dims = img_src_->get_plane_dims();
  unsigned int cols = dims[0];
  unsigned int rows = dims[1];
  
  
  
  // create a wrapper
  wrapper = new Wrapper_i_plu(frames,dims);
  wrapper->set_Md_store_size(frames);  

  // set up parser for meta-morph tiffs
  
  ptime prev_time,cur_time;
  string time_str;
  
  
  // loop over frames
  for(unsigned int j = start;j<(frames+start);++j)
  {
    

    // load frame
    img_src_->select_plane(j);

    Image2D image_in(rows,cols);
    image_in.set_data(*img_src_);
    


    // trim off the top .1% of the pixels to deal with outliers
    image_in.trim_max(top_cut);
    
    
    // extract meta data
    // this does not need to be cleaned up, that will be handled by the wrapper
    
    utilities::Md_store * md_store = img_src_->get_plane_md(*parser_);
    if(md_store->contains_key("acquisition-time-local"))
    {
      
      int dtime;
    
    
	
      // deal with time
      if(j == start)
      {
	prev_time = time_from_string(md_store->get_value("acquisition-time-local",time_str));
	dtime = 0;
      }
	
      else
      {
	cur_time  = time_from_string(md_store->get_value("acquisition-time-local",time_str));
	dtime = (cur_time-prev_time).total_milliseconds();
	prev_time = cur_time;
      }
      md_store->add_element("dtime",dtime);
    }
      
    
    

    
    // object for holding band passed image
    Image2D image_bpass(rows,cols);
    // object for holding the thresholded band passed image
    Image2D image_bpass_thresh(rows,cols);
    // object that holds the local max
    Image2D image_localmax(rows,cols);


    RecenterImage(image_in);

    IppStatus status;
    
    status = BandPass_2D(image_in,
			 image_bpass,
                         feature_rad,
                         hwhm);
    


    
    status = FindLocalMax_2D(image_bpass,
			     image_bpass_thresh,
			     image_localmax,
			     thresh,
                             dilation_rad);
    
    
    
    
    RecenterImage(image_bpass_thresh);
    // image_in.display_image();
    //     image_bpass.display_image();
    //     image_bpass_thresh.display_image();
    //    image_localmax.display_image();
    // get out massive nx9 array
    int counter;
    Ipp32f (*particledata)[9] =
      ParticleStatistics(image_localmax,
			 image_bpass_thresh, 
			 mask_rad,
                         feature_rad,
			 counter);
        
    // put it in a wrapper

    //     cout<<"---------------"<<endl;
    //     cout<<counter<<endl;
    //     cout<<"---------------"<<endl;

    // set data in wrapper
    wrapper->add_frame_data(particledata,j-start,counter);
    // set metadata in wrapper
    // the wrapper takes responsibility for the object
    wrapper->set_Md_store(j-start,md_store);
    //    cout<<j<<endl;
    // clean up wrapper
    
    
  }
  
  
  return wrapper;
  

}

void Iden::set_params(const Md_store & param_in)
{
  params_ = Md_store(param_in);
}
void Iden::set_image_src(Image_base * img_src)
{
  img_src_ = img_src;
}

void Iden::set_md_parser(MD_parser * parser)
{
  parser_ = parser;
}

Wrapper_i_plu * Iden::fill_wrapper_avg(unsigned int frames,unsigned int start)
{
  
  if(!img_src_)
    throw runtime_error("Iden: did not provide a image source");
  if(!parser_)
    throw runtime_error("Iden: did not provide a meta data parser");

  Wrapper_i_plu * wrapper = NULL;
  unsigned int img_frames = img_src_->get_frame_count();

  float hwhm,thresh,top_cut,a,b;
  int feature_rad,dilation_rad, mask_rad;
  unsigned int avg_count;

  try
  {
    params_.get_value("threshold",thresh);
    params_.get_value("p_rad",feature_rad);
    params_.get_value("hwhm",hwhm);
    params_.get_value("d_rad",dilation_rad);
    params_.get_value("mask_rad",mask_rad);
    params_.get_value("top_cut",top_cut);
    params_.get_value("avg_count",avg_count);
  }
  catch(logic_error & e)
  {
    cerr<<"error parsing the parameters"<<endl;
    cerr<<e.what()<<endl;
    throw;
    
  }
  

  if(params_.contains_key("a"))
    params_.get_value("a",a);
  else
    a = -1;
  if(params_.contains_key("b"))
    params_.get_value("b",b);
  else
    b = .2;





  if(start > img_frames)
    throw runtime_error("Iden: start is larger than the number of frames in image");

  if(frames == 0)
  {
    frames = img_frames-start;
  }
  else if(start + frames > img_frames)
  {
    cout<<"start "<<start<<endl;
    cout<<"frames "<<frames<<endl;
    cout<<"img_frames"<<img_frames<<endl;
    
    
    throw runtime_error("Iden: asking for more frames than the stack has");
  }

  
  if(avg_count > frames)
    throw runtime_error("Iden: avg_count is greater than the number for frames");
  

 


  unsigned int wrapper_frames = frames/avg_count;
  
  
  img_src_->select_plane(start);
  

  
  Tuple<unsigned int,2> dims = img_src_->get_plane_dims();
  unsigned int cols = dims[0];
  unsigned int rows = dims[1];
  
  
  // create the wrapper
  wrapper = new Wrapper_i_plu(wrapper_frames,dims);
  wrapper->set_Md_store_size(wrapper_frames);

  
  float scx=0,scy=0;
  string exp_unit,cal_units;
  bool cal_state = false;
  ptime prev_time,cur_time;
  
  // loop over frames
  for(unsigned int j = 0;j<wrapper_frames;++j)
  {
    float exposure_sum = 0,x = 0,y = 0,z=0,tmp = 0;
    
    
    
    string time_str;
    unsigned int dtime=0;
    
    

    Image2D image_in(rows,cols);
    for(unsigned int k = 0; k<avg_count;++k)
    {
      // load frame
      img_src_->select_plane(j*avg_count + k +start);
            
      if(k == 0)
	image_in.set_data(*img_src_);
      else
	image_in.add_data(*img_src_);
      
      
      
      // extract meta data from tiff
      Md_store * md_store = img_src_->get_plane_md(*parser_);
      
      

        
      // deal with time
      if(k ==0)
	cur_time  = time_from_string(md_store->get_value("acquisition-time-local",time_str));
    
      exposure_sum += md_store->get_value("Exposure",tmp);
      if(j == 0)
      {
	md_store->get_value("spatial-calibration-x",scx);
	md_store->get_value("spatial-calibration-y",scy);
	md_store->get_value("Exposure units",exp_unit);
	md_store->get_value("spatial-calibration-state",cal_state);
	md_store->get_value("spatial-calibration-units",cal_units);
	prev_time = cur_time;
	
      }
      
      
     
  
      x += md_store->get_value("stage-position-x",tmp);
      y += md_store->get_value("stage-position-y",tmp);      
      z += md_store->get_value("z-position",tmp);  
      // clean up meta data from tiff
      delete md_store;
      

    }
    dtime = (cur_time-prev_time).total_milliseconds();

    prev_time = cur_time;
    
    
    // average 
    x /=avg_count;
    y /=avg_count;
    z /=avg_count;

    Md_store *avg_md_store = new Md_store();
    avg_md_store->add_element("Exposure",exposure_sum);
    avg_md_store->add_element("Exposure units","string",exp_unit.c_str());
    avg_md_store->add_element("stage-position-x",x);
    avg_md_store->add_element("stage-position-y",y);      
    avg_md_store->add_element("z-position",z);  
    avg_md_store->add_element("spatial-calibration-state",cal_state);
    avg_md_store->add_element("spatial-calibration-units","string",cal_units.c_str());
    avg_md_store->add_element("spatial-calibration-x",scx);
    avg_md_store->add_element("spatial-calibration-y",scy);
    avg_md_store->add_element("pixel-size-x",cols);
    avg_md_store->add_element("pixel-size-y",rows);
    avg_md_store->add_element("dtime",dtime);
    avg_md_store->add_element("acquisition-time-local","string",time_str.c_str());
    
    
    
    // object for holding band passed image
    Image2D image_bpass(rows,cols);
    // object for holding the thresholded band passed image
    Image2D image_bpass_thresh(rows,cols);
    // object that holds the local max
    Image2D image_localmax(rows,cols);

    
    // trim off the top fraction of pixels
    image_in.trim_max(top_cut);

    RecenterImage(image_in);

    IppStatus status;
    
    status = BandPass_2D(image_in,
			 image_bpass,
                         feature_rad,
                         hwhm);
    


    
    status = FindLocalMax_2D(image_bpass,
			     image_bpass_thresh,
			     image_localmax,
			     thresh,
                             dilation_rad);
    
    

    
    RecenterImage(image_bpass_thresh);
    // image_in.display_image();
    //     image_bpass.display_image();
    //     image_bpass_thresh.display_image();
    //    image_localmax.display_image();
    // get out massive nx9 array
    int counter;
    Ipp32f (*particledata)[9] =
      ParticleStatistics(image_localmax,
			 image_bpass_thresh, 
			 mask_rad,
                         feature_rad,
			 counter);
    
    // put it in a wrapper

    //     cout<<"---------------"<<endl;
    //     cout<<counter<<endl;
    //     cout<<"---------------"<<endl;
    
    wrapper->add_frame_data(particledata,j-start,counter);
    wrapper->set_Md_store(j-start,avg_md_store);
    

    
    
    //    cout<<j<<endl;
  }
  
  return wrapper;
  

}

void Iden::process_frame(const Image2D & img_in,
                         unsigned int frame_number,
                         const Md_store * md_store_in,
                         Wrapper_i_plu & wrapper_out) const
{
  
  Md_store *  md_store_local = new Md_store();
  if (! md_store_in)
    md_store_local->add_elements(md_store_in);
  
  float hwhm,thresh,top_cut,a,b;
  int feature_rad,dilation_rad, mask_rad;
  unsigned int avg_count;
  
  // the parameters should be verified when, they are set, but the try
  // can't hurt.
  try
  {
    params_.get_value("threshold",thresh);
    params_.get_value("p_rad",feature_rad);
    params_.get_value("hwhm",hwhm);
    params_.get_value("d_rad",dilation_rad);
    params_.get_value("mask_rad",mask_rad);
    params_.get_value("top_cut",top_cut);
  }
  catch(logic_error & e)
  {
    std::cerr<<"Iden::error parsing the parameters"<<std::endl;
    std::cerr<<e.what()<<std::endl;
    throw;
    
  }

  unsigned int height = img_in.get_height();
  unsigned int width = img_in.get_width();



  // object for holding band passed image
  Image2D image_bpass(height,width);
  // object for holding the thresholded band passed image
  Image2D image_bpass_thresh(height,width);
  // object that holds the local max
  Image2D image_localmax(height,width);




  // local mutable copy of image
  Image2D image_in_local(img_in);    
  RecenterImage(image_in_local);


  IppStatus status;
    
  status = BandPass_2D(image_in_local,
                       image_bpass,
                       feature_rad,
                       hwhm);
        
  status = FindLocalMax_2D(image_bpass,
                           image_bpass_thresh,
                           image_localmax,
                           thresh,
                           dilation_rad);
    
    
    
    
  RecenterImage(image_bpass_thresh);
  
  // get out massive nx9 array
  int counter;
  Ipp32f (*particledata)[9] =
    ParticleStatistics(image_localmax,
                       image_bpass_thresh, 
                       mask_rad,
                       feature_rad,
                       counter);
        
  
  // set data in wrapper
  wrapper_out.add_frame_data(particledata,frame_number,counter);
  // set metadata in wrapper

  // the wrapper takes responsibility for the object
  wrapper_out.set_Md_store(frame_number,md_store_local);

  // clean up wrapper
}
