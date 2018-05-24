/*-----------------------------------------------------------------*/
/*! 
  \file calcephlink.c 
  \brief manage the link tables for the SPICE KERNEL ephemeris data file 

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

#include "calcephconfig.h"
#if HAVE_STDIO_H
#include <stdio.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#if HAVE_MATH_H
#include <math.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "calcephdebug.h"
#include "real.h"
#define __CALCEPH_WITHIN_CALCEPH 1
#include "calceph.h"
#include "calcephinternal.h"
#include "util.h"

static int calceph_spice_tablelinkbody_makecomplexlink(struct SPICEtablelinkbody *table, int j, int k);

static int calceph_spice_cache_init(struct SPICEcache **cache, int count_body);

static void calceph_spice_cache_close(struct SPICEcache *cache, int count_body);

/*! invalid naif id */
#define NAIFID_INVALID (-(1<<24))

/*--------------------------------------------------------------------------*/
/*!  
     Initialize the content of table of link between the bodies.
  @param table (inout) table of links.   
*/
/*--------------------------------------------------------------------------*/
void calceph_spice_tablelinkbody_init(struct SPICEtablelinkbody *table)
{
    table->matrix_link = NULL;
    table->matrix_countlisttime = NULL;
    table->array_spiceid = NULL;
    table->count_body = 0;
    table->initemrat = 0;
    table->lastlookup_target = NAIFID_INVALID;
    table->lastlookup_targetpos = -1;
    table->lastlookup_center = NAIFID_INVALID;
    table->lastlookup_targetpos = -1;
    table->lastlookup_centerpos = -1;
    table->array_cache = NULL;
}

/*--------------------------------------------------------------------------*/
/*!  
     Free the content of table of link between the bodies.
  @param table (inout) table of links.   
*/
/*--------------------------------------------------------------------------*/
void calceph_spice_tablelinkbody_close(struct SPICEtablelinkbody *table)
{
    int j, n;

    for (j = 0; j < table->count_body * table->count_body; j++)
    {
        for (n = 0; n < table->matrix_countlisttime[j]; n++)
        {
            if (table->matrix_link[j][n].array_body)
                free(table->matrix_link[j][n].array_body);
            if (table->matrix_link[j][n].array_factors)
                free(table->matrix_link[j][n].array_factors);
            if (table->matrix_link[j][n].array_segment)
                free(table->matrix_link[j][n].array_segment);
            if (table->matrix_link[j][n].array_spkfile)
                free(table->matrix_link[j][n].array_spkfile);
            if (table->matrix_link[j][n].array_cache)
                free(table->matrix_link[j][n].array_cache);
        }
        if (table->matrix_link[j])
            free(table->matrix_link[j]);
    }
    if (table->matrix_link)
        free(table->matrix_link);
    if (table->matrix_countlisttime)
        free(table->matrix_countlisttime);
    if (table->array_spiceid)
        free(table->array_spiceid);
    calceph_spice_cache_close(table->array_cache, table->count_body);
}

#if DEBUG
/*--------------------------------------------------------------------------*/
/*!  
     Dump the content of the table
  @param table (inout) table
*/
/*--------------------------------------------------------------------------*/
static void calceph_spice_tablelinkbody_dump(struct SPICEtablelinkbody *table)
{
    int j, k, n, p;

    for (j = 0; j < table->count_body; j++)
    {
        printf("object %d :\n", table->array_spiceid[j]);
        for (k = 0; k < table->count_body; k++)
        {
            const struct SPICElinktime *link = table->matrix_link[j * table->count_body + k];

            int count = table->matrix_countlisttime[j * table->count_body + k];

            if (count > 0)
            {
                printf("\tlink to %d : number of time slice : %d\n", table->array_spiceid[k], count);
                for (n = 0; n < count; n++)
                {

                    printf("\t\t%g --> %g : %d  = ", link[n].T_begin, link[n].T_end, link[n].count_body);
                    for (p = 0; p < link[n].count_body; p++)
                    {
                        printf("{%d, %g, %p, %p, %p} ",
                               link[n].array_body[p],
                               link[n].array_factors[p], link[n].array_segment[p], link[n].array_spkfile[p],
                               link[n].array_cache[p]);
                    }
                    printf("\n");

                }
                printf("\n");
            }
        }
    }
}
#endif

