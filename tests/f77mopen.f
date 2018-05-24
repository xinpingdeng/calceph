!/*-----------------------------------------------------------------*/
!/*! 
!  \file f77mopen.f 
!  \brief Check if calceph_open works with fortran 77 compiler.
!
!  \author  M. Gastineau 
!           Astronomie et Systemes Dynamiques, IMCCE, CNRS, Observatoire de Paris. 
!
!   Copyright, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, CNRS
!   email of the author : Mickael.Gastineau@obspm.fr
!
!*/
!/*-----------------------------------------------------------------*/

!/*-----------------------------------------------------------------*/
!/* License  of this file :
!  This file is "triple-licensed", you have to choose one  of the three licenses 
!  below to apply on this file.
!  
!     CeCILL-C
!     	The CeCILL-C license is close to the GNU LGPL.
!     	( http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html )
!  
!  or CeCILL-B
!       The CeCILL-B license is close to the BSD.
!       ( http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.txt)
!  
!  or CeCILL v2.1
!       The CeCILL license is compatible with the GNU GPL.
!       ( http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.html )
!  
! 
! This library is governed by the CeCILL-C, CeCILL-B or the CeCILL license under 
! French law and abiding by the rules of distribution of free software.  
! You can  use, modify and/ or redistribute the software under the terms 
! of the CeCILL-C,CeCILL-B or CeCILL license as circulated by CEA, CNRS and INRIA  
! at the following URL "http://www.cecill.info". 
!
! As a counterpart to the access to the source code and  rights to copy,
! modify and redistribute granted by the license, users are provided only
! with a limited warranty  and the software's author,  the holder of the
! economic rights,  and the successive licensors  have only  limited
! liability. 
!
! In this respect, the user's attention is drawn to the risks associated
! with loading,  using,  modifying and/or developing or reproducing the
! software by the user in light of its specific status of free software,
! that may mean  that it is complicated to manipulate,  and  that  also
! therefore means  that it is reserved for developers  and  experienced
! professionals having in-depth computer knowledge. Users are therefore
! encouraged to load and test the software's suitability as regards their
! requirements in conditions enabling the security of their systems and/or 
! data to be ensured and,  more generally, to use and operate it in the 
! same conditions as regards security. 
!
! The fact that you are presently reading this means that you have had
! knowledge of the CeCILL-C,CeCILL-B or CeCILL license and that you accept its terms.
!*/
!/*-----------------------------------------------------------------*/

!/*-----------------------------------------------------------------*/
!/* main program */
!/*-----------------------------------------------------------------*/
       program f77multiple
           implicit none
           include 'f90calceph.h'
           integer*8 peph
           integer res
           character*80 filear(2)
           include 'fopenfiles.h'
           
! open the ephemeris file 
           res = f90calceph_open(peph, trim(TOPSRCDIR)                     &
     &      //"../examples/example1.dat")
           if (res.ne.1) then
            write(*,*) "failure on f90calceph_open - dat"
            stop 2
           endif

! open the SPK ephemeris file
           res = f90calceph_open(peph, trim(TOPSRCDIR)                     &
     &      //"../examples/example1.bsp")
           if (res.ne.1) then
            write(*,*) "failure on f90calceph_open - bsp"
            stop 2
           endif

! open the txt PCK ephemeris file 
           res = f90calceph_open(peph, trim(TOPSRCDIR)                     &
     &      //"../examples/example1.tpc")
           if (res.ne.1) then
            write(*,*) "failure on f90calceph_open - tpc"
            stop 2
           endif
          
! open the SPK and txt PCK ephemeris file 
           filear(1) = trim(TOPSRCDIR)//"../examples/example1.bsp"
           filear(2) = trim(TOPSRCDIR)//"../examples/example1.tpc"
           res = f90calceph_open_array(peph, 2, filear, 80)
           if (res.ne.1) then
            write(*,*) "failure on f90calceph_open_array"
            stop 2
           endif

           stop 
       end
      