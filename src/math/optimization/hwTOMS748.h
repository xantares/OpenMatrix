/**
* @file hwTOMS748.h
* @date October 2016
* Copyright (C) 2016-2018 Altair Engineering, Inc.  
* This file is part of the OpenMatrix Language ("OpenMatrix") software.
* Open Source License Information:
* OpenMatrix is free software. You can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* OpenMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.
* You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
* 
* Commercial License Information: 
* For a copy of the commercial license terms and conditions, contact the Altair Legal Department at Legal@altair.com and in the subject line, use the following wording: Request for Commercial License Terms for OpenMatrix.
* Altair's dual-license business model allows companies, individuals, and organizations to create proprietary derivative works of OpenMatrix and distribute them - whether embedded or bundled with other software - under a commercial license agreement.
* Use of Altair's trademarks and logos is subject to Altair's trademark licensing policies.  To request a copy, email Legal@altair.com and in the subject line, enter: Request copy of trademark and logo usage policy.
*/
#ifndef _TOMS748_Funcs_h
#define _TOMS748_Funcs_h

#include "hwOptimizationExports.h"

template <typename T> class hwTComplex;
template <typename T1, typename T2> class hwTMatrix;
typedef hwTMatrix<double, hwTComplex<double> > hwMatrix;
class hwMathStatus;
typedef hwMathStatus (*TOMS748Func)(double x, double& min);

//------------------------------------------------------------------------------
//!
//! \brief Algorithm 748 from Transactions on Mathematical Software
//!
//------------------------------------------------------------------------------

//!
//! Finds zero of a function without derivates using TOMS 748 algorithm
//! \param f           Function which gets initial function values f(a), f(b)
//! \param a
//! \param b
//! \param fa          f(a)
//! \param fb          f(b)
//! \param root        Output solution of this equation
//! \param froot 
//! \param eps         Used to determine the termination criterion
//! \param numIters    Number of iterations
//! \param numFunEvals
//!
hwMathStatus TOMS748(const TOMS748Func f, 
                     double&           a, 
                     double&           b, 
                     double&           fa, 
                     double&           fb,
                     double&           root, 
                     double&           froot, 
                     int&              numIters, 
                     int&              numFunEvals,
                     double            eps, 
                     hwMatrix*         objHist    = nullptr,
                     hwMatrix*         designHist = nullptr);

//!
//! Indicates sign of the input, 1 if positive, -1 if negative and 0 otherwise
//! \param x
//!
int ISIGN(double x);
//!
//! Determines the termination criterion
//! \param b
//! \param neps
//! \param eps
//! \param tol  Output as termination criterion
//!
void tole(double  b, 
          int     neps, 
          double  eps, 
          double& tol);
//!
//! Gets a shrinked enclosing interval and updates termination criterion
//! \param f
//! \param a    Enclosing interval [a, b]
//! \param b    Enclosing interval [a, b]
//! \param c    Number c is within [a, b], used to get new enclosing interval
//! \param fa   f(a)
//! \param fb   f(b)
//! \param tol  Termination criterion
//! \param neps Used to determine termination criterion
//! \param eps
//! \param d    Output
//! \param fd   f(d)
//!
void Bracket(const TOMS748Func f, 
             double&           a, 
             double&           b, 
             double            c, 
             double&           fa, 
             double&           fb,
             double&           tol, 
             int               neps,
             double            eps, 
             double&           d,
             double&           fd);
//!
//! Uses Newton steps to approximate 0 in (A,B) of the quadratic polynomial
//! interpolating f(x) at a, b and d.
//! \param a
//! \param b
//! \param d Lies outside the interval [a, b]
//! \param fa f(a)
//! \param fb f(b)
//! \param fd f(d)
//! \param c  Output as the approximate zero in (a,b) of the quadratic polynomial
//! \param k  Number of Newton steps to take
//!
void NewtonQuad(double  a, 
                double  b, 
                double  d, 
                double  fa, 
                double  fb, 
                double  fd,
                double& c, 
                int     k);
//!
//! Uses cubic inverse interpolation of f(x) at a, b, d and e to get approximate
//! root of f(x)
//! \param a
//! \param b
//! \param d
//! \param e
//! \param fa f(a)
//! \param fb f(b)
//! \param fd f(d)
//! \param fe f(e)
//! \param c  Output
//!
void Pzero(double  a, 
           double  b, 
           double  d, 
           double  e, 
           double  fa, 
           double  fb, 
           double  fd,
           double  fe, 
           double& c);

#endif // _TOMS748_Funcs_h
