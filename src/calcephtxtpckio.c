/*-----------------------------------------------------------------*/
/*! 
  \file calcephtxtpckio.c 
  \brief perform I/O on text PCK KERNEL ephemeris data file
         read all constants from a text PCK KERNEL Ephemeris data.
         
  \author  M. Gastineau 
           Astronomie et Systemes Dynamiques, IMCCE, CNRS, Observatoire de Paris. 

   Copyright, 2011, 2012, 2013, 2014, 2015, 2016, 2017, CNRS
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
#ifndef __cplusplus
#if HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#endif                          /* __cplusplus */
#include <ctype.h>

#include "calcephdebug.h"
#include "real.h"
#define __CALCEPH_WITHIN_CALCEPH 1
#include "calceph.h"
#include "calcephspice.h"
#include "util.h"
#include "calcephinternal.h"

/*--------------------------------------------------------------------------*/
/* private functions */
/*--------------------------------------------------------------------------*/
static int calceph_txtpck_getconstant_intmatrixnxn(struct TXTPCKfile *eph, const char *name, int n1, int n2,
                                                   int *p_pival);
static int calceph_txtpck_getconstant_matrixnxn(struct TXTPCKfile *eph, const char *name, int n1, int n2,
                                                double *p_pdval);

#if DEBUG
/*--------------------------------------------------------------------------*/
/*!  
     Debug the list of constant of a text PCK kernel
  
  @param header (in) header of text PCK
*/
/*--------------------------------------------------------------------------*/
void calceph_txtpck_debug(const struct TXTPCKfile *header)
{
    struct TXTPCKconstant *listconstant;

    printf("list of constant of the text PCK file : \n");
    for (listconstant = header->listconstant; listconstant != NULL; listconstant = listconstant->next)
    {
        struct TXTPCKvalue *listvalue;

        printf("'%s' = ( ", listconstant->name);
        for (listvalue = listconstant->value; listvalue != NULL; listvalue = listvalue->next)
        {
            printf("%.*s", (int) (listvalue->loclast - listvalue->locfirst), listvalue->buffer + listvalue->locfirst);
            if (listvalue->next != NULL)
                printf(" , ");
        }
        printf(" ) \n");
    }
}
#endif

/*--------------------------------------------------------------------------*/
/*!  Read the constants of the text PCK kernel file

  @return  1 on sucess and 0 on failure
 
 @param file (inout) file descriptor.   
 @param filename (in) A character string giving the name of an ephemeris data file.   
 @param res (out) descriptor of the ephemeris.
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_open(FILE * file, const char *filename, struct SPICEkernel *res)
{
    res->filetype = TXT_PCK;
    return calceph_txtpck_load(file, filename, "KPL/PCK", &res->filedata.txtpck);
}

/*--------------------------------------------------------------------------*/
/*!  Read the constants of the text PCK kernel file

  @return  1 on sucess and 0 on failure
 
 @param file (inout) file descriptor.   
 @param filename (in) A character string giving the name of an ephemeris data file.   
 @param header (in) header in the file (e.g., "PKL/MK" or "KPL/PCK")
 @param res (out) descriptor of the text file.
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_load(FILE * file, const char *filename, const char *header, struct TXTPCKfile *res)
{
    off_t lenfile;

    size_t ulenfile;

    char *buffer;

    off_t curpos;

    const char *begindata = "\\begindata";

    const size_t lenbegindata = strlen(begindata);

    const char *begintext = "\\begintext";

    const size_t lenbegintext = strlen(begintext);

    struct TXTPCKconstant *listconstant = NULL, *precconstant = NULL;

    const size_t lenheader = strlen(header);

    /*---------------------------------------------------------*/
    /* get the file size and read all the file to the memory */
    /*---------------------------------------------------------*/
    /* go to the end of the file */
    if (fseeko(file, 0L, SEEK_END) != 0)
    {
        fatalerror("Can't jump to the end of the ephemeris file '%s'\nSystem error : '%s'\n",
                   filename, strerror(errno));
        return 0;
    }
    lenfile = ftello(file);
    if (lenfile < 0)
    {
        fatalerror("Can't get the size of the ephemeris file '%s'\nSystem error : '%s'\n", filename, strerror(errno));
        return 0;
    }
    /* allocate memory */
    lenfile++;
    buffer = (char *) malloc(sizeof(char) * (size_t)lenfile);
    if (buffer == NULL)
    {
        fatalerror("Can't allocate memory for %lu bytes\nSystem error : '%s'\n",
                   (unsigned long) (sizeof(char) * (size_t)lenfile), strerror(errno));
        return 0;
    }
    buffer[lenfile - 1] = '\0';

    /* read the file  */
    if (fseeko(file, 0L, SEEK_SET) != 0)
    {
        fatalerror("Can't jump to the end of the ephemeris file '%s'\nSystem error : '%s'\n",
                   filename, strerror(errno));
        free(buffer);
        return 0;
    }
    /* printf("lenfile=%d\n", (int) lenfile); */
    lenfile--;
    ulenfile = (size_t) lenfile;
    if (fread(buffer, sizeof(char), ulenfile, file) != ulenfile)
    {
        fatalerror("Can't read the ephemeris file '%s'\nSystem error : '%s'\n", filename, strerror(errno));
        free(buffer);
        return 0;
    }
    /* check the header  */
    if (ulenfile < lenheader || strncmp(buffer, header, lenheader) != 0)
    {
        fatalerror("Can't read the header of the ephemeris file '%s'\nSystem error : '%s'\n",
                   filename, strerror(errno));
        free(buffer);
        return 0;
    }
    /*---------------------------------------------------------*/
    /* loop until the end of file to read the constant */
    /*---------------------------------------------------------*/
    curpos = lenheader;
    while (curpos < lenfile)
    {
        while (curpos < lenfile && buffer[curpos] != '\\')
            curpos++;
        /* look for data section */
        if (curpos + lenbegindata < ulenfile && strncmp(buffer + curpos, begindata, lenbegindata) == 0)
        {
            /* printf("enter in a  data section : '%.50s'\n", buffer + curpos); */
            curpos += lenbegindata;
            /* to skip \begindata inside a text ssection */
            while (curpos < lenfile && buffer[curpos] != '\n' && buffer[curpos] != '\r' && isspace(buffer[curpos]))
                curpos++;
            if (buffer[curpos] != '\n' && buffer[curpos] != '\r')
                continue;

            while (curpos < lenfile && buffer[curpos] != '\\')
            {
                while (curpos < lenfile && isspace(buffer[curpos]))
                    curpos++;
                if (curpos < lenfile && buffer[curpos] != '\\')
                {
                    struct TXTPCKconstant *pnewcst;

                    struct TXTPCKvalue *listvalue = NULL, *prec = NULL;

                    /* read the keyword */
                    off_t begkeyword = curpos;

                    off_t endkeyword;

                    off_t begvalue, endvalue;

                    while (curpos < lenfile && !isspace(buffer[curpos]) && buffer[curpos] != '=')
                        curpos++;
                    endkeyword = curpos;
                    /* printf("keyword= %d %d '%.*s'\n", (int) begkeyword, (int) endkeyword, (int) (endkeyword - begkeyword), buffer + begkeyword); */
                    while (curpos < lenfile && (isspace(buffer[curpos]) || buffer[curpos] == '='))
                        curpos++;

                    /* find the ( or ' or digit or sign +- */
                    while (curpos < lenfile && isspace(buffer[curpos]))
                        curpos++;
                    /* check the parenthesis or ' or digit or sign +- */
                    if (buffer[curpos] != '(' && buffer[curpos] != '\'' && !isdigit(buffer[curpos])
                        && buffer[curpos] != '+' && buffer[curpos] != '-')
                    {
                        fatalerror("Can't find  the character '(' , ' or a digit after the keyword '%.*s'\n",
                                   endkeyword - begkeyword, buffer + begkeyword);
                        free(buffer);
                        return 0;
                    }
                    /* single string */
                    if (buffer[curpos] == '\'')
                    {
                        begvalue = curpos;
                        curpos++;
                        while (curpos < lenfile && buffer[curpos] != '\'')
                            curpos++;
                        curpos++;
                        endvalue = curpos;
                        /* create the new value */
                        listvalue = (struct TXTPCKvalue *) malloc(sizeof(struct TXTPCKvalue));
                        if (listvalue == NULL)
                        {
                            fatalerror("Can't allocate memory for %lu bytes\nSystem error : '%s'\n",
                                       (unsigned long) (sizeof(struct TXTPCKvalue)), strerror(errno));
                            free(buffer);
                            return 0;
                        }
                        listvalue->next = NULL;
                        listvalue->buffer = buffer;
                        listvalue->locfirst = begvalue;
                        listvalue->loclast = endvalue;
                    }
                    else if (buffer[curpos] == '(')
                    {
                        curpos++;   /* skip parenthesis */
                        while (curpos < lenfile && buffer[curpos] != ')')
                        {
                            struct TXTPCKvalue *pnewval;

                            /* read the value */
                            while (curpos < lenfile && isspace(buffer[curpos]))
                                curpos++;
                            begvalue = curpos;

                            if (curpos < lenfile && buffer[curpos] == '\'')
                            {   /* begin a string */
                                curpos++;
                                while (curpos < lenfile && buffer[curpos] != '\'')
                                    curpos++;
                                curpos++;
                                endvalue = curpos;
                            }
                            else
                            {   /* begin a value */
                                off_t k;

                                while (curpos < lenfile && !isspace(buffer[curpos]) && buffer[curpos] != ',' &&
                                       buffer[curpos] != ')')
                                {
                                    curpos++;
                                }
                                endvalue = curpos;
                                /*replace fortran number by C number */
                                for (k = begvalue; k <= endvalue; k++)
                                {
                                    if (buffer[k] == 'D' || buffer[k] == 'd')
                                        buffer[k] = 'E';
                                }
                            }
                            /* printf("value= %d %d '%.*s'\n", (int) begvalue, (int) endvalue, (int) (endvalue - begvalue), buffer + begvalue); */
                            /*dvalue = strtod(buffer + begvalue, NULL);
                               if (buffer[curpos]=='D' || buffer[curpos]=='d') buffer[curpos]='E';  */

                            /* create the new value */
                            pnewval = (struct TXTPCKvalue *) malloc(sizeof(struct TXTPCKvalue));
                            if (pnewval == NULL)
                            {
                                fatalerror("Can't allocate memory for %lu bytes\nSystem error : '%s'\n",
                                           (unsigned long) (sizeof(struct TXTPCKvalue)), strerror(errno));
                                free(buffer);
                                return 0;
                            }
                            pnewval->next = NULL;
                            pnewval->buffer = buffer;
                            pnewval->locfirst = begvalue;
                            pnewval->loclast = endvalue;
                            if (prec == NULL)
                                listvalue = prec = pnewval;
                            else
                            {
                                prec->next = pnewval;
                                prec = pnewval;
                            }

                            /* find the ) or , */
                            while (curpos < lenfile && isspace(buffer[curpos]))
                                curpos++;

                            if (curpos < lenfile && buffer[curpos] == ',')
                                curpos++;
                        }
                        curpos++;   /* skip parenthesis */
                    }
                    /* digit case */
                    else if (isdigit(buffer[curpos]) || buffer[curpos] == '+' || buffer[curpos] == '-')
                    {
                        struct TXTPCKvalue *pnewval;

                        off_t k;

                        /* begin a value */
                        begvalue = curpos;
                        while (curpos < lenfile && (isdigit(buffer[curpos]) || buffer[curpos] == '.'
                                                    || buffer[curpos] == 'D' || buffer[curpos] == 'd'
                                                    || buffer[curpos] == 'E' || buffer[curpos] == '+'
                                                    || buffer[curpos] == '-'))
                        {
                            curpos++;
                        }
                        endvalue = curpos;
                        /*replace fortran number by C number */
                        for (k = begvalue; k <= endvalue; k++)
                        {
                            if (buffer[k] == 'D' || buffer[k] == 'd')
                                buffer[k] = 'E';
                        }

                        /* create the new value */
                        pnewval = (struct TXTPCKvalue *) malloc(sizeof(struct TXTPCKvalue));
                        if (pnewval == NULL)
                        {
                            fatalerror("Can't allocate memory for %lu bytes\nSystem error : '%s'\n",
                                       (unsigned long) (sizeof(struct TXTPCKvalue)), strerror(errno));
                            free(buffer);
                            return 0;
                        }
                        pnewval->next = NULL;
                        pnewval->buffer = buffer;
                        pnewval->locfirst = begvalue;
                        pnewval->loclast = endvalue;
                        if (prec == NULL)
                            listvalue = prec = pnewval;
                        else
                        {
                            prec->next = pnewval;
                            prec = pnewval;
                        }
                    }

                    /* create the new constant */
                    pnewcst = (struct TXTPCKconstant *) malloc(sizeof(struct TXTPCKconstant));
                    if (pnewcst == NULL)
                    {
                        fatalerror("Can't allocate memory for %lu bytes\nSystem error : '%s'\n",
                                   (unsigned long) (sizeof(struct TXTPCKconstant)), strerror(errno));
                        free(buffer);
                        return 0;
                    }
                    pnewcst->next = NULL;
                    pnewcst->value = listvalue;
                    pnewcst->name = (char *) malloc(((size_t)(endkeyword - begkeyword + 1)) * sizeof(char));
                    if (pnewcst->name == NULL)
                    {
                        fatalerror("Can't allocate memory for %lu bytes\nSystem error : '%s'\n",
                                   (unsigned long) (sizeof(endkeyword - begkeyword + 1)), strerror(errno));
                        free(buffer);
                        return 0;
                    }
                    memcpy(pnewcst->name, buffer + begkeyword, (endkeyword - begkeyword) * sizeof(char));
                    pnewcst->name[endkeyword - begkeyword] = '\0';
                    if (precconstant == NULL)
                        precconstant = listconstant = pnewcst;
                    else
                    {
                        precconstant->next = pnewcst;
                        precconstant = precconstant->next;
                    }
                }
                if (curpos + lenbegintext < ulenfile && strncmp(buffer + curpos, begintext, lenbegintext) == 0)
                {
                    /* printf("start of a text section : %d\n", (int) curpos); */
                    curpos += lenbegintext;
                    break;      /* end the data section */
                }
            }
        }
        curpos++;

    }

    res->listconstant = listconstant;
    res->file = file;
    res->buffer = buffer;

    return 1;
}

/*--------------------------------------------------------------------------*/
/*
 * ! Close the  text PCK kernel file
 *
 *
 * @param eph (inout) descriptor of the ephemeris.
 */
/*--------------------------------------------------------------------------*/
void calceph_txtpck_close(struct TXTPCKfile *eph)
{
    struct TXTPCKconstant *listconstant = eph->listconstant;

    while (listconstant != NULL)
    {
        struct TXTPCKconstant *pcst = listconstant;

        struct TXTPCKvalue *listvalue = listconstant->value;

        free(listconstant->name);

        while (listvalue != NULL)
        {
            struct TXTPCKvalue *pvalue = listvalue;

            listvalue = listvalue->next;
            free(pvalue);
        }
        listconstant = listconstant->next;
        free(pcst);
    }

    free(eph->buffer);
    if (eph->file)
    {
        fclose(eph->file);
        eph->file = NULL;
    }
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pdval.
  @param eph (inout) ephemeris file
  @param name (in) constant name
  @param p_pdval (out) value of the constant
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstant(struct TXTPCKfile *eph, const char *name, double *p_pdval)
{
    struct TXTPCKconstant *listconstant;

    int found = 0;

    for (listconstant = eph->listconstant; listconstant != NULL && found == 0; listconstant = listconstant->next)
    {
        if (strcmp(listconstant->name, name) == 0)
        {
            struct TXTPCKvalue *listvalue;

            for (listvalue = listconstant->value; listvalue != NULL && found == 0; listvalue = listvalue->next)
            {
                if (listvalue->buffer[listvalue->locfirst] != '\'')
                {
                    char *perr;

                    *p_pdval = strtod(listvalue->buffer + listvalue->locfirst, &perr);
                    found = (((off_t) (perr - listvalue->buffer)) <= listvalue->loclast);
                    /* printf("dvalue=%g found=%d %d %d\n", *p_pdval, found, (int)(perr-eph->buffer), (int)listvalue->loclast); */
                }
            }
        }
    }
    return found;
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pival.
  @param listconstant (in) list of constants
  @param p_pival (out) value of the constant
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstant_int2(struct TXTPCKconstant *listconstant, int *p_pival)
{
    int found = 0;

    struct TXTPCKvalue *listvalue;

    for (listvalue = listconstant->value; listvalue != NULL && found == 0; listvalue = listvalue->next)
    {
        if (listvalue->buffer[listvalue->locfirst] != '\'')
        {
            char *perr;

            long l = strtol(listvalue->buffer + listvalue->locfirst, &perr, 10);

            *p_pival = (int) l;
            found = (((off_t) (perr - listvalue->buffer)) <= listvalue->loclast);
            /* printf("dvalue=%d found=%d %d %d\n", *p_pival, found, (int)(perr-eph->buffer), (int)listvalue->loclast); */
        }
    }
    return found;
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pival.
  @param eph (inout) ephemeris file
  @param name (in) constant name
  @param p_pival (out) value of the constant
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstant_int(struct TXTPCKfile *eph, const char *name, int *p_pival)
{
    struct TXTPCKconstant *listconstant;

    int found = 0;

    for (listconstant = eph->listconstant; listconstant != NULL && found == 0; listconstant = listconstant->next)
    {
        if (strcmp(listconstant->name, name) == 0)
        {
            found = calceph_txtpck_getconstant_int2(listconstant, p_pival);
        }
    }
    return found;
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pdval.
  @param eph (inout) ephemeris file
  @param n1 (in) 1st dimension of the matrix
  @param n2 (in) 2nd dimension of the matrix
  @param name (in) constant name
  @param p_pival (out) matrix n1xn2 : value of the constant
*/
/*--------------------------------------------------------------------------*/
static int calceph_txtpck_getconstant_intmatrixnxn(struct TXTPCKfile *eph, const char *name, int n1, int n2,
                                                   int *p_pival)
{
    int j;

    int n = n1 * n2;

    struct TXTPCKconstant *ptr;

    ptr = calceph_txtpck_getptrconstant(eph, name);
    if (ptr)
    {
        int found = 1;

        struct TXTPCKvalue *listvalue;

        j = 0;
        for (listvalue = ptr->value; listvalue != NULL && found == 1 && j < n; listvalue = listvalue->next, j++)
        {
            if (listvalue->buffer[listvalue->locfirst] != '\'')
            {
                char *perr;

                long l = strtol(listvalue->buffer + listvalue->locfirst, &perr, 10);

                p_pival[j] = (int) l;

                found = (isspace(*perr) || *perr == ',') &&
                    (((off_t) (perr - listvalue->buffer)) <= listvalue->loclast);
            }
            else
            {
                found = 0;
            }
        }
        if (j != n)
            found = 0;
        return found;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pdval.
  @param eph (inout) ephemeris file
  @param name (in) constant name
  @param p_pival (out) matrix 1x3 : value of the constant
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstant_intmatrix1x3(struct TXTPCKfile *eph, const char *name, int *p_pival)
{
    return calceph_txtpck_getconstant_intmatrixnxn(eph, name, 1, 3, p_pival);
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pdval.
  @param eph (inout) ephemeris file
  @param n1 (in) 1st dimension of the matrix
  @param n2 (in) 2nd dimension of the matrix
  @param name (in) constant name
  @param p_pdval (out) matrix n1xn2 : value of the constant
*/
/*--------------------------------------------------------------------------*/
static int calceph_txtpck_getconstant_matrixnxn(struct TXTPCKfile *eph, const char *name, int n1, int n2,
                                                double *p_pdval)
{
    int j;

    int n = n1 * n2;

    struct TXTPCKconstant *ptr;

    ptr = calceph_txtpck_getptrconstant(eph, name);
    if (ptr)
    {
        int found = 1;

        struct TXTPCKvalue *listvalue;

        j = 0;
        for (listvalue = ptr->value; listvalue != NULL && found == 1 && j < n; listvalue = listvalue->next, j++)
        {
            if (listvalue->buffer[listvalue->locfirst] != '\'')
            {
                char *perr;

                p_pdval[j] = strtod(listvalue->buffer + listvalue->locfirst, &perr);
                found = (isspace(*perr) || *perr == ',') &&
                    (((off_t) (perr - listvalue->buffer)) <= listvalue->loclast);
            }
            else
            {
                found = 0;
            }
        }
        if (j != n)
            found = 0;
        return found;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pdval.
  @param eph (inout) ephemeris file
  @param name (in) constant name
  @param p_pdval (out) matrix 3x3 : value of the constant
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstant_matrix3x3(struct TXTPCKfile *eph, const char *name, double *p_pdval)
{
    return calceph_txtpck_getconstant_matrixnxn(eph, name, 3, 3, p_pdval);
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pdval.
  @param eph (inout) ephemeris file
  @param name (in) constant name
  @param p_pdval (out) matrix 1x3 : value of the constant
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstant_matrix1x3(struct TXTPCKfile *eph, const char *name, double *p_pdval)
{
    return calceph_txtpck_getconstant_matrixnxn(eph, name, 1, 3, p_pdval);
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  the value must be a string
  return 0 if constant is not found
  return 1 if constant is found and set p_ptxtpckvalue.
  @param eph (inout) ephemeris file
  @param name (in) constant name
  @param p_ptxtpckvalue (out) value of the constant
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstant_txtpckvalue(struct TXTPCKfile *eph, const char *name,
                                           struct TXTPCKvalue **p_ptxtpckvalue)
{
    struct TXTPCKconstant *listconstant;

    int found = 0;

    *p_ptxtpckvalue = NULL;

    for (listconstant = eph->listconstant; listconstant != NULL && found == 0; listconstant = listconstant->next)
    {
        if (strcmp(listconstant->name, name) == 0)
        {
            struct TXTPCKvalue *listvalue;

            for (listvalue = listconstant->value; listvalue != NULL && found == 0; listvalue = listvalue->next)
            {
                if (listvalue->buffer[listvalue->locfirst] == '\'')
                {
                    *p_ptxtpckvalue = listvalue;
                    found = 1;
                    /* printf("dvalue=%g found=%d %d %d\n", *p_pdval, found, (int)(perr-listvalue->buffer), (int)listvalue->loclast); */
                }
            }
        }
    }
    return found;
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified constant from the text PCK kernel 
  return NULL if constant is not found
  return the address of the constant if constant is found.
  
  @param eph (inout) ephemeris file
  @param name (in) constant name
*/
/*--------------------------------------------------------------------------*/
struct TXTPCKconstant *calceph_txtpck_getptrconstant(struct TXTPCKfile *eph, const char *name)
{
    struct TXTPCKconstant *foundconstant = NULL;

    struct TXTPCKconstant *listconstant;

    for (listconstant = eph->listconstant; listconstant != NULL && foundconstant == NULL;
         listconstant = listconstant->next)
    {
        if (strcmp(listconstant->name, name) == 0)
        {
            foundconstant = listconstant;
        }
    }
    return foundconstant;
}

/*--------------------------------------------------------------------------*/
/*! return the number of constants available in the ephemeris file 
  
  @param eph (inout) ephemeris descriptor
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstantcount(struct TXTPCKfile *eph)
{
    int res = 0;

    struct TXTPCKconstant *listconstant;

    for (listconstant = eph->listconstant; listconstant != NULL; listconstant = listconstant->next)
    {
        res++;
    }
    return res;
}

/*--------------------------------------------------------------------------*/
/*! get the name and its value of the constant available 
    at some index from the ephemeris file 
   return 1 if index is valid
   return 0 if index isn't valid
  
  @param eph (inout) ephemeris descriptor
  @param index (in) index of the constant (must be between 1 and calceph_getconstantcount() )
   if index > number of constant in this file => decrement it
  @param name (out) name of the constant 
  @param value (out) value of the constant 
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getconstantindex(struct TXTPCKfile *eph, int *index, char name[CALCEPH_MAX_CONSTANTNAME],
                                    double *value)
{
    struct TXTPCKconstant *listconstant = eph->listconstant;

    int res = 0;

    while (listconstant != NULL && *index > 1)
    {
        *index = *index - 1;
        listconstant = listconstant->next;
    }

    if (listconstant != NULL && *index == 1)
    {
        struct TXTPCKvalue *listvalue = listconstant->value;

        strcpy(name, listconstant->name);
        if (listvalue->buffer[listvalue->locfirst] != '\'')
        {
            char *perr;

            *value = strtod(listvalue->buffer + listvalue->locfirst, &perr);
            res = (((off_t) (perr - listvalue->buffer)) <= listvalue->loclast);
            /*printf("dvalue=%g found=%d %d %d\n", *value, res, (int)(perr-eph->buffer), (int)listvalue->loclast); */
        }
    }
    return res;
}

/*--------------------------------------------------------------------------*/
/*! return 0 if the value1==value2
    return -1 or +1 if not the same
   @param value1 (in) first value
   @param value2 (in) second value
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_cmpvalue(struct TXTPCKvalue *value1, struct TXTPCKvalue *value2)
{
    off_t loc1 = value1->locfirst;

    off_t loc2 = value2->locfirst;

    while (loc1 <= value1->loclast && loc2 <= value2->loclast)
    {
        if (value1->buffer[loc1] != value2->buffer[loc2])
            return 1;
        loc1++;
        loc2++;
    }

    return (loc1 > value1->loclast && loc2 > value2->loclast) ? 0 : 1;
}

/*--------------------------------------------------------------------------*/
/*! return 0 if the value1==value2
    return -1 or +1 if not the same
   @param value1 (in) first value
   @param value2 (in) second value
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_cmpszvalue(struct TXTPCKvalue *value1, const char *value2)
{
    off_t loc1 = value1->locfirst;

    while (loc1 <= value1->loclast && *value2 != '\0')
    {
        if (value1->buffer[loc1] != *value2)
            return 1;
        loc1++;
        value2++;
    }
    return (loc1 >= value1->loclast && *value2 == '\0') ? 0 : 1;
}

/*--------------------------------------------------------------------------*/
/*! return the value of the specified value from the text PCK kernel 
  return 0 if constant is not found
  return 1 if constant is found and set p_pival.
  @param listvalue (in) list of values
  @param p_pszval (inout) string of the value. 
  on enter, the array must have enough room.
*/
/*--------------------------------------------------------------------------*/
int calceph_txtpck_getvalue_char(struct TXTPCKvalue *listvalue, char *p_pszval)
{
    int found = 0;

    off_t l, j;

    p_pszval[0] = '\0';
    if (listvalue->buffer[listvalue->locfirst] == '\'')
    {
        j = 0;
        for (l = listvalue->locfirst + 1; l < listvalue->loclast - 1; l++)
        {
            p_pszval[j++] = listvalue->buffer[l];
        }
        p_pszval[j] = '\0';
        found = 1;
    }
    return found;
}
