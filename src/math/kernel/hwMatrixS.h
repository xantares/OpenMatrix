/**
* @file hwMatrixS.h
* @date May 2019
* Copyright (C) 2014-2019 Altair Engineering, Inc.
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

//:-----------------------------------------------------------------
//:Description
//
//  hwTMatrixS include file for matrices with explicit <double>
//  specialization and MKL dependency. When including this header
//  the client must link to MKL.
//
//:-----------------------------------------------------------------
#ifndef _hwMatrixS_h
#define _hwMatrixS_h

#include "tmpl/hwTMatrixS.h"      // class definition
#include "utl/hwTMatrixSUtil.cc"  // MKL dependent implementation file

typedef hwTMatrixS<double> hwMatrixS;
typedef hwTMatrixS<float> hwMatrixSF;
typedef hwTMatrixS<int> hwMatrixSI;

#endif // _hwMatrixS_h