/*--------------------------------------------------------------------------*/
/*! Locate the link between target and center 
  return 0 on error.
  return 1 on success.

  @param table (inout) table
  @param target (in) target object
  @param center (in) source object
  @param link (out) pointer to the link between target and center.
  @param countlisttime (out) pointer to the counter of time slices between target and center.
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_locatelinktime(struct SPICEtablelinkbody *table,
                                                      int target, int center, struct SPICElinktime ***link,
                                                      int **countlisttime)
{
    int postarget = -1, poscenter = -1;

    int j;

    int res = 0;

    for (j = 0; j < table->count_body; j++)
    {
        if (target == table->array_spiceid[j])
            postarget = j;
        if (center == table->array_spiceid[j])
            poscenter = j;
    }
    res = (postarget != -1 && poscenter != -1);
    if (res)
    {
        *link = table->matrix_link + (postarget * table->count_body + poscenter);
        *countlisttime = table->matrix_countlisttime + (postarget * table->count_body + poscenter);
    }
    else
    {
        *link = NULL;
        *countlisttime = NULL;
    }
    return res;
}

/*--------------------------------------------------------------------------*/
/*! Locate the link between target and center and insert complex link if no link exists
  return 0 on error and the error is already reportded.
  return -1 on error and error is not yet reportded.
  return 1 on success.

  @param table (inout) table
  @param target (in) target object
  @param center (in) source object
  @param link (out) pointer to the link between target and center.
  @param countlisttime (out) pointer to the counter of time slices between target and center.
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_locatelinktime_complex(struct SPICEtablelinkbody *table,
                                                              int target, int center, struct SPICElinktime ***link,
                                                              int **countlisttime)
{
    int postarget = -1, poscenter = -1;

    int j;

    int res = 0;

    if (table->lastlookup_target == target)
        postarget = table->lastlookup_targetpos;
    if (table->lastlookup_center == center)
    {
        poscenter = table->lastlookup_center;
        res = (postarget != -1 && poscenter != -1);
    }

    for (j = 0; j < table->count_body && res == 0; j++)
    {
        if (target == table->array_spiceid[j])
            postarget = j;
        if (center == table->array_spiceid[j])
            poscenter = j;
        res = (postarget != -1 && poscenter != -1);
    }
    if (res)
    {
        *link = table->matrix_link + (postarget * table->count_body + poscenter);
        *countlisttime = table->matrix_countlisttime + (postarget * table->count_body + poscenter);
        if (*countlisttime != NULL && **countlisttime == 0)
        {
            res = calceph_spice_tablelinkbody_makecomplexlink(table, postarget, poscenter);
            if (res)
            {
                res = calceph_spice_tablelinkbody_locatelinktime(table, target, center, link, countlisttime);
            }
            else
            {
                *link = NULL;
                *countlisttime = NULL;
            }
        }
    }
    else
    {
        res = -1;
        *link = NULL;
        *countlisttime = NULL;
    }
    return res;
}

/*--------------------------------------------------------------------------*/
/*! Locate the link between taget and center for the specified time
  return 0 on error.
  return 1 on success.

  @param table (inout) table
  @param target (in) target object
  @param center (in) source object
  @param link (out) links that corresponds to the objects and time.
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_locatebody(struct SPICEtablelinkbody *table,
                                                  int target, int center, struct SPICElinktime **link)
{
    int res;

    int *countlisttime;

    struct SPICElinktime **plink;

    res = calceph_spice_tablelinkbody_locatelinktime(table, target, center, &plink, &countlisttime);
    if (res)
        *link = *plink;
    return res;

}

/*--------------------------------------------------------------------------*/
/*!  
    Insert one body and adjust the tables to this new entry.
  return 0 on error.
  return 1 on success.

  @param table (inout) table.   
  @param body (in) body .
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_insertonebody(struct SPICEtablelinkbody *table, int body)
{
    int res = 1;

    int bfound = 0;

    int insert = 0;

    int j, k;

    /* find element */
    for (j = 0; j < table->count_body && bfound == 0; j++)
    {
        if (table->array_spiceid[j] < body)
            insert = j + 1;
        if (table->array_spiceid[j] == body)
        {
            bfound = 1;
            insert = j;
        }
    }
    if (bfound == 0)
    {
        /* insert new element */
        int *newarray_spiceid;

        int *newarray_countlisttime;

        struct SPICElinktime **newarray_link;

        int newcount = table->count_body + 1;

        /* increase the storage */
        newarray_spiceid = (int *) malloc(sizeof(int) * newcount);
        if (newarray_spiceid == NULL)
        {
            fatalerror("Can't allocate memory for %lu integers.\n", (unsigned long) newcount);
            return 0;
        }
        newarray_countlisttime = (int *) malloc(sizeof(int) * newcount * newcount);
        if (newarray_countlisttime == NULL)
        {
            fatalerror("Can't allocate memory for %lu integers.\n", (unsigned long) newcount * newcount);
            return 0;
        }
        newarray_link = (struct SPICElinktime **) malloc(sizeof(struct SPICElinktime *) * newcount * newcount);
        if (newarray_link == NULL)
        {
            fatalerror("Can't allocate memory for %lu integers.\n",
                       (unsigned long) newcount * newcount * sizeof(struct SPICElinktime *));
            return 0;
        }

        /* copy old data */
        for (j = 0; j < table->count_body; j++)
        {
            int newindex = (j < insert ? j : j + 1);

            newarray_spiceid[newindex] = table->array_spiceid[j];
            for (k = 0; k < table->count_body; k++)
            {
                int newindex2 = (k < insert ? k : k + 1);

                newarray_countlisttime[newindex * newcount + newindex2] =
                    table->matrix_countlisttime[j * table->count_body + k];
                newarray_link[newindex * newcount + newindex2] = table->matrix_link[j * table->count_body + k];
            }
        }
        /* create the new element */
        newarray_spiceid[insert] = body;
        for (j = 0; j < newcount; j++)
        {
            newarray_countlisttime[j * newcount + insert] = 0;
            newarray_countlisttime[insert * newcount + j] = 0;
            newarray_link[j * newcount + insert] = NULL;
            newarray_link[insert * newcount + j] = NULL;
        }

        /* release old storage */
        if (table->array_spiceid)
            free(table->array_spiceid);
        if (table->matrix_countlisttime)
            free(table->matrix_countlisttime);
        if (table->matrix_link)
            free(table->matrix_link);
        table->array_spiceid = newarray_spiceid;
        table->matrix_countlisttime = newarray_countlisttime;
        table->matrix_link = newarray_link;
        table->count_body = newcount;

    }
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
    Insert for "body" the link to "source" and the return the link and number of time slices.
  return 0 on error.
  return 1 on success.

  @param table (inout) table.   
  @param body (in) body to change in table.
  @param source (in) origin of the segment
  @param link (out) pointer to link between body and source
  @param countlisttime (out) number of time slice between body and source
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_inserttwobodies(struct SPICEtablelinkbody *table, int body, int source,
                                                       struct SPICElinktime ***link, int **countlisttime)
{
    int res;

    *link = NULL;
    *countlisttime = NULL;

    res = calceph_spice_tablelinkbody_insertonebody(table, body)
        && calceph_spice_tablelinkbody_insertonebody(table, source);

    if (res)
    {
        res = calceph_spice_tablelinkbody_locatelinktime(table, body, source, link, countlisttime);
    }
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
    Insert in the array link the new time slice.
  return 0 on error.
  return 1 on success.

  @param link (inout) pointer to the link : array of time slice.   
  @param countlisttime (inout) number of elements inside link
  @param alink (in) new time slice
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_inserttime(struct SPICElinktime **link, int *countlisttime,
                                                  struct SPICElinktime alink)
{
    int res = 1;

    int j;

    struct SPICElinktime *poldarray;

    struct SPICElinktime *newlink;

    int oldsize;

    /* add the time slice at insert */
    oldsize = *countlisttime;
    poldarray = *link;
    *link = newlink = (struct SPICElinktime *) malloc(sizeof(struct SPICElinktime) * (oldsize + 1));
    if (newlink == NULL)
    {
        fatalerror("Can't allocate memory for %lu integers.\n",
                   (unsigned long) (oldsize + 1) * sizeof(struct SPICElinktime));
        return 0;
    }
    for (j = 0; j < oldsize; j++)
    {
        newlink[j] = poldarray[j];
    }
    if (poldarray)
        free(poldarray);
    newlink[oldsize] = alink;
    *countlisttime = oldsize + 1;

    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
    Insert for "body" the link to "source"  for the time slice.
  return 0 on error.
  return 1 on success.

  @param table (inout) table.   
  @param body (in) body to change in table.
  @param source (in) origin of the segment
  @param alink (inout) link not duplicated
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_insertlink(struct SPICEtablelinkbody *table, int body, int source,
                                                  struct SPICElinktime alink)
{
    struct SPICElinktime **link;

