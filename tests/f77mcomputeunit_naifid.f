!/*-----------------------------------------------------------------*/
!/*! 
!  \file f77mcomputeunit_naifid.f 
!  \brief Check the output of the function calceph_compute_unit if CALCPEH_USE_NAIFID is set.
!
!  \author  M. Gastineau 
!           Astronomie et Systemes Dynamiques, IMCCE, CNRS, Observatoire de Paris. 
!
!   Copyright, 2014, 2015, 2016, 2017, CNRS
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
!   or CeCILL-B
!       The CeCILL-B license is close to the BSD.
!       (http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.txt)
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
! check the array PV and PVcheck
!/*-----------------------------------------------------------------*/
      subroutine  checkoutput(n, PV, PVcheck,seuil,jd0, dt,center,        &
     &                       cible1,res, unit)
           implicit none
           integer n, center,cible1
           integer res
           double precision PV(6), PVcheck(6)
           double precision seuil, jd0, dt
           character(*)  unit
           integer j

               do j=1,n
                 if (abs(PV(j)-PVcheck(j))>=seuil) then
                  write(*,*) "failure for ",cible1, center
                  write(*,*) "at time ", jd0, dt
                  write(*,*) "unit=",unit
                  write(*,*) "diff=",PVcheck(j)-PV(j),PVcheck(j),PV(j)
                  write(*,*) PVcheck
                  write(*,*) PV
                  res = 2               
                 endif
                enddo
             
           return
           end 
                

!/*-----------------------------------------------------------------*/
!/* main program */
!/*-----------------------------------------------------------------*/
       program f77mcomputeunit_naifid
           implicit none
           include 'f90calceph.h'
           integer res, res2
           integer n, cible1, center, status, j
           integer fileref, filenaifid, unit
           integer ciblenaifid, centernaifid
           DATA fileref /12/
           DATA filenaifid /13/
           double precision seuil
           double precision jd0
           double precision  dt, AU
           double precision PV(6), PVcheck(6), PVunused(6)
           integer*8 :: peph
           include 'fopenfiles.h'
           
           res = 0
           
