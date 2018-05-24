/*-----------------------------------------------------------------*/
/*! 
  \file cversion.c
  \brief Check the version opf the library

  \author  M. Gastineau 
           Astronomie et Systemes Dynamiques, IMCCE, CNRS, Observatoire de Paris. 

   Copyright, 2016, 2017, CNRS
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
 
 or CeCILL-B
        The CeCILL-B license is close to the BSD.
        (http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.txt)
  
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

#include "calcephconfig.h"
#include <stdio.h>
#if HAVE_STRING_H
#include <string.h>
#endif
#include "calceph.h"
#include "openfiles.h"

/*-----------------------------------------------------------------*/
/* main program */
/*-----------------------------------------------------------------*/
int main(void)
{
    char cversion[CALCEPH_MAX_CONSTANTNAME];
    calceph_getversion_str(cversion);
/*    fprintf (stdout, "library version is '%s'\n", cversion);
    fprintf (stdout, "string library version is '%s'\n", CALCEPH_VERSION_STRING);*/
    if (strcmp (cversion, CALCEPH_VERSION_STRING)!=0)
    {
        fprintf (stderr, "Error: header and library do not match\n");
        fprintf (stderr, "Error: header version is '%s' (%d.%d.%d)\n",
                 CALCEPH_VERSION_STRING, CALCEPH_VERSION_MAJOR, CALCEPH_VERSION_MINOR, CALCEPH_VERSION_PATCH);
        fprintf (stderr, "Error: library version is '%s'\n", cversion);
        return 1;
    }
    return 0;
}
