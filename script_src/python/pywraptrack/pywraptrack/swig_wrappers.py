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

import numpy as np
import ciden2 as ci
import copy
import json


class IdenProcessBackend(object):
    '''A class to wrap the details of dealing with the swig wrapers '''

    type_map = {'x':ci.D_XPOS,
                'y':ci.D_YPOS,
                'I':ci.D_I,
                'e':ci.D_E,
                'r2':ci.D_R2,
                'dx':ci.D_DX,
                'dy':ci.D_DY}

    int_params = ["p_rad", 'd_rad', 'mask_rad']
    float_params = ['threshold', 'hwhm', 'top_cut']
    filter_params =  ['shift_cut', 'rg_cut', 'e_cut']

    def __init__(self, params):
        # add checking to make sure parameters are right
        self.params = params

    def update_param(self, name, x):
        '''Sets the parameters of to use to process frames '''
        # add type checking etc
        self.params[name] = x
        pass

    def process_single_frame(self, img, ret_types=('x', 'y')):
        '''processes a single frame'''
        for key in ret_types:
            if key not in self.type_map:
                raise Exception("invalid data type")
            
        img = np.asarray(img)
        # wrap the numpy data for the C++
        img2d = ci.Image2D(*img.shape)
        img2d.set_data(img)

        # create new md_store object
        iden_prams = ci.Md_store()
        for key in IdenProcessBackend.int_params:
            iden_prams.set_value_i(key, int(self.params[key]))

        for key in IdenProcessBackend.float_params:
            iden_prams.set_value_f(key,float(self.params[key]))

        md_parser = ci.Mm_md_parser()
        #        print 'all that noise set up'
        iden = ci.Iden(iden_prams)
        iden.set_md_parser(md_parser)
        #        print 'made iden object'
        dim = ci.Tuplef(*img.shape)
        #        print 'made tuple'
        wrap_plu = ci.Wrapper_i_plu(1,dim)
        #        print 'made wrapper'
        md = ci.Md_store()
        #        print 'objects all set up'

        iden.process_frame(img2d,0,md,wrap_plu)
        res = {key:
               np.array([wrap_plu.get_value_f(j, self.type_map[key],int(0)) for j in range(wrap_plu.get_num_entries(0))])
               for key in ret_types}


        return res
    def process_sequence(self, img_src):
        '''Process the entire sequence

        Returns a Wrapper_i_plu object.
        '''

        # create new md_store object
        iden_prams = ci.Md_store()
        for key in IdenProcessBackend.int_params:
            iden_prams.set_value_i(key, int(self.params[key]))

        for key in IdenProcessBackend.float_params:
            iden_prams.set_value_f(key,float(self.params[key]))

        md_parser = ci.Mm_md_parser()
        #        print 'all that noise set up'
        iden = ci.Iden(iden_prams)
        iden.set_md_parser(md_parser)
        #        print 'made iden object'
        dim = ci.Tuplef(*img.shape)
        #        print 'made tuple'
        wrap_plu = ci.Wrapper_i_plu(len(img_src),dim)
        #        print 'made wrapper'
        md = ci.Md_store()
        #        print 'objects all set up'

        for j,img in enumerate(img_src):
            img2d = ci.Image2D(*img.shape)
            img2d.set_data(img)
            iden.process_frame(img2d,j,md,wrap_plu)



        return wrap_plu

    def save_params(self, fname_out, filter_param=None):
        f = open(fname_out, 'w')
        dump_dict = {'params':self.params,
                     'filters':filter_param}
        f.write(json.dumps(dump_dict))
        f.close()

    def set_param_from_file(self,fname_in):
        with open(fname_in, 'r') as f:
            tmp_dict = json.loads(f.readline())
            
        self.params = tmp_dict['params']
        return tmp_dict['filters']
        
