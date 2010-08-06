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

#ifndef ENUM_UTILS
#define ENUM_UTILS

#include<string>

namespace utilities
{

/**
   Enumeration for wrapper types
 */
typedef enum W_TYPE{                                                                
  W_MATLAB = 0,                                                           
  W_TEXT,                                                                 
  W_NING,                                                                 
  W_HDF,                                                                  
} W_TYPE;                                                                        

/**
   Enumeration for data types (where type is the physical meaning of
   the data).  If you add to this you MUST update the count below this,
   add the D_TYPE to the V_TYPE conversion function, and D2Str function.
*/
typedef enum D_TYPE {
  D_INDEX=0, 			// index of particle in case,
				// (frame,indx) is unique and should
				// be persistent in wrappers (wrapper)

  D_XPOS, 			// x-position (plu)
  D_YPOS, 			// y-position (plu)
  D_FRAME, 			// frame (wrapper)

  D_I, 				// integrated intensity (plu)
  D_R2, 			// radius of gyration (plu)
  D_E,				// eccentricity (plu)
  
  D_NEXT_INDX,			// index of the next particle in the
				// track (track)
  D_PREV_INDX, 			// index of the previous particle in
				// the track (track)
  
  D_DX, 			// x-shift (plu)
  D_DY, 			// y-shift (plu)
  D_ORGX, 			// original x-position (computed)
  D_ORGY, 			// original y-position (computed)
  
  D_ZPOS,			// z-position (link or meta-data)

  D_TRACKID,			// track id (track)
  D_S_ORDER_PARAMETER,		// scalar order parameter (computed)
  D_MULT,			// multiplicity (plu)

  D_N_SIZE, 			// neighborhood size (computed)

  
} D_TYPE;


/**
   Number of enumerated data types.  This needs to be right for the
   flat wrappers to work properly.  If this is wrong, there will be
   buffer overflow which will screw you over.
 */
const int D_TYPE_COUNT = 18;

/**
   Enumeration for the type of value the data is (ie float, int,complex)
 */
typedef enum V_TYPE
  {
    V_ERROR = -1,		// error type
    V_INT = 0,			// integer
    V_FLOAT,			// float
    V_COMPLEX,			// complex
    V_STRING,			// string
    V_BOOL,			// bool
    V_TIME,			// date/time
    V_GUID,			// guid from MM meta-data
    
  }V_TYPE;
/**
   Helper function to map between D_TYPE and V_TYPE
 */
V_TYPE v_type(D_TYPE in);


/**
   Helper functions to convert D_TYPEs in to short name strings
 */
std::string DT2str_s(D_TYPE in);
/**
   Helper functions to convert short name strings in to D_TYPEs 
 */
D_TYPE str2DT_s(std::string in);
/**
   Helper function to format dset names with computation numbers attached
 */
std::string format_dset_name(D_TYPE type,int comp_num);
/**
   Helper function to format computation numbers on to strings.
 */
std::string format_name(const std::string & name ,int comp_num);


/**
   Helper function to make v_types to strings
 */
std::string VT2str_s(V_TYPE);

/**
   Helper function to make string in to V_TYPE
 */
V_TYPE str2VT_s(const std::string& in);
/**
   Helper function to make string in to V_TYPE
 */
V_TYPE str2VT_s(const char  * const in);

}


#endif
