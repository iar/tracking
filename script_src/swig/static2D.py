#Copyright 2012 Thomas A Caswell
#tcaswell@uchicago.edu
#http://jfi.uchicago.edu/~tcaswell
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 3 of the License, or (at
#your option) any later version.
#
#This program is distributed in the hope that it will be useful, but
#WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, see <http://www.gnu.org/licenses>.
import cstatic2 as cs
import numpy as np

def compute_gofr(x,y,max_range,nbins):
    '''
    edges,val = compute_gofr(x,y,max_range,bins)
    
    Computes g(r) for a single plane of 2D data.  Casts data to
    float32
     x - particle x position 
     y - particle y position
     max_range - maximum r to compute to
     bins - number of bins

     edges - left edge of bins
     val - normalized g(r)
       
    '''

    # reset data to start at (0,0)
    x = x-np.min(x)
    y = y-np.min(y)
    # get dimensions
    Dx = float(np.ceil(np.max(x) ))
    Dy = float(np.ceil(np.max(y) ))

    # set up dimension object
    dim = cs.Tuplef(float(Dx),float(Dy))


    dset = cs.Dset()
    dset.insert(cs.D_XPOS)
    dset.insert(cs.D_YPOS)

    # cast down to 32bit float, it won't do this automatically as it isn't safe
    x = x.astype('float32')
    y = y.astype('float32')

    # set up wrapper
    w = cs.Wrapper_i_generic()
    w.setup(dset,1,dim)
    w.open_frame(0,int(len(x)),0)
    w.set_data_type(cs.D_XPOS)
    w.add_float_data(x)
    w.set_data_type(cs.D_YPOS)
    w.add_float_data(y)
    w.close_frame()
    w.finalize_wrapper()

    # set up filter
    filt = cs.Filter_trivial()

    # set up hash case
    hc = cs.hash_case()
    hc.init(w,filt,max_range)

    # set up g(r) object 
    cgofr = cs.Corr_gofr(nbins,max_range)

    # do computation
    hc.compute_corr(cgofr)

    # get bin values out 
    v = cs.FloatVector()
    cgofr.normalize(v)
    # get edges out
    e = cs.FloatVector()
    cgofr.get_bin_edges(e)
        
    return np.array(e),np.array(v)

def md_store_to_dict(md_obj):
    out = dict()
    count = md_obj.size()
    for j in range(count):
        key = md_obj.get_key(j)
        dtype = md_obj.get_type(j)
        if dtype == cs.V_INT:
            out[key] = md_obj.get_value_i(key)
        elif dtype == cs.V_UINT:
            out[key] = md_obj.get_value_ui(key)
        elif dtype == cs.V_FLOAT:
            out[key] = md_obj.get_value_f(key)
    

    return out


def md_store_to_dict(md_obj):
    out = dict()
    count = md_obj.size()
    for j in range(count):
        key = md_obj.get_key(j)
        dtype = md_obj.get_type(j)
        if dtype == cs.V_INT:
            out[key] = md_obj.get_value_i(key)
        elif dtype == cs.V_UINT:
            out[key] = md_obj.get_value_ui(key)
        elif dtype == cs.V_FLOAT:
            out[key] = md_obj.get_value_f(key)
        elif dtype == cs.V_BOOL:
            out[key] = md_obj.get_value_b(key)
        elif dtype == cs.V_STRING:
            out[key] = md_obj.get_value_s(key)
        else:
            print dtype

    return out
