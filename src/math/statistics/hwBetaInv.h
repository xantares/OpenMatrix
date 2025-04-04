/**
* @file hwBetaInv.h
* @date June 2007
* Copyright (C) 2007-2018 Altair Engineering, Inc.  
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
#ifndef _Stats_hwBetaInv_h
#define _Stats_hwBetaInv_h

#include "hwDistribution.h"

class hwMersenneTwisterState;   // forward declaration
class hwBeta;                   // forward declaration

//------------------------------------------------------------------------------
//!
//! \class hwBetaInv
//! \brief Inverse beta distribution class
//!
//------------------------------------------------------------------------------
class hwBetaInv : public hwDistribution
{
public:
    //!
    //! Constructor
    //! \param alpha
    //! \param beta
    //! \param pMTState
    //!
    hwBetaInv(double                  alpha, 
              double                  beta, 
              hwMersenneTwisterState* pMTState = nullptr);
    //!
    //! Destructor
    //!
    virtual ~hwBetaInv();

    //!
    //! Returns probability density function
    //! \param x Domain value
    //!
    double Pdf(double x);
    //!
    //! Returns cumulative density function
    //! \param x Domain value
    //!
    double Cdf(double x);
    //!
    //! Returns inverse cumulative density function
    //! \param prob Probability value
    //!
    double CdfInv(double prob);
    //!
    //! Returns distribution mean
    //!
    double Mean();
    //!
    //! Returns distribution variance
    //!
    double Variance();
    //!
    //! Returns distribution mode
    //!
    double Mode();
    //!
    //! Returns random value
    //!
    double GetDeviate();

private:
    double  m_alpha; //!< shape parameter
    double  m_beta;  //!< shape parameter
    hwBeta* m_pBeta; //!< pointer to beta distribution
};

#endif // _Stats_hwBetaInv_h