    int *countlisttime;

    int res;

    res = calceph_spice_tablelinkbody_inserttwobodies(table, body, source, &link, &countlisttime);
    if (res)
    {
        res = calceph_spice_tablelinkbody_inserttime(link, countlisttime, alink);
    }

    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
    Allocate arrays for a link using coutn bodies
  return 0 on error.
  return 1 on success.

  @param newlink (inout) n allocated link.   
  @param count (in) number of bodies
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_SPICElinktime_init(struct SPICElinktime *newlink, int count)
{
    struct SPICElinktime link;

    link = *newlink;

    link.count_body = count;
    link.array_body = (int *) malloc(sizeof(int) * count);
    if (link.array_body == NULL)
    {
        fatalerror("Can't allocate memory for %d integers.\n", 1);
        return 0;
    }
    link.array_segment = (struct SPKSegmentHeader **) malloc(sizeof(struct SPKSegmentHeader *) * count);
    if (link.array_segment == NULL)
    {
        fatalerror("Can't allocate memory for %d data structure.\n", count);
        return 0;
    }
    link.array_spkfile = (struct SPKfile **) malloc(sizeof(struct SPKfile *) * count);
    if (link.array_spkfile == NULL)
    {
        fatalerror("Can't allocate memory for %d data structure.\n", count);
        return 0;
    }
    link.array_cache = (struct SPICEcache **) malloc(sizeof(struct SPICEcache *) * count);
    if (link.array_cache == NULL)
    {
        fatalerror("Can't allocate memory for %d data structure.\n", count);
        return 0;
    }
    link.array_factors = (double *) malloc(sizeof(double) * count);
    if (link.array_factors == NULL)
    {
        fatalerror("Can't allocate memory for %d floating point numbers.\n", count);
        return 0;
    }

    *newlink = link;

    return 1;
}

/*--------------------------------------------------------------------------*/
/*!  
    Insert for "body" the link to "source"  using "destination"*factor for the time slice.
    the vector assumes "source-->body" = "destination"*factor f
  return 0 on error.
  return 1 on success.

  @param table (inout) table.   
  @param body (in) body to change in table.
  @param source (in) origin of the segment
  @param destination (in) target of the segment
  @param factor (in) factor to apply
  @param T_begin (in) beginning of the time slice
  @param T_end (in) end of the time slice
  @param seg (in) segment that owns this link
  @param newkernel (in) kernel that owns this link
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_insert(struct SPICEtablelinkbody *table, int body, int source, int destination,
                                              double factor, double T_begin, double T_end,
                                              const struct SPKSegmentHeader *seg, struct SPKfile *newkernel)
{
    struct SPICElinktime link;

    int res;

    res = calceph_spice_SPICElinktime_init(&link, 1);
    if (res)
    {
        link.T_begin = T_begin;
        link.T_end = T_end;
        link.array_body[0] = destination;
        link.array_segment[0] = (struct SPKSegmentHeader *) seg;
        link.array_spkfile[0] = newkernel;
        link.array_cache[0] = NULL;
        link.array_factors[0] = factor;

        res = calceph_spice_tablelinkbody_insertlink(table, body, source, link);
    }
#if DEBUG
    if (res == 0)
        printf("calceph_spice_tablelinkbody_insert(%d,%d) returns %d\n", body, source, res);
#endif
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
    Insert into link a new connection using factor1*link1+factor2*link2
  return 0 on error.
  return 1 on success.

  @param link (inout) link to update.   
  @param newcounttimeslice (inout)  number of time slice
  @param factor1 (in) factor to apply link1
  @param link1 (in) link1 to connect the object
  @param factor2 (in) factor to apply link2
  @param link2 (in) link2 to connect the object
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_mergelink(struct SPICElinktime **link, int *newcounttimeslice,
                                                 double factor1, const struct SPICElinktime *link1,
                                                 double factor2, const struct SPICElinktime *link2)
{
    int res = 1;

    int j, k;

    struct SPICElinktime newlink;

    int oldsize;

    int countlink = link1->count_body + link2->count_body;

    struct SPICElinktime *oldlink = *link;

    double common_tmin = (link1->T_begin <= link2->T_begin ? link2->T_begin : link1->T_begin);

    double common_tmax = (link1->T_end <= link2->T_end ? link1->T_end : link2->T_end);

    oldsize = *newcounttimeslice;

    /* check if a link exist with the same date but shortest path */
    for (j = 0; j < oldsize; j++)
    {
        if (oldlink[j].T_begin == common_tmin && oldlink[j].T_end == common_tmax)
        {
            if (countlink >= oldlink[j].count_body)
                return 1;
        }
    }

#if DEBUG
    printf("calceph_spice_tablelinkbody_mergelink : add link %E %E\n", link1->T_begin, link1->T_end);
#endif

