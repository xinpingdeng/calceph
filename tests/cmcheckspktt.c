/*-----------------------------------------------------------------*/
/*! 
  \file cmcheckspktt.c 
  \brief Check the output of the function calceph_compute with a SPICE kernel reference file 
  for the Coordinate Time Ephemeris

  \author  M. Gastineau 
           Astronomie et Systemes Dynamiques, IMCCE, CNRS, Observatoire de Paris. 

   Copyright, 2013, 2014, 2015, 2016, 2017, CNRS
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

static int maincheck(int prefetch);

int main(void);

static double myabs(double x)
{
    if (x != x)
        return 1E300;
    return (x > 0 ? x : -x);
}

/*-----------------------------------------------------------------*/
/* main program */
/*-----------------------------------------------------------------*/
static int maincheck(int prefetch)
{
    const char *filenamespk = "../examples/example1spk_time.bsp";

    int res = 1;
    int l, n, j, k;

    int target, center;

    double jd0;

    double dt;

    double PV[6];

    double PVcheck[6];

    double seuil;

    FILE *fcheck;

    t_calcephbin *peph;

    fcheck = tests_fopen("example1_tt.dat", "rt");
    if (fcheck == NULL)
    {
        printf("example1_tt.dat is missing\n");
        return 1;
    }
    /* open the ephemeris file */
    target = 16;
    center = 0;
    for (j = 1; j < 6; j++)
        PVcheck[j] = 0.E0;
    peph = tests_calceph_open(filenamespk);
    if (peph != NULL)
    {
        if (prefetch)
        {
            res = calceph_prefetch(peph);
            if (res==0) 
            {
                calceph_close(peph);
                return 1;
            }
        }

        while (!feof(fcheck))
        {
            /* read the references coordinates */
            fscanf(fcheck, "%le %le %le ", &jd0, &dt, &(PVcheck[0]));
            PVcheck[0] += 9.930698526885966E-005;   /* ecart au fichier */
            if (feof(fcheck))
            {
                res = 0;
                break;
            }

            /*printf("%23.16E %23.16E %d %d ", jd0, dt, target, center);
               for (int j=0; j<1; j++) printf("%23.16E ", PVcheck[j]);
               printf("\n"); */

            for (l = 0; l < (prefetch == 1 ? 20 : 1); l++)
            {
                /* compute  the coordinates */
                if (calceph_compute(peph, jd0, dt, target, center, PV) == 0)
                {
                    printf("prefetch=%d l=%d\n", prefetch, l);
                    res = 1;
                    break;
                }
                seuil = 6E-10;

/* check with references coordinates */
                n = 3;
                for (j = 0; j < n; j++)
                {
                    if (myabs(PVcheck[j] - PV[j]) >= seuil)
                    {
                        printf("failure for %d %d at time %23.16E %23.16E : diff=%e (%23.16E - %23.16E)\n",
                               target, center, jd0, dt, PVcheck[j] - PV[j], PVcheck[j], PV[j]);
                        for (k = 0; k < n; k++)
                            printf("%d : diff=%e (%23.16E - %23.16E)\n", k, PVcheck[k] - PV[k], PVcheck[k], PV[k]);
                        printf("prefetch=%d l=%d\n", prefetch, l);
                        res = 1;
                        return res;
                    }
                }
            }
        };

        /* close the ephemeris file */
        calceph_close(peph);
    }
    fclose(fcheck);
    return res;
}

/*-----------------------------------------------------------------*/
/* main program */
/*-----------------------------------------------------------------*/
int main(void)
{
    int res = maincheck(0);

    if (res == 0)
        res = maincheck(1);
    return res;
}