! open refernce data file
           open(fileref, file=trim(TOPSRCDIR)                            &
     &      //"example1_tests.dat",status="old")
           open(filenaifid, file=trim(TOPSRCDIR)                         &
     &      //"example1_tests_naifid.dat",                               &
     &          status="old")
           
! open the ephemeris file 
           res2=f90calceph_open(peph,trim(TOPSRCDIR)                     &
     &      //"../examples/example1.dat")
           if (res2.eq.1) then
             
! read the AU
               res2 = f90calceph_getconstant(peph, "AU", AU);
               if (res2.eq.0) then
                res = 2
               endif

             status = 0
             do while((status.eq.0).and.(res.eq.0))
! read the reference filejd0, cible1, center, 
              read(fileref, *,iostat=status) jd0, cible1, center,        &
     &              (PVcheck(j),j=1,6)
              if (status.eq.0) then
              
               read(filenaifid, *,iostat=status) jd0, ciblenaifid,       &
     &              centernaifid, (PVunused(j),j=1,6)

              dt = jd0-int(jd0)
               jd0 = int(jd0)+2.4515450000000000000D+06
! compute  the coordinates 
               res2=f90calceph_compute(peph,jd0, dt, cible1, center,PV)
               if (res2.eq.0) then
                res = 2
               endif
! check the coordinates               
               seuil=3D-15
               n=6
               if ((cible1.ne.15).and.(cible1.ne.16)) then
                
!  check UA/DAY 
                unit = CALCEPH_UNIT_AU+CALCEPH_UNIT_DAY+                 &
     &                 CALCEPH_USE_NAIFID
                do j=1, 6
                 PVcheck(j)=0
                enddo
                res2 = f90calceph_compute_unit(peph,jd0, dt,              &
     &               ciblenaifid, centernaifid, unit, PVcheck)
               if (res2.eq.0) then
                res = 2
               endif
                call checkoutput(n,PV,PVcheck,seuil,jd0, dt,center,      &
     &               cible1,res,"AU/DAY")
                
!  check UA/SEC 
                unit = CALCEPH_UNIT_AU+CALCEPH_UNIT_SEC+                 &
     &                 CALCEPH_USE_NAIFID
                do j=1, 6
                 PVcheck(j)=0
                enddo
                res2 = f90calceph_compute_unit(peph,jd0, dt,               &
     &               ciblenaifid, centernaifid, unit, PVcheck)
               if (res2.eq.0) then
                res = 2
               endif
                do j=4, 6
                 PVcheck(j) = PVcheck(j)*86400D0
                enddo 
                call checkoutput(n,PV,PVcheck,seuil,jd0, dt,center,      &
     &               cible1,res,"AU/SEC")
                
               seuil=3D-14
!  check KM/DAY 
                unit = CALCEPH_UNIT_KM+CALCEPH_UNIT_DAY+                 &
     &                 CALCEPH_USE_NAIFID
                do j=1, 6
                 PVcheck(j)=0
                enddo
                res2 = f90calceph_compute_unit(peph,jd0, dt,               &
     &               ciblenaifid, centernaifid, unit, PVcheck)
               if (res2.eq.0) then
                res = 2
               endif
                do j=1, 6
                 PVcheck(j) = PVcheck(j)/AU
                enddo 
                call checkoutput(n,PV,PVcheck,seuil,jd0, dt,center,      &
     &               cible1,res,"KM/DAY")
                
!  check KM/SEC 
                unit = CALCEPH_UNIT_KM+CALCEPH_UNIT_SEC+                 &
     &                 CALCEPH_USE_NAIFID
                do j=1, 6
                 PVcheck(j)=0
                enddo
                res2 = f90calceph_compute_unit(peph,jd0, dt,              &
     &               ciblenaifid, centernaifid, unit, PVcheck)
               if (res2.eq.0) then
                res = 2
               endif
                do j=1, 6
                 PVcheck(j) = PVcheck(j)/AU
                enddo 
                do j=4, 6
                 PVcheck(j) = PVcheck(j)*86400D0
                enddo 
                call checkoutput(n,PV,PVcheck,seuil,jd0, dt,center,      &
     &               cible1,res,"KM/SEC")
              endif
              
               !Librations
               if (cible1.eq.15) then
 !  check RAD/DAY 
                unit = CALCEPH_UNIT_RAD+CALCEPH_UNIT_DAY+                 &
     &                 CALCEPH_USE_NAIFID
                do j=1, 6
                 PVcheck(j)=0
                enddo
                res2 = f90calceph_orient_unit(peph,jd0, dt,               &
     &               NAIFID_MOON, unit, PVcheck)
               if (res2.eq.0) then
                res = 2
               endif
                call checkoutput(n,PV,PVcheck,seuil,jd0, dt,center,      &
     &               cible1,res,"RAD/DAY")
                
!  check RAD/SEC 
                unit = CALCEPH_UNIT_RAD+CALCEPH_UNIT_SEC+                 &
     &                 CALCEPH_USE_NAIFID
                do j=1, 6
                 PVcheck(j)=0
                enddo
                res2 = f90calceph_orient_unit(peph,jd0, dt,               &
     &               NAIFID_MOON, unit, PVcheck)
               if (res2.eq.0) then
                res = 2
               endif
                do j=4, 6
                 PVcheck(j) = PVcheck(j)*86400D0
                enddo 
                call checkoutput(n,PV,PVcheck,seuil,jd0, dt,center,      &
     &               cible1,res,"RAD/SEC")
              endif

               !TT-TDB
               if (cible1.eq.16) then
 !  check RAD/DAY 
                unit = CALCEPH_UNIT_SEC+CALCEPH_USE_NAIFID
                do j=1, 6
                 PVcheck(j)=0
                enddo
                res2 = f90calceph_compute_unit(peph,jd0, dt,               &
     &               ciblenaifid, centernaifid, unit, PVcheck)
               if (res2.eq.0) then
                res = 2
               endif
                call checkoutput(n,PV,PVcheck,seuil,jd0, dt,center,      &
     &               cible1,res,"SEC")
              endif

              endif
             enddo

! close the ephemeris file 
             call f90calceph_close(peph)
            else
               write (*,*) "The ephemeris can't be opened"
               res = 2
            endif
       close(fileref)
       if (res.eq.0) then
        stop
       else
        stop 2
       endif
       end
      
