//Peter J. Lu
//Copyright 2008 Peter J. Lu. 
//http://www.peterlu.org
//plu@fas.harvard.edu
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
//combining it with FreeImage (or a modified version of that library),
//containing parts covered by the terms of freeimage-license, the
//licensors of this Program grant you additional permission to convey
//the resulting work.  Corresponding Source for a non-source form of
//such a combination shall include the source code for the parts of
//FreeImage used as well as that of the covered work.
// 
//If you modify this Program, or any covered work, by linking or
//combining it with IPP (or a modified version of that library),
//containing parts covered by the terms of End User License Agreement
//for the Intel(R) Software Development Products, the licensors of
//this Program grant you additional permission to convey the resulting
//work.

//copied form https://plutarc.svn.sourceforge.net/svnroot/plutarc/trunk/matlab_wrapper rev9
// Modified by Thomas A Caswell tcaswell@uchicago.edu 09/2009-
#include "ipp.h"
#if MATLAB_MEX_FILE
#include "mex.h"
#endif 


#include <fstream>
#include <iostream>
#include <math.h>
// tac 2009-09-15
// 
#include "image1.h"
// tac 2010-07-20
// added exception library
#include <stdexcept>
// tac 2011-03-02
// Adding tuple header
#include "tuple.h"

// tac 2009-09-15
// 
using iden::Image2D;
using std::cout;
using std::endl;

// tac 2010-07-20
// added using statements
using std::runtime_error;
// tac 2011-03-02
// added using statement
using std::logic_error;

// tac 2011-03-02
// added to update import methods

using iden::Image_base;
using iden::PIX_TYPE;
using utilities::Tuple;

// tac 2009-09-21
// added displaying functionality
//#include "gnuplot_i.hpp" //Gnuplot class handles POSIX-Pipe-communikation with Gnuplot


/* modified 6/08
 * Beating this code in to a form that can be compiled in to mex files
 * tcaswell
 */

Image2D::Image2D(const int image_length, const int image_width):
  imagedata_(NULL), stepsize_(0), width_(0), height_(0)
{
  numberofpixels_ = image_width * image_length;
  height_ = image_length;
  width_ = image_width;
  imagedata_ = ippsMalloc_32f(numberofpixels_);
  stepsize_ = 4 * image_width;
  ROIfull_.width = image_width;
  ROIfull_.height = image_length;
  // tac 2009-03-02 
  // initialize the arrays to 0 to prevent junk in
  // them from around the edges from screwing with the
  // recentering processes
  // for(int j = 0;j<image_width*image_length; j++)
  //     *(imagedata + j) = 0;
  // tac 2009-09-17
  // changed to use ipp methods
  
  ippiSet_32f_C1R(0,imagedata_,stepsize_,ROIfull_);
  
  
}

// tac 2012-11-07
// added copy constructor
Image2D::Image2D(const Image2D & other):
  imagedata_(NULL), 
  stepsize_(other.stepsize_), 
  width_(other.width_), 
  height_(other.height_),
  numberofpixels_(other.numberofpixels_),
  ROIfull_(other.ROIfull_)
{
  imagedata_ = ippsMalloc_32f(numberofpixels_);
  ippiCopy_32f_C1R(other.imagedata_,
                   other.stepsize_,
                   imagedata_,
                   stepsize_,
                   ROIfull_);
  
}



Image2D::~Image2D()
{
  ippsFree(imagedata_);
  imagedata_ = NULL;
}

int Image2D::getx(const int index1D)
{
  return index1D % width_;
}

int Image2D::gety(const int index1D)
{
  return (int) floor((float) index1D / width_);
}

