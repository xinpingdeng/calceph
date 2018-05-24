/*-----------------------------------------------------------------*/
/*! 
  \file cmcomputeunit_hermite.c 
  \brief Check the output of the function calceph_compute_unit for the segment 12/13.

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

#include <stdio.h>
#include "calceph.h"
#include "openfiles.h"

static double myabs(double x);

int maincheck(int nbfile, const char *const filear[], const char * checkfile,  int prefetch);


static double myabs(double x)
{
    if (x!=x) return 1E300;
    return (x > 0 ? x : -x);
}

/*-----------------------------------------------------------------*/
/* main check function */
/*-----------------------------------------------------------------*/
int maincheck(int nbfile, const char *const filear[], const char * checkfile, int prefetch)
{
    int res, n, j;

    int target, center;

    double jd0;

    double dt;

    double PV[6];

    double PVcheck[6];

    double seuil;

    FILE *fcheck;

    t_calcephbin *peph;

    int unit;

    fcheck = tests_fopen(checkfile, "rt");
    if (fcheck == NULL)
    {
        printf("%s is missing\n", checkfile);
        return 1;
    }

    /* open the ephemeris file */
    peph = tests_calceph_open_array(nbfile, filear);
    res = (peph != NULL);

    if (res)
    {
        while (!feof(fcheck))
        {
            /* read the references coordinates */
            fscanf(fcheck, "%le %d %d ", &jd0, &target, &center);
            if (feof(fcheck))
            {
                res = 0;
                break;
            }
            
            for (j = 0; j < 6; j++)
                fscanf(fcheck, "%le", PV + j);

            dt = jd0 - (int) jd0;
            jd0 = ((int) jd0) + 2.4515450000000000000E+06;

            /* compute  the coordinates */
            seuil = 1E-10;
            unit = CALCEPH_UNIT_KM + CALCEPH_UNIT_SEC + CALCEPH_USE_NAIFID;
            n = 6;
            for (j = 0; j < n; j++)
                PVcheck[j] = 0.E0;
            if (calceph_compute_unit(peph, jd0, dt, target, center, unit, PVcheck) == 0)
            {
                res = 1;
                break;
            }
            for (j = 0; j < n; j++)
            {
                if (myabs((PVcheck[j] - PV[j])/PV[j]) >= seuil)
                {
                    printf("failure KM/SEC for %d %d at time %23.16E %23.16E : diff=%e %e (%23.16E - %23.16E)\n",
                           target, center, jd0, dt, PVcheck[j] - PV[j], (PVcheck[j] - PV[j])/PV[j], PVcheck[j], PV[j]);
                    printf("prefetch=%d file='%s'\n", prefetch, filear[0]);
                    res = 1;
                    return res;
                }
            }
        };

        /* close the ephemeris file */
        calceph_close(peph);
    }
    else
        res = 1;
    fclose(fcheck);
    return res;
}

