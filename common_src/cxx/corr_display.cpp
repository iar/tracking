//Copyright 2009-2010 Thomas A Caswell
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

//If you modify this Program, or any covered work, by linking or
//combining it with IPP (or a modified version of that library),
//containing parts covered by the terms of End User License Agreement
//for the Intel(R) Software Development Products, the licensors of
//this Program grant you additional permission to convey the resulting
//work.

#include "corr_gofr.h"
#include "corr_goftheta.h"



#include "gnuplot_i.hpp"

using gnuplot::Gnuplot;
using gnuplot::GnuplotException;
using gnuplot::wait_for_key;


using std::vector;
using std::string;




using tracking::Corr_gofr;
using tracking::particle;


const float pi = acos(-1);

void Corr_gofr::display()const
{

  vector<float> tmp;
  normalize(tmp);
  
  Gnuplot g(bin_edges_,tmp,"g(r)","steps");
  g.set_grid();
  wait_for_key();
  


}


void Corr_goftheta::display()const
{

  vector<float> tmp;
  normalize(tmp);
  
  Gnuplot g(bin_edges_,tmp,"g(theta)","steps");
  g.set_grid().replot();
  wait_for_key();
  


}