int Image2D::get1Dindex(const int x, const int y)
{
  return y * width_ + x;
}
// tac 2011-03-02
// 
void Image2D::set_data(const Image_base & image)
{
  proc_data(image,false);
}
// tac 2011-03-02
// 
void Image2D::add_data(const Image_base & image)
{
  proc_data(image,true);
}
// tac 2011-03-02
// 
void Image2D::proc_data(const Image_base & image,bool add)
{
  
  // image dimensions
  Tuple<unsigned int,2> dims = image.get_plane_dims();
  unsigned int cols = dims[0];
  unsigned int rows = dims[1];

  
  if(!(cols==width_ && rows == height_))
  {
    std::cerr<<"height: "<<height_<<'\t'<<"rows: "<<rows<<std::endl;
    std::cerr<<"width: "<<width_<<'\t'<<"cols: "<<cols<<std::endl;
    throw runtime_error("Image2D: data is wrong size");
  }
  

  



  // figure out the data type
  PIX_TYPE img_type = image.get_pixel_type();
  // figure out step for input
  WORD in_step = image.get_scan_step();
  // figure out step for 
  int data_step = cols*sizeof(Ipp32f);

  // data pointer

  const void * data_ptr = image.get_plane_pixels();
  switch(img_type)
  {
  case U16:	// array of unsigned short	: unsigned 16-bit
    // trust the id, assume in really 16u, go on
    if(!add)
      ippiConvert_16u32f_C1R((const uint16_t *) data_ptr, in_step,imagedata_,data_step,ROIfull_);
    else
      ippiAdd_16u32f_C1IR((const uint16_t *) data_ptr,in_step,imagedata_,data_step,ROIfull_);
    break;
 case U8:	// standard image			: 1-, 4-, 8-, 16-, 24-, 32-bit  
    if(!add)
      ippiConvert_8u32f_C1R((const uint8_t *) data_ptr, in_step,imagedata_,data_step,ROIfull_);
    else
      ippiAdd_8u32f_C1IR((const uint8_t *) data_ptr,in_step,imagedata_,data_step,ROIfull_);
    break;
  case S16:	// array of short			: signed 16-bit
    // trust the id, assume in really 16s, go on
    if(!add)
      ippiConvert_16s32f_C1R((const int16_t *)data_ptr, in_step,imagedata_,data_step,ROIfull_);
    else
      throw logic_error("Due to a gap in IPP this adding is not supported in this mode");
    //   ippiAdd_16s32f_C1IR((const int16_t *) data_ptr,in_step,imagedata_,data_step,ROIfull_);
    break;
  case F32:	// array of float			: 32-bit IEEE floating point
    // trust the id, assume in really 32f, go on
    if(!add)
      ippiCopy_32f_C1R((const float *) data_ptr, in_step,imagedata_,data_step,ROIfull_);
     else
       ippiAdd_32f_C1IR((const float *) data_ptr,in_step,imagedata_,data_step,ROIfull_);
    break;
  case ERROR:
    throw logic_error("An object with the pixeltype error should have never gotten to this function");
    
    
  }
  


}



void Image2D::trim_max(float cut_percent)
{
  
  if(cut_percent ==0)
    return;
  
  if(cut_percent>1)
    throw runtime_error("image2D: nonsense percent");
  
  // find max and min
  Ipp32f max=0,min=0;
  ippiMinMax_32f_C1R(imagedata_,stepsize_,ROIfull_,&min,&max);
  
  
  // make histogram, set bin count by cut_percent, so that if flat
  // would cut top two boxes
  int n_bins = (int)ceil(2.0/cut_percent)+1;
  Ipp32f bin_step = (max-min)/((Ipp32f)n_bins);
  Ipp32f * p_levels = new Ipp32f [n_bins];
  Ipp32s * p_hist = new Ipp32s[n_bins -1];

  for(int j = 0;j<n_bins;++j)
    p_levels[j] = min + bin_step*j;
  

  ippiHistogramRange_32f_C1R(imagedata_,stepsize_,ROIfull_,p_hist,p_levels,n_bins);
  
  // sum from the top of the histogram until cut_percent of pixels are
  // included, use the top of the last bin as the cut off
  Ipp32s sum = 0;
  Ipp32s sum_max = (Ipp32s)(numberofpixels_*cut_percent);
  int bin_indx = n_bins-1;
  while(sum < sum_max)
    sum+=p_hist[--bin_indx];

  // move back one bin
  ++bin_indx;

  // cout<<"cutting top "<<bin_indx<<" bins"<<endl;
  
  // trim data in place, set all values above the cutoff to the cutoff
  ippiThreshold_GTVal_32f_C1IR(imagedata_,stepsize_,ROIfull_,
			       p_levels[bin_indx],p_levels[bin_indx]);
  


  // clean up memory
  delete [] p_levels;
  delete [] p_hist;
  
}

void Image2D::set_data(const float * IN_ARRAY2,int DIM1,int DIM2)
{
  // free the data we currently have
  ippsFree(imagedata_);
  imagedata_ = NULL;  

  height_ = DIM1;
  width_ = DIM2;
  numberofpixels_ = width_*height_;
    
  imagedata_ = ippsMalloc_32f(numberofpixels_);
  stepsize_ = 4 * width_;
  
  ROIfull_.width = width_;
  ROIfull_.height = height_;
  

  

  imagedata_ = ippsMalloc_32f(numberofpixels_);
  ippiCopy_32f_C1R(IN_ARRAY2,
                   stepsize_,
                   imagedata_,
                   stepsize_,
                   ROIfull_);
 
  
}