    calceph_spice_SPICElinktime_init(&newlink, countlink);
    newlink.T_begin = common_tmin;
    newlink.T_end = common_tmax;

    for (k = 0; k < link1->count_body; k++)
    {
        newlink.array_body[k] = link1->array_body[k];
        newlink.array_segment[k] = link1->array_segment[k];
        newlink.array_spkfile[k] = link1->array_spkfile[k];
        newlink.array_cache[k] = link1->array_cache[k];
        newlink.array_factors[k] = factor1 * link1->array_factors[k];
    }
    for (k = 0; k < link2->count_body; k++)
    {
        int l = link1->count_body + k;

        newlink.array_body[l] = link2->array_body[k];
        newlink.array_segment[l] = link2->array_segment[k];
        newlink.array_spkfile[l] = link2->array_spkfile[k];
        newlink.array_cache[l] = link2->array_cache[k];
        newlink.array_factors[l] = factor2 * link2->array_factors[k];
    }

    res = calceph_spice_tablelinkbody_inserttime(link, newcounttimeslice, newlink);

#if DEBUG
    printf("calceph_spice_tablelinkbody_mergelink : returns %d\n", res);
#endif
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
    make all links between the objects j and k
    assume that no link exist between j and k before that call
  return 0 on error.
  return 1 on success.

