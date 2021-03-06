#Copyright 2009 Thomas A Caswell
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


import xml.dom.minidom
import subprocess
import h5py
import datetime
import os.path
import re

# parses the attributes from the xml tree
def _parse_attr(h5obj,dom_obj):
    if dom_obj.getAttribute("id") =="Description":
        _parse_des(h5obj,dom_obj)
        _parse_region(h5obj)
    elif dom_obj.getAttribute("type") =="int":
        h5obj.attrs.create(dom_obj.getAttribute("id"), int(dom_obj.getAttribute("value")),None,'int32')
    elif dom_obj.getAttribute("type") =="float":
        h5obj.attrs.create(dom_obj.getAttribute("id"), float(dom_obj.getAttribute("value")),None,'float32')

    else: 
        h5obj.attrs[dom_obj.getAttribute("id")] =  dom_obj.getAttribute("value").encode('ascii')

# parses the long string that isn't really part of the xml structure,
# but has lots of interesting information in it
def _parse_des(h5obj,des_obj):
    des_string = des_obj.getAttribute("value")
    des_split = des_string.split("&#13;&#10;")

    for x in des_split:
        tmp_split = x.split(":")
        # /todo add parsing to proper data types
        if len(tmp_split) ==2:
            h5obj.attrs[tmp_split[0]] = tmp_split[1].encode('ascii')

def _parse_params_attr(h5file,i_str):
    ssplit = i_str.strip().split(':')

    if len(ssplit)==3:
        print ssplit
    elif ssplit[0].strip()=='p_rad' or ssplit[0].strip()=='d_rad'or ssplit[0].strip()=='mask_rad':
        h5file.attrs[ssplit[0].strip()] = int(float(ssplit[1].strip()))
        print ssplit
    elif ssplit[0].strip()=='threshold'or ssplit[0].strip()=='hwhm' or ssplit[0].strip()=='shift_cut' or ssplit[0].strip()=='rg_cut' or ssplit[0].strip()=='e_cut':
        h5file.attrs[ssplit[0].strip()] = float(ssplit[1].strip())
        print ssplit
    else:
        print ssplit[0].strip()


def _parse_params(h5file,fname):
    f = open(fname)
    for line in f:
        _parse_params_attr(h5file,line)
    f.close()
    n_fname  = fname[:(len(fname)-5)] + ".done"
    os.rename(fname,n_fname)

def _parse_temp(h5obj,fname):
    rexp = re.compile('\d\d-\d')
    ma = rexp.findall(fname)
    if(len(ma)>=1):
        h5obj.attrs['probe_temp'] = float('.'.join(ma[0].split('-')))
    if(len(ma)==2):
        h5obj.attrs['lens_temp'] = float('.'.join(ma[1].split('-')))

def _add_name(f,fname):

    f.attrs['image_name'] = fname

        
def _parse_region(h5obj):
    r_str =  h5obj.attrs['Region'].strip().split(' ')

    dim = [int(r_str[0].strip(',')) ,int(r_str[2].strip(',')) ]
    h5obj.attrs.create('dim_count', 2,None,'int32')
    h5obj.attrs.create('dim_unit','px')
    h5obj.attrs.create('dims', dim,None,'float32')
    


def make_h5(fin,fout):
    fname = fin

    out_fname = fout





    # make sure the files exist
    if not (os.path.exists(fname) ):
        print fname
        print "files don't exist"
        exit()


    f = h5py.File(out_fname,'w-')

    print fname
    
    _add_name(f,fname.encode('ascii'))
    _parse_temp(f,fname)



    #changed to deal with 2.5v2.6
    #g = f.create_group("frame{0:06d}".format(0))
    g = f.create_group("frame%(#)06d"%{"#":0})

    null_path = open('/dev/null','w')

    a = subprocess.Popen(["tiffinfo","-0",fname] ,stdout=subprocess.PIPE,stderr=null_path)
    tiff_string = (a.stdout).readlines();
    tiff_string = "".join(tiff_string)

    xml_str = tiff_string[(tiff_string.find("<MetaData>")):(10 + tiff_string.rfind("MetaData>"))]
    dom = xml.dom.minidom.parseString(xml_str)

    props = dom.getElementsByTagName("prop")

    for p in props:
        if p.parentNode.nodeName == "PlaneInfo":
            _parse_attr(g,p)
            if p.getAttribute("id") == "acquisition-time-local":
                tmp = p.getAttribute("value")
                initial_t  = datetime.datetime.strptime(tmp[:17],"%Y%m%d %H:%M:%S") 
                initial_t.replace(microsecond=int(tmp[18:])*1000)


        elif p.parentNode.nodeName == "MetaData":
            _parse_attr(f,p)
        elif p.parentNode.nodeName == "SetInfo":
            _parse_attr(f,p)
            if p.getAttribute("id") == "number-of-planes":
                frame_count = int(p.getAttribute("value"))
    g.attrs["dtime"] = 0.0


    for frame in range(1,frame_count):
        a = subprocess.Popen(["tiffinfo","-"+str(frame),fname] ,stdout=subprocess.PIPE,stderr=null_path)
        tiff_string = (a.stdout).readlines();
        tiff_string = "".join(tiff_string)
        xml_str = tiff_string[(tiff_string.find("<MetaData>")):(10 + tiff_string.rfind("MetaData>"))]
        dom = xml.dom.minidom.parseString(xml_str)
        props = dom.getElementsByTagName("prop")

        #g = f.create_group("frame{0:06d}".format(0))
        #g = f.create_group("frame{0:06d}".format(frame))
        g = f.create_group("frame%(#)06d"%{"#":frame})
        for p in props:
            if p.parentNode.nodeName == "PlaneInfo":
                _parse_attr(g,p)
                if p.getAttribute("id") == "acquisition-time-local":
                    tmp = p.getAttribute("value")
                    current_t = datetime.datetime.strptime(tmp[:17],"%Y%m%d %H:%M:%S") 
                    current_t = current_t.replace(microsecond=int(tmp[18:])*1000)
        dt = current_t - initial_t
        g.attrs["dtime"] = dt.seconds + dt.microseconds/(pow(10.,6))
        initial_t = current_t

    f.close()


