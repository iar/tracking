//Copyright 2009,2010 Thomas A Caswell
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
#ifndef ARRAY_T
#define ARRAY_T

#include <vector>
#include "part_def.h"

namespace utilities{
class Generic_wrapper_base;

/**
   Class to accumulate tuples into.  Each tuple added is a row.  The
   number of columns is set by the size of Tuplei and Tuplef.  This
   has some very bad naming conventions and is primarily used in the
   matlab related code.  This class should probably go away.
 */
class Array{
public:
  /**
     Outputs the whole array to the specified output wrapper.
   */
  void set_array(Generic_wrapper_base * out_wrapper) const;
  /**
     Adds a tuple to the end of the array.
   */
  void push(const Tuplef& tuple_in);
  /**
     Sensible printing
   */
  void print();
  /**
     Constructor
   */
  Array(int rows);
   /**
     Returns the number of rows
   */
  int get_row_num()const{
    return rows_;
  }
  /**
     Returns the number of columns
   */
  int get_col_num()const{
    return cols_;
  }
  /**
     Clears the array
   */
  void clear();

  /**
     Clears and resizes the array
   */
  void clear(int rows);

  /**
     resizes the array
   */
  void resize(int rows);
protected:
  /**
     Number of rows
   */
  int rows_;
  /**
     number of colmuns
   */
  int cols_;
  /**
     Vector to hold data
   */
  std::vector<double> data_;
  /**
     The current row, in [0,rows_-1]
   */
  int cur_index_;
};
}

#endif