  @param table (inout) table. 
  @param j (in) index of the first body
  @param k (in) index of the second body
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_makecomplexlink(struct SPICEtablelinkbody *table, int j, int k)
{
    int countbody = table->count_body;

    int res = 1;

    int l;

    int u, v;

    for (l = 0; l < countbody; l++)
    {
        int count1, count2;

        if (table->array_spiceid[l] == -1 || j == l || k == l)
            continue;
        count1 = table->matrix_countlisttime[k * countbody + l];

        count2 = table->matrix_countlisttime[l * countbody + j];

        if (count1 != 0 && count2 != 0)
        {
            const struct SPICElinktime *link1 = table->matrix_link[k * countbody + l];

            const struct SPICElinktime *link2 = table->matrix_link[l * countbody + j];

#if DEBUG
            printf("look to a common interval %d %d %d l=%d\n", table->array_spiceid[j],
                   table->array_spiceid[l], table->array_spiceid[k], l);
#endif
            for (u = 0; u < count1; u++)
            {
                for (v = 0; v < count2; v++)
                {
                    double common_tmin = (link1[u].T_begin <= link2[v].T_begin ? link2[v].T_begin : link1[u].T_begin);

                    double common_tmax = (link1[u].T_end <= link2[v].T_end ? link1[u].T_end : link2[v].T_end);

                    if (common_tmin <= common_tmax)
                    {
                        if (calceph_spice_tablelinkbody_mergelink
                            (table->matrix_link + j * countbody + k,
                             table->matrix_countlisttime + j * countbody + k, -1E0, link1 + u, -1E0, link2 + v) == 0)
                            return 0;
                        if (calceph_spice_tablelinkbody_mergelink
                            (table->matrix_link + k * countbody + j,
                             table->matrix_countlisttime + k * countbody + j, 1E0, link2 + v, 1E0, link1 + u) == 0)
                            return 0;
                    }
                }
            }
        }
    }

#if DEBUG
    printf("ap insertlink=%d\n", table->count_body);
    for (j = 0; j < table->count_body; j++)
    {
        for (k = 0; k < table->count_body; k++)
        {
            printf("%d ", table->matrix_countlisttime[j * table->count_body + k]);
        }
        printf("\n");
    }
#endif
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
    make all links between the objects with the same time limit
  return 0 on error.
  return 1 on success.

  @param table (inout) table.   
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_makealllinks(struct SPICEtablelinkbody *table)
{
    int countbody = table->count_body;

    int res = 1;

    int insert;

    int j, k, l;

    do
    {
        insert = 0;
        for (j = 0; j < countbody; j++)
        {
            if (table->array_spiceid[j] == -1)
                continue;
            for (k = j + 1; k < countbody; k++)
            {
                if (table->array_spiceid[k] == -1 || j == k)
                    continue;
                if (table->matrix_countlisttime[j * countbody + k] == 0)
                {
                    for (l = 0; l < countbody; l++)
                    {
                        int count1, count2;

                        if (table->array_spiceid[l] == -1 || j == l || k == l)
                            continue;
                        count1 = table->matrix_countlisttime[k * countbody + l];

                        count2 = table->matrix_countlisttime[l * countbody + j];

                        if (count1 != 0 && count2 != 0)
                        {
                            const struct SPICElinktime *link1 = table->matrix_link[k * countbody + l];

                            const struct SPICElinktime *link2 = table->matrix_link[l * countbody + j];

                            if (count1 == 1 && count2 == 1
                                && link1[0].T_begin == link2[0].T_begin && link1[0].T_end == link2[0].T_end)
                            {
#if DEBUG
                                printf("makealllinks %d --> %d --> %d\n", table->array_spiceid[j],
                                       table->array_spiceid[l], table->array_spiceid[k]);
#endif
                                if (calceph_spice_tablelinkbody_mergelink
                                    (table->matrix_link + j * countbody + k,
                                     table->matrix_countlisttime + j * countbody + k, -1E0, link1, -1E0, link2) == 0)
                                    return 0;
                                if (calceph_spice_tablelinkbody_mergelink(table->matrix_link + k * countbody + j,
                                                                          table->matrix_countlisttime + k * countbody +
                                                                          j, 1E0, link2, 1E0, link1) == 0)
                                    return 0;
                                insert = 1;
                            }
                        }
                    }
                }
            }
        }

#if DEBUG
        printf("ap insertlink=%d\n", table->count_body);
        for (j = 0; j < table->count_body; j++)
        {
            for (k = 0; k < table->count_body; k++)
            {
                printf("%d ", table->matrix_countlisttime[j * table->count_body + k]);
            }
            printf("\n");
        }
#endif
    }
    while (insert);
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
     Add the the segment in the table 
     Build all possible link between the objects
  return 0 on error.
  return 1 on success.

  @param table (inout) table.   
  @param seg (in) segment to add.
  @param newkernel (in) kernel that owns this link
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_addseg(struct SPICEtablelinkbody *table, const struct SPKSegmentHeader *seg,
                                              struct SPKfile *newkernel)
{
    int res;

    int body = seg->body;

    int center = seg->center;

    res = calceph_spice_tablelinkbody_insert(table, body, center, body, 1.E0, seg->T_begin, seg->T_end, seg, newkernel);
    if (res)
        res = calceph_spice_tablelinkbody_insert(table, center, body, body, -1.0, seg->T_begin, seg->T_end, seg,
                                                 newkernel);
#if DEBUG
    if (res == 0)
        printf("calceph_spice_tablelinkbody_addseg() returns %d\n", res);
#endif
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
     Add the list of the segment in the table 
     Build all possible link between the objects
  return 0 on error.
  return 1 on success.

  @param table (inout) table.   
  @param list_seg (in) list of segments to add.
  @param newkernel (in) links are extract from that kernel.
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_addlistseg(struct SPICEtablelinkbody *table,
                                                  const struct SPKSegmentList *list_seg, struct SPKfile *newkernel)
{
    int res = 1;

    const struct SPKSegmentList *ptr = list_seg;

    int j;

    while (ptr != NULL)
    {
        for (j = 0; j < ptr->array_seg.count && res == 1; j++)
        {
            res = calceph_spice_tablelinkbody_addseg(table, ptr->array_seg.array + j, newkernel);
        }
        ptr = ptr->next;
    }

#if DEBUG
    calceph_spice_tablelinkbody_dump(table);
#endif
    if (res)
    {
        res = calceph_spice_tablelinkbody_makealllinks(table);
    }
#if DEBUG
    calceph_spice_tablelinkbody_dump(table);
#endif
#if DEBUG
    if (res == 0)
        printf("calceph_spice_tablelinkbody_addlistseg() returns %d\n", res);
#endif
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
     Add the content of newkernel to the table inside eph.
     Build all possible link between the objects
  return 0 on error.
  return 1 on success.
  
  @param eph (inout) SPICE ephemeris.   
  @param newkernel (in) links are extract from that kernel.
*/
/*--------------------------------------------------------------------------*/
int calceph_spice_tablelinkbody_addfile(struct calcephbin_spice *eph, struct SPICEkernel *newkernel)
{
    int res = 1;

    int n;

    struct SPICEtablelinkbody *table = &eph->tablelink;

    struct SPICElinktime *link;

    switch (newkernel->filetype)
    {
        case DAF_PCK:
        case DAF_SPK:
            res = calceph_spice_tablelinkbody_addlistseg(table, newkernel->filedata.spk.list_seg,
                                                         &newkernel->filedata.spk);
            break;

        case TXT_PCK:
        case TXT_FK:
        default:
            break;
    }
    if (table->initemrat == 0)
    {
        double demrat = calceph_spice_getEMRAT(eph);

#if DEBUG
        printf("initemrat=0 EMRAT=%g\n", demrat);
#endif

        if (demrat != 0.E0)
        {
            int found = 0;

            if (calceph_spice_tablelinkbody_locatebody(table, 301, 399, &link))
                found = 1;

            if (found)
            {
                for (n = 0; n < 1; n++)
                {
                    if (link[n].count_body == 1)
                    {
                        /* ajout de Terre/BTL */
                        res =
                            calceph_spice_tablelinkbody_insert(table, 399, 3, 399, -1.E0 / (demrat + 1.E0),
                                                               link[n].T_begin,
                                                               link[n].T_end,
                                                               link[n].array_segment[0], link[n].array_spkfile[0]);
                        calceph_spice_tablelinkbody_locatebody(table, 301, 399, &link);
                        res =
                            calceph_spice_tablelinkbody_insert(table, 3, 399, 399, 1.E0 / (demrat + 1.E0),
                                                               link[n].T_begin,
                                                               link[n].T_end,
                                                               link[n].array_segment[0], link[n].array_spkfile[0]);
                        calceph_spice_tablelinkbody_locatebody(table, 301, 399, &link);
                        res =
                            calceph_spice_tablelinkbody_insert(table, 301, 3, 399, demrat / (demrat + 1.E0),
                                                               link[n].T_begin,
                                                               link[n].T_end,
                                                               link[n].array_segment[0], link[n].array_spkfile[0]);
                        calceph_spice_tablelinkbody_locatebody(table, 301, 399, &link);
                        res =
                            calceph_spice_tablelinkbody_insert(table, 3, 301, 399, -demrat / (demrat + 1.E0),
                                                               link[n].T_begin,
                                                               link[n].T_end,
                                                               link[n].array_segment[0], link[n].array_spkfile[0]);

                        if (res == 0)
                        {
#if DEBUG
                            printf("calceph_spice_tablelinkbody_addfile() returns %d\n", res);
#endif
                            return 0;
                        }

                    }
                }
                table->initemrat = 1;
                res = calceph_spice_tablelinkbody_makealllinks(table);
            }
        }
    }

#if DEBUG
    calceph_spice_tablelinkbody_dump(table);
    printf("calceph_spice_tablelinkbody_addfile() returns %d\n", res);
#endif
    return res;
}

/*--------------------------------------------------------------------------*/
/*! Locate the link between taget and center for the specified time
  return 0 on error.
  return 1 on success.

  @param table (inout) table
  @param JD0 (in) integer part time
  @param time (in) fractional part of time
  @param target (in) target object
  @param center (in) source object
  @param link (out) links that corresponds to the objects and time.
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_tablelinkbody_locate(struct SPICEtablelinkbody *table, double JD0, double time,
                                              int target, int center, struct SPICElinktime **link)
{
    int n;

    int *countlisttime;

    int res;

    double dtime = JD0 + time;

    struct SPICElinktime *poslink;

    struct SPICElinktime **loclink;

    double dtimsec = ((JD0 - 2.451545E+06) + time) * 86400E0;

    res = calceph_spice_tablelinkbody_locatelinktime_complex(table, target, center, &loclink, &countlisttime);
    if (res == 1)
    {
        poslink = *loclink;
        res = 0;
        for (n = 0; n < *countlisttime; n++)
        {

            if (poslink[n].T_begin <= dtimsec && dtimsec <= poslink[n].T_end)
            {
                *link = poslink + n;
                return 1;
            }
        }
#if DEBUG
        printf("time : %d %d : %g %g\n", target, center, poslink[0].T_begin, poslink[0].T_end);
        printf("time : %d %d : %g \n", target, center, dtimsec);
#endif
        if (res == 0)
            fatalerror("Can't find time %23.16E in the ephemeris file\n", dtime);
    }
    else if (res == -1)
        fatalerror("Can't find time %23.16E in the ephemeris file\n", dtime);

    return 0;
}

/*--------------------------------------------------------------------------*/
/*! Locate the link between taget and center for the specified time
  return 0 on error.
  return 1 on success.

  @param eph (inout) ephemeris
  @param JD0 (in) integer part time
  @param time (in) fractional part of time
  @param target (in) target object
  @param center (in) source object
  @param statetarget (out) position and veloticity                                                                        
*/
/*--------------------------------------------------------------------------*/
int calceph_spice_tablelinkbody_compute(struct calcephbin_spice *eph, double JD0, double time, int target,
                                        int center, stateType * statetarget)
{
    struct SPICEtablelinkbody *table = &eph->tablelink;

    struct SPICElinktime *link;

    stateType state;

    int res;

    int j, kcache;

    double rotationmatrix[3][3];

    state.order = statetarget->order;
    calceph_stateType_set_0(statetarget, state.order);

    /* initialize the cache */
    if (table->array_cache == NULL)
    {
        res = calceph_spice_cache_init(&table->array_cache, table->count_body);
        if (res == 0)
            return 0;
    }

    /* locate the objects */
    res = calceph_spice_tablelinkbody_locate(table, JD0, time, target, center, &link);

    if (res)
    {
        /* perform the computation */
        for (j = 0; j < link->count_body && res == 1; j++)
        {
            double factor = link->array_factors[j];

            /* find the cache */
            if (link->array_spkfile[j]->prefetch == 0 && link->array_cache[j] == NULL)
            {
                for (kcache = 0; kcache < table->count_body && link->array_body[j] != table->array_spiceid[kcache];
                     kcache++)
                {
                }
                link->array_cache[j] = table->array_cache + kcache;
            }

            if (link->array_segment[j]->refframe == 1)
            {
                res =
                    calceph_spk_interpol_PV_segment(link->array_spkfile[j], link->array_segment[j],
                                                    link->array_cache[j], JD0, time, &state);
            }
            else if (calceph_spice_findframe_matrix(eph, link->array_segment[j]->refframe, rotationmatrix) != 0)
            {
                res =
                    calceph_spk_interpol_PV_segment(link->array_spkfile[j], link->array_segment[j],
                                                    link->array_cache[j], JD0, time, &state);
                calceph_stateType_rotate(&state, rotationmatrix);
            }
            else
            {
                fatalerror("Time %23.16E found in the ephemeris file but in an unsupported reference frame (%d)\n",
                           JD0 + time, link->array_segment[j]->refframe);
                res = 0;
            }
            calceph_stateType_fma(statetarget, factor, state);
        }
    }
    return res;
}

/*--------------------------------------------------------------------------*/
/*!  
     Free the cache per body.
  @param cache (inout) array of caches   
  @param count_body (in) number of element of cache   
*/
/*--------------------------------------------------------------------------*/
static void calceph_spice_cache_close(struct SPICEcache *cache, int count_body)
{
    int j;

    if (cache)
    {
        for (j = 0; j < count_body; j++)
        {
            if (cache[j].buffer)
                free(cache[j].buffer);
        }
        free(cache);
    }
}

/*--------------------------------------------------------------------------*/
/*!  
     initialize the cache per body.
     
     return 1 on success
     return 0 on error
     
  @param cache (inout) array of caches   
  @param count_body (in) number of element of cache   
*/
/*--------------------------------------------------------------------------*/
static int calceph_spice_cache_init(struct SPICEcache **cache, int count_body)
{
    struct SPICEcache *pcache;

    int res;

    int j;

    *cache = pcache = (struct SPICEcache *) malloc(sizeof(struct SPICEcache) * count_body);
    if (pcache)
    {
        for (j = 0; j < count_body; j++)
        {
            pcache[j].buffer = NULL;
            pcache[j].segment = NULL;
            pcache[j].buffer_size = 0;
            pcache[j].rec_begin = -1;
        }
        res = 1;
    }
    else
    {
        fatalerror("Can't allocate memory for %lu bytes\n", (unsigned long) (sizeof(struct SPICEcache) * count_body));
        res = 0;
    }
    return res;
}
