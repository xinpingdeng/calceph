#/*-----------------------------------------------------------------*/
#/*! 
#  \file calcephpy.pxd 
#  \brief public Python API for calceph library
#        access and interpolate INPOP and JPL Ephemeris data.
#
#  \author  M. Gastineau 
#           Astronomie et Systemes Dynamiques, IMCCE, CNRS, Observatoire de Paris. 
#
#   Copyright, 2016, 2017, CNRS
#   email of the author : Mickael.Gastineau@obspm.fr
#*/
#/*-----------------------------------------------------------------*/
# 
#/*-----------------------------------------------------------------*/
#/* License  of this file :
# This file is "triple-licensed", you have to choose one  of the three licenses 
# below to apply on this file.
# 
#    CeCILL-C
#    	The CeCILL-C license is close to the GNU LGPL.
#    	( http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html )
#   
# or CeCILL-B
#        The CeCILL-B license is close to the BSD.
#        (http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.txt)
#  
# or CeCILL v2.1
#      The CeCILL license is compatible with the GNU GPL.
#      ( http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.html )
# 
#
# This library is governed by the CeCILL-C, CeCILL-B or the CeCILL license under 
# French law and abiding by the rules of distribution of free software.  
# You can  use, modify and/ or redistribute the software under the terms 
# of the CeCILL-C,CeCILL-B or CeCILL license as circulated by CEA, CNRS and INRIA  
# at the following URL "http://www.cecill.info". 
# 
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 
# 
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 
# 
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL-C,CeCILL-B or CeCILL license and that you accept its terms.
# */
# /*-----------------------------------------------------------------*/


cdef extern from "calceph.h":
    
    #/*-----------------------------------------------------------------*/
    #/* error handler */
    #/*-----------------------------------------------------------------*/
    #/*! set the error handler */
    void calceph_seterrorhandler(int typehandler, void (*userfunc)(const char*));

    #/*-----------------------------------------------------------------*/
    #/* multiple access API per thread/process */
    #/*-----------------------------------------------------------------*/
    #/*! ephemeris descriptor */
    ctypedef struct t_calcephbin:
        pass
    
    #/*! open an ephemeris data file */
    t_calcephbin* calceph_open(const char *filename)
    
    #/*! open a list of ephemeris data file */
    t_calcephbin* calceph_open_array(int n, const char  ** filename)
    
    #/*! prefetch all data to memory */
    int calceph_prefetch(t_calcephbin* eph)
    
    #/*! compute the position <x,y,z> and velocity <xdot,ydot,zdot>
    #    for a given target and center at a single time. The output is in UA, UA/day, radians */
    int calceph_compute(t_calcephbin* eph, double JD0, double time, int target, 
                                           int center, double PV[6])
    
    #/*! compute the position <x,y,z> and velocity <xdot,ydot,zdot>
    #    for a given target and center at a single time. The output is expressed according to unit */
    int calceph_compute_unit(t_calcephbin* eph, double JD0, double time, int target, 
                                           int center, int unit, double PV[6])
    
    #/*! compute the orientation <euler angles> and their derivatives for a given target  at a single time. 
    #    The output is expressed according to unit */
    int calceph_orient_unit(t_calcephbin* eph, double JD0, double time, int target, 
                                               int unit, double PV[6])
                                               
    #/*! compute the rotational angular momentum G/(mR^2) and their derivatives for a given target  at a single time.
    #    The output is expressed according to unit */
    int calceph_rotangmom_unit(t_calcephbin* eph, double JD0, double time, int target,
                                              int unit, double PV[6])

    #/*! According to the value of order, compute the position <x,y,z> 
    #    and their first, second and third derivatives (velocity, acceleration, jerk)
    #    for a given target and center at a single time. The output is expressed according to unit */
    int calceph_compute_order(t_calcephbin* eph, double JD0, double time, int target,
                                                    int center, int unit, int order, double *PVAJ)
        
    #/*! According to the value of order,  compute the orientation <euler angles> 
    #    and their first, second and third derivatives for a given target  at a single time.
    #     The output is expressed according to unit */
    int calceph_orient_order(t_calcephbin* eph, double JD0, double time, int target,
                                                int unit, int order, double *PVAJ)
    
    #/*! According to the value of order,  compute the rotational angular momentum G/(mR^2) 
    #     and their first, second and third derivatives for a given target at a single time.
    # The output is expressed according to unit */
    int calceph_rotangmom_order(t_calcephbin* eph, double JD0, double time, int target,
                                              int unit, int order, double *PVAJ)


    #/*! get constant value from the specified name in the ephemeris file */
    int calceph_getconstant(t_calcephbin* eph, const char* name, double *value)
    
    #/*! return the number of constants available in the ephemeris file */
    int calceph_getconstantcount(t_calcephbin* eph)
    
    #/*! return the name and the associated value of the constant available at some index in the ephemeris file */
    int calceph_getconstantindex(t_calcephbin* eph, int index, char name[33], double *value)
    
    #/*! return the time scale in the ephemeris file */
    int calceph_gettimescale (t_calcephbin* eph)

    #/*! return the time span in the ephemeris file */
    int calceph_gettimespan (t_calcephbin* eph, double *firsttime, double *lasttime, int *continuous)

    #/*! return the number of position’s records available in the ephemeris file */
    int calceph_getpositionrecordcount(t_calcephbin* eph);

    #/*! return the target and origin bodies, the first and last time, and the reference frame available at the specified index for the position's records of the ephemeris file */
    int calceph_getpositionrecordindex(t_calcephbin* eph, int index, int* target, int* center, double* firsttime, double* lasttime, int* frame);

    #/*! return the number of orientation’s records available in the ephemeris file */
    int calceph_getorientrecordcount(t_calcephbin* eph);

    #/*! return the target body, the first and last time, and the reference frame available at the specified index for the orientation's records of the ephemeris file */
    int calceph_getorientrecordindex(t_calcephbin* eph, int index, int* target,double* firsttime, double* lasttime, int* frame);

    #/*! close an ephemeris data file and destroy the ephemeris descriptor */
    void calceph_close(t_calcephbin* eph)
    
    #/*! return the version of the library as a null-terminated string */
    void calceph_getversion_str(char szversion[33])



