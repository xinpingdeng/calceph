/*-----------------------------------------------------------------*/
/*! 
  \file openfiles.h 
  \brief Open files for the tests.

  \author  M. Gastineau 
           Astronomie et Systemes Dynamiques, IMCCE, CNRS, Observatoire de Paris. 

   Copyright, 2015, 2016, 2017, CNRS
   email of the author : Mickael.Gastineau@obspm.fr

  History:                                                                
*/
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/* License  of this file :
 This file is "triple-licensed", you have to choose one  of the three licenses 
 below to apply on this file.
 
    CeCILL-C
    	The CeCILL-C license is close to the GNU LGPL.
    	( http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html )
 
 or CeCILL v2.1
      The CeCILL license is compatible with the GNU GPL.
      ( http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.html )
 

This library is governed by the CeCILL-C, CeCILL-B or the CeCILL license under 
French law and abiding by the rules of distribution of free software.  
You can  use, modify and/ or redistribute the software under the terms 
of the CeCILL-C,CeCILL-B or CeCILL license as circulated by CEA, CNRS and INRIA  
at the following URL "http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-C,CeCILL-B or CeCILL license and that you accept its terms.
*/
/*-----------------------------------------------------------------*/

/*disable warning for intel compilers : warning #161: unrecognized #pragma  */
#if __INTEL_COMPILER
#pragma warning(disable:161)
#endif

/*enable unused pragma for : clang, intel   */
#if defined(__clang__) || defined(__INTEL_COMPILER)
#ifndef HAVE_PRAGMA_UNUSED
#define HAVE_PRAGMA_UNUSED 1
#endif
#else
/*disable unknown pragma for pragma unused if not available  */
#ifndef HAVE_PRAGMA_UNUSED
#define HAVE_PRAGMA_UNUSED 0
#endif
#endif

/* set the unused attribute for the parameters */
#ifdef HAVE_VAR_ATTRIBUTE_UNUSED
#  define PARAMETER_UNUSED(x)  x __attribute__((__unused__))
#else
#  define PARAMETER_UNUSED(x)  x
#endif

/*! open the data files using fopen */
FILE* tests_fopen(const char *filename, const char* mode);

/*! open the tests files using calceph_open */
t_calcephbin* tests_calceph_open(const char *filename);

/*! open the tests files using calceph_open_array */
t_calcephbin* tests_calceph_open_array(int n, const char  * const filename[/*n*/]);

/*! open the tests files using calceph_sopen */
int tests_calceph_sopen(const char *filename);