/**
* @file SignalsTboxFuncs.cxx
* @date January 2015
* Copyright (C) 2015-2018 Altair Engineering, Inc.  
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

#include "SignalsTboxFuncs.h"

#include <memory>  // For std::unique_ptr

#include "hwMatrix.h"
#include "hwMatrixN.h"
#include "FourierFuncs.h"
#include "FilterFuncs.h"
#include "WaveformFuncs.h"

#include "BuiltInFuncs.h"
#include "BuiltInFuncsMKL.h"
#include "BuiltInFuncsUtils.h"
#include "MKLutilities.h"
#include "StructData.h"
#include "MatrixNUtils.h"
#include "OML_Error.h"

#define SIGN "SignalProcessing"
#define TBOXVERSION 2019.0

//------------------------------------------------------------------------------
// Entry point which registers oml Signals functions with oml
//------------------------------------------------------------------------------
int InitDll(EvaluatorInterface eval)
{
    eval.RegisterBuiltInFunction("ifft",        OmlIfft,       FunctionMetaData(-2, 1, SIGN), false);
    eval.RegisterBuiltInFunction("fft",         OmlFft,        FunctionMetaData(-2, 1, SIGN), false);
    eval.RegisterBuiltInFunction("fft2",        OmlFft2,       FunctionMetaData(-2, 1, SIGN), false);
    eval.RegisterBuiltInFunction("ifft2",       OmlIfft2,      FunctionMetaData(-2, 1, SIGN), false);
    eval.RegisterBuiltInFunction("fftn",        OmlFftN,       FunctionMetaData(-2, 1, SIGN), false);
    eval.RegisterBuiltInFunction("ifftn",       OmlIfftN,      FunctionMetaData(-2, 1, SIGN), false);
    eval.RegisterBuiltInFunction("stft",        OmlStft,       FunctionMetaData(-2, -2, SIGN), false);
    eval.RegisterBuiltInFunction("istft",       OmlIstft,      FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("spectrogram", OmlSpectrogram,FunctionMetaData(-2, -2, SIGN), false);
    eval.RegisterBuiltInFunction("fftshift",    OmlFftShift,   FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("ifftshift",   OmlIFftShift,  FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("freq",        OmlFreq,       FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("fold",        OmlFold,       FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("xcorr",       OmlXcorr,      FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("unwrap",      OmlUnwrap,     FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("pwelch",      OmlPwelch,     FunctionMetaData(-2, 2, SIGN), false);
    eval.RegisterBuiltInFunction("cpsd",        OmlCpsd,       FunctionMetaData(-3, 2, SIGN));
    eval.RegisterBuiltInFunction("tfestimate",  OmlTFestimate, FunctionMetaData(-3, 2, SIGN), false);
    eval.RegisterBuiltInFunction("mscohere",    OmlMScohere,   FunctionMetaData(-3, 2, SIGN), false);
    eval.RegisterBuiltInFunction("freqz",       OmlFreqz,      FunctionMetaData(4, 2, SIGN));
    eval.RegisterBuiltInFunction("freqs",       OmlFreqs,      FunctionMetaData(3, 2, SIGN));
    eval.RegisterBuiltInFunction("invfreqz",    OmlInvFreqz,   FunctionMetaData(-5, 2, SIGN));
    eval.RegisterBuiltInFunction("invfreqs",    OmlInvFreqs,   FunctionMetaData(-5, 2, SIGN));
    eval.RegisterBuiltInFunction("impz",        OmlImpz,       FunctionMetaData(-3, 2, SIGN));
    eval.RegisterBuiltInFunction("fir1",        OmlFir1,       FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("firls",       OmlFirls,      FunctionMetaData(-4, 1, SIGN));
    eval.RegisterBuiltInFunction("besself",     OmlBesself,    FunctionMetaData(-3, 2, SIGN));
    eval.RegisterBuiltInFunction("besself3",    OmlBesself3,   FunctionMetaData(-3, 2, SIGN));
    eval.RegisterBuiltInFunction("butter",      OmlButter,     FunctionMetaData(-3, 2, SIGN));
    eval.RegisterBuiltInFunction("cheby1",      OmlCheby1,     FunctionMetaData(-4, 2, SIGN));
    eval.RegisterBuiltInFunction("cheby2",      OmlCheby2,     FunctionMetaData(-4, 2, SIGN));
    eval.RegisterBuiltInFunction("ellip",       OmlEllip,      FunctionMetaData(-5, 2, SIGN));
    eval.RegisterBuiltInFunction("buttord",     OmlButtord,    FunctionMetaData(-5, 2, SIGN));
    eval.RegisterBuiltInFunction("cheb1ord",    OmlCheb1ord,   FunctionMetaData(-5, 2, SIGN));
    eval.RegisterBuiltInFunction("cheb2ord",    OmlCheb2ord,   FunctionMetaData(-5, 2, SIGN));
    eval.RegisterBuiltInFunction("ellipord",    OmlEllipord,   FunctionMetaData(-5, 2, SIGN));
    eval.RegisterBuiltInFunction("blackman",    OmlBlackman,   FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("barthannwin", OmlBartHann,   FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("hann",        OmlHann,       FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("hamming",     OmlHamming,    FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("parzenwin",   OmlParzenwin,  FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("welchwin",    OmlWelchwin,   FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("chebwin",     OmlChebwin,    FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("kaiser",      OmlKaiser,     FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("filter",      OmlFilter,     FunctionMetaData(-4, 1, SIGN));
    eval.RegisterBuiltInFunction("filtfilt",    OmlFiltfilt,   FunctionMetaData(3, 1, SIGN));
    eval.RegisterBuiltInFunction("sinc",        OmlSinc,       FunctionMetaData(1, 1, SIGN));
    eval.RegisterBuiltInFunction("upsample",    OmlUpsample,   FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("downsample",  OmlDownsample, FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("resample",    OmlResample,   FunctionMetaData(-4, 1, SIGN), false);
    eval.RegisterBuiltInFunction("findpeaks",   OmlFindPeaks,  FunctionMetaData(-2, 3, SIGN));
    eval.RegisterBuiltInFunction("dba",         OmldbA,        FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("dbb",         OmldbB,        FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("dbc",         OmldbC,        FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("dbu",         OmldbU,        FunctionMetaData(-3, 1, SIGN));
    eval.RegisterBuiltInFunction("rectpuls",    OmlRectPuls,   FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("tripuls",     OmlTriPuls,    FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("gauspuls",    OmlGausPuls,   FunctionMetaData(-4, 1, SIGN));
    eval.RegisterBuiltInFunction("pulstran",    OmlPulsTran,   FunctionMetaData(-4, 1, SIGN));
    eval.RegisterBuiltInFunction("square",      OmlSquare,     FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("sawtooth",    OmlSawtooth,   FunctionMetaData(-2, 1, SIGN));
    eval.RegisterBuiltInFunction("diric",       OmlDiric,      FunctionMetaData(2, 1, SIGN));
    eval.RegisterBuiltInFunction("chirp",       OmlChirp,      FunctionMetaData(-3, 1, SIGN));

    return 1;
}
//------------------------------------------------------------------------------
// Inverse Fast Fourier transform
//------------------------------------------------------------------------------
bool OmlIfft(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // collect input info
    const Currency& input1 = inputs[0];

    if (!(input1.IsMatrix() || input1.IsNDMatrix() ||
        input1.IsScalar() || input1.IsComplex()))
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    int fftSize = -1;

    if (nargin > 1)
    {
        const Currency& input2 = inputs[1];

        if (input2.IsPositiveInteger())
        {
            fftSize = static_cast<int>(input2.Scalar());
        }
        else if (input2.IsMatrix())
        {
            const hwMatrix* matrix = input2.Matrix();

            if (!matrix->Is0x0())
                throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_VALUE);
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_VALUE);
        }
    }

    int dim = -1;

    if (nargin > 2)
    {
        const Currency& input3 = inputs[2];

        if (nargin == 3 && !input3.IsPositiveInteger())
        {
            throw OML_Error(OML_ERR_POSINTEGER, 3, OML_VAR_DIM);
        }

        dim = static_cast<int>(input3.Scalar());
    }

    if (dim == -1)
    {
        if (input1.IsNDMatrix())
        {
            const hwMatrixN* matrix = input1.MatrixN();
            const std::vector<int>& dims = matrix->Dimensions();

            // use first non-singleton dimension
            for (int i = 0; i < dims.size(); ++i)
            {
                if (dims[i] != 1)
                {
                    dim = i + 1;
                    break;
                }
            }
        }
        else
        {
            const hwMatrix* matrix = input1.ConvertToMatrix();

            if (matrix->M() == 1)
            {
                dim = 2;
            }
            else
            {
                dim = 1;
            }
        }
    }

    if (fftSize == -1)
    {
        if (input1.IsNDMatrix())
        {
            const hwMatrixN* matrix = input1.MatrixN();
            const std::vector<int>& dims = matrix->Dimensions();

            fftSize = dims[dim - 1];
        }
        else
        {
            const hwMatrix* matrix = input1.ConvertToMatrix();

            if (dim == 1)
            {
                fftSize = matrix->M();
            }
            else   // dim == 2
            {
                fftSize = matrix->N();
            }
        }
    }

    // perform fft ops
    if (!input1.IsNDMatrix())
    {
        const hwMatrix* matrix = input1.ConvertToMatrix();
        std::unique_ptr<hwMatrix> signal(EvaluatorInterface::allocateMatrix());

        if ((dim == 1 && matrix->N() < 2) || (dim == 2 && matrix->M() < 2))
        {
            if (matrix->IsReal())
            {
                hwMatrix input;
                hwMathStatus status = input.PackComplex(*matrix);
                BuiltInFuncsUtils::CheckMathStatus(eval, status);

                status = Ifft(input, *signal, fftSize);
                BuiltInFuncsUtils::CheckMathStatus(eval, status);
            }
            else
            {
                hwMathStatus status = Ifft(*matrix, *signal, fftSize);
                BuiltInFuncsUtils::CheckMathStatus(eval, status);
            }
        }
        else if (dim == 1)
        {
            signal.reset(EvaluatorInterface::allocateMatrix(
                fftSize, matrix->N(), true));

            std::unique_ptr<hwMatrix> colin(EvaluatorInterface::allocateMatrix());
            std::unique_ptr<hwMatrix> colout(EvaluatorInterface::allocateMatrix());

            for (int i = 0; i < matrix->N(); ++i)
            {
                hwMathStatus stat = matrix->ReadColumn(i, *colin);
                BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                if (colin->IsReal())
                {
                    stat = colin->MakeComplex();
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                }
                stat = Ifft(*colin, *colout, fftSize);
                BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                if (colout->N() != 1)
                {
                    colout->Transpose();
                }
                stat = signal->WriteColumn(i, *colout);
                BuiltInFuncsUtils::CheckMathStatus(eval, stat);
            }
        }
        else if (dim == 2)
        {
            if (fftSize == matrix->N())
            {
                hwMathStatus status = Ifft(*matrix, *signal, dim - 1, fftSize);
            }
            else
            {
                signal.reset(EvaluatorInterface::allocateMatrix(matrix->M(),
                             fftSize, true));
                std::unique_ptr<hwMatrix> rowin(EvaluatorInterface::allocateMatrix());
                std::unique_ptr<hwMatrix> rowout(EvaluatorInterface::allocateMatrix());

                for (int i = 0; i < matrix->M(); i++)
                {
                    hwMathStatus stat = matrix->ReadRow(i, *rowin);
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                    if (rowin->IsReal())
                    {
                        stat = rowin->MakeComplex();
                        BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                    }
                    stat = Ifft(*rowin, *rowout, fftSize);
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                    if (rowout->M() != 1)
                    {
                        rowout->Transpose();
                    }
                    stat = signal->WriteRow(i, *rowout);
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                }
            }
        }
        else
        {
            throw OML_Error(OML_ERR_UNSUPPORTDIM, 3);
        }

        outputs.push_back(signal.release());
    }
    else        // ND case
    {
        const hwMatrixN* matrix = input1.MatrixN();
        const std::vector<int>& dims = matrix->Dimensions();

        if (fftSize == dims[dim - 1])
        {
            hwMatrixN* signal = new hwMatrixN;

            hwMathStatus status = Ifft(*matrix, *signal, dim - 1, fftSize);
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
            outputs.push_back(signal);
        }
        else if (nargin != 3)
        {
            return oml_MatrixNUtil4(eval, inputs, outputs, OmlIfft);
        }
        else
        {
            return oml_MatrixNUtil4(eval, inputs, outputs, OmlIfft, 3);
        }
    }

    return true;
}
//------------------------------------------------------------------------------
// Fast Fourier transform
//------------------------------------------------------------------------------
bool OmlFft(EvaluatorInterface           eval, 
            const std::vector<Currency>& inputs, 
            std::vector<Currency>&       outputs)

{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // collect input info
    const Currency& input1 = inputs[0];

    if (!(input1.IsMatrix() || input1.IsNDMatrix() ||
          input1.IsScalar() || input1.IsComplex()))
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    int fftSize = -1;

    if (nargin > 1)
    {
        const Currency& input2 = inputs[1];

        if (input2.IsPositiveInteger())
        {
            fftSize = static_cast<int>(input2.Scalar());
        }
        else if (input2.IsMatrix())
        {
            const hwMatrix* matrix = input2.Matrix();

            if (!matrix->Is0x0())
                throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_VALUE);
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_VALUE);
        }
    }

    int dim = -1;

    if (nargin > 2)
    {
        const Currency& input3 = inputs[2];

        if (nargin == 3 && !input3.IsPositiveInteger())
        {
            throw OML_Error(OML_ERR_POSINTEGER, 3, OML_VAR_DIM);
        }

        dim = static_cast<int>(input3.Scalar());
    }

    if (dim == -1)
    {
        if (input1.IsNDMatrix())
        {
            const hwMatrixN* matrix = input1.MatrixN();
            const std::vector<int>& dims = matrix->Dimensions();

            // use first non-singleton dimension
            for (int i = 0; i < dims.size(); ++i)
            {
                if (dims[i] != 1)
                {
                    dim = i + 1;
                    break;
                }
            }
        }
        else
        {
            const hwMatrix* matrix = input1.ConvertToMatrix();

            if (matrix->M() == 1)
            {
                dim = 2;
            }
            else
            {
                dim = 1;
            }
        }
    }

    if (fftSize == -1)
    {
        if (input1.IsNDMatrix())
        {
            const hwMatrixN* matrix = input1.MatrixN();
            const std::vector<int>& dims = matrix->Dimensions();

            fftSize = dims[dim - 1];
        }
        else
        {
            const hwMatrix* matrix = input1.ConvertToMatrix();

            if (dim == 1)
            {
                fftSize = matrix->M();
            }
            else   // dim == 2
            {
                fftSize = matrix->N();
            }
        }
    }

    // perform fft ops
    if (!input1.IsNDMatrix())
    {
        const hwMatrix* matrix = input1.ConvertToMatrix();
        std::unique_ptr<hwMatrix> freq(EvaluatorInterface::allocateMatrix());

        if ((dim == 1 && matrix->N() < 2) || (dim == 2 && matrix->M() < 2))
        {
            hwMathStatus status = Fft(*matrix, *freq, fftSize);
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
        else if (dim == 1)
        {
            freq.reset(EvaluatorInterface::allocateMatrix(fftSize, matrix->N(), false));
            hwComplex* response = freq->GetComplexData();
            hwMathStatus status;

            if (matrix->IsReal())
            {
                if (fftSize == matrix->M())
                {
                    status = Fft(*matrix, *freq, dim - 1, fftSize);
                }
                else
                {
                    double* signal = const_cast<double*> (matrix->GetRealData());

                    for (int i = 0; i < matrix->N(); ++i)
                    {
                        hwMatrix col_in(matrix->M(), 1, reinterpret_cast<void*> (signal), hwMatrix::REAL);
                        hwMatrix col_out(fftSize, 1, reinterpret_cast<void*> (response), hwMatrix::COMPLEX);
                        status = Fft(col_in, col_out, fftSize);
                        signal += matrix->M();
                        response += fftSize;
                    }
                }
            }
            else
            {
                hwComplex* signal = const_cast<hwComplex*> (matrix->GetComplexData());

                for (int i = 0; i < matrix->N(); ++i)
                {
                    hwMatrix col_in(matrix->M(), 1, reinterpret_cast<void*> (signal), hwMatrix::COMPLEX);
                    hwMatrix col_out(fftSize, 1, reinterpret_cast<void*> (response), hwMatrix::COMPLEX);
                    status = Fft(col_in, col_out, fftSize);
                    signal += matrix->M();
                    response += fftSize;
                }
            }
        }
        else if (dim == 2)
        {
            if (fftSize == matrix->N())
            {
                hwMathStatus status = Fft(*matrix, *freq, dim - 1, fftSize);
            }
            else
            {
                freq.reset(EvaluatorInterface::allocateMatrix(matrix->M(),
                           fftSize, false));

                std::unique_ptr<hwMatrix> rowin(EvaluatorInterface::allocateMatrix());
                std::unique_ptr<hwMatrix> rowout(EvaluatorInterface::allocateMatrix());

                for (int i = 0; i < matrix->M(); i++)
                {
                    hwMathStatus status = matrix->ReadRow(i, *rowin);
                    BuiltInFuncsUtils::CheckMathStatus(eval, status);

                    status = Fft(*rowin, *rowout, fftSize);
                    BuiltInFuncsUtils::CheckMathStatus(eval, status);
                    if (rowout->M() != 1)
                    {
                        rowout->Transpose();
                    }
                    status = freq->WriteRow(i, *rowout);
                    BuiltInFuncsUtils::CheckMathStatus(eval, status);
                }
            }
        }
        else
        {
            throw OML_Error(OML_ERR_UNSUPPORTDIM, 3);
        }

        outputs.push_back(freq.release());
    }
    else        // ND case
    {
        const hwMatrixN* matrix = input1.MatrixN();
        const std::vector<int>& dims = matrix->Dimensions();

        if (fftSize == dims[dim - 1])
        {
            hwMatrixN* freq = new hwMatrixN;

            hwMathStatus status = Fft(*matrix, *freq, dim - 1, fftSize);
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
            outputs.push_back(freq);
        }
        else if (nargin != 3)
        {
            return oml_MatrixNUtil4(eval, inputs, outputs, OmlFft);
        }
        else
        {
            return oml_MatrixNUtil4(eval, inputs, outputs, OmlFft, 3);
        }
    }

    return true;
}
//------------------------------------------------------------------------------
// Two dimensional Fast Fourier transform
//------------------------------------------------------------------------------
bool OmlFft2(EvaluatorInterface            eval, 
              const std::vector<Currency>& inputs, 
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();    
    if (nargin != 1 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar() || inputs[0].IsComplex())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    const hwMatrix* signal = inputs[0].ConvertToMatrix();
    std::unique_ptr<hwMatrix> freqRes(EvaluatorInterface::allocateMatrix());

    hwMathStatus status;
    if (nargin == 1)
    {
        status = Fft2(*signal, *freqRes);
    }
    else
    {
        if (!inputs[1].IsInteger())
        {
            throw OML_Error(OML_ERR_INTEGER, 2, OML_VAR_TYPE);
        }
        if (!inputs[2].IsInteger())
        {
            throw OML_Error(OML_ERR_INTEGER, 3, OML_VAR_TYPE);
        }

        int m = static_cast<int>(inputs[1].Scalar());
        int n = static_cast<int>(inputs[2].Scalar());

        status = Fft2(*signal, m, n, *freqRes);
    }
    
    if (!status.IsOk())
    {
        throw OML_Error(status);
    }

    outputs.push_back(freqRes.release());
    return true;
}
//------------------------------------------------------------------------------
// Two dimensional inverse Fast Fourier transform
//------------------------------------------------------------------------------
bool OmlIfft2(EvaluatorInterface           eval,
              const std::vector<Currency>& inputs,
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin != 1 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar() || inputs[0].IsComplex())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    const hwMatrix* signal = inputs[0].ConvertToMatrix();
    std::unique_ptr<hwMatrix> freqRes(EvaluatorInterface::allocateMatrix());

    hwMathStatus status;
    if (nargin == 1)
    {
        status = Ifft2(*signal, *freqRes, false);
    }
    else
    {
        if (!inputs[1].IsInteger())
        {
            throw OML_Error(OML_ERR_INTEGER, 2, OML_VAR_TYPE);
        }
        if (!inputs[2].IsInteger())
        {
            throw OML_Error(OML_ERR_INTEGER, 3, OML_VAR_TYPE);
        }

        int m = static_cast<int>(inputs[1].Scalar());
        int n = static_cast<int>(inputs[2].Scalar());

        status = Ifft2(*signal, m, n, *freqRes, false);
    }

    if (!status.IsOk())
    {
        throw OML_Error(status);
    }

    outputs.push_back(freqRes.release());
    return true;
}
//------------------------------------------------------------------------------
// N dimensional Fast Fourier transform
//------------------------------------------------------------------------------
bool OmlFftN(EvaluatorInterface           eval,
             const std::vector<Currency>& inputs,
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 1 && nargin != 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (inputs[0].IsNDMatrix())
    {
        const hwMatrixN* signal = inputs[0].MatrixN();
        hwMatrixN* freqRes = new hwMatrixN;
        hwMathStatus status;

        if (nargin == 1)
        {
            status = FftN(*signal, *freqRes);
        }
        else
        {
            if (!inputs[1].IsRealVector())
            {
                delete freqRes;
                throw OML_Error(OML_ERR_INTEGER, 2, OML_VAR_TYPE);
            }

            const hwMatrix* dims = inputs[1].Matrix();
            int numDims = dims->Size();
            std::vector<int> size(numDims);

            for (int i = 0; i < numDims; ++i)
            {
                if (!IsInteger((*dims)(i)).IsOk())
                {
                    delete freqRes;
                    throw OML_Error(OML_ERR_NATURALNUM, 1, OML_VAR_DIM);
                }

                size[i] = static_cast<int> ((*dims)(i));
            }

            status = FftN(*signal, size, *freqRes);

            if (!status.IsOk())
            {
                delete freqRes;
                throw OML_Error(status);
            }
        }

        outputs.push_back(freqRes);
    }
    else if (inputs[0].IsMatrix() || inputs[0].IsScalar() || inputs[0].IsComplex())
    {
        return OmlFft2(eval, inputs, outputs);
    }
    else
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    return true;
}
//------------------------------------------------------------------------------
// N dimensional inverse Fast Fourier transform
//------------------------------------------------------------------------------
bool OmlIfftN(EvaluatorInterface           eval,
              const std::vector<Currency>& inputs,
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 1 && nargin != 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (inputs[0].IsNDMatrix())
    {
        const hwMatrixN* freqRes = inputs[0].MatrixN();
        hwMatrixN* signal = new hwMatrixN;
        hwMathStatus status;

        if (nargin == 1)
        {
            status = IfftN(*freqRes, *signal, false);
        }
        else
        {
            if (!inputs[1].IsRealVector())
            {
                delete signal;
                throw OML_Error(OML_ERR_INTEGER, 2, OML_VAR_TYPE);
            }

            const hwMatrix* dims = inputs[1].Matrix();
            int numDims = dims->Size();
            std::vector<int> size(numDims);

            for (int i = 0; i < numDims; ++i)
            {
                if (!IsInteger((*dims)(i)).IsOk())
                {
                    delete freqRes;
                    throw OML_Error(OML_ERR_NATURALNUM, 1, OML_VAR_DIM);
                }

                size[i] = static_cast<int> ((*dims)(i));
            }

            // status = IfftN(*freqRes, size, *signal, false);

            if (!status.IsOk())
            {
                delete signal;
                throw OML_Error(status);
            }
        }

        outputs.push_back(signal);
    }
    else if (inputs[0].IsMatrix() || inputs[0].IsScalar() || inputs[0].IsComplex())
    {
        return OmlIfft2(eval, inputs, outputs);
    }
    else
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    return true;
}
//------------------------------------------------------------------------------
// Short-time Fourier transform
//------------------------------------------------------------------------------
// [S,t,f] = stft(x, fs, window, overlap, nfft, range)
bool OmlStft(EvaluatorInterface           eval,
             const std::vector<Currency>& inputs,
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 6)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& input1 = inputs[0];

    if (!input1.IsMatrix() && !input1.IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    // sampling frequency argument
    double sampFreq = 1.0;

    if (nargin > 1)
    {
        if (inputs[1].IsScalar())
        {
            sampFreq = inputs[1].Scalar();
        }
        else if (inputs[1].IsMatrix())
        {
            if (!(inputs[1].Matrix()->M() == 0 && inputs[1].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 1, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALAR, 1, OML_VAR_VALUE);
        }
    }

    // window argument
    int windowSize = 256;
    const hwMatrix* window = nullptr;
    std::unique_ptr<hwMatrix> windowTemp(EvaluatorInterface::allocateMatrix());

    if (nargin > 2)
    {
        if (inputs[2].IsPositiveInteger())
        {
            windowSize = static_cast<int>(inputs[2].Scalar());
            windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
            hwMathStatus status = HannWin(*windowTemp, "periodic");

            if (!status.IsOk())
            {
                status.SetArg1(3);
                throw OML_Error(status);
            }
        }
        else if (inputs[2].IsMatrix())
        {
            if (inputs[2].Matrix()->M() == 0 && inputs[2].Matrix()->N() == 0)
            {
                windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
                hwMathStatus status = HannWin(*windowTemp, "periodic");

                if (!status.IsOk())
                {
                    status.SetArg1(3);
                    throw OML_Error(status);
                }
            }
            else
            {
                window = inputs[2].Matrix();
                windowSize = window->Size();
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALARVECTOR, 3);
        }
    }
    else
    {
        windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
        hwMathStatus status = HannWin(*windowTemp, "periodic");

        if (!status.IsOk())
        {
            status.SetArg1(3);
            throw OML_Error(status);
        }
    }

    // overlap argument
    int numOverlapPts = 0;

    if (nargin > 3)
    {
        if (inputs[3].IsInteger())
        {
            numOverlapPts = static_cast<int>(inputs[3].Scalar());
        }
        else if (inputs[3].IsMatrix())
        {
            if (!(inputs[3].Matrix()->M() == 0 && inputs[3].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
        }
    }

    // FFT size argument
    int fftSize = windowSize;

    if (nargin > 4)
    {
        if (inputs[4].IsPositiveInteger())
        {
            fftSize = static_cast<int>(inputs[4].Scalar());
        }
        else if (inputs[4].IsMatrix())
        {
            if (!(inputs[4].Matrix()->M() == 0 && inputs[4].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
        }
    }

    // spectrum sides argument
    std::string sideOpt = "onesided";

    if (nargin > 5)
    {
        if (!inputs.back().IsString())
        {
            throw OML_Error(OML_ERR_STRING, 6, OML_VAR_TYPE);
        }

        sideOpt = readOption(eval, inputs.back());

        if (sideOpt != "onesided" && sideOpt != "onesided_db" && sideOpt != "twosided")
        {
            throw OML_Error(HW_ERROR_INVALIDOPTION(sideOpt));
        }
    }

    // check inputs
    const hwMatrix* signal = inputs[0].ConvertToMatrix();

    if (!signal->IsEmptyOrVector())
    {
        throw OML_Error(OML_ERR_VECTOR, 1, OML_VAR_TYPE);
    }

    if (!signal->IsReal())
    {
        // throw OML_Error(OML_ERR_REAL, 1, OML_VAR_DATA);
        sideOpt = "twosided";
    }

    if (window && !window->IsEmptyOrVector())
    {
        throw OML_Error(OML_ERR_VECTOR, 2, OML_VAR_TYPE);
    }

    if (window && !window->IsReal())
    {
        throw OML_Error(OML_ERR_REAL, 2, OML_VAR_DATA);
    }

    int numPnts = signal->Size();

    if (windowSize > numPnts)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_FTBLOCKSIZE, 1, 2));
    }

    if (numOverlapPts >= windowSize)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_OVERLAPPOINTS, 3));
    }

    if (numOverlapPts < 0)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_NONNONNEGINT, 3));
    }

    if (sampFreq <= 0.0)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_NONPOSITIVE, 5));
    }

    if (fftSize < windowSize)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_FFTSIZE, 2, 4));
    }

    // perform stft ops
    int inc = windowSize - numOverlapPts;
    int numWin = (int)floor((double)(numPnts - windowSize) / (double)(inc)) + 1;
    hwMatrix signalMatrix(fftSize, numWin, hwMatrix::REAL);
    const double* signalVecData = signal->GetRealData();
    double* signalMatData = signalMatrix.GetRealData();

    signalMatrix.SetElements(0.0);

    for (int i = 0; i < numWin; ++i)
    {
        hwMatrix segment(windowSize, 1, (void*)signalVecData, hwMatrix::REAL);
        hwMatrix col(windowSize, 1, signalMatData, hwMatrix::REAL);

        if (window)
            MKLutilitiesD::MultByElems(segment, *window, col);
        else
            MKLutilitiesD::MultByElems(segment, *windowTemp, col);

        signalVecData += inc;
        signalMatData += fftSize;
    }

    // perform FFT and create output
    std::unique_ptr<hwMatrix> freqRes(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> timeVec(EvaluatorInterface::allocateMatrix(numWin, 1, true));
    std::unique_ptr<hwMatrix> freqVec(EvaluatorInterface::allocateMatrix());

    hwMathStatus status = Fft(signalMatrix, *freqRes, 0, 0);

    if (!status.IsOk())
    {
        status.ResetArgs();
        throw OML_Error(status);
    }

    int nargout = eval.GetNargoutValue();

    if (nargout == 0 || nargout > 1)
    {
        double dt = inc / sampFreq;

        for (int i = 0; i < numWin; ++i)
            (*timeVec)(i) = i * dt;
    }

    if (nargout == 0 || nargout > 2)
    {
        if (sideOpt == "onesided_db")
            status = Freq(fftSize, sampFreq, *freqVec, "onesided");
        else
            status = Freq(fftSize, sampFreq, *freqVec, sideOpt.c_str());
    }

    if (nargout == 0)
    {
        // get plot function pointers
        FUNCPTR contourPtr = eval.GetStdFunction("contour");
        FUNCPTR view = eval.GetStdFunction("view");
        FUNCPTR xLabelPtr = eval.GetStdFunction("xlabel");
        FUNCPTR yLabelPtr = eval.GetStdFunction("ylabel");
        FUNCPTR plotTitle = eval.GetStdFunction("title");
        FUNCPTR colorBar = eval.GetStdFunction("colorbar");
        FUNCPTR gcaPtr = eval.GetStdFunction("gca");
        FUNCPTR setPtr = eval.GetStdFunction("set");

        // compute magnitudes and handle side options
        std::vector<Currency> inputs2;
        std::vector<Currency> outputs2;

        inputs2.push_back(freqRes.release());
        BuiltInFuncsMKL::Abs(eval, inputs2, outputs2);

        if (sideOpt != "twosided")
        {
            inputs2.clear();
            inputs2.push_back(outputs2[0]);
            outputs2.clear();
            OmlFold(eval, inputs2, outputs2);
        }

        if (sideOpt == "onesided_db")
        {
            inputs2.clear();
            inputs2.push_back(outputs2[0]);
            outputs2.clear();
            BuiltInFuncsMKL::Log10(eval, inputs2, outputs2);
            outputs2[0].GetWritableMatrix()->MultEquals(20.0);
        }

        // create contour plot
        std::vector<Currency> plotInput;
        std::vector<Currency> dummyOutput;
        plotInput.push_back(timeVec.release());
        plotInput.push_back(freqVec.release());
        plotInput.push_back(outputs2[0]);
        contourPtr(eval, plotInput, dummyOutput);

        // set(ch, 'meshlines', 'off');
        if (!dummyOutput.empty())
        {
            plotInput.clear();
            plotInput.push_back(dummyOutput[0]);
            plotInput.push_back("meshlines");
            plotInput.push_back("off");
            setPtr(eval, plotInput, dummyOutput);
        }
        plotInput.clear();
        dummyOutput.clear();

        // add label axes and title
        std::vector<Currency> viewType;
        std::vector<Currency> x_label;
        std::vector<Currency> y_label;
        std::vector<Currency> title;
        viewType.push_back(2);
        x_label.push_back("Time [sec]");
        y_label.push_back("Frequency (Hz)");

        if (sideOpt == "onesided_db")
            title.push_back("Short Time Fourier Transform [dB]");
        else
            title.push_back("Short Time Fourier Transform [amplitude]");

        // view(eval, viewType, dummyOutput);
        // dummyOutput.clear();
        xLabelPtr(eval, x_label, dummyOutput);
        dummyOutput.clear();
        yLabelPtr(eval, y_label, dummyOutput);
        dummyOutput.clear();
        plotTitle(eval, title, dummyOutput);
        dummyOutput.clear();
        colorBar(eval, dummyOutput, dummyOutput);

        // gca
        dummyOutput.clear();
        gcaPtr(eval, plotInput, dummyOutput);
        plotInput.clear();

        // set(gca, ’contourtype’, ’blended’);
        plotInput.push_back(dummyOutput[0]);
        plotInput.push_back("contourtype");
        plotInput.push_back("blended");
        dummyOutput.clear();
        setPtr(eval, plotInput, dummyOutput);
    }
    else
    {
        if (sideOpt == "onesided")
        {
            status = freqRes->Resize(fftSize / 2 + 1, numWin);
            outputs.push_back(freqRes.release());
        }
        else if (sideOpt == "onesided_db")
        {
            std::vector<Currency> inputs2;
            std::vector<Currency> outputs2;

            inputs2.push_back(freqRes.release());
            BuiltInFuncsMKL::Abs(eval, inputs2, outputs2);
            inputs2.clear();
            inputs2.push_back(outputs2[0]);
            outputs2.clear();
            OmlFold(eval, inputs2, outputs2);

            inputs2.clear();
            inputs2.push_back(outputs2[0]);
            outputs2.clear();
            BuiltInFuncsMKL::Log10(eval, inputs2, outputs2);
            outputs2[0].GetWritableMatrix()->MultEquals(20.0);
            outputs.push_back(outputs2[0]);
        }
        else
        {
            outputs.push_back(freqRes.release());
        }

        if (nargout > 1)
        {
            outputs.push_back(timeVec.release());
        }

        if (nargout > 2)
        {
            outputs.push_back(freqVec.release());
        }
    }

    return true;
}
//------------------------------------------------------------------------------
// Short-time Fourier transform
//------------------------------------------------------------------------------
// [x,t] = istft(S, fs, window, overlap, nfft, range)
bool OmlIstft(EvaluatorInterface           eval,
              const std::vector<Currency>& inputs,
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 6)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& input1 = inputs[0];

    if (!input1.IsMatrix() && !input1.IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    // sampling frequency argument
    double sampFreq = 1.0;

    if (nargin > 1)
    {
        if (inputs[1].IsScalar())
        {
            sampFreq = inputs[1].Scalar();
        }
        else if (inputs[1].IsMatrix())
        {
            if (!(inputs[1].Matrix()->M() == 0 && inputs[1].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 1, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALAR, 1, OML_VAR_VALUE);
        }
    }

    // window argument
    int windowSize = 256;
    const hwMatrix* window = nullptr;
    std::unique_ptr<hwMatrix> windowTemp(EvaluatorInterface::allocateMatrix());

    if (nargin > 2)
    {
        if (inputs[2].IsPositiveInteger())
        {
            windowSize = static_cast<int>(inputs[2].Scalar());
            windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
            hwMathStatus status = HannWin(*windowTemp, "periodic");

            if (!status.IsOk())
            {
                status.SetArg1(3);
                throw OML_Error(status);
            }
        }
        else if (inputs[2].IsMatrix())
        {
            if (inputs[2].Matrix()->M() == 0 && inputs[2].Matrix()->N() == 0)
            {
                windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
                hwMathStatus status = HannWin(*windowTemp, "periodic");

                if (!status.IsOk())
                {
                    status.SetArg1(3);
                    throw OML_Error(status);
                }
            }
            else
            {
                window = inputs[2].Matrix();
                windowSize = window->Size();
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALARVECTOR, 3);
        }
    }
    else
    {
        windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
        hwMathStatus status = HannWin(*windowTemp, "periodic");

        if (!status.IsOk())
        {
            status.SetArg1(3);
            throw OML_Error(status);
        }
    }

    // overlap argument
    int numOverlapPts = 0;

    if (nargin > 3)
    {
        if (inputs[3].IsInteger())
        {
            numOverlapPts = static_cast<int>(inputs[3].Scalar());
        }
        else if (inputs[3].IsMatrix())
        {
            if (!(inputs[3].Matrix()->M() == 0 && inputs[3].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
        }
    }

    // FFT size argument
    int fftSize = -1;

    if (nargin > 4)
    {
        if (inputs[4].IsPositiveInteger())
        {
            fftSize = static_cast<int>(inputs[4].Scalar());
        }
        else if (inputs[4].IsMatrix())
        {
            if (!(inputs[4].Matrix()->M() == 0 && inputs[4].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
        }
    }

    // spectrum sides argument
    std::string sideOpt = "twosided";

    if (nargin > 5)
    {
        if (!inputs.back().IsString())
        {
            throw OML_Error(OML_ERR_STRING, 6, OML_VAR_TYPE);
        }

        sideOpt = readOption(eval, inputs.back());

        if (sideOpt != "onesided" && sideOpt != "twosided")
        {
            throw OML_Error(HW_ERROR_INVALIDOPTION(sideOpt));
        }
    }

    // check inputs
    const hwMatrix* freqRes = inputs[0].ConvertToMatrix();

    if (sideOpt == "onesided")
    {
        // convert to two-sided
        int m = freqRes->M();
        int n = freqRes->N();

        if (m != fftSize / 2 + 1)
        {
            throw OML_Error(OML_ERR_ARRAYSIZE, 1, 5, OML_VAR_DIMS);
        }

        hwMatrix* freqRes2 = EvaluatorInterface::allocateMatrix(fftSize, n, false);

        if (fftSize % 2 == 0)   // has Nyquist
            --m;

        freqRes2->WriteSubmatrix(0, 0, *freqRes);

        for (int j = 0; j < n; ++j)
        {
            for (int i = 1; i < m; ++i)
            {
                freqRes2->z(fftSize - i, j).Real() =  freqRes->z(i, j).Real();
                freqRes2->z(fftSize - i, j).Imag() = -freqRes->z(i, j).Imag();
            }
        }

        std::vector<Currency> inputs2;
        inputs2.push_back(freqRes2);

        if (nargin < 6)
        {
            for (size_t i = 1; i < nargin; ++i)
                inputs2.push_back(inputs[i]);

            for (size_t i = nargin; i < 5; ++i)
                inputs2.push_back(EvaluatorInterface::allocateMatrix());
        }
        else
        {
            for (size_t i = 1; i < 5; ++i)
                inputs2.push_back(inputs[i]);
        }

        inputs2.push_back("twosided");

        return OmlIstft(eval, inputs2, outputs);
    }
    else
    {
        fftSize = freqRes->M();
    }

    if (window && !window->IsEmptyOrVector())
    {
        throw OML_Error(OML_ERR_VECTOR, 2, OML_VAR_TYPE);
    }

    if (window && !window->IsReal())
    {
        throw OML_Error(OML_ERR_REAL, 2, OML_VAR_DATA);
    }

    int numPnts = freqRes->M();

    if (windowSize > numPnts)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_FTBLOCKSIZE, 1, 2));
    }

    if (numOverlapPts >= windowSize)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_OVERLAPPOINTS, 3));
    }

    if (numOverlapPts < 0)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_NONNONNEGINT, 3));
    }

    if (sampFreq <= 0.0)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_NONPOSITIVE, 5));
    }

    if (fftSize < windowSize)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_FFTSIZE, 2, 4));
    }

    // perform IFFT and resize
    int numWin = freqRes->N();
    hwMatrix signalMatrix;
    hwMathStatus status;

    status = Ifft(*freqRes, signalMatrix, 0, 0);

    // perform istft ops and create output
    // Daniel W. Griffin and Jae S. Lim "Signal Estimation from Modified Short - Time Fourier Transform",
    // IEEE 1984, 10.1109.
    int inc = windowSize - numOverlapPts;
    int signalLength = inc * (numWin - 1) + windowSize;
    std::unique_ptr<hwMatrix> signalVec(EvaluatorInterface::allocateMatrix(signalLength, 1, true));
    const double* signalVecData = signalVec->GetRealData();
    double* signalMatData = signalMatrix.GetRealData();
    hwMatrix segment1(windowSize, 1, hwMatrix::REAL);
    hwMatrix segment2(windowSize, 1, hwMatrix::REAL);
    hwMatrix norm(signalLength, 1, hwMatrix::REAL);
    double* normVecData = norm.GetRealData();

    signalVec->SetElements(0.0);
    norm.SetElements(0.0);

    if (window)
    {
        MKLutilitiesD::MultByElems(*window, *window, segment2);
    }
    else
    {
        MKLutilitiesD::MultByElems(*windowTemp, *windowTemp, segment2);
    }

    for (int i = 0; i < numWin; ++i)
    {
        hwMatrix segment3(windowSize, 1, (void*)signalVecData, hwMatrix::REAL);
        hwMatrix segment4(windowSize, 1, (void*)normVecData, hwMatrix::REAL);
        hwMatrix col(windowSize, 1, signalMatData, hwMatrix::REAL);

        if (window)
        {
            MKLutilitiesD::MultByElems(col, *window, segment1);
        }
        else
        {
            MKLutilitiesD::MultByElems(col, *windowTemp, segment1);
        }

        segment3 += segment1;
        segment4 += segment2;
        signalVecData += inc;
        normVecData += inc;
        signalMatData += fftSize;
    }

    for (int i = 0; i < signalLength; ++i)
    {
        if (norm(i) != 0.0)
            (*signalVec)(i) /= norm(i);
    }

    // perform FFT and create output
    std::unique_ptr<hwMatrix> timeVec(EvaluatorInterface::allocateMatrix(signalLength, 1, true));
    int nargout = eval.GetNargoutValue();

    if (nargout == 0 || nargout > 1)
    {
        double dt = 1.0 / sampFreq;

        for (int i = 0; i < signalLength; ++i)
            (*timeVec)(i) = i * dt;
    }

    if (nargout == 0)
    {
        // get plot function pointers
        FUNCPTR plotPtr = eval.GetStdFunction("plot");
        FUNCPTR xLabelPtr = eval.GetStdFunction("xlabel");
        FUNCPTR yLabelPtr = eval.GetStdFunction("ylabel");
        FUNCPTR plotTitle = eval.GetStdFunction("title");

        // create plot
        std::vector<Currency> plotInput;
        std::vector<Currency> dummyOutput;
        plotInput.push_back(timeVec.release());
        plotInput.push_back(signalVec.release());
        plotPtr(eval, plotInput, dummyOutput);
        dummyOutput.clear();

        // add label axes and title
        std::vector<Currency> x_label;
        std::vector<Currency> y_label;
        std::vector<Currency> title;
        x_label.push_back("Time [sec]");
        y_label.push_back("Amplitude");
        title.push_back("Inverse Short Time Fourier Transform");

        xLabelPtr(eval, x_label, dummyOutput);
        dummyOutput.clear();
        yLabelPtr(eval, y_label, dummyOutput);
        dummyOutput.clear();
        plotTitle(eval, title, dummyOutput);
        dummyOutput.clear();
    }
    else
    {
        outputs.push_back(signalVec.release());

        if (nargout > 1)
            outputs.push_back(timeVec.release());
    }

    return true;
}
//------------------------------------------------------------------------------
// Spectrogram
//------------------------------------------------------------------------------
// [S,t,f] = spectrogram(x, fs, window, overlap, nfft, range)
bool OmlSpectrogram(EvaluatorInterface           eval,
                    const std::vector<Currency>& inputs,
                    std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 6)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& input1 = inputs[0];

    if (!input1.IsMatrix() && !input1.IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    // sampling frequency argument
    double sampFreq = 1.0;

    if (nargin > 1)
    {
        if (inputs[1].IsScalar())
        {
            sampFreq = inputs[1].Scalar();
        }
        else if (inputs[1].IsMatrix())
        {
            if (!(inputs[1].Matrix()->M() == 0 && inputs[1].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 1, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALAR, 1, OML_VAR_VALUE);
        }
    }

    // window argument
    int windowSize = 256;
    const hwMatrix* window = nullptr;
    std::unique_ptr<hwMatrix> windowTemp(EvaluatorInterface::allocateMatrix());

    if (nargin > 2)
    {
        if (inputs[2].IsPositiveInteger())
        {
            windowSize = static_cast<int>(inputs[2].Scalar());
            windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
            hwMathStatus status = HannWin(*windowTemp, "periodic");

            if (!status.IsOk())
            {
                status.SetArg1(3);
                throw OML_Error(status);
            }
        }
        else if (inputs[2].IsMatrix())
        {
            if (inputs[2].Matrix()->M() == 0 && inputs[2].Matrix()->N() == 0)
            {
                windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
                hwMathStatus status = HannWin(*windowTemp, "periodic");

                if (!status.IsOk())
                {
                    status.SetArg1(3);
                    throw OML_Error(status);
                }
            }
            else
            {
                window = inputs[2].Matrix();
                windowSize = window->Size();
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALARVECTOR, 3);
        }
    }
    else
    {
        windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
        hwMathStatus status = HannWin(*windowTemp, "periodic");

        if (!status.IsOk())
        {
            status.SetArg1(3);
            throw OML_Error(status);
        }
    }

    // overlap argument
    int numOverlapPts = 0;

    if (nargin > 3)
    {
        if (inputs[3].IsInteger())
        {
            numOverlapPts = static_cast<int>(inputs[3].Scalar());
        }
        else if (inputs[3].IsMatrix())
        {
            if (!(inputs[3].Matrix()->M() == 0 && inputs[3].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
        }
    }

    // FFT size argument
    int fftSize = windowSize;

    if (nargin > 4)
    {
        if (inputs[4].IsPositiveInteger())
        {
            fftSize = static_cast<int>(inputs[4].Scalar());
        }
        else if (inputs[4].IsMatrix())
        {
            if (!(inputs[4].Matrix()->M() == 0 && inputs[4].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
        }
    }

    // spectrum sides argument
    std::string sideOpt = "onesided";

    if (nargin > 5)
    {
        if (!inputs.back().IsString())
        {
            throw OML_Error(OML_ERR_STRING, 6, OML_VAR_TYPE);
        }

        sideOpt = readOption(eval, inputs.back());

        if (sideOpt != "onesided" && sideOpt != "onesided_db" && sideOpt != "twosided")
        {
            throw OML_Error(HW_ERROR_INVALIDOPTION(sideOpt));
        }
    }

    // check inputs
    const hwMatrix* signal = inputs[0].ConvertToMatrix();

    if (!signal->IsEmptyOrVector())
    {
        throw OML_Error(OML_ERR_VECTOR, 1, OML_VAR_TYPE);
    }

    if (!signal->IsReal())
    {
        sideOpt = "twosided";
    }

    if (window && !window->IsEmptyOrVector())
    {
        throw OML_Error(OML_ERR_VECTOR, 2, OML_VAR_TYPE);
    }

    if (window && !window->IsReal())
    {
        throw OML_Error(OML_ERR_REAL, 2, OML_VAR_DATA);
    }

    int numPnts = signal->Size();

    if (windowSize > numPnts)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_FTBLOCKSIZE, 1, 2));
    }

    if (numOverlapPts >= windowSize)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_OVERLAPPOINTS, 3));
    }

    if (numOverlapPts < 0)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_NONNONNEGINT, 3));
    }

    if (sampFreq <= 0.0)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_NONPOSITIVE, 5));
    }

    if (fftSize < windowSize)
    {
        throw OML_Error(hwMathStatus(HW_MATH_ERR_FFTSIZE, 2, 4));
    }

    // perform stft ops
    int inc = windowSize - numOverlapPts;
    int numWin = (int)floor((double)(numPnts - windowSize) / (double)(inc)) + 1;
    hwMatrix signalMatrix(fftSize, numWin, hwMatrix::REAL);
    const double* signalVecData = signal->GetRealData();
    double* signalMatData = signalMatrix.GetRealData();

    signalMatrix.SetElements(0.0);

    for (int i = 0; i < numWin; ++i)
    {
        hwMatrix segment(windowSize, 1, (void*)signalVecData, hwMatrix::REAL);
        hwMatrix col(windowSize, 1, signalMatData, hwMatrix::REAL);

        if (window)
            MKLutilitiesD::MultByElems(segment, *window, col);
        else
            MKLutilitiesD::MultByElems(segment, *windowTemp, col);

        signalVecData += inc;
        signalMatData += fftSize;
    }

    // create spectrogram and output
    std::unique_ptr<hwMatrix> freqRes(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> timeVec(EvaluatorInterface::allocateMatrix(numWin, 1, true));
    std::unique_ptr<hwMatrix> freqVec(EvaluatorInterface::allocateMatrix());

    hwMathStatus status = Fft(signalMatrix, *freqRes, 0, 0);

    if (!status.IsOk())
    {
        status.ResetArgs();
        throw OML_Error(status);
    }

    std::vector<Currency> inputs2;
    std::vector<Currency> outputs2;
    inputs2.push_back(freqRes.release());
    BuiltInFuncsMKL::Abs(eval, inputs2, outputs2);

    hwMatrix* absSTFT = outputs2[0].GetWritableMatrix();
    std::unique_ptr<hwMatrix> psdSTFT(EvaluatorInterface::allocateMatrix());
    MKLutilitiesD::PowerByElems(*absSTFT, 2.0, *psdSTFT);
    double scale = 1.0 / ((double)windowSize * sampFreq);
    psdSTFT->MultEquals(scale);

    int nargout = eval.GetNargoutValue();

    if (nargout == 0 || nargout > 1)
    {
        double dt = inc / sampFreq;

        for (int i = 0; i < numWin; ++i)
            (*timeVec)(i) = i * dt;
    }

    if (nargout == 0 || nargout > 2)
    {
        if (sideOpt == "onesided_db")
            status = Freq(fftSize, sampFreq, *freqVec, "onesided");
        else
            status = Freq(fftSize, sampFreq, *freqVec, sideOpt.c_str());
    }

    if (nargout == 0)
    {
        // get plot function pointers
        FUNCPTR contourPtr = eval.GetStdFunction("contour");
        FUNCPTR view = eval.GetStdFunction("view");
        FUNCPTR xLabelPtr = eval.GetStdFunction("xlabel");
        FUNCPTR yLabelPtr = eval.GetStdFunction("ylabel");
        FUNCPTR plotTitle = eval.GetStdFunction("title");
        FUNCPTR colorBar = eval.GetStdFunction("colorbar");
        FUNCPTR gcaPtr = eval.GetStdFunction("gca");
        FUNCPTR setPtr = eval.GetStdFunction("set");

        // compute magnitudes and handle side options
        outputs2.clear();

        if (sideOpt == "twosided")
        {
            outputs2.push_back(psdSTFT.release());
        }
        else
        {
            inputs2.clear();
            inputs2.push_back(psdSTFT.release());
            OmlFold(eval, inputs2, outputs2);

            if (sideOpt == "onesided_db")
            {
                inputs2.clear();
                inputs2.push_back(outputs2[0]);
                outputs2.clear();
                BuiltInFuncsMKL::Log10(eval, inputs2, outputs2);
                outputs2[0].GetWritableMatrix()->MultEquals(20.0);
            }
        }

        // create contour plot
        std::vector<Currency> plotInput;
        std::vector<Currency> dummyOutput;
        plotInput.push_back(timeVec.release());
        plotInput.push_back(freqVec.release());
        plotInput.push_back(outputs2[0]);
        contourPtr(eval, plotInput, dummyOutput);
        
        // set(ch, 'meshlines', 'off');
        if (!dummyOutput.empty())
        {
            plotInput.clear();
            plotInput.push_back(dummyOutput[0]);
            plotInput.push_back("meshlines");
            plotInput.push_back("off");
            setPtr(eval, plotInput, dummyOutput);
        }
        plotInput.clear();
        dummyOutput.clear();

        // add label axes and title
        std::vector<Currency> viewType;
        std::vector<Currency> x_label;
        std::vector<Currency> y_label;
        std::vector<Currency> title;
        viewType.push_back(2);
        x_label.push_back("Time [sec]");
        y_label.push_back("Frequency (Hz)");

        if (sideOpt == "onesided_db")
            title.push_back("Spectrogram [dB]");
        else
            title.push_back("Spectrogram [amp^2 / Hz]");

        // view(eval, viewType, dummyOutput);
        // dummyOutput.clear();
        xLabelPtr(eval, x_label, dummyOutput);
        dummyOutput.clear();
        yLabelPtr(eval, y_label, dummyOutput);
        dummyOutput.clear();
        plotTitle(eval, title, dummyOutput);
        dummyOutput.clear();
        colorBar(eval, dummyOutput, dummyOutput);

        // gca
        dummyOutput.clear();
        gcaPtr(eval, plotInput, dummyOutput);
        plotInput.clear();

        // set(gca, ’contourtype’, ’blended’);
        plotInput.push_back(dummyOutput[0]);
        plotInput.push_back("contourtype");
        plotInput.push_back("blended");
        dummyOutput.clear();
        setPtr(eval, plotInput, dummyOutput);
    }
    else
    {
        if (sideOpt == "onesided")
        {
            inputs2.clear();
            outputs2.clear();
            inputs2.push_back(psdSTFT.release());
            OmlFold(eval, inputs2, outputs2);
            outputs.push_back(outputs2[0]);
        }
        else if (sideOpt == "onesided_db")
        {
            inputs2.clear();
            outputs2.clear();
            inputs2.push_back(psdSTFT.release());
            OmlFold(eval, inputs2, outputs2);

            inputs2.clear();
            inputs2.push_back(outputs2[0]);
            outputs2.clear();
            BuiltInFuncsMKL::Log10(eval, inputs2, outputs2);
            outputs2[0].GetWritableMatrix()->MultEquals(20.0);
            outputs.push_back(outputs2[0]);
        }
        else
        {
            outputs.push_back(psdSTFT.release());
        }

        if (nargout > 1)
        {
            outputs.push_back(timeVec.release());
        }

        if (nargout > 2)
        {
            outputs.push_back(freqVec.release());
        }
    }

    if (nargout > 1)
    {
        outputs.push_back(timeVec.release());
    }

    if (nargout > 2)
    {
        outputs.push_back(freqVec.release());
    }

    return true;
}
//------------------------------------------------------------------------------
// Shift FFT related data to center DC component
//------------------------------------------------------------------------------
bool OmlFftShift(EvaluatorInterface           eval,
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    std::vector<Currency> inputs2;
    std::vector<Currency> outputs2;
    std::vector<Currency> sliceArgs;

    if (nargin == 1)
    {
        inputs2.push_back(inputs[0]);
        oml_isvector(eval, inputs2, outputs2);

        if (outputs2[0].Scalar() == 1.0)    // vector case
        {
            // get vector length and DC position
            outputs2.clear();
            oml_length(eval, inputs2, outputs2);
            int length = static_cast<int>(outputs2[0].Scalar());
            outputs2.clear();
            int dc = static_cast<int>(ceil(length / 2.0));
            int offset = length - dc;

            // create index vector
            hwMatrix* index = new hwMatrix(length, hwMatrix::REAL);

            for (int i = 0; i < offset; ++i)
                (*index)(i) = i + dc + 1;

            for (int i = offset; i < length; ++i)
                (*index)(i) = i - offset + 1;

            // create slice arguments
            sliceArgs.push_back(index);
        }
        else
        {
            // get matrix dimension info
            outputs2.clear();
            oml_ndims(eval, inputs2, outputs2);
            int numDims = static_cast<int> (outputs2[0].Scalar());
            outputs2.clear();

            outputs2 = eval.DoMultiReturnFunctionCall(oml_size, inputs2, 1, 1, true);
            Currency lengths = outputs2[0];

            // get DC positions
            hwMatrix* length_vec = lengths.GetWritableMatrix();
            hwMatrix* halfLen = new hwMatrix(1, numDims, hwMatrix::REAL);
            (*halfLen) = (*length_vec) / 2.0;

            inputs2.clear();
            inputs2.push_back(halfLen);
            outputs2.clear();
            BuiltInFuncsMKL::Ceil(eval, inputs2, outputs2);
            hwMatrix* dc_vec = outputs2[0].GetWritableMatrix();

            // create index vectors and slice arguments
            for (int i = 0; i < numDims; ++i)
            {
                int dc = static_cast<int> ((*dc_vec)(i));
                int length = static_cast<int> ((*length_vec)(i));
                int offset = length - dc;
                hwMatrix* index = new hwMatrix(length, hwMatrix::REAL);

                for (int j = 0; j < offset; ++j)
                    (*index)(j) = j + dc + 1;

                for (int j = offset; j < length; ++j)
                    (*index)(j) = j - offset + 1;

                sliceArgs.push_back(index);
            }
        }
    }
    else if (nargin == 2)
    {
        // get matrix dimension info
        inputs2.push_back(inputs[0]);
        oml_ndims(eval, inputs2, outputs2);
        int numDims = static_cast<int> (outputs2[0].Scalar());
        outputs2.clear();

        outputs2 = eval.DoMultiReturnFunctionCall(oml_size, inputs2, 1, 1, true);
        Currency dims = outputs2[0];
        outputs2.clear();

        // get dimension of interest
        if (!inputs[1].IsPositiveInteger())
        {
            throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_DIM);
        }

        int dim = static_cast<int> (inputs[1].Scalar());

        if (dim < 1)
        {
            throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_DIM);
        }

        if (dim > numDims)
        {
            outputs.push_back(inputs[0]);
            return true;
        }

        // get dimension length and DC position
        int length = static_cast<int>((*dims.Matrix())(dim - 1));
        int dc = static_cast<int>(ceil(length / 2.0));
        int offset = length - dc;

        // create index vector
        hwMatrix* index = new hwMatrix(length, hwMatrix::REAL);

        for (int i = 0; i < offset; ++i)
            (*index)(i) = i + dc + 1;

        for (int i = offset; i < length; ++i)
            (*index)(i) = i - offset + 1;

        // create slice arguments
        for (int i = 0; i < dim - 1; ++i)
            sliceArgs.push_back(Currency(0.0, Currency::TYPE_COLON));

        sliceArgs.push_back(index);

        for (int i = dim; i < numDims; ++i)
            sliceArgs.push_back(Currency(0.0, Currency::TYPE_COLON));
    }
    else
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // re-index
    outputs.push_back(eval.VariableIndex(inputs[0], sliceArgs));

    return true;
}
//------------------------------------------------------------------------------
// Reverse shift FFT related data to uncenter DC component
//------------------------------------------------------------------------------
bool OmlIFftShift(EvaluatorInterface           eval,
                  const std::vector<Currency>& inputs,
                  std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    std::vector<Currency> inputs2;
    std::vector<Currency> outputs2;
    std::vector<Currency> sliceArgs;

    if (nargin == 1)
    {
        inputs2.push_back(inputs[0]);
        oml_isvector(eval, inputs2, outputs2);

        if (outputs2[0].Scalar() == 1.0)    // vector case
        {
            // get vector length and DC position
            outputs2.clear();
            oml_length(eval, inputs2, outputs2);
            int length = static_cast<int>(outputs2[0].Scalar());
            outputs2.clear();
            int dc = static_cast<int>(ceil(length / 2.0));
            int offset = length - dc;

            // create index vector
            hwMatrix* index = new hwMatrix(length, hwMatrix::REAL);

            for (int i = 0; i < offset; ++i)
                (*index)(i + dc) = i + 1;

            for (int i = offset; i < length; ++i)
                (*index)(i - offset) = i + 1;

            // create slice arguments
            sliceArgs.push_back(index);
        }
        else
        {
            // get matrix dimension info
            outputs2.clear();
            oml_ndims(eval, inputs2, outputs2);
            int numDims = static_cast<int> (outputs2[0].Scalar());
            outputs2.clear();

            outputs2 = eval.DoMultiReturnFunctionCall(oml_size, inputs2, 1, 1, true);
            Currency lengths = outputs2[0];

            // get DC positions
            hwMatrix* length_vec = lengths.GetWritableMatrix();
            hwMatrix* halfLen = new hwMatrix(1, numDims, hwMatrix::REAL);
            (*halfLen) = (*length_vec) / 2.0;

            inputs2.clear();
            inputs2.push_back(halfLen);
            outputs2.clear();
            BuiltInFuncsMKL::Ceil(eval, inputs2, outputs2);
            hwMatrix* dc_vec = outputs2[0].GetWritableMatrix();

            // create index vectors and slice arguments
            for (int i = 0; i < numDims; ++i)
            {
                int dc = static_cast<int> ((*dc_vec)(i));
                int length = static_cast<int> ((*length_vec)(i));
                int offset = length - dc;
                hwMatrix* index = new hwMatrix(length, hwMatrix::REAL);

                for (int j = 0; j < offset; ++j)
                    (*index)(j + dc) = j + 1;

                for (int j = offset; j < length; ++j)
                    (*index)(j - offset) = j + 1;

                sliceArgs.push_back(index);
            }
        }
    }
    else if (nargin == 2)
    {
        // get matrix dimension info
        inputs2.push_back(inputs[0]);
        oml_ndims(eval, inputs2, outputs2);
        int numDims = static_cast<int> (outputs2[0].Scalar());
        outputs2.clear();

        outputs2 = eval.DoMultiReturnFunctionCall(oml_size, inputs2, 1, 1, true);
        Currency dims = outputs2[0];
        outputs2.clear();

        // get dimension of interest
        if (!inputs[1].IsPositiveInteger())
        {
            throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_DIM);
        }

        int dim = static_cast<int> (inputs[1].Scalar());

        if (dim < 1)
        {
            throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_DIM);
        }

        if (dim > numDims)
        {
            outputs.push_back(inputs[0]);
            return true;
        }

        // get dimension length and DC position
        int length = static_cast<int>((*dims.Matrix())(dim - 1));
        int dc = static_cast<int>(ceil(length / 2.0));
        int offset = length - dc;

        // create index vector
        hwMatrix* index = new hwMatrix(length, hwMatrix::REAL);

        for (int i = 0; i < offset; ++i)
            (*index)(i + dc) = i + 1;

        for (int i = offset; i < length; ++i)
            (*index)(i - offset) = i + 1;

        // create slice arguments
        for (int i = 0; i < dim - 1; ++i)
            sliceArgs.push_back(Currency(0.0, Currency::TYPE_COLON));

        sliceArgs.push_back(index);

        for (int i = dim; i < numDims; ++i)
            sliceArgs.push_back(Currency(0.0, Currency::TYPE_COLON));
    }
    else
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // re-index
    outputs.push_back(eval.VariableIndex(inputs[0], sliceArgs));

    return true;
}
//------------------------------------------------------------------------------
// Generates a vector of frequency locations
//------------------------------------------------------------------------------
bool OmlFreq(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    std::unique_ptr<hwMatrix> output(EvaluatorInterface::allocateMatrix());
    hwMathStatus status;

    if (nargin < 1 || nargin > 4)
        throw OML_Error(OML_ERR_NUMARGIN);

    std::string option;

    if (inputs.back().IsString())
    {
        option = inputs.back().StringVal();
        --nargin;
    }
    else
    {
        option = "twosided";
    }

    if (nargin == 1)
    {
        if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
        {
            throw OML_Error(OML_ERR_VECTOR, 1, OML_VAR_DATA);
        }

        const hwMatrix* time = inputs[0].ConvertToMatrix();
        status               = Freq(*time, *output, option.c_str());
    }
    else if (nargin == 2)
    {
        if (!inputs[0].IsPositiveInteger())
        {
            throw OML_Error(OML_ERR_POSINTEGER, 1, OML_VAR_VALUE);
        }
        if (!inputs[1].IsScalar())
        {
            throw OML_Error(OML_ERR_SCALAR, 2);
        }

        int    numPnts  = static_cast<int>(inputs[0].Scalar());
        double sampFreq = inputs[1].Scalar();
        status          = Freq(numPnts, sampFreq, *output, option.c_str());
    }
    else
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    BuiltInFuncsUtils::CheckMathStatus(eval, status);
    
    status = output->Transpose();  // switch to row vectors
    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(output.release());
    return true;
}
//------------------------------------------------------------------------------
// Creates one-sided spectrum amplitude vectors from the two-sided equivalents
//------------------------------------------------------------------------------
bool OmlFold(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 1, OML_VAR_DATA);
    }

    const hwMatrix* mtx = inputs[0].ConvertToMatrix();
    int dim = 1;

    if (nargin > 1)
    {
        if (!inputs[1].IsPositiveInteger())
        {
            throw OML_Error(OML_ERR_POSINTEGER, 1);
        }
        dim = static_cast<int>(inputs[1].Scalar());
    }
    else
    {
        dim = mtx->M() == 1 ? 2 : 1;
    }

    int i;
    hwMatrix* result = nullptr;
    if (dim == 1)
    {
        hwMatrix* col = new hwMatrix;
        Currency colcur(col);
        for (i = 0; i < mtx->N(); ++i)
        {
            hwMathStatus stat = mtx->ReadColumn (i, *col);
            if (!stat.IsOk() && i)
            {
                delete result;
                result = nullptr;
            }
            BuiltInFuncsUtils::CheckMathStatus(eval, stat);

            hwMatrix* temp  = EvaluatorInterface::allocateMatrix();
            stat = Fold(*col, *temp);
            if (!stat.IsOk())
            {
                delete result;
                result = nullptr;
            }
            BuiltInFuncsUtils::CheckMathStatus(eval, stat);

            if (!i)
            {
                result = new hwMatrix(temp->Size(), mtx->N(), hwMatrix::REAL);
            }
            writeCol(eval, result, temp, i);
            delete temp;
            temp = nullptr;
        }
        if (i) 
        {
            outputs.push_back(result);
        }
        else
        {
            outputs.push_back(new hwMatrix);
        }
        return true;
    }
   
    // dim == 2
    hwMatrix* row = new hwMatrix;
    Currency rowcur(row);
    for (i = 0; i < mtx->M(); ++i)
    {
        hwMathStatus stat = mtx->ReadRow (i, *row);
        if (!stat.IsOk() && i)
        {
            delete result;
            result = nullptr;
        }
        BuiltInFuncsUtils::CheckMathStatus(eval, stat);

        hwMatrix* temp  = EvaluatorInterface::allocateMatrix();
        stat = Fold(*row, *temp);
        if (!stat.IsOk())
        {
            delete temp;
            temp = nullptr;
        }
        BuiltInFuncsUtils::CheckMathStatus(eval, stat);

        if (!i)
        {
            result = new hwMatrix(mtx->M(), temp->Size(), hwMatrix::REAL);
        }
        writeRow(eval, result, temp, i);

        delete temp;
        temp = nullptr;
    }
    if (i)
    {
        outputs.push_back(result);
    }
    else
    {
        outputs.push_back(new hwMatrix);
    }

    return true;
}
//------------------------------------------------------------------------------
// Performs cross correlation [xcorr command]
//------------------------------------------------------------------------------
bool OmlXcorr(EvaluatorInterface eval, const std::vector<Currency>& inputs, std::vector<Currency>& outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    Currency cur1 = inputs[0];
    if (!cur1.IsMatrix() && !cur1.IsScalar())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 1, OML_VAR_DATA);
    }

    const hwMatrix* v1 = cur1.ConvertToMatrix();
    std::unique_ptr<hwMatrix> result(EvaluatorInterface::allocateMatrix());

    if (v1->IsVector())
    {
        const hwMatrix* v2 = nullptr;
        if (nargin == 1)
        {
            v2 = v1;
        }
        else
        {
            if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
            {
                throw OML_Error(OML_ERR_REALVECTOR, 2, OML_VAR_DATA);
            }
            v2 = inputs[1].ConvertToMatrix();
        }
        hwMathStatus stat = result->CorrLin(*v1, *v2);
        BuiltInFuncsUtils::CheckMathStatus(eval, stat);
    }
    else
    {
        if (nargin != 1)
        {
            throw OML_Error(OML_ERR_NUMARGIN);
        }

        hwMathStatus stat = result->CorrLin(*v1);
        BuiltInFuncsUtils::CheckMathStatus(eval, stat);
    }

    outputs.push_back(result.release());
    return true;
}
//------------------------------------------------------------------------------
// Unwraps a vector of phase angles [unwrap command]
//------------------------------------------------------------------------------
bool OmlUnwrap(EvaluatorInterface           eval, 
               const std::vector<Currency>& inputs, 
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 1, OML_VAR_DATA);
    }
    const hwMatrix* mtx    = inputs[0].ConvertToMatrix();
    hwMatrix*       result = EvaluatorInterface::allocateMatrix();
    double          tol    = PI;
    int             dim    = mtx->M() == 1 ? 2 : 1;

    if (nargin > 1)
    {
        if (!inputs[1].IsScalar())
        {
            throw OML_Error(OML_ERR_REAL, 2, OML_VAR_VALUE);
        }
        tol = inputs[1].Scalar();

        if (nargin > 2)
        {
            if (!inputs[2].IsPositiveInteger())
            {
                throw OML_Error(OML_ERR_POSINTEGER, 3, OML_VAR_DIM);
            }
            dim = static_cast<int>(inputs[2].Scalar());
        }
    }

    if (dim > 2)
    {
        throw OML_Error(HW_ERROR_UNSUPP2DIM);
    }

    hwMathStatus stat = result->UnwrapMat(*mtx, dim, tol);
    BuiltInFuncsUtils::CheckMathStatus(eval, stat);

    outputs.push_back(result);
    return true;
}
//------------------------------------------------------------------------------
// Computes power spectral density [pwelch command]
//------------------------------------------------------------------------------
bool OmlPwelch(EvaluatorInterface           eval, 
               const std::vector<Currency>& inputs, 
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 6)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& input1 = inputs[0];

    if (!input1.IsMatrix() && !input1.IsNDMatrix() && !input1.IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_DATA);
    }

    // window argument
    int windowSize = -1;
    const hwMatrix* window  = nullptr;
    std::unique_ptr<hwMatrix> windowTemp(EvaluatorInterface::allocateMatrix());

    if (nargin > 1)
    {
        if (inputs[1].IsPositiveInteger())
        {
            windowSize = static_cast<int>(inputs[1].Scalar());
            windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
            hwMathStatus status = HammWin(*windowTemp, "periodic");

            if (!status.IsOk())
            {
                status.SetArg1(2);
                throw OML_Error(status);
            }
        }
        else if (inputs[1].IsMatrix())
        {
            if (!(inputs[1].Matrix()->M() == 0 && inputs[1].Matrix()->N() == 0))
            {
                window = inputs[1].Matrix();
                windowSize = window->Size();
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALARVECTOR, 2);
        }
    }

    // overlap argument
    int numOverlapPts = -1;
    if (nargin > 2)
    {
        if (inputs[2].IsInteger())
        {
            numOverlapPts = static_cast<int>(inputs[2].Scalar());
        }
        else if (inputs[2].IsMatrix())
        {
            if (!(inputs[2].Matrix()->M() == 0 && inputs[2].Matrix()->N() == 0))
            {                
                throw OML_Error(OML_ERR_POSINTEGER, 3, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 3, OML_VAR_VALUE);
        }
    }

    // mangage default window size and/or overlap
    if (windowSize == -1)
    {
        // 8 blocks
        int dataSize;

        if (input1.IsMatrix() || input1.IsScalar())
        {
            const hwMatrix* signal = input1.ConvertToMatrix();

            if (signal->IsVector())
                dataSize = signal->Size();
            else
                dataSize = signal->M();
        }
        else    // input1.IsMatrix()
        {
            dataSize = input1.MatrixN()->Dimensions()[0];
        }

        if (numOverlapPts == -1)
        {
            windowSize    = (2*dataSize) / 9;
            numOverlapPts = windowSize / 2;
        }
        else
        {
            windowSize = ((dataSize + 7*numOverlapPts)) / 8;
        }

        windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
        hwMathStatus status = HammWin(*windowTemp, "periodic");

        if (!status.IsOk())
        {
            status.SetArg1(2);
            throw OML_Error(status);
        }
    }
    else if (numOverlapPts == -1)
    {
        numOverlapPts = windowSize / 2;
    }

    // FFT size argument
    int fftSize = windowSize;
    if (nargin > 3)
    {
        if (inputs[3].IsPositiveInteger())
        {
            fftSize = static_cast<int>(inputs[3].Scalar());
        }
        else if (inputs[3].IsMatrix())
        {
            if (!(inputs[3].Matrix()->M() == 0 && inputs[3].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
        }
    }

    // sampling frequency argument
    double sampFreq = 1.0;
    if (nargin > 4)
    {
        if (inputs[4].IsScalar())
        {
            sampFreq = inputs[4].Scalar();
        }
        else if (inputs[4].IsMatrix())
        {
            if (!(inputs[4].Matrix()->M() == 0 && inputs[4].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALAR, 5, OML_VAR_VALUE);
        }
    }

    //  spectrum sides argument
    bool onesided = true;
    if (nargin > 1 && inputs.back().IsString())
    {
        std::string opt = readOption(eval, inputs.back());
        if (opt == "twosided")
        {
            onesided = false;
        }
        else if (opt != "onesided")
        {
            throw OML_Error(HW_ERROR_INVALIDOPTION(opt));
        }
    }

    // perform psd ops
    std::unique_ptr<hwMatrix> density(EvaluatorInterface::allocateMatrix());

    if (!input1.IsNDMatrix())
    {
        const hwMatrix* matrix = input1.ConvertToMatrix();

        if (!matrix->IsReal())
        {
            throw OML_Error(OML_ERR_REAL, 1, OML_VAR_DATA);
        }

        std::unique_ptr<hwMatrix> freq(EvaluatorInterface::allocateMatrix());

        if (matrix->IsVector())
        {
            hwMathStatus status = (window) ?
                BlockPSD(*matrix, *window, numOverlapPts, sampFreq, *density, fftSize) :
                BlockPSD(*matrix, *windowTemp, numOverlapPts, sampFreq, *density, fftSize);

            BuiltInFuncsUtils::CheckMathStatus(eval, status);

            if (onesided)
            {
                std::unique_ptr<hwMatrix> one_sided(EvaluatorInterface::allocateMatrix());
                status = Fold(*density, *one_sided);
                density.reset(one_sided.release());
                BuiltInFuncsUtils::CheckMathStatus(eval, status);
            }
        }
        else
        {
            if (onesided)
            {
                int fftSize2 = fftSize / 2 + 1;
                density.reset(EvaluatorInterface::allocateMatrix(fftSize2, matrix->N(), true));
                double* signal = const_cast<double*> (matrix->GetRealData());
                hwMatrix col_out(fftSize, 1, hwMatrix::REAL);
                double* response = const_cast<double*> (col_out.GetRealData());
                hwMatrix col_out2(fftSize2, 1, hwMatrix::REAL);

                for (int i = 0; i < matrix->N(); ++i)
                {
                    hwMatrix col_in(matrix->M(), 1, reinterpret_cast<void*> (signal), hwMatrix::REAL);

                    hwMathStatus status = (window) ?
                        BlockPSD(col_in, *window, numOverlapPts, sampFreq, col_out, fftSize) :
                        BlockPSD(col_in, *windowTemp, numOverlapPts, sampFreq, col_out, fftSize);

                    status = Fold(col_out, col_out2);

                    density->WriteColumn(i, col_out2);
                    signal += matrix->M();
                }
            }
            else
            {
                density.reset(EvaluatorInterface::allocateMatrix(fftSize, matrix->N(), true));
                double* response = density->GetRealData();
                double* signal = const_cast<double*> (matrix->GetRealData());

                for (int i = 0; i < matrix->N(); ++i)
                {
                    hwMatrix col_in(matrix->M(), 1, reinterpret_cast<void*> (signal), hwMatrix::REAL);
                    hwMatrix col_out(fftSize, 1, reinterpret_cast<void*> (response), hwMatrix::REAL);

                    hwMathStatus status = (window) ?
                        BlockPSD(col_in, *window, numOverlapPts, sampFreq, col_out, fftSize) :
                        BlockPSD(col_in, *windowTemp, numOverlapPts, sampFreq, col_out, fftSize);

                    signal += matrix->M();
                    response += fftSize;
                }
            }
        }
    }
    else        // ND case
    {
        throw OML_Error(OML_ERR_UNSUPPORTDIM, 1, OML_VAR_MATRIX);
        // return oml_MatrixNUtil4(eval, inputs, outputs, OmlPwelch);
    }

	// output
	int nargout = eval.GetNargoutValue();

    if (nargout != 0)
    {
        outputs.push_back(density.release());
    }

    if (nargout != 1)
    {
        std::unique_ptr<hwMatrix> freqresult(EvaluatorInterface::allocateMatrix());

        if (onesided)
        {
            hwMathStatus status = Freq(fftSize, sampFreq, *freqresult, "onesided");
        }
        else
        {
            hwMathStatus status = Freq(fftSize, sampFreq, *freqresult, "twosided");
        }
        
        if (nargout == 0)
        {
            std::vector<Currency> plotInput;
            plotInput.push_back(freqresult.release());
            plotInput.push_back(density.release());
            FUNCPTR plotPtr = eval.GetStdFunction("plot");
            return plotPtr(eval, plotInput, outputs);
        }
        else
        {
            outputs.push_back(freqresult.release());
        }
    }

    return true;
}
//------------------------------------------------------------------------------
// Computes cross power spectral density [cpsd command]
//------------------------------------------------------------------------------
bool OmlCpsd(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 2 || nargin > 7)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_VECTOR, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_VECTOR, 2, OML_VAR_TYPE);
    }

    const hwMatrix* signal1 = inputs[0].ConvertToMatrix();
    if (!signal1->IsVector())
    {
        throw OML_Error(OML_ERR_VECTOR, 1);
    }
    if (!signal1->IsRealData())
    {
        throw OML_Error(OML_ERR_REAL, 1, OML_VAR_DATA);
    }

    const hwMatrix* signal2 = inputs[1].ConvertToMatrix();
    if (!signal2->IsVector())
    {
        throw OML_Error(OML_ERR_VECTOR, 2);
    }
    if (!signal2->IsRealData())
    {
        throw OML_Error(OML_ERR_REAL, 2, OML_VAR_DATA);
    }

    // window argument
    int windowSize = -1;
    const hwMatrix* window  = nullptr;
    std::unique_ptr<hwMatrix> windowTemp(EvaluatorInterface::allocateMatrix());

    if (nargin > 2)
    {
        if (inputs[2].IsPositiveInteger())
        {
            windowSize = static_cast<int>(inputs[2].Scalar());
            windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
            hwMathStatus status = HammWin(*windowTemp, "periodic");

            if (!status.IsOk())
            {
                status.SetArg1(3);
                throw OML_Error(status);
            }
        }
        else if (inputs[2].IsMatrix())
        {
            if (!(inputs[2].Matrix()->M() == 0 && inputs[2].Matrix()->N() == 0))
            {
                window = inputs[2].Matrix();
                windowSize = window->Size();
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALARVECTOR, 3);
        }
    }

    // overlap argument
    int numOverlapPts = -1;
    if (nargin > 3)
    {
        if (inputs[3].IsInteger())
        {
            numOverlapPts = static_cast<int>(inputs[3].Scalar());
        }
        else if (inputs[3].IsMatrix())
        {
            if (!(inputs[3].Matrix()->M() == 0 && inputs[3].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_VALUE);
        }
    }

    // mangage default window size and/or overlap
    if (windowSize == -1)
    {
        // 8 blocks
        if (numOverlapPts == -1)
        {
            windowSize    = (2*signal1->Size()) / 9;
            numOverlapPts = windowSize / 2;
        }
        else
        {
            windowSize = ((signal1->Size() + 7*numOverlapPts)) / 8;
        }

        windowTemp.reset(EvaluatorInterface::allocateMatrix(windowSize, 1, true));
        hwMathStatus status = HammWin(*windowTemp, "periodic");

        if (!status.IsOk())
        {
            status.SetArg1(2);
            throw OML_Error(status);
        }
    }
    else if (numOverlapPts == -1)
    {
        numOverlapPts = windowSize / 2;
    }

    // FFT size argument
    int fftSize = windowSize;
    if (nargin > 4)
    {
        if (inputs[4].IsPositiveInteger())
        {
            fftSize = static_cast<int>(inputs[4].Scalar());
        }
        else if (inputs[4].IsMatrix())
        {
            if (!(inputs[4].Matrix()->M() == 0 && inputs[4].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_POSINTEGER, 5, OML_VAR_VALUE);
        }
    }

    // sampling frequency argument
    double sampFreq = 1.0;
    if (nargin > 5)
    {
        if (inputs[5].IsScalar())
        {
            sampFreq = inputs[5].Scalar();
        }
        else if (inputs[5].IsMatrix())
        {
            if (!(inputs[5].Matrix()->M() == 0 && inputs[5].Matrix()->N() == 0))
            {
                throw OML_Error(OML_ERR_POSINTEGER, 6, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_SCALAR, 6, OML_VAR_VALUE);
        }
    }

    //  spectrum sides argument
    bool onesided = true;
    if (nargin > 2 && inputs.back().IsString())
    {
        std::string opt = readOption(eval, inputs.back());
        if (opt == "twosided")
        {
            onesided = false;
        }
        else if (opt != "onesided")
        {
            throw OML_Error(HW_ERROR_INVALIDOPTION(opt));
        }
    }

    // calculate cpsd
    std::unique_ptr<hwMatrix> density(EvaluatorInterface::allocateMatrix());

    hwMathStatus status = (window) ?
        BlockCPSD(*signal1, *signal2, *window, numOverlapPts, sampFreq, *density, fftSize):
        BlockCPSD(*signal1, *signal2, *windowTemp, numOverlapPts, sampFreq, *density, fftSize);
    
    BuiltInFuncsUtils::CheckMathStatus(eval, status);

	// output
	int nargout = eval.GetNargoutValue();

    if (nargout == 0)
    {
        std::vector<Currency> plotInput;
		std::unique_ptr<hwMatrix> freqresult(EvaluatorInterface::allocateMatrix());
		status = Freq(fftSize, sampFreq, *freqresult, "twosided");
		BuiltInFuncsUtils::CheckMathStatus(eval, status);

	    if (onesided)
		{
			status = freqresult->Resize(fftSize/2+1, 1);
			BuiltInFuncsUtils::CheckMathStatus(eval, status);
			plotInput.push_back(freqresult.release());

	        std::unique_ptr<hwMatrix> one_sided(EvaluatorInterface::allocateMatrix());
	        status = Fold(*density, *one_sided);
		    BuiltInFuncsUtils::CheckMathStatus(eval, status);
			plotInput.push_back(one_sided.release());
		}
		else
		{
			plotInput.push_back(freqresult.release());
			plotInput.push_back(density.release());
		}

        FUNCPTR plotPtr = eval.GetStdFunction("plot");
		return plotPtr(eval, plotInput, outputs);
	}

    // process spectral output
    if (onesided)
    {
        std::unique_ptr<hwMatrix> one_sided(EvaluatorInterface::allocateMatrix());
        status = Fold(*density, *one_sided);
        BuiltInFuncsUtils::CheckMathStatus(eval, status);
        outputs.push_back(one_sided.release());
    }
    else
    {
        outputs.push_back(density.release());
    }

    // process frequency output
    if (nargout > 1)
    {
        std::unique_ptr<hwMatrix> freqresult(EvaluatorInterface::allocateMatrix());
        hwMathStatus status = Freq(fftSize, sampFreq, *freqresult, "twosided");
	    BuiltInFuncsUtils::CheckMathStatus(eval, status);

        if (onesided)
        {
	        hwMathStatus status = freqresult->Resize(fftSize/2+1, 1);
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        if (freqresult->M() == 1)
        {
            status = freqresult->Transpose();
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
        outputs.push_back(freqresult.release());
    }

    return true;
}
//------------------------------------------------------------------------------
// Computes estimated transfer function [tfestimate command]
//------------------------------------------------------------------------------
bool OmlTFestimate(EvaluatorInterface           eval, 
				   const std::vector<Currency>& inputs, 
				   std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    int nargout = eval.GetNargoutValue();

    std::vector<Currency> inputs_copy = inputs;
    std::string estimator = "H1";

    if (nargin == 9)
    {
        if (!inputs[7].IsString() || inputs[7].StringVal() != "Estimator")
        {
            throw OML_Error(OML_ERR_ESTIMATOR_STR, 8);
        }

        if (!inputs[8].IsString())
        {
            throw OML_Error(OML_ERR_ESTIMATOR_TYPE, 9, OML_VAR_STRING);
        }

        estimator = inputs[8].StringVal();
        inputs_copy.erase(inputs_copy.end() - 1);
        inputs_copy.erase(inputs_copy.end() - 1);
    }

    std::vector<Currency> outputs1;
    hwMatrix* tf = new hwMatrix;

    if (estimator == "H1")
    {
        // cpsd(x,y)
        if (nargout)
            outputs1 = eval.DoMultiReturnFunctionCall(OmlCpsd, inputs_copy, static_cast<int>(inputs_copy.size()), nargout, true);
        else
            outputs1 = eval.DoMultiReturnFunctionCall(OmlCpsd, inputs_copy, static_cast<int>(inputs_copy.size()), 2, true);

        // psd(x)
        inputs_copy.erase(inputs_copy.begin() + 1); // remove y
        std::vector<Currency> outputs2;
        outputs2 = eval.DoMultiReturnFunctionCall(OmlPwelch, inputs_copy, static_cast<int>(inputs_copy.size()), 1, true);

        // tfestimate
        hwMatrix* Pxy = outputs1[0].GetWritableMatrix();
        hwMatrix* Pxx = outputs2[0].GetWritableMatrix();
        MKLutilitiesD::DivideByElems(*Pxy, *Pxx, *tf);
    }
    else if (estimator == "H2")
    {
        // psd(y)
        inputs_copy.erase(inputs_copy.begin()); // remove x

        if (nargout)
            outputs1 = eval.DoMultiReturnFunctionCall(OmlPwelch, inputs_copy, static_cast<int>(inputs_copy.size()), nargout, true);
        else
            outputs1 = eval.DoMultiReturnFunctionCall(OmlPwelch, inputs_copy, static_cast<int>(inputs_copy.size()), 2, true);

        // cpsd(y,x)
        inputs_copy.insert(inputs_copy.begin() + 1, 1, inputs[0]);  // reinsert x after y
        std::vector<Currency> outputs2;
        outputs2 = eval.DoMultiReturnFunctionCall(OmlCpsd, inputs_copy, static_cast<int>(inputs_copy.size()), 1, true);

        // tfestimate
        hwMatrix* Pyy = outputs1[0].GetWritableMatrix();
        hwMatrix* Pyx = outputs2[0].GetWritableMatrix();
        MKLutilitiesD::DivideByElems(*Pyy, *Pyx, *tf);
    }
    else if (estimator == "H3")
    {
        // cpsd(x,y)
        if (nargout)
            outputs1 = eval.DoMultiReturnFunctionCall(OmlCpsd, inputs_copy, static_cast<int>(inputs_copy.size()), nargout, true);
        else
            outputs1 = eval.DoMultiReturnFunctionCall(OmlCpsd, inputs_copy, static_cast<int>(inputs_copy.size()), 2, true);

        // psd(x)
        inputs_copy.erase(inputs_copy.begin() + 1); // remove y
        std::vector<Currency> outputs2;
        outputs2 = eval.DoMultiReturnFunctionCall(OmlPwelch, inputs_copy, static_cast<int>(inputs_copy.size()), 1, true);

        // psd(y)
        inputs_copy.erase(inputs_copy.begin()); // remove x
        inputs_copy.insert(inputs_copy.begin(), 1, inputs[1]);  // reinsert y
        std::vector<Currency> outputs4;
        outputs4 = eval.DoMultiReturnFunctionCall(OmlPwelch, inputs_copy, static_cast<int>(inputs_copy.size()), 1, true);

        // |cpsd(x,y)|
        std::vector<Currency> inputs5;
        inputs5.push_back(outputs1[0]);
        std::vector<Currency> outputs5;
        BuiltInFuncsMKL::Abs(eval, inputs5, outputs5);

        //// tfestimate
        hwMatrix* Pxy = outputs1[0].GetWritableMatrix();
        hwMatrix* Pxx = outputs2[0].GetWritableMatrix();
        hwMatrix* Pyy = outputs4[0].GetWritableMatrix();
        hwMatrix* AbsPxy = outputs5[0].GetWritableMatrix();

        // sqrt(psd(y) / psd(x))
        hwMatrix* psdRatio = new hwMatrix;
        MKLutilitiesD::DivideByElems(*Pyy, *Pxx, *psdRatio);
        std::vector<Currency> inputs6;
        inputs6.push_back(psdRatio);
        std::vector<Currency> outputs6;
        BuiltInFuncsMKL::Sqrt(eval, inputs6, outputs6);
        hwMatrix* psdSqrtRatio = outputs6[0].GetWritableMatrix();

        // cpsd(x, y) / |cpsd(x,y)|
        hwMatrix* PxyNormal = new hwMatrix;
        MKLutilitiesD::DivideByElems(*Pxy, *AbsPxy, *PxyNormal);

        // sqrt(psd(y) / psd(x)) * cpsd(x, y) / |cpsd(x,y)|
        MKLutilitiesD::MultByElems(*psdSqrtRatio, *PxyNormal, *tf);
    }
    else
    {
        throw OML_Error(OML_ERR_ESTIMATOR_TYPE, 9, OML_VAR_STRING);
    }
        
	if (nargout == 0)
    {
		std::vector<Currency> inputs2;
		std::vector<Currency> outputs2;
        inputs2.push_back(tf);
        BuiltInFuncsMKL::Abs(eval, inputs2, outputs2);

        std::vector<Currency> plotInput;
        plotInput.push_back(outputs1[1]);
        plotInput.push_back(outputs2[0]);
        FUNCPTR plotPtr = eval.GetStdFunction("plot");
        return plotPtr(eval, plotInput, outputs);
    }

	outputs.push_back(tf);

	if (nargout > 1)
	    outputs.push_back(outputs1[1]);

    return true;
}
//------------------------------------------------------------------------------
// Computes estimated mean square coherence [mscohere command]
//------------------------------------------------------------------------------
bool OmlMScohere(EvaluatorInterface           eval, 
				 const std::vector<Currency>& inputs, 
				 std::vector<Currency>&       outputs)
{
	// cpsd(x,y)
	int nargout = eval.GetNargoutValue();
	std::vector<Currency> inputs1 = inputs;
	std::vector<Currency> outputs1;

	if (nargout)
		outputs1 = eval.DoMultiReturnFunctionCall(OmlCpsd, inputs1, static_cast<int>(inputs1.size()), nargout, true);
	else
		outputs1 = eval.DoMultiReturnFunctionCall(OmlCpsd, inputs1, static_cast<int>(inputs1.size()), 2, true);

	hwMatrix* cpsd  = outputs1[0].GetWritableMatrix();

	// psd(x)
	std::vector<Currency> inputs2 = inputs;
	inputs2.erase(inputs2.begin() + 1);
	std::vector<Currency> outputs2;
	OmlPwelch(eval, inputs2, outputs2);
    outputs2 = eval.DoMultiReturnFunctionCall(OmlPwelch, inputs2, static_cast<int>(inputs2.size()), 1, true);
	hwMatrix* psdx  = outputs2[0].GetWritableMatrix();

	// psd(y)
	std::vector<Currency> inputs3 = inputs;
	inputs3.erase(inputs3.begin());
	std::vector<Currency> outputs3;
	OmlPwelch(eval, inputs3, outputs3);
    outputs3 = eval.DoMultiReturnFunctionCall(OmlPwelch, inputs3, static_cast<int>(inputs3.size()), 1, true);
	hwMatrix* psdy  = outputs3[0].GetWritableMatrix();

	// cpsd(x,y) conjugate
	std::vector<Currency> inputs4;
	std::vector<Currency> outputs4;
	inputs4.push_back(outputs1[0]);
	oml_conj(eval, inputs4, outputs4);
	hwMatrix* cpsdC = outputs4[0].GetWritableMatrix();

	// cpsd(x,y) squared magnitude
	hwMatrix temp;
	hwMatrix cpsdSq;
	BuiltInFuncsUtils::CheckMathStatus(eval, temp.MultByElems(*cpsd, *cpsdC));
	BuiltInFuncsUtils::CheckMathStatus(eval, temp.UnpackComplex(&cpsdSq, nullptr));

	// mscohere
	temp.Dimension(0, 0, hwMatrix::REAL);
	hwMatrix* msc = new hwMatrix;
	BuiltInFuncsUtils::CheckMathStatus(eval, temp.DivideByElems(cpsdSq, *psdx));
	BuiltInFuncsUtils::CheckMathStatus(eval, msc->DivideByElems(temp, *psdy));
	
	if (nargout == 0)
    {
        std::vector<Currency> plotInput;
        plotInput.push_back(outputs1[1]);
        plotInput.push_back(msc);
        FUNCPTR plotPtr = eval.GetStdFunction("plot");
        return plotPtr(eval, plotInput, outputs);
    }
	
	outputs.push_back(msc);

	if (nargout > 1)
	    outputs.push_back(outputs1[1]);

	return true;
}
//------------------------------------------------------------------------------
// Computes digital filter frequency response values [freqz command]
//------------------------------------------------------------------------------
bool OmlFreqz(EvaluatorInterface           eval, 
              const std::vector<Currency>& inputs, 
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 3 || nargin > 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 2, OML_VAR_TYPE);
    }
    if (!inputs[2].IsMatrix() && !inputs[2].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 3, OML_VAR_TYPE);
    }

    const hwMatrix* numerCoef = inputs[0].ConvertToMatrix();
    const hwMatrix* denomCoef = inputs[1].ConvertToMatrix();
    const hwMatrix* freqVec   = inputs[2].ConvertToMatrix();
    double sampFreq           = 1.0;

    if (nargin > 3)
    {
        if (!inputs[3].IsScalar())
        {
            throw OML_Error(OML_ERR_SCALAR, 4);
        }
        sampFreq = inputs[3].Scalar();
    }

    std::unique_ptr<hwMatrix> response(EvaluatorInterface::allocateMatrix());
    const hwMatrix* pDenomCoef = (!denomCoef->IsEmpty()) ? denomCoef : nullptr;

    hwMathStatus status = Response(*numerCoef, pDenomCoef, *freqVec, *response, &sampFreq);

    if (!status.IsOk())
    {
        if (status.IsWarning())
        {
            status(HW_MATH_WARN_NYQUIST, 3, 4);
            BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
        }
        else
        {
            if (status.GetArg1() == 5)
            {
                status.SetArg1(4);
            }
            else if (!(status.GetArg1() < 4))
            {
                status.ResetArgs();
            }
            throw OML_Error(status);
        }
    }

    int nargout = eval.GetNargoutValue();
    int numPnts = freqVec->Size();
    hwMatrix* freq = nullptr;

    if (nargout != 1)
    {
        freq = (hwMatrix*) freqVec;
        freq->IncrRefCount();
    }

    if (nargout == 0)
    {
        std::unique_ptr<hwMatrix> mag(EvaluatorInterface::allocateMatrix(1, numPnts, true));
        std::unique_ptr<hwMatrix> phase(EvaluatorInterface::allocateMatrix(1, numPnts, true));

        for (int i = 0; i < numPnts; ++i)
        {
            (*mag)(i) = (*response).z(i).Mag();
            (*phase)(i) = (*response).z(i).Arg();
        }

        // get plot function pointers
        FUNCPTR plotPtr = eval.GetStdFunction("plot");
        FUNCPTR subplotPtr = eval.GetStdFunction("subplot");
        FUNCPTR xLabelPtr = eval.GetStdFunction("xlabel");
        FUNCPTR yLabelPtr = eval.GetStdFunction("ylabel");

        // create first subplot
        std::vector<Currency> dummyOutput;
        std::vector<Currency> subplotInput;
        subplotInput.push_back(2);
        subplotInput.push_back(1);
        subplotInput.push_back(1);
        subplotPtr(eval, subplotInput, dummyOutput);

        // plot magnitude
        std::vector<Currency> plotInput;
        plotInput.push_back(freq);
        plotInput.push_back(mag.release());
        plotPtr(eval, plotInput, dummyOutput);

        // label axes
        std::vector<Currency> x_label;
        std::vector<Currency> y_label;
        x_label.push_back("Frequency (Hz)");
        y_label.push_back("Magnitude");
        dummyOutput.clear();
        xLabelPtr(eval, x_label, dummyOutput);
        dummyOutput.clear();
        yLabelPtr(eval, y_label, dummyOutput);

        // create second subplot
        subplotInput[2] = 2;
        subplotPtr(eval, subplotInput, dummyOutput);

        // plot phase
        plotInput.pop_back();
        plotInput.push_back(phase.release());
        dummyOutput.clear();
        plotPtr(eval, plotInput, dummyOutput);

        // label axes
        y_label.clear();
        y_label.push_back("Phase (radians)");
        dummyOutput.clear();
        xLabelPtr(eval, x_label, dummyOutput);
        dummyOutput.clear();
        yLabelPtr(eval, y_label, dummyOutput);

        return true;
    }
    else
    {
        outputs.push_back(response.release());

        if (nargout > 1)
            outputs.push_back(freq);
    }

    return true;
}
//------------------------------------------------------------------------------
// Computes analog filter frequency response values [freqs command]
//------------------------------------------------------------------------------
bool OmlFreqs(EvaluatorInterface           eval,
              const std::vector<Currency>& inputs, 
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 1, OML_VAR_DATA);
    }
    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 2, OML_VAR_DATA);
    }
    if (!inputs[2].IsMatrix() && !inputs[2].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 3, OML_VAR_DATA);
    }

    const hwMatrix* numerCoef = inputs[0].ConvertToMatrix();
    const hwMatrix* denomCoef = inputs[1].ConvertToMatrix();
    const hwMatrix* freqVec   = inputs[2].ConvertToMatrix();  // angular

    std::unique_ptr<hwMatrix> response(EvaluatorInterface::allocateMatrix());

    const hwMatrix* pDenomCoef = (!denomCoef->IsEmpty()) ?
                                 denomCoef : nullptr;

    hwMathStatus status = Response(*numerCoef, pDenomCoef, *freqVec, *response);

    if (!status.IsOk())
    {
        if (status.IsWarning())
        {
            BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
        }
        else
        {
            if (status.GetArg1() >= 4)
            {
                status.ResetArgs();
            }
            throw OML_Error(status);
        }
    }

    int nargout = eval.GetNargoutValue();
    int numPnts = freqVec->Size();
    hwMatrix* omega = nullptr;

    if (nargout != 1)
    {
        omega = (hwMatrix*) freqVec;
        omega->IncrRefCount();
    }

    if (nargout == 0)
    {
        std::unique_ptr<hwMatrix> mag(EvaluatorInterface::allocateMatrix(1, numPnts, true));
        std::unique_ptr<hwMatrix> phase(EvaluatorInterface::allocateMatrix(1, numPnts, true));

        for (int i = 0; i < numPnts; ++i)
        {
            (*mag)(i)   = (*response).z(i).Mag();
            (*phase)(i) = (*response).z(i).Arg();
        }

        // get plot function pointers
        FUNCPTR plotPtr    = eval.GetStdFunction("plot");
        FUNCPTR subplotPtr = eval.GetStdFunction("subplot");
        FUNCPTR xLabelPtr  = eval.GetStdFunction("xlabel");
        FUNCPTR yLabelPtr  = eval.GetStdFunction("ylabel");

        // create first subplot
        std::vector<Currency> dummyOutput;
        std::vector<Currency> subplotInput;
        subplotInput.push_back(2);
        subplotInput.push_back(1);
        subplotInput.push_back(1);
        subplotPtr(eval, subplotInput, dummyOutput);

        // plot magnitude
        std::vector<Currency> plotInput;
        plotInput.push_back(omega);
        plotInput.push_back(mag.release());
        plotPtr(eval, plotInput, dummyOutput);

        // label axes
        std::vector<Currency> x_label;
        std::vector<Currency> y_label;
        x_label.push_back("Angular Frequency (radians/second)");
        y_label.push_back("Magnitude");
        dummyOutput.clear();
        xLabelPtr(eval, x_label, dummyOutput);
        dummyOutput.clear();
        yLabelPtr(eval, y_label, dummyOutput);

        // create second subplot
        subplotInput[2] = 2;
        subplotPtr(eval, subplotInput, dummyOutput);

        // plot phase
        plotInput.pop_back();
        plotInput.push_back(phase.release());
        dummyOutput.clear();
        plotPtr(eval, plotInput, dummyOutput);

        // label axes
        y_label.clear();
        y_label.push_back("Phase (radians)");
        dummyOutput.clear();
        xLabelPtr(eval, x_label, dummyOutput);
        dummyOutput.clear();
        yLabelPtr(eval, y_label, dummyOutput);

        return true;
    }
    else
    {
        outputs.push_back(response.release());

        if (nargout > 1)
            outputs.push_back(omega);
    }

    return true;
}
//------------------------------------------------------------------------------
// Computes digital filter coefficients from frequency response values [invfreqz]
//------------------------------------------------------------------------------
bool OmlInvFreqz(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin  = inputs.size();
    int    nargout = eval.GetNargoutValue();
    
    if (nargin != 4 && nargin != 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }
    if (nargout != 2)
    {
        throw OML_Error(OML_ERR_NUMARGOUT);
    }
    if (!inputs[0].IsMatrix())
    {
        throw OML_Error(OML_ERR_VECTOR, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsMatrix())
    {
        throw OML_Error(OML_ERR_VECTOR, 2, OML_VAR_TYPE);
    }
    if (!inputs[2].IsInteger())
    {
        throw OML_Error(OML_ERR_NATURALNUM, 3, OML_VAR_ORDER);
    }
    if (!inputs[3].IsInteger())
    {
        throw OML_Error(OML_ERR_NATURALNUM, 4, OML_VAR_ORDER);
    }

    int numerOrder = static_cast<int>(inputs[2].Scalar());
    if (numerOrder < 0)
    {
        throw OML_Error(OML_ERR_NATURALNUM, 3, OML_VAR_ORDER);
    }

    int denomOrder = static_cast<int>(inputs[3].Scalar());
    if (denomOrder < 0)
    {
        throw OML_Error(OML_ERR_NATURALNUM, 4, OML_VAR_ORDER);
    }

    const hwMatrix* response   = inputs[0].Matrix();
    const hwMatrix* freq       = inputs[1].Matrix();
    const hwMatrix* weight     = nullptr;
    if (nargin == 5)
    {
        if (!inputs[4].IsMatrix())
        {
            throw OML_Error(OML_ERR_VECTOR, 5, OML_VAR_TYPE);
        }
        weight = inputs[4].Matrix();
    }

    std::unique_ptr<hwMatrix> numer(
        EvaluatorInterface::allocateMatrix(1, numerOrder+1, true));
    std::unique_ptr<hwMatrix> denom(
        EvaluatorInterface::allocateMatrix(1, denomOrder+1, true));

    hwMathStatus status = FilterID(*response, *freq, *numer, *denom, weight, "z");
    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(numer.release());
    outputs.push_back(denom.release());

    return true;
}
//------------------------------------------------------------------------------
// Computes analog filter coefficients from frequency response values [invfreqs]
//------------------------------------------------------------------------------
bool OmlInvFreqs(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin  = inputs.size();
    int    nargout = eval.GetNargoutValue();
    
    if (nargin != 4 && nargin != 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }
    if (nargout != 2)
    {
        throw OML_Error(OML_ERR_NUMARGOUT);
    }
    if (!inputs[0].IsMatrix())
    {
        throw OML_Error(OML_ERR_VECTOR, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsMatrix())
    {
        throw OML_Error(OML_ERR_VECTOR, 2, OML_VAR_TYPE);
    }
    if (!inputs[2].IsInteger())
    {
        throw OML_Error(OML_ERR_NATURALNUM, 3, OML_VAR_ORDER);
    }
    if (!inputs[3].IsInteger())
    {
        throw OML_Error(OML_ERR_NATURALNUM, 4, OML_VAR_ORDER);
    }

    int numerOrder = static_cast<int>(inputs[2].Scalar());
    if (numerOrder < 0)
    {
        throw OML_Error(OML_ERR_NATURALNUM, 3, OML_VAR_ORDER);
    }

    int denomOrder = static_cast<int>(inputs[3].Scalar());
    if (denomOrder < 0)
    {
        throw OML_Error(OML_ERR_NATURALNUM, 4, OML_VAR_ORDER);
    }

    const hwMatrix* response = inputs[0].Matrix();
    const hwMatrix* freq     = inputs[1].Matrix();
    const hwMatrix* weight   = nullptr;
    if (nargin == 5)
    {
        if (!inputs[4].IsMatrix())
        {
            throw OML_Error(OML_ERR_VECTOR, 5, OML_VAR_TYPE);
        }
        weight = inputs[4].Matrix();
    }

    std::unique_ptr<hwMatrix> numer(
        EvaluatorInterface::allocateMatrix(1, numerOrder+1, true));
    std::unique_ptr<hwMatrix> denom(
        EvaluatorInterface::allocateMatrix(1, denomOrder+1, true));

    hwMathStatus status = FilterID(*response, *freq, *numer, *denom, weight, "s");
    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(numer.release());
    outputs.push_back(denom.release());

    return true;
}
//------------------------------------------------------------------------------
// Computes the impulse response of a digital filter [impz command]
//------------------------------------------------------------------------------
bool OmlImpz(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 2 || nargin > 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }
    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 1, OML_VAR_DATA);
    }
    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 2, OML_VAR_DATA);
    }

    const hwMatrix* numerCoef = inputs[0].ConvertToMatrix();
    const hwMatrix* denomCoef = inputs[1].ConvertToMatrix();
    if (denomCoef->M() == 0 && denomCoef->N() == 0)
    {
        denomCoef = nullptr;
    }

    int numPnts = 0;
    if (nargin > 2)
    {
        if (inputs[2].IsInteger())
        {
            numPnts = static_cast<int>(inputs[2].Scalar());
        }
        else if (inputs[2].IsMatrix())
        {
            const hwMatrix* sf = inputs[2].Matrix();
            if (!(sf->M() == 0 && sf->N() == 0))
            {
                throw OML_Error(OML_ERR_INTEGER, 3, OML_VAR_VALUE);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_INTEGER, 3, OML_VAR_VALUE);
        }
    }

    double sampFreq = 1.0;
    if (nargin > 3)
    {
        if (!inputs[3].IsScalar())
        {
            throw OML_Error(OML_ERR_SCALAR, 4);
        }
        sampFreq = inputs[3].Scalar();
    }

    std::unique_ptr<hwMatrix> impulseRes(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> time(EvaluatorInterface::allocateMatrix());

    hwMathStatus status;
    int          nargout = eval.GetNargoutValue();
    switch (nargout)
    {
        case 0:
        case 2:
            status = ImpulseRes(*numerCoef, denomCoef, *impulseRes, time.get(), 
                                numPnts, sampFreq);
            break;

        case 1:
            status = ImpulseRes(*numerCoef, denomCoef, *impulseRes, NULL,
                                numPnts, sampFreq);
            break;

        default:    
            throw OML_Error(OML_ERR_NUMARGOUT);
            break;
    }

    if (!status.IsOk())
    {
        switch (status.GetArg1())
        {
            case 1: 
            case 2: break;
            
            case 6: status.SetArg1(3); break;

            case 7: status.SetArg1(4); break;
        
            default: status.ResetArgs(); break;
        }

        throw OML_Error(status);
    }

    if (impulseRes->M() != 1)
    {
        impulseRes->Transpose();
    }

    if (nargout != 1 && time->M() != 1)
    {
        time->Transpose();
    }

    if (nargout == 0)
    {
        std::vector<Currency> plotInput;
        plotInput.push_back(time.release());
        plotInput.push_back(impulseRes.release());
        FUNCPTR plotPtr = eval.GetStdFunction("plot");
        return plotPtr(eval, plotInput, outputs);
    }
    else
    {
        outputs.push_back(impulseRes.release());
        if (nargout > 1)
        {
            outputs.push_back(time.release());
        }
    }

    return true;
}
//------------------------------------------------------------------------------
// Creates a digital FIR filter [fir1 command]
//------------------------------------------------------------------------------
bool OmlFir1(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 2 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // process argument 1, the filter order argument
    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(inputs[0].Scalar());

    // process type argument 3
    hwMathStatus status;
    std::string  arg3S;
    int          omitType = 0;

    if (nargin > 2)
    {
        if (inputs[2].IsString())
        {
            arg3S = inputs[2].StringVal();
        }
        else if (inputs[2].IsMatrix())
        {
            omitType = 1;
        }
        else
        {
            throw OML_Error(OML_ERR_STRINGVECTOR, 3, OML_VAR_TYPE);
        }
    }

    // process window argument 4
    const hwMatrix* window  = nullptr;
    std::unique_ptr<hwMatrix> windowTemp(EvaluatorInterface::allocateMatrix());

    if (nargin > 3-omitType)
    {
        if (!inputs[3-omitType].IsMatrix())
        {
            throw OML_Error(OML_ERR_VECTOR, 4-omitType, OML_VAR_TYPE);
        }
        window = inputs[3-omitType].ConvertToMatrix();

        if (window->M() == 0 && window->N() == 0)
        {
            window = nullptr;
            windowTemp.reset(EvaluatorInterface::allocateMatrix(order+1, 1, true));
            status = HammWin(*windowTemp, "symmetric");
            if (!status.IsOk())
            {
                status.ResetArgs();
                throw OML_Error(status);
            }
        }
    }
    else
    {
        windowTemp.reset(EvaluatorInterface::allocateMatrix(order+1, 1, true));
        status = HammWin(*windowTemp, "symmetric");
        if (!status.IsOk())
        {
            status.ResetArgs();
            throw OML_Error(status);
        }
    }

    // process noscale argument 5
    bool normalize = true;
    if (nargin > 4-omitType)
    {
        if (!inputs[4-omitType].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 5-omitType);
        }

        if (inputs[4-omitType].StringVal() == "noscale")
        {
            normalize = false;
        }
        else
        {
            throw OML_Error(OML_ERR_OPTIONVAL, 5-omitType, OML_VAR_STRING);
        }
    }

    // process argument 2 and filter
    std::unique_ptr<hwMatrix> numerCoef(EvaluatorInterface::allocateMatrix());

    if (inputs[1].IsScalar())
    {
        double cutoffFreq = inputs[1].Scalar();

        if (arg3S.empty() || arg3S == "low")
        {
            status = (window) ? 
                Fir(order, 0.0, cutoffFreq, window, *numerCoef, normalize):
                Fir(order, 0.0, cutoffFreq, windowTemp.get(), *numerCoef, normalize);

            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 5)
            {
                status.ResetArgs();
            }
        }
        else if (arg3S == "high")
        {
            if (!window && order%2 == 1)
            {
                BuiltInFuncsUtils::SetWarning(eval, "Warning: odd filter order in argument 1 will be incremented to satisfy filter requirement");
                order++;
                windowTemp.reset(EvaluatorInterface::allocateMatrix(order+1, 1, true));
                status = HammWin(*windowTemp, "symmetric");
            }

            status = (window) ?
                Fir(order, cutoffFreq, 0.0, window, *numerCoef, normalize) :
                Fir(order, cutoffFreq, 0.0, windowTemp.get(), *numerCoef, normalize);

            if (status.GetArg1() == 5)
            {
                status.ResetArgs();
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 3));
        }

        if (!status.IsOk())
        {
            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() > 4)
            {
                status.ResetArgs();
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else if (inputs[1].IsMatrix() && inputs[1].Matrix()->Size() == 2)
    {
        const hwMatrix* cutoffFreq = inputs[1].Matrix();
        if (!cutoffFreq->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }

        if ((*cutoffFreq)(0) >= (*cutoffFreq)(1))
        {
            throw OML_Error(status(HW_MATH_ERR_NONINCREASE, 2));
        }

        if (arg3S.empty() || arg3S == "bandpass")
        {
            status = (window) ?
                Fir(order, (*cutoffFreq)(0), (*cutoffFreq)(1), window, *numerCoef, normalize):
                Fir(order, (*cutoffFreq)(0), (*cutoffFreq)(1), windowTemp.get(), *numerCoef, normalize);

            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 5)
            {
                status.ResetArgs();
            }
        }
        else if (arg3S == "stop")
        {
            if (!window && order%2 == 1)
            {
                BuiltInFuncsUtils::SetWarning(eval, 
                    "Warning: odd filter order in argument 1 will be incremented to satisfy filter requirement");
                order++;
                windowTemp.reset(EvaluatorInterface::allocateMatrix(order+1, 1, true));
                status = HammWin(*windowTemp, "symmetric");
            }

            status = (window) ? 
                Fir(order, (*cutoffFreq)(1), (*cutoffFreq)(0), window, *numerCoef, normalize) :
                Fir(order, (*cutoffFreq)(1), (*cutoffFreq)(0), windowTemp.get(), *numerCoef, normalize);

            if (status.GetArg1() == 5)
            {
                status.ResetArgs();
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 3));
        }

        if (!status.IsOk())
        {
            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() > 4)
            {
                status.ResetArgs();
            }
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        // check to see if digital filter was simplified
        if (!arg3S.empty() && arg3S != "stop")  // band pass
        {
            if ((*cutoffFreq)(0) == 0.0)
            {
                status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
            }
            else if ((*cutoffFreq)(1) == 1.0)
            {
                status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
            }
        }
        else                        // band stop
        {
            if ((*cutoffFreq)(0) == 0.0)
            {
                status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
            }
            else if ((*cutoffFreq)(1) == 1.0)
            {
                status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
            }
        }
    }
    else if (inputs[1].IsMatrix())
    {
        const hwMatrix* cutoffFreq = inputs[1].Matrix();

        if (!window && order%2 == 1)
        {
            if (arg3S.empty() || arg3S == "DC-0")
            {
                if (cutoffFreq->Size()%2 == 1)
                {
                    BuiltInFuncsUtils::SetWarning(eval, 
                        "Warning: odd filter order in argument 1 will be incremented to satisfy filter requirement");
                    order++;
                    windowTemp.reset(EvaluatorInterface::allocateMatrix(order+1, 1, true));
                    status = HammWin(*windowTemp, "symmetric");
                }
            }
            else if (arg3S == "DC-1")
            {
                BuiltInFuncsUtils::SetWarning(eval, 
                    "Warning: odd filter order in argument 1 will be incremented to satisfy filter requirement");
                order++;
                windowTemp.reset(EvaluatorInterface::allocateMatrix(order+1, 1, true));
                status = HammWin(*windowTemp, "symmetric");
            }
        }

        if (arg3S.empty() || arg3S == "DC-0" || arg3S == "DC-1")
        {
            status = (window) ?
                Fir(order, *cutoffFreq, window, *numerCoef, arg3S.c_str(), normalize):
                Fir(order, *cutoffFreq, windowTemp.get(), *numerCoef, arg3S.c_str(), normalize);

            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 5)
            {
                status.ResetArgs();
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 2));
        }

        if (!status.IsOk())
        {
            if (status.GetArg1() == 3)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() > 3)
            {
                status.ResetArgs();
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else
    {
        throw OML_Error(OML_ERR_SCALARVECTOR, 2, OML_VAR_TYPE);
    }

    numerCoef->Transpose();     // make row vector
    outputs.push_back(numerCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Creates a digital FIR filter [firls command]
//------------------------------------------------------------------------------
bool OmlFirls(EvaluatorInterface           eval,
              const std::vector<Currency>& inputs,
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 3 || nargin > 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& cur1 = inputs[0];
    if (!cur1.IsInteger()) // Process arg 1 - filter order
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(cur1.Scalar());

    const Currency cur2 = inputs[1];
    if (!cur2.IsScalar() && !cur2.IsMatrix())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 2, OML_VAR_TYPE);
    }
    const hwMatrix* freq = cur2.Matrix();

    const Currency cur3 = inputs[2];
    if (!cur3.IsScalar() && !cur3.IsMatrix())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 3, OML_VAR_TYPE);
    }
    const hwMatrix* mag = cur3.Matrix();

    const hwMatrix* weight = nullptr;
    if (nargin == 4)
    {
        const Currency cur4 = inputs[3];
        if (!cur4.IsScalar() && !cur4.IsMatrix())
        {
            throw OML_Error(OML_ERR_REALVECTOR, 4, OML_VAR_TYPE);
        }
        weight = cur4.Matrix();
    }

    std::unique_ptr<hwMatrix> filterCoef(EvaluatorInterface::allocateMatrix());
    BuiltInFuncsUtils::CheckMathStatus(eval, FirLS(order, *freq, *mag, weight, *filterCoef));
    outputs.push_back(filterCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Create a Butterworth filter [butter command]
//------------------------------------------------------------------------------
bool OmlButter(EvaluatorInterface           eval, 
               const std::vector<Currency>& inputs,
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 2 || nargin > 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& cur1 = inputs[0];
    if (!cur1.IsPositiveInteger()) // Process arg 1 - filter order
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(cur1.Scalar());

    const Currency& cur2 = inputs[1];
    if (!cur2.IsScalar() && !cur2.IsMatrix())
    {
        throw OML_Error(OML_ERR_REAL, 2, OML_VAR_TYPE);
    }

    std::string arg3S;  // Process arg 3
    if (nargin > 2)
    {
        if (!inputs[2].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 3);
        }
        arg3S = inputs[2].StringVal();
    }

    std::string arg4S; // Process arg 4
    if (nargin > 3)
    {
        if (!inputs[3].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 4);
        }
        arg4S = inputs[3].StringVal();
    }

    hwMathStatus status;

    std::unique_ptr<hwMatrix> numerCoef(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> denomCoef(EvaluatorInterface::allocateMatrix());

    if (cur2.IsScalar())
    {
        double cutoffFreq = cur2.Scalar();

        if (arg3S == "s" || arg4S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (arg3S.empty() || arg3S == "low")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;

            status = Butter(order, 0.0, cutoffFreq, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg3S == "high")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Butter(order, cutoffFreq, 0.0, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg4S.empty())
        {
            status = Butter(order, 0.0, cutoffFreq, *numerCoef, *denomCoef, arg3S.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(33);
            }
        }
        else
        {
            if (!arg3S.empty() && arg3S != "s" && arg3S != "z")     // arg3 is junk
            {
                status(HW_MATH_ERR_FILTERTYPE, 3);
                if (!arg4S.empty() && arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status.SetArg2(4);
                }
            }
            else if (!arg4S.empty())
            {
                if (arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                else  // arg3, arg4 both good, but conflict
                {
                    status(HW_MATH_ERR_FILTERTYPE, 3, 4);   
                }
            }

            throw OML_Error(status);
        }

        if (!status.IsOk())
        {
            if (status == HW_MATH_ERR_FILTERFREQS_EQ)
            {
                if (arg3S == "s" || arg4S == "s")
                {
                    status(HW_MATH_ERR_FILTERFREQ_A, 2);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERFREQ_D, 2);
                }
            }
            else if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 33)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() > 3)
            {
                status.ResetArgs();
            }
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else
    {
        hwMatrix cutoffFreq(*cur2.Matrix());

        if (arg3S == "s" || arg4S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (cutoffFreq.Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 2));
        }
        if (!cutoffFreq.IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }

        if (cutoffFreq(0) >= cutoffFreq(1))
        {
            throw OML_Error(status(HW_MATH_ERR_NONINCREASE, 2));
        }

        if (arg3S.empty() || arg3S == "bandpass")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Butter(order, cutoffFreq(0), cutoffFreq(1), *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg3S == "stop")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Butter(order, cutoffFreq(1), cutoffFreq(0), *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg4S.empty())
        {
            status = Butter(order, cutoffFreq(0), cutoffFreq(1), *numerCoef, 
                            *denomCoef, arg3S.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(33);
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 3, 4));
        }

        if (!status.IsOk())
        {
            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 33)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() > 3)
            {
                status.ResetArgs();
            }

            if (!arg3S.empty() && arg3S != "s" && arg3S != "z")     // arg3 is junk
            {
                if (arg3S != "bandpass" && arg3S != "stop")
                {
                    status(HW_MATH_ERR_FILTERTYPE, 3);
                }
                if (!arg4S.empty() && arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status.SetArg2(4);
                }
            }
            else if (!arg4S.empty())
            {
                if (arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                else
                {   // arg3, arg4 both good, but conflict
                    status(HW_MATH_ERR_FILTERTYPE, 3, 4);  
                }
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        // check to see if digital filter was simplified
        bool isDigital = (arg3S == "s" || arg4S == "s") ? false : true;
        if (isDigital)
        {
            if (!arg3S.empty() && arg3S != "stop")  // band pass
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
            else                        // band stop
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
        }
    }

    // make row vectors
    numerCoef->Transpose();
    denomCoef->Transpose();

    outputs.push_back(numerCoef.release());
    outputs.push_back(denomCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Creates a Bessel filter [besself command]
//------------------------------------------------------------------------------
bool OmlBesself(EvaluatorInterface           eval, 
                const std::vector<Currency>& inputs, 
                std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 2 || nargin > 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& cur1 = inputs[0];
    if (!cur1.IsPositiveInteger()) // Process arg 1 - filter order
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(cur1.Scalar());

    const Currency& cur2 = inputs[1];
    if (!cur2.IsScalar() && !cur2.IsMatrix())
    {
        throw OML_Error(OML_ERR_REAL, 2, OML_VAR_TYPE);
    }

    std::string arg3S;  // Process arg 3
    if (nargin > 2)
    {
        if (!inputs[2].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 3);
        }
        arg3S = inputs[2].StringVal();
    }

    std::string arg4S; // Process arg 4
    if (nargin > 3)
    {
        if (!inputs[3].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 4);
        }
        arg4S = inputs[3].StringVal();
    }

    hwMathStatus status;

    std::unique_ptr<hwMatrix> numerCoef(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> denomCoef(EvaluatorInterface::allocateMatrix());

    if (cur2.IsScalar())
    {
        double cutoffFreq = cur2.Scalar();

        if (arg3S == "s" || arg4S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (arg3S.empty() || arg3S == "low")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;

            status = Besself(order, 0.0, cutoffFreq, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg3S == "high")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Besself(order, cutoffFreq, 0.0, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg4S.empty())
        {
            status = Besself(order, 0.0, cutoffFreq, *numerCoef, *denomCoef, arg3S.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(33);
            }
        }
        else
        {
            if (!arg3S.empty() && arg3S != "s" && arg3S != "z")     // arg3 is junk
            {
                status(HW_MATH_ERR_FILTERTYPE, 3);
                if (!arg4S.empty() && arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status.SetArg2(4);
                }
            }
            else if (!arg4S.empty())
            {
                if (arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                else  // arg3, arg4 both good, but conflict
                {
                    status(HW_MATH_ERR_FILTERTYPE, 3, 4);   
                }
            }

            throw OML_Error(status);
        }

        if (!status.IsOk())
        {
            if (status == HW_MATH_ERR_FILTERFREQS_EQ)
            {
                if (arg3S == "s" || arg4S == "s")
                {
                    status(HW_MATH_ERR_FILTERFREQ_A, 2);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERFREQ_D, 2);
                }
            }
            else if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 33)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() > 3)
            {
                status.ResetArgs();
            }
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else
    {
        hwMatrix cutoffFreq(*cur2.Matrix());

        if (arg3S == "s" || arg4S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (cutoffFreq.Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 2));
        }
        if (!cutoffFreq.IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }

        if (cutoffFreq(0) >= cutoffFreq(1))
        {
            throw OML_Error(status(HW_MATH_ERR_NONINCREASE, 2));
        }

        if (arg3S.empty() || arg3S == "bandpass")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Besself(order, cutoffFreq(0), cutoffFreq(1), *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg3S == "stop")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Besself(order, cutoffFreq(1), cutoffFreq(0), *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg4S.empty())
        {
            status = Besself(order, cutoffFreq(0), cutoffFreq(1), *numerCoef, 
                            *denomCoef, arg3S.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(33);
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 3, 4));
        }

        if (!status.IsOk())
        {
            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 33)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() > 3)
            {
                status.ResetArgs();
            }

            if (!arg3S.empty() && arg3S != "s" && arg3S != "z")     // arg3 is junk
            {
                if (arg3S != "bandpass" && arg3S != "stop")
                {
                    status(HW_MATH_ERR_FILTERTYPE, 3);
                }
                if (!arg4S.empty() && arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status.SetArg2(4);
                }
            }
            else if (!arg4S.empty())
            {
                if (arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                else
                {   // arg3, arg4 both good, but conflict
                    status(HW_MATH_ERR_FILTERTYPE, 3, 4);  
                }
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        // check to see if digital filter was simplified
        bool isDigital = (arg3S == "s" || arg4S == "s") ? false : true;
        if (isDigital)
        {
            if (!arg3S.empty() && arg3S != "stop")  // band pass
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
            else                        // band stop
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
        }
    }

    // make row vectors
    numerCoef->Transpose();
    denomCoef->Transpose();

    outputs.push_back(numerCoef.release());
    outputs.push_back(denomCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Creates a Bessel filter [besself3 command]
//------------------------------------------------------------------------------
bool OmlBesself3(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 2 || nargin > 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    const Currency& cur1 = inputs[0];
    if (!cur1.IsPositiveInteger()) // Process arg 1 - filter order
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(cur1.Scalar());

    const Currency& cur2 = inputs[1];
    if (!cur2.IsScalar() && !cur2.IsMatrix())
    {
        throw OML_Error(OML_ERR_REAL, 2, OML_VAR_TYPE);
    }

    std::string arg3S;  // Process arg 3
    if (nargin > 2)
    {
        if (!inputs[2].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 3);
        }
        arg3S = inputs[2].StringVal();
    }

    std::string arg4S; // Process arg 4
    if (nargin > 3)
    {
        if (!inputs[3].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 4);
        }
        arg4S = inputs[3].StringVal();
    }

    hwMathStatus status;

    std::unique_ptr<hwMatrix> numerCoef(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> denomCoef(EvaluatorInterface::allocateMatrix());

    if (cur2.IsScalar())
    {
        double cutoffFreq = cur2.Scalar();

        if (arg3S == "s" || arg4S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (arg3S.empty() || arg3S == "low")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;

            status = Besself3(order, 0.0, cutoffFreq, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg3S == "high")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Besself3(order, cutoffFreq, 0.0, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg4S.empty())
        {
            status = Besself3(order, 0.0, cutoffFreq, *numerCoef, *denomCoef, arg3S.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(33);
            }
        }
        else
        {
            if (!arg3S.empty() && arg3S != "s" && arg3S != "z")     // arg3 is junk
            {
                status(HW_MATH_ERR_FILTERTYPE, 3);
                if (!arg4S.empty() && arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status.SetArg2(4);
                }
            }
            else if (!arg4S.empty())
            {
                if (arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                else  // arg3, arg4 both good, but conflict
                {
                    status(HW_MATH_ERR_FILTERTYPE, 3, 4);   
                }
            }

            throw OML_Error(status);
        }

        if (!status.IsOk())
        {
            if (status == HW_MATH_ERR_FILTERFREQS_EQ)
            {
                if (arg3S == "s" || arg4S == "s")
                {
                    status(HW_MATH_ERR_FILTERFREQ_A, 2);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERFREQ_D, 2);
                }
            }
            else if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 33)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() > 3)
            {
                status.ResetArgs();
            }
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else
    {
        hwMatrix cutoffFreq(*cur2.Matrix());

        if (arg3S == "s" || arg4S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (cutoffFreq.Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 2));
        }
        if (!cutoffFreq.IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }

        if (cutoffFreq(0) >= cutoffFreq(1))
        {
            throw OML_Error(status(HW_MATH_ERR_NONINCREASE, 2));
        }

        if (arg3S.empty() || arg3S == "bandpass")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Besself3(order, cutoffFreq(0), cutoffFreq(1), *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg3S == "stop")
        {
            std::string type = (arg4S.empty()) ? "z" : arg4S;
            status = Besself3(order, cutoffFreq(1), cutoffFreq(0), *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(44);
            }
        }
        else if (arg4S.empty())
        {
            status = Besself3(order, cutoffFreq(0), cutoffFreq(1), *numerCoef, 
                            *denomCoef, arg3S.c_str());
            if (status.GetArg1() == 6)
            {
                status.SetArg1(33);
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 3, 4));
        }

        if (!status.IsOk())
        {
            if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 33)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() > 3)
            {
                status.ResetArgs();
            }

            if (!arg3S.empty() && arg3S != "s" && arg3S != "z")     // arg3 is junk
            {
                if (arg3S != "bandpass" && arg3S != "stop")
                {
                    status(HW_MATH_ERR_FILTERTYPE, 3);
                }
                if (!arg4S.empty() && arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status.SetArg2(4);
                }
            }
            else if (!arg4S.empty())
            {
                if (arg4S != "s" && arg4S != "z") // arg4 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                else
                {   // arg3, arg4 both good, but conflict
                    status(HW_MATH_ERR_FILTERTYPE, 3, 4);  
                }
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        // check to see if digital filter was simplified
        bool isDigital = (arg3S == "s" || arg4S == "s") ? false : true;
        if (isDigital)
        {
            if (!arg3S.empty() && arg3S != "stop")  // band pass
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
            else                        // band stop
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 2);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
        }
    }

    // make row vectors
    numerCoef->Transpose();
    denomCoef->Transpose();

    outputs.push_back(numerCoef.release());
    outputs.push_back(denomCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Creates a Chebyshev I filter [cheby1 command]
//------------------------------------------------------------------------------
bool OmlCheby1(EvaluatorInterface           eval, 
               const std::vector<Currency>& inputs, 
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 3 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // process argument 1, the filter order argument
    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(inputs[0].Scalar());

    // process argument 2, the filter order passband requirement
    if (!inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 2);
    }
    double Rp = inputs[1].Scalar();

    // process argument 4
    std::string arg4S;
    if (nargin > 3)
    {
        if (!inputs[3].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 4);
        }
        arg4S = inputs[3].StringVal();
    }

    // process argument 5
    std::string arg5S;
    if (nargin > 4)
    {
        if (!inputs[4].IsString())
            throw OML_Error(OML_ERR_STRING, 5);

        arg5S = inputs[4].StringVal();
    }

    std::unique_ptr<hwMatrix> numerCoef(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> denomCoef(EvaluatorInterface::allocateMatrix());
    hwMathStatus status;  

    if (inputs[2].IsScalar())
    {
        double cutoffFreq = inputs[2].Scalar();

        if (arg4S == "s" || arg5S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (arg4S.empty() || arg4S == "low")
        {
            std::string type = (arg5S.empty()) ? "z" : arg5S;
            status = Cheby1(order, 0.0, cutoffFreq, Rp, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg4S == "high")
        {
            std::string type = (arg5S.empty()) ? "z" : arg5S;
            status = Cheby1(order, cutoffFreq, 0.0, Rp, *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg5S.empty())
        {
            status = Cheby1(order, 0.0, cutoffFreq, Rp, *numerCoef, *denomCoef, arg4S.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(44);
            }
        }
        else
        {
            if (!arg4S.empty() && arg4S != "s" && arg4S != "z")     // arg4 is junk
            {
                status(HW_MATH_ERR_FILTERTYPE, 4);
                if (!arg5S.empty() && arg5S == "s" && arg5S == "z") // arg5 is junk
                {
                    status.SetArg2(5);
                }
            }
            else if (!arg5S.empty())
            {
                if (arg5S != "s" && arg5S != "z") // arg5 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 5);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4, 5);   // arg4, arg5 both good, but conflict
                }
            }

            throw OML_Error(status);
        }

        if (!status.IsOk())
        {
            if (status == HW_MATH_ERR_FILTERFREQS_EQ)
            {
                if (arg4S == "s" || arg5S == "s")
                {
                    status(HW_MATH_ERR_FILTERFREQ_A, 3);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERFREQ_D, 3);
                }
            }
            else if (status.GetArg1() == 2)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 4)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() == 55)
            {
                status.SetArg1(5);
            }
            else if (status.GetArg1() > 4)
            {
                status.ResetArgs();
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else if (inputs[2].IsMatrix())
    {
        hwMatrix cutoffFreq(*inputs[2].Matrix());
        if (arg4S == "s" || arg5S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (cutoffFreq.Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 3));
        }

        if (!cutoffFreq.IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 3));
        }

        if (cutoffFreq(0) >= cutoffFreq(1))
        {
            throw OML_Error(status(HW_MATH_ERR_NONINCREASE, 3));
        }

        if (arg4S.empty() || arg4S == "bandpass")
        {
            std::string type = (arg5S.empty()) ? "z" : arg5S;
            status = Cheby1(order, cutoffFreq(0), cutoffFreq(1), Rp, *numerCoef,
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg4S == "stop")
        {
            std::string type = (arg5S.empty()) ? "z" : arg5S;
            status = Cheby1(order, cutoffFreq(1), cutoffFreq(0), Rp, *numerCoef,
                            *denomCoef, type.c_str());

            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg5S.empty())
        {
            status = Cheby1(order, cutoffFreq(0), cutoffFreq(1), Rp, *numerCoef, 
                            *denomCoef, arg4S.c_str());

            if (status.GetArg1() == 7)
            {
                status.SetArg1(44);
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 4, 5));
        }

        if (!status.IsOk())
        {
            switch (status.GetArg1())
            {
                case 2:  status.SetArg1(3); break;
                case 4:  status.SetArg1(2); break;
                case 44: status.SetArg1(4); break;
                case 55: status.SetArg1(5); break;
                default:
                    if (status.GetArg1() > 4)
                    {
                        status.ResetArgs();
                    }
                    break;
            }

            if (!arg4S.empty() && arg4S != "s" && arg4S != "z")     // arg4 is junk
            {
                if (arg4S != "bandpass" && arg4S != "stop")
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                if (!arg5S.empty() && arg5S != "s" && arg5S != "z") // arg5 is junk
                {
                    status.SetArg2(5);
                }
            }
            else if (!arg5S.empty())
            {
                if (arg5S != "s" && arg5S != "z") // arg5 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 5);
                }
                else
                {   // arg4, arg5 both good, but conflict
                    status(HW_MATH_ERR_FILTERTYPE, 4, 5);   
                }
            }
            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        // check to see if digital filter was simplified
        bool isDigital = (arg4S == "s" || arg5S == "s") ? false : true;
        if (isDigital)
        {
            if (!arg4S.empty() && arg4S != "stop")  // band pass
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
            else                        // band stop
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
        }
    }
    else
    {
        throw OML_Error(OML_ERR_REAL, 3, OML_VAR_TYPE);
    }

    // make row vectors
    numerCoef->Transpose();
    denomCoef->Transpose();

    outputs.push_back(numerCoef.release());
    outputs.push_back(denomCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Creates a Chebyshev II filter [cheby2 command]
//------------------------------------------------------------------------------
bool OmlCheby2(EvaluatorInterface           eval, 
               const std::vector<Currency>& inputs, 
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 3 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // process argument 1, the filter order argument
    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(inputs[0].Scalar());

    // process argument 2, the filter order stopband requirement
    if (!inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 2);
    }
    double Rs = inputs[1].Scalar();

    // process argument 4
    std::string arg4S;
    if (nargin > 3)
    {
        if (!inputs[3].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 4);
        }
        arg4S = inputs[3].StringVal();
    }

    // process argument 5
    std::string arg5S;
    if (nargin > 4)
    {
        if (!inputs[4].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 5);
        }
        arg5S = inputs[4].StringVal();
    }

    hwMathStatus status;
    std::unique_ptr<hwMatrix> numerCoef(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> denomCoef(EvaluatorInterface::allocateMatrix());

    if (inputs[2].IsScalar())
    {
        double cutoffFreq = inputs[2].Scalar();
        if (arg4S == "s" || arg5S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        std::string type = (arg5S.empty()) ? "z" : arg5S;
        if (arg4S.empty() || arg4S == "low")
        {
            status = Cheby2(order, 0.0, cutoffFreq, Rs, *numerCoef,
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg4S == "high")
        {
            status = Cheby2(order, cutoffFreq, 0.0, Rs, *numerCoef, *denomCoef, 
                            type.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg5S.empty())
        {
            status = Cheby2(order, 0.0, cutoffFreq, Rs, *numerCoef, *denomCoef, 
                            arg4S.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(44);
            }
        }
        else
        {
            if (!arg4S.empty() && arg4S != "s" && arg4S != "z")  // arg4 is junk
            {
                status(HW_MATH_ERR_FILTERTYPE, 4);
                if (!arg5S.empty() && arg5S != "s" && arg5S != "z") // arg5 is junk
                {
                    status.SetArg2(5);
                }
            }
            else if (!arg5S.empty())
            {
                if (arg5S != "s" && arg5S != "z") // arg5 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 5);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4, 5);   // arg4, arg5 both good, but conflict
                }
            }

            throw OML_Error(status);
        }

        if (!status.IsOk())
        {
            if (status == HW_MATH_ERR_FILTERFREQS_EQ)
            {
                if (arg4S == "s" || arg5S == "s")
                {
                    status(HW_MATH_ERR_FILTERFREQ_A, 3);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERFREQ_D, 3);
                }
            }
            else if (status.GetArg1() == 2)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 4)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 44)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() == 55)
            {
                status.SetArg1(5);
            }
            else if (status.GetArg1() > 4)
            {
                status.ResetArgs();
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else if (inputs[2].IsMatrix())
    {
        hwMatrix cutoffFreq(*inputs[2].Matrix());
        if (arg4S == "s" || arg5S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }
        if (cutoffFreq.Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 3));
        }

        if (!cutoffFreq.IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 3));
        }

        if (cutoffFreq(0) >= cutoffFreq(1))
        {
            throw OML_Error(status(HW_MATH_ERR_NONINCREASE, 3));
        }

        std::string type = (arg5S.empty()) ? "z" : arg5S;
        if (arg4S.empty() || arg4S == "bandpass")
        {
            status = Cheby2(order, cutoffFreq(0), cutoffFreq(1), Rs, *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg4S == "stop")
        {
            status = Cheby2(order, cutoffFreq(1), cutoffFreq(0), Rs, *numerCoef, 
                            *denomCoef, type.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(55);
            }
        }
        else if (arg5S.empty())
        {
            status = Cheby2(order, cutoffFreq(0), cutoffFreq(1), Rs, *numerCoef, 
                            *denomCoef, arg4S.c_str());
            if (status.GetArg1() == 7)
            {
                status.SetArg1(44);
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 4, 5));
        }

        if (!status.IsOk())
        {
            switch (status.GetArg1())
            {
                case 2:  status.SetArg1(3); break;
                case 4:  status.SetArg1(2); break;
                case 44: status.SetArg1(4); break;
                case 55: status.SetArg1(5); break;
                default:
                    if (status.GetArg1() > 4)
                    {
                        status.ResetArgs();
                    }
                    break;
            }

            if (!arg4S.empty() && arg4S != "s" && arg4S != "z")    // arg4 is junk
            {
                if (arg4S != "bandpass" && arg4S != "stop")
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4);
                }
                if (!arg5S.empty() && arg5S != "s" && arg5S != "z") // arg5 is junk
                {
                    status.SetArg2(5);
                }
            }
            else if (!arg5S.empty())
            {
                if (arg5S != "s" && arg5S != "z") // arg5 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 5);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERTYPE, 4, 5);   // arg4, arg5 both good, but conflict
                }
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        // check to see if digital filter was simplified
        bool isDigital = (arg4S == "s" || arg5S == "s") ? false : true;
        if (isDigital)
        {
            if (!arg4S.empty() && arg4S != "stop")  // band pass
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
            else                        // band stop
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 3);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
        }
    }
    else
    {
        throw OML_Error(OML_ERR_REAL, 3, OML_VAR_TYPE);
    }

    // make row vectors
    numerCoef->Transpose();
    denomCoef->Transpose();

    outputs.push_back(numerCoef.release());
    outputs.push_back(denomCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Creates an Elliptic filter [ellip command]
//------------------------------------------------------------------------------
bool OmlEllip(EvaluatorInterface           eval, 
              const std::vector<Currency>& inputs, 
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 4 || nargin > 6)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    // process argument 1, the filter order argument
    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int order = static_cast<int>(inputs[0].Scalar());

    // process argument 2, the filter order passband requirement
    if (!inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 2);
    }
    double Rp = inputs[1].Scalar();

    // process argument 3, the filter order stopband requirement
    if (!inputs[2].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 3);
    }
    double Rs = inputs[2].Scalar();

    // process argument 5
    if (nargin > 4 && !inputs[4].IsString())
    {
        throw OML_Error(OML_ERR_STRING, 5);
    }
    std::string arg5S = (nargin > 4) ? inputs[4].StringVal() : "";

    // process argument 6
    if (nargin > 5 && !inputs[5].IsString())
    {
        throw OML_Error(OML_ERR_STRING, 6);
    }
    std::string arg6S = (nargin > 5) ? inputs[5].StringVal() : "";
        
    hwMathStatus status;
    std::unique_ptr<hwMatrix> numerCoef(EvaluatorInterface::allocateMatrix());
    std::unique_ptr<hwMatrix> denomCoef(EvaluatorInterface::allocateMatrix());

    if (inputs[3].IsScalar())
    {
        double cutoffFreq = inputs[3].Scalar();
        if (arg5S == "s" || arg6S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        std::string type = (arg6S.empty()) ? "z" : arg6S;
        if (arg5S.empty() || arg5S == "low")
        {
            status = Ellip(order, 0.0, cutoffFreq, Rp, Rs, *numerCoef, 
                           *denomCoef, type.c_str());
            if (status.GetArg1() == 8)
            {
                status.SetArg1(66);
            }
        }
        else if (arg5S == "high")
        {
            status = Ellip(order, cutoffFreq, 0.0, Rp, Rs, *numerCoef, 
                           *denomCoef, type.c_str());
            if (status.GetArg1() == 8)
            {
                status.SetArg1(66);
            }
        }
        else if (arg6S.empty())
        {
            status = Ellip(order, 0.0, cutoffFreq, Rp, Rs, *numerCoef, 
                           *denomCoef, arg5S.c_str());
            if (status.GetArg1() == 8)
            {
                status.SetArg1(55);
            }
        }
        else
        {
            if (!arg5S.empty() && arg5S != "s" && arg5S != "z")     // arg5 is junk
            {
                status(HW_MATH_ERR_FILTERTYPE, 5);
                if (!arg6S.empty() && arg6S != "s" && arg6S != "z") // arg6 is junk
                {
                    status.SetArg2(6);
                }
            }
            else if (!arg6S.empty())
            {
                if (arg6S != "s" && arg6S != "z") // arg6 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 6);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERTYPE, 5, 6);   // arg5, arg6 both good, but conflict
                }
            }

            throw OML_Error(status);
        }

        if (!status.IsOk())
        {
            if (status == HW_MATH_ERR_FILTERFREQS_EQ)
            {
                if (arg5S == "s" || arg6S == "s")
                {
                    status(HW_MATH_ERR_FILTERFREQ_A, 4);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERFREQ_D, 4);
                }
            }
            else if (status.GetArg1() == 2)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() == 3)
            {
                status.SetArg1(4);
            }
            else if (status.GetArg1() == 4)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() == 5)
            {
                status.SetArg1(3);
            }
            else if (status.GetArg1() == 55)
            {
                status.SetArg1(5);
            }
            else if (status.GetArg1() == 66)
            {
                status.SetArg1(6);
            }
            else if (status.GetArg1() > 4)
            {
                status.ResetArgs();
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }
    }
    else if (inputs[3].IsMatrix())
    {
        hwMatrix cutoffFreq(*inputs[3].Matrix());
        if (arg5S == "s" || arg6S == "s")
        {
            cutoffFreq /= 2.0 * PI; // convert from radians/sec to Hz
        }

        if (cutoffFreq.Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 4));
        }

        if (!cutoffFreq.IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 4));
        }

        if (cutoffFreq(0) >= cutoffFreq(1))
        {
            throw OML_Error(status(HW_MATH_ERR_NONINCREASE, 4));
        }

        std::string type = (arg6S.empty()) ? "z" : arg6S;
        if (arg5S.empty() || arg5S == "bandpass")
        {
            status = Ellip(order, cutoffFreq(0), cutoffFreq(1), Rp, Rs, 
                           *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 8)
            {
                status.SetArg1(66);
            }
        }
        else if (arg5S == "stop")
        {
            status = Ellip(order, cutoffFreq(1), cutoffFreq(0), Rp, Rs, 
                           *numerCoef, *denomCoef, type.c_str());
            if (status.GetArg1() == 8)
            {
                status.SetArg1(66);
            }
        }
        else if (arg6S.empty())
        {
            status = Ellip(order, cutoffFreq(0), cutoffFreq(1), Rp, Rs, 
                           *numerCoef, *denomCoef, arg5S.c_str());
            if (status.GetArg1() == 8)
            {
                status.SetArg1(55);
            }
        }
        else
        {
            throw OML_Error(status(HW_MATH_ERR_FILTERTYPE, 5, 6));
        }

        if (!status.IsOk())
        {
            switch (status.GetArg1())
            {
                case 2:  status.SetArg1(4); break;
                case 4:  status.SetArg1(2); break;
                case 5:  status.SetArg1(3); break;
                case 55: status.SetArg1(5); break;
                case 66: status.SetArg1(6); break;
                default:
                    if (status.GetArg1() > 5)
                    {
                        status.ResetArgs();
                    }
                    break;
            }

            if (!arg5S.empty() && arg5S != "s" && arg5S != "z")     // arg5 is junk
            {
                if (arg5S != "bandpass" && arg5S != "stop")
                {
                    status(HW_MATH_ERR_FILTERTYPE, 5);
                }
                if (!arg6S.empty() && arg6S != "s" && arg6S != "z") // arg6 is junk
                {
                    status.SetArg2(6);
                }
            }
            else if (!arg6S.empty())
            {
                if (arg6S != "s" && arg6S != "z") // arg6 is junk
                {
                    status(HW_MATH_ERR_FILTERTYPE, 6);
                }
                else
                {
                    status(HW_MATH_ERR_FILTERTYPE, 5, 6);   // arg5, arg6 both good, but conflict
                }
            }

            BuiltInFuncsUtils::CheckMathStatus(eval, status);
        }

        // check to see if digital filter was simplified
        bool isDigital = (arg5S == "s" || arg6S == "s") ? false : true;

        if (isDigital)
        {
            if (!arg5S.empty() && arg5S != "stop")  // band pass
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 4);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 4);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
            else                        // band stop
            {
                if (cutoffFreq(0) == 0.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPHIGH, 4);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
                else if (cutoffFreq(1) == 1.0)
                {
                    status(HW_MATH_WARN_FILTERSIMPLOW, 4);
                    BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
                }
            }
        }
    }
    else
    {
        throw OML_Error(OML_ERR_REAL, 4, OML_VAR_TYPE);
    }

    // make row vectors
    numerCoef->Transpose();
    denomCoef->Transpose();

    outputs.push_back(numerCoef.release());
    outputs.push_back(denomCoef.release());

    return true;
}
//------------------------------------------------------------------------------
// Designs a Butterworth filter
//------------------------------------------------------------------------------
bool OmlButtord(EvaluatorInterface           eval, 
                const std::vector<Currency>& inputs, 
                std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 4 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[2].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 3);
    }
    double Rp = inputs[2].Scalar();

    if (!inputs[3].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 4);
    }
    double Rs = inputs[3].Scalar();

    std::string arg5S = "z";
    if (nargin > 4)
    {
        if (!inputs[4].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 5, OML_VAR_TYPE);
        }
        arg5S = inputs[4].StringVal();        
    }

    bool returnW0 = (fabs(Rp - 3.0103) < 1.0e-6) ? false : true;

    hwMathStatus status;
    int order;
    hwMatrix Wn;

    if (inputs[0].IsScalar() && inputs[1].IsScalar())
    {
        double Wp = inputs[0].Scalar();
        double Ws = inputs[1].Scalar();

        status = ButterOrd(Wp, Ws, Rp, Rs, order, Wn, arg5S.c_str());
        if (!status.IsOk())
        {
            if (status.GetArg1() == 7)
            {
                status.SetArg1(5);
            }
            throw OML_Error(status);
        }

        outputs.push_back(order);

        if (returnW0)
        {
            outputs.push_back(Wn(0));
        }
        else
        {
            outputs.push_back(Wn(1));
        }
        return true;
    }
    else if (inputs[0].IsMatrix() && inputs[1].IsMatrix())
    {
        const hwMatrix* Wp = inputs[0].Matrix();
        if (!Wp->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 1));
        }
        if (Wp->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 1));
        }

        const hwMatrix* Ws = inputs[1].Matrix();
        if (!Ws->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }
        if (Ws->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 2));
        }

        std::unique_ptr<hwMatrix> Wn1 (new hwMatrix);
        std::unique_ptr<hwMatrix> Wn2 (new hwMatrix);

        status = ButterOrd(*Wp, *Ws, Rp, Rs, order, *Wn1, *Wn2, arg5S.c_str());
        if (!status.IsOk())
        {
            throw OML_Error(status);
        }

        outputs.push_back(order);

        if (returnW0)
        {
            outputs.push_back(Wn1.release());
        }
        else
        {
            outputs.push_back(Wn2.release());
        }
    }
    else
    {
        throw OML_Error(HW_ERROR_1STAND2NDINPBOTHSCALORBOTHVEC);
    }

    return true;
}
//------------------------------------------------------------------------------
// Designs a Chebyshev I filter [cheb1ord command]
//------------------------------------------------------------------------------
bool OmlCheb1ord(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 4 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[2].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 3);
    }
    double Rp = inputs[2].Scalar();

    if (!inputs[3].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 4);
    }
    double Rs = inputs[3].Scalar();

    std::string arg5S = "z";
    if (nargin > 4)
    {
        if (!inputs[4].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 5, OML_VAR_TYPE);
        }
        arg5S = inputs[4].StringVal();        
    }

    hwMathStatus status;
    int order;
    hwMatrix Wn;

    if (inputs[0].IsScalar() && inputs[1].IsScalar())
    {
        double Wp = inputs[0].Scalar();
        double Ws = inputs[1].Scalar();

        status = Cheby1Ord(Wp, Ws, Rp, Rs, order, Wn, arg5S.c_str());

        if (!status.IsOk())
        {
            if (status.GetArg1() == 7)
            {
                status.SetArg1(5);
            }
            throw OML_Error(status);
        }

        outputs.push_back(order);
        outputs.push_back(Wn(1));
        return true;
    }
    
    if (inputs[0].IsMatrix() && inputs[1].IsMatrix())
    {
        const hwMatrix* Wp = inputs[0].Matrix();
        if (!Wp->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 1));
        }
        if (Wp->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 1));
        }

        const hwMatrix* Ws = inputs[1].Matrix();
        if (!Ws->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }
        if (Ws->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 2));
        }

        std::unique_ptr<hwMatrix> Wn1 (new hwMatrix);
        std::unique_ptr<hwMatrix> Wn2 (new hwMatrix);

        status = Cheby1Ord(*Wp, *Ws, Rp, Rs, order, *Wn1, *Wn2, arg5S.c_str());
        if (!status.IsOk())
        {
            throw OML_Error(status);
        }

        outputs.push_back(order);
        outputs.push_back(Wn2.release());
    }
    else
    {
        throw OML_Error(HW_ERROR_1STAND2NDINPBOTHSCALORBOTHVEC);
    }

    return true;
}
//------------------------------------------------------------------------------
// Designs a Chebyshev II filter [cheb2ord command]
//------------------------------------------------------------------------------
bool OmlCheb2ord(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 4 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[2].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 3);
    }
    double Rp = inputs[2].Scalar();

    if (!inputs[3].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 4);
    }
    double Rs = inputs[3].Scalar();

    std::string arg5S = "z";
    if (nargin > 4)
    {
        if (!inputs[4].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 5, OML_VAR_TYPE);
        }
        arg5S = inputs[4].StringVal();        
    }

    hwMathStatus status;
    int order;
    hwMatrix Wn;

    if (inputs[0].IsScalar() && inputs[1].IsScalar())
    {
        double Wp = inputs[0].Scalar();
        double Ws = inputs[1].Scalar();

        status = Cheby2Ord(Wp, Ws, Rp, Rs, order, Wn, arg5S.c_str());

        if (!status.IsOk())
        {
            if (status.GetArg1() == 7)
            {
                status.SetArg1(5);
            }
            throw OML_Error(status);
        }

        outputs.push_back(order);
        outputs.push_back(Wn(1));
        return true;
    }

    if (inputs[0].IsMatrix() && inputs[1].IsMatrix())
    {
        const hwMatrix* Wp = inputs[0].Matrix();
        if (!Wp->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 1));
        }
        if (Wp->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 1));
        }

        const hwMatrix* Ws = inputs[1].Matrix();
        if (!Ws->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }
        if (Ws->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 2));
        }

        std::unique_ptr<hwMatrix> Wn1 (new hwMatrix);
        std::unique_ptr<hwMatrix> Wn2 (new hwMatrix);

        status = Cheby2Ord(*Wp, *Ws, Rp, Rs, order, *Wn1, *Wn2, arg5S.c_str());

        if (!status.IsOk())
        {
            throw OML_Error(status);
        }

        outputs.push_back(order);
        outputs.push_back(Wn2.release());
    }
    else
    {
        throw OML_Error(HW_ERROR_1STAND2NDINPBOTHSCALORBOTHVEC);
    }

    return true;
}
//------------------------------------------------------------------------------
// Designs a Elliptic filter [ellipord command]
//------------------------------------------------------------------------------
bool OmlEllipord(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 4 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[2].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 3);
    }
    double Rp = inputs[2].Scalar();

    if (!inputs[3].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 4);
    }
    double Rs = inputs[3].Scalar();

    std::string arg5S = "z";
    if (nargin > 4)
    {
        if (!inputs[4].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 5, OML_VAR_TYPE);
        }
        arg5S = inputs[4].StringVal();        
    }

    hwMathStatus status;
    int order;
    hwMatrix Wn;

    if (inputs[0].IsScalar() && inputs[1].IsScalar())
    {
        double Wp = inputs[0].Scalar();
        double Ws = inputs[1].Scalar();

        status = EllipOrd(Wp, Ws, Rp, Rs, order, Wn, arg5S.c_str());

        if (!status.IsOk())
        {
            if (status.GetArg1() == 7)
            {
                status.SetArg1(5);
            }
            throw OML_Error(status);
        }

        outputs.push_back(order);
        outputs.push_back(Wn(1));
        return true;
    }
    if (inputs[0].IsMatrix() && inputs[1].IsMatrix())
    {
        const hwMatrix* Wp = inputs[0].Matrix();
        if (!Wp->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 1));
        }

        if (Wp->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 1));
        }

        const hwMatrix* Ws = inputs[1].Matrix();
        if (!Ws->IsReal())
        {
            throw OML_Error(status(HW_MATH_ERR_COMPLEX, 2));
        }

        if (Ws->Size() != 2)
        {
            throw OML_Error(status(HW_MATH_ERR_VECTOR2, 2));
        }

        std::unique_ptr<hwMatrix> Wn1 (new hwMatrix);
        std::unique_ptr<hwMatrix> Wn2 (new hwMatrix);

        status = EllipOrd(*Wp, *Ws, Rp, Rs, order, *Wn1, *Wn2, arg5S.c_str());

        if (!status.IsOk())
        {
            throw OML_Error(status);
        }

        outputs.push_back(order);
        outputs.push_back(Wn2.release());
    }
    else
    {
        throw OML_Error(HW_ERROR_1STAND2NDINPBOTHSCALORBOTHVEC);
    }

    return true;
}
//------------------------------------------------------------------------------
// Returns Blackman window coefficients [blackman command]
//------------------------------------------------------------------------------
bool OmlBlackman(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)

{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    std::string winType ("symmetric");
    if (nargin > 1)
    {
        if (!inputs[1].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 2);
        }

        winType = inputs[1].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 2, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));

    hwMathStatus mstat = BlackmanWin(*window, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Returns Bartlett-Hann window coefficients [barthann command]
//------------------------------------------------------------------------------
bool OmlBartHann(EvaluatorInterface           eval,
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    std::string winType("symmetric");
    if (nargin > 1)
    {
        if (!inputs[1].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 2);
        }

        winType = inputs[1].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 2, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));

    hwMathStatus mstat = BartHannWin(*window, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Returns Hann window coefficients [hann command]
//------------------------------------------------------------------------------
bool OmlHann(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    std::string winType ("symmetric");
    if (nargin > 1)
    {
        if (!inputs[1].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 2);
        }

        winType = inputs[1].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 2, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));
    
    hwMathStatus mstat = HannWin(*window, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Returns Hamming window coefficients [hamming command]
//------------------------------------------------------------------------------
bool OmlHamming(EvaluatorInterface           eval, 
                const std::vector<Currency>& inputs, 
                std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    std::string winType ("symmetric");
    if (nargin > 1)
    {
        if (!inputs[1].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 2);
        }
        winType = inputs[1].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 2, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));

    hwMathStatus mstat = HammWin(*window, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Returns Parzen window coefficients [parzenwin command]
//------------------------------------------------------------------------------
bool OmlParzenwin(EvaluatorInterface           eval, 
                  const std::vector<Currency>& inputs, 
                  std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    std::string winType ("symmetric");
    if (nargin > 1)
    {
        if (!inputs[1].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 2);
        }
        winType = inputs[1].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 2, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));

    hwMathStatus mstat = ParzenWin(*window, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Returns Welch window coefficients [welchwin command]
//------------------------------------------------------------------------------
bool OmlWelchwin(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    std::string winType ("symmetric");
    if (nargin > 1)
    {
        if (!inputs[1].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 2);
        }
        winType = inputs[1].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 2, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));

    hwMathStatus mstat = WelchWin(*window, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Returns Dolph-Chebyshev window coefficients [chebwin command]
//------------------------------------------------------------------------------
bool OmlChebwin(EvaluatorInterface           eval, 
                const std::vector<Currency>& inputs, 
                std::vector<Currency>&       outputs)

{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    if (!inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 2);
    }
    double value = inputs[1].Scalar();  // kaiser: value = beta, chebwin: value = sidelobe

    std::string winType = "symmetric";
    if (nargin > 2)
    {
        if (!inputs[2].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 3);
        }
        winType = inputs[2].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));

    hwMathStatus mstat = ChebyWin(*window, value, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Returns Kaiser-Bessel window coefficients [kaiser command]
//------------------------------------------------------------------------------
bool OmlKaiser(EvaluatorInterface           eval, 
               const std::vector<Currency>& inputs, 
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 1 || nargin > 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 1);
    }
    int n = static_cast<int>(inputs[0].Scalar());

    if (!inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_SCALAR, 2);
    }
    double value = inputs[1].Scalar();  // kaiser: value = beta, chebwin: value = sidelobe

    std::string winType = "symmetric";
    if (nargin > 2)
    {
        if (!inputs[2].IsString())
        {
            throw OML_Error(OML_ERR_STRING, 3);
        }
        winType = inputs[2].StringVal();
    }
    if (winType != "symmetric" && winType != "periodic")
    {
        throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_STRING);
    }

    std::unique_ptr<hwMatrix> window(
        EvaluatorInterface::allocateMatrix(n, 1, true));

    hwMathStatus mstat = KaiserBesselWin(*window, value, winType.c_str());
    BuiltInFuncsUtils::CheckMathStatus(eval, mstat);

    outputs.push_back(window.release());
    return true;
}
//------------------------------------------------------------------------------
// Filters a signal with an IIR or FIR filter [filter command]
//------------------------------------------------------------------------------
bool OmlFilter(EvaluatorInterface           eval, 
               const std::vector<Currency>& inputs, 
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin < 3 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 2, OML_VAR_TYPE);
    }

    const hwMatrix* num = inputs[0].ConvertToMatrix();
    const hwMatrix* den = inputs[1].ConvertToMatrix();

    if (inputs[2].IsMatrix() || inputs[2].IsScalar())
    {
        const hwMatrix* data = inputs[2].ConvertToMatrix();
        int dim;

        if (nargin < 5)
        {
            dim = data->M() == 1 ? 2 : 1;
        }
        else
        {
            if (!inputs[3].IsEmpty())
            {
                throw OML_Error(HW_ERROR_INITSTEMPTOSPECDIM);
            }

            if (!inputs[4].IsPositiveInteger())
            {
                throw OML_Error(OML_ERR_POSINTEGER, 5);
            }
            dim = static_cast<int>(inputs[4].Scalar());

            if (dim > 2)
            {
                throw OML_Error(HW_ERROR_UNSUPP2DIM);
            }
        }

#if 0 // Commented code
        // This is the condensed, less-redundant way, but casting lambdas doesn't work in VS 2010:
        //Currency result = doAlongDim(eval, data.get(), dim,
        //    (hwMatrix*(*)(EvaluatorInterface&, const hwMatrix*, void*)) ([&num, &den, &temp](EvaluatorInterface& e, const hwMatrix* vec, void*) -> hwMatrix* {
        //        std::unique_ptr<hwMatrix> temp(EvaluatorInterface::allocateMatrix());
        //        hwMatrix* temp = EvaluatorInterface::allocateMatrix()
        //        checkMathStatus(e, Filter(*num, den.get(), *vec, *temp));
        //        return temp.release();
        //}));
        //
        //outputs.push_back(result);
        //return true;
#endif

        std::unique_ptr<hwMatrix> result(EvaluatorInterface::allocateMatrix());
        std::unique_ptr<hwMatrix> temp(EvaluatorInterface::allocateMatrix());

        if (data->IsVector())
        {
            hwMathStatus stat = Filter(*num, den, *data, *result);
            BuiltInFuncsUtils::CheckMathStatus(eval, stat);
        }
        else if (dim == 1)
        {
            std::unique_ptr<hwMatrix> col(EvaluatorInterface::allocateMatrix());
            if (data->N())
            {
                for (int i = 0; i < data->N(); ++i)
                {
                    hwMathStatus stat = data->ReadColumn(i, *col);
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);

                    stat = Filter(*num, den, *col, *temp);
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                    if (!i)
                    {
                        result.reset(EvaluatorInterface::allocateMatrix(temp->Size(), 
                            data->N(), true));
                    }
                    writeCol(eval, result.get(), temp.get(), i);
                }
            }
            else
            {
                result.reset(EvaluatorInterface::allocateMatrix());
            }
        }
        else // dim == 2
        {
            std::unique_ptr<hwMatrix> row(EvaluatorInterface::allocateMatrix());
            if (data->M())
            {
                for (int i = 0; i < data->M(); ++i)
                {
                    hwMathStatus stat = data->ReadRow(i, *row);
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);

                    stat = Filter(*num, den, *row, *temp);
                    BuiltInFuncsUtils::CheckMathStatus(eval, stat);
                    if (!i)
                    {
                        result.reset(EvaluatorInterface::allocateMatrix(data->M(), 
                            temp->Size(), true));
                    }
                    writeRow(eval, result.get(), temp.get(), i);
                }
            }
            else
            {
                result.reset(EvaluatorInterface::allocateMatrix());
            }
        }

        outputs.push_back(result.release());
    }
    else if (inputs[2].IsNDMatrix())
    {
        if (nargin < 5)
            return oml_MatrixNUtil4(eval, inputs, outputs, OmlFilter, 0, 3);
        else
            return oml_MatrixNUtil4(eval, inputs, outputs, OmlFilter, 5, 3);
    }
    else
    {
        throw OML_Error(OML_ERR_MATRIX, 3, OML_VAR_TYPE);
    }

    return true;
}
//------------------------------------------------------------------------------
// Filter a signal forward and then backward, compensating for end effects
// [filtfilt command]
//------------------------------------------------------------------------------
bool OmlFiltfilt(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_REALVECTOR, 2, OML_VAR_TYPE);
    }

    const hwMatrix* num = inputs[0].ConvertToMatrix();
    const hwMatrix* den = inputs[1].ConvertToMatrix();

    if (inputs[2].IsMatrix() || inputs[2].IsScalar())
    {
        const hwMatrix* data = inputs[2].ConvertToMatrix();

        std::unique_ptr<hwMatrix> result(EvaluatorInterface::allocateMatrix(
            data->M(), data->N(), true));

        if (data->IsVector())
        {
            hwMathStatus stat = FiltFilt(*num, den, *data, *result);
            BuiltInFuncsUtils::CheckMathStatus(eval, stat);
        }
        else
        {
            result.reset(EvaluatorInterface::allocateMatrix(data->M(), data->N(), 0.0));
            int outM = data->M();

            for (int i = 0; i < data->N(); ++i)
            {
                hwMatrix dataCol(outM, 1, (void*) &(*data)(0,i), hwMatrix::REAL);
                hwMatrix resultCol(outM, 1, (void*) &(*result)(0,i), hwMatrix::REAL);

                hwMathStatus stat = FiltFilt(*num, den, dataCol, resultCol);
                BuiltInFuncsUtils::CheckMathStatus(eval, stat);
            }
        }

        outputs.push_back(result.release());
    }
    else if (inputs[2].IsNDMatrix())
    {
        return oml_MatrixNUtil4(eval, inputs, outputs, OmlFiltfilt, -1, 3);
    }
    else
    {
        throw OML_Error(OML_ERR_MATRIX, 3, OML_VAR_TYPE);
    }

    return true;
}
//------------------------------------------------------------------------------
// Computes the sinc function [sinc command]
//------------------------------------------------------------------------------
bool OmlSinc(EvaluatorInterface           eval, 
             const std::vector<Currency>& inputs, 
             std::vector<Currency>&       outputs)
{
    // This function should be in /common/math/signals, but it would need a new 
    // file. So keep it here for now.
    if (eval.GetNarginValue() != 1)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }
    if (eval.GetNargoutValue() > 1)
    {
        throw OML_Error(OML_ERR_NUMARGOUT);
    }

    if (inputs[0].IsMatrix())
    {
        const hwMatrix* matrix = inputs[0].Matrix();
        hwMatrix*       out    = nullptr;
        int size = matrix->Size();

        if (matrix->IsReal())
        {
            out = new hwMatrix(matrix->M(), matrix->N(), hwMatrix::REAL);

            for (int i = 0; i < size; ++i)
            {
                double x = (*matrix)(i);

                if (x == 0.0)
                {
                    (*out)(i) = 1.0;
                }
                else
                {
                    x *= PI;
                    (*out)(i) = sin(x) / x;
                }
            }
        }
        else
        {
            out = new hwMatrix(matrix->M(), matrix->N(), hwMatrix::COMPLEX);

            for (int i = 0; i < size; ++i)
            {
                hwComplex z = (*matrix).z(i);

                if (z == 0.0)
                {
                    (*out).z(i) = 1.0;
                }
                else
                {
                    z *= PI;
                    (*out).z(i) = hwComplex::sin(z) / z;
                }
            }
        }

        outputs.push_back(out);
    }
    else if (inputs[0].IsScalar())
    {
        double x = inputs[0].Scalar();

        if (x == 0.0)
        {
            outputs.push_back(1.0);
        }
        else
        {
            x *= PI;
            outputs.push_back(sin(x) / x);
        }
    }
    else if (inputs[0].IsComplex())
    {
        hwComplex z = inputs[0].Complex();

        if (z == 0.0)
        {
            outputs.push_back(1.0);
        }
        else
        {
            z *= PI;
            outputs.push_back(hwComplex::sin(z) / z);
        }
    }
    else
    {
        throw OML_Error(OML_ERR_SCALARCOMPLEXMTX, -1, OML_VAR_VALUE);
    }

    return true;
}
//------------------------------------------------------------------------------
// Upsamples a signal [upsample command]
//------------------------------------------------------------------------------
bool OmlUpsample(EvaluatorInterface           eval, 
                 const std::vector<Currency>& inputs, 
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 2 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }
    if (eval.GetNargoutValue() > 1)
    {
        throw OML_Error(OML_ERR_NUMARGOUT);
    }
    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsInteger())
    {
        throw OML_Error(OML_ERR_INTEGER, 2, OML_VAR_TYPE);
    }
    const hwMatrix* inSignal = inputs[0].ConvertToMatrix();
    int k = static_cast<int>(inputs[1].Scalar());

    int phase = 0;
    if (nargin == 3)
    {
        if (!inputs[2].IsInteger())
        {
            throw OML_Error(OML_ERR_INTEGER, 3, OML_VAR_TYPE);
        }
        phase = static_cast<int>(inputs[2].Scalar());
    }

    std::unique_ptr<hwMatrix> outSignal(EvaluatorInterface::allocateMatrix());

    if (inSignal->IsVector())
    {
        hwMathStatus stat = UpSample(*inSignal, *outSignal, k, phase);
        BuiltInFuncsUtils::CheckMathStatus(eval, stat);

        if (inSignal->M() != outSignal->M() && inSignal->N() != outSignal->N())
        {
            outSignal->Transpose();
        }
    }
    else
    {
        int inM  = inSignal->M();
        int outM = inM * k;
        outSignal.reset(EvaluatorInterface::allocateMatrix(outM, inSignal->N(), 0.0));

        for (int i = 0; i < inSignal->N(); ++i)
        {
            hwMatrix inCol(inM, 1, (void*) &(*inSignal)(0,i), hwMatrix::REAL);
            hwMatrix outCol(outM, 1, (void*) &(*outSignal)(0,i), hwMatrix::REAL);

            hwMathStatus stat = UpSample(inCol, outCol, k, phase);
            BuiltInFuncsUtils::CheckMathStatus(eval, stat);
        }
    }

    outputs.push_back(outSignal.release());
    return true;
}
//------------------------------------------------------------------------------
// Downsamples a signal [downsample command]
//------------------------------------------------------------------------------
bool OmlDownsample(EvaluatorInterface           eval, 
                   const std::vector<Currency>& inputs, 
                   std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    if (nargin != 2 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }
    if (eval.GetNargoutValue() > 1)
    {
        throw OML_Error(OML_ERR_NUMARGOUT);
    }
    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }
    if (!inputs[1].IsInteger())
    {
        throw OML_Error(OML_ERR_INTEGER, 2, OML_VAR_TYPE);
    }
    const hwMatrix* inSignal = inputs[0].ConvertToMatrix();
    int k = static_cast<int>(inputs[1].Scalar());

    int phase = 0;
    if (nargin == 3)
    {
        if (!inputs[2].IsInteger())
        {
            throw OML_Error(OML_ERR_INTEGER, 3, OML_VAR_TYPE);
        }
        phase = static_cast<int>(inputs[2].Scalar());
    }

    std::unique_ptr<hwMatrix> outSignal(EvaluatorInterface::allocateMatrix());

    if (inSignal->IsVector())
    {
        hwMathStatus stat = DownSample(*inSignal, *outSignal, k, phase);
        BuiltInFuncsUtils::CheckMathStatus(eval, stat);

        if (inSignal->M() != outSignal->M() && inSignal->N() != outSignal->N())
        {
            outSignal->Transpose();
        }
    }
    else
    {
        int inM = inSignal->M();
        if (inM)
        {
            int outM = (inM - phase - 1) / k + 1;
            outSignal.reset(EvaluatorInterface::allocateMatrix(outM, inSignal->N(), 0.0));

            for (int i = 0; i < inSignal->N(); ++i)
            {
                hwMatrix inCol(inM, 1, (void*) &(*inSignal)(0,i), hwMatrix::REAL);
                hwMatrix outCol(outM, 1, (void*) &(*outSignal)(0,i), hwMatrix::REAL);

                hwMathStatus stat = DownSample(inCol, outCol, k, phase);
                BuiltInFuncsUtils::CheckMathStatus(eval, stat);
            }
        }
        else
        {
            outSignal.reset(EvaluatorInterface::allocateMatrix(0, inSignal->N(), 0.0));
        }
    }

    outputs.push_back(outSignal.release());
    return true;
}
//------------------------------------------------------------------------------
// Resamples a signal [resample command]
//------------------------------------------------------------------------------
bool OmlResample(EvaluatorInterface           eval,
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    size_t nargout = eval.GetNargoutValue();

    if (nargin != 3 && nargin != 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (inputs[0].IsNDMatrix())
    {
        std::vector<Currency> inputs2(inputs);
        inputs2.push_back(1);

        return oml_MatrixNUtil4(eval, inputs2, outputs, OmlResample, 4);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_TYPE);
    }

    if (!inputs[2].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 3, OML_VAR_TYPE);
    }

    const hwMatrix* signal = inputs[0].ConvertToMatrix();
    int p = static_cast<int> (inputs[1].Scalar());
    int q = static_cast<int> (inputs[2].Scalar());
    const hwMatrix* h = nullptr;

    if (nargin == 4)
    {
        if (!inputs[3].IsMatrix() && !inputs[3].IsScalar())
            throw OML_Error(OML_ERR_MATRIX, 4, OML_VAR_TYPE);

        h = inputs[3].ConvertToMatrix();
    }

    // convert p/q to lowest terms
    std::vector<Currency> inputs2;
    std::vector<Currency> outputs2;

    inputs2.push_back(p);
    inputs2.push_back(q);
    oml_gcd(eval, inputs2, outputs2);
    int gcd = static_cast<int> (outputs2[0].Scalar());
    p /= gcd;
    q /= gcd;

    std::unique_ptr<hwMatrix> resamp(EvaluatorInterface::allocateMatrix());

    hwMathStatus status = Resample(*signal, *resamp, p, q, h);

    outputs.push_back(resamp.release());
    return true;
}
//------------------------------------------------------------------------------
// Resamples a signal [upfirdn command]
//------------------------------------------------------------------------------
bool OmlUpFirDn(EvaluatorInterface           eval,
                const std::vector<Currency>& inputs,
                std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    size_t nargout = eval.GetNargoutValue();

    if (nargin != 4)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 2, OML_VAR_TYPE);
    }

    if (!inputs[2].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 3, OML_VAR_TYPE);
    }

    if (!inputs[3].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 4, OML_VAR_TYPE);
    }

    const hwMatrix* signal = inputs[0].ConvertToMatrix();
    const hwMatrix* h = inputs[1].ConvertToMatrix();
    int p = static_cast<int> (inputs[2].Scalar());
    int q = static_cast<int> (inputs[3].Scalar());

    // convert p/q to lowest terms
    std::vector<Currency> inputs2;
    std::vector<Currency> outputs2;

    inputs2.push_back(p);
    inputs2.push_back(q);
    oml_gcd(eval, inputs2, outputs2);
    int gcd = static_cast<int> (outputs2[0].Scalar());
    p /= gcd;
    q /= gcd;

    std::unique_ptr<hwMatrix> resamp(EvaluatorInterface::allocateMatrix());

    hwMathStatus status = UpFirDown(*signal, *resamp, p, q, *h);

    outputs.push_back(resamp.release());
    return true;
}
//------------------------------------------------------------------------------
// Find peaks of a signal [findpeaks command]
//------------------------------------------------------------------------------
bool OmlFindPeaks(EvaluatorInterface           eval, 
                  const std::vector<Currency>& inputs, 
                  std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();
    size_t nargout = eval.GetNargoutValue();

    if (nargin < 1)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }
    if (nargout > 3)
    {
        throw OML_Error(OML_ERR_NUMARGOUT);
    }
    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    const hwMatrix* signal          = inputs[0].ConvertToMatrix();
    bool            twosided        = false;
    double          minPeakHeight   = -1.0;
    int             minPeakDistance = -1;
    int             minPeakWidth    = -1;
    int             arg             = 1;

    while (arg < nargin)
    {
        if (!inputs[arg].IsString())
        {
            throw OML_Error(OML_ERR_STRING, arg+1, OML_VAR_TYPE);
        }
        std::string temp_str = inputs[arg].StringVal();
        ++arg;

        if (temp_str == "DoubleSided")
        {
            twosided = true;
        }
        else if (temp_str == "MinPeakHeight")
        {
            if (!inputs[arg].IsScalar())
            {
                throw OML_Error(OML_ERR_SCALAR, arg+1, OML_VAR_TYPE);
            }
            minPeakHeight = inputs[arg].Scalar();
            ++arg;
        }
        else if (temp_str == "MinPeakDistance")
        {
            if (!inputs[arg].IsInteger())
            {
                throw OML_Error(OML_ERR_INTEGER, arg+1, OML_VAR_TYPE);
            }
            minPeakDistance = static_cast<int>(inputs[arg].Scalar());
            ++arg;
        }
        else if (temp_str == "MinPeakWidth")
        {
            if (!inputs[arg].IsInteger())
            {
                throw OML_Error(OML_ERR_INTEGER, arg+1, OML_VAR_TYPE);
            }
            minPeakWidth = static_cast<int>(inputs[arg].Scalar());
            ++arg;
        }
        else
        {
            throw OML_Error(HW_ERROR_INVALIDOPTION(temp_str));
        }
    }

    // prepare algorithm function call
    std::unique_ptr<hwMatrix> peaks(EvaluatorInterface::allocateMatrix());
    hwMatrixI index;
    PeakInfo* extra = nullptr;

    if (nargout == 3)
    {
        extra = new PeakInfo;
    }
    hwMathStatus status = FindPeaks(*signal, twosided, minPeakHeight,
        minPeakDistance, minPeakWidth, *peaks, index, 1, extra);

    if (!status.IsOk())
    {
        throw OML_Error(status);
    }

    hwMatrix* dindex = EvaluatorInterface::allocateMatrix(index.M(), 
                                           index.N(), true);

    int size = index.Size();
    for (int i = 0; i < size; ++i)
    {
        (*dindex)(i) = static_cast<double>(index(i));
    }

    outputs.push_back(peaks.release());
    outputs.push_back(dindex);

    if (extra)
    {
        Currency parabol = EvaluatorInterface::allocateStruct();
        parabol.Struct()->SetValue(0, -1, "pp", extra->parabol_pp);
        parabol.Struct()->SetValue(0, -1, "x", extra->parabol_x);

        Currency out = EvaluatorInterface::allocateStruct();
        out.Struct()->SetValue(0, -1, "parabol", parabol);
        out.Struct()->SetValue(0, -1, "height", extra->height);
        out.Struct()->SetValue(0, -1, "baseline", extra->baseline);
        out.Struct()->SetValue(0, -1, "roots", extra->roots);
        outputs.push_back(out);
    }

    if (extra)
    {
        delete extra;
        extra = nullptr;
    }

    return true;
}
//------------------------------------------------------------------------------
//! Computes A-weighted acoustic function [dba command]
//------------------------------------------------------------------------------
bool OmldbA(EvaluatorInterface           eval,
            const std::vector<Currency>& inputs,
            std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 2 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 2, OML_VAR_TYPE);
    }

    const hwMatrix* freq = inputs[0].ConvertToMatrix();
    const hwMatrix* mag_in = inputs[1].ConvertToMatrix();
    double reference = 1.0;

    if (nargin == 3)
    {
        if (!inputs[2].IsScalar())
        {
            throw OML_Error(OML_ERR_POSITIVE_SCALAR, 3, OML_VAR_TYPE);
        }

        reference = inputs[2].Scalar();
    }

    hwMatrix* mag_out = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = dBa(*freq, *mag_in, *mag_out, reference);

    if (status.GetArg1() == 3)
        status.ResetArgs();
    else if (status.GetArg1() == 4)
        status.SetArg1(3);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);
    outputs.push_back(mag_out);
    return true;
}
//------------------------------------------------------------------------------
//! Computes B-weighted acoustic function [dbb command]
//------------------------------------------------------------------------------
bool OmldbB(EvaluatorInterface           eval,
            const std::vector<Currency>& inputs,
            std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 2 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 2, OML_VAR_TYPE);
    }

    const hwMatrix* freq = inputs[0].ConvertToMatrix();
    const hwMatrix* mag_in = inputs[1].ConvertToMatrix();
    double reference = 1.0;

    if (nargin == 3)
    {
        if (!inputs[2].IsScalar())
        {
            throw OML_Error(OML_ERR_POSITIVE_SCALAR, 3, OML_VAR_TYPE);
        }

        reference = inputs[2].Scalar();
    }

    hwMatrix* mag_out = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = dBb(*freq, *mag_in, *mag_out, reference);

    if (status.GetArg1() == 3)
        status.ResetArgs();
    else if (status.GetArg1() == 4)
        status.SetArg1(3);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);
    outputs.push_back(mag_out);
    return true;
}
//------------------------------------------------------------------------------
//! Computes C-weighted acoustic function [dbc command]
//------------------------------------------------------------------------------
bool OmldbC(EvaluatorInterface           eval,
            const std::vector<Currency>& inputs,
            std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 2 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 2, OML_VAR_TYPE);
    }

    const hwMatrix* freq = inputs[0].ConvertToMatrix();
    const hwMatrix* mag_in = inputs[1].ConvertToMatrix();
    double reference = 1.0;

    if (nargin == 3)
    {
        if (!inputs[2].IsScalar())
        {
            throw OML_Error(OML_ERR_POSITIVE_SCALAR, 3, OML_VAR_TYPE);
        }

        reference = inputs[2].Scalar();
    }

    hwMatrix* mag_out = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = dBc(*freq, *mag_in, *mag_out, reference);

    if (status.GetArg1() == 3)
        status.ResetArgs();
    else if (status.GetArg1() == 4)
        status.SetArg1(3);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);
    outputs.push_back(mag_out);
    return true;
}
//------------------------------------------------------------------------------
//! Computes U-weighted acoustic function [dbu command]
//------------------------------------------------------------------------------
bool OmldbU(EvaluatorInterface           eval,
            const std::vector<Currency>& inputs,
            std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 2 && nargin != 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 2, OML_VAR_TYPE);
    }

    const hwMatrix* freq = inputs[0].ConvertToMatrix();
    const hwMatrix* mag_in = inputs[1].ConvertToMatrix();
    double reference = 1.0;

    if (nargin == 3)
    {
        if (!inputs[2].IsScalar())
        {
            throw OML_Error(OML_ERR_POSITIVE_SCALAR, 3, OML_VAR_TYPE);
        }

        reference = inputs[2].Scalar();
    }

    hwMatrix* mag_out = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = dBu(*freq, *mag_in, *mag_out, reference);

    if (status.GetArg1() == 3)
        status.ResetArgs();
    else if (status.GetArg1() == 4)
        status.SetArg1(3);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);
    outputs.push_back(mag_out);
    return true;
}
//------------------------------------------------------------------------------
//! Computes rectangular pulse signal [rectpuls command]
//------------------------------------------------------------------------------
bool OmlRectPuls(EvaluatorInterface           eval,
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    const hwMatrix* time = inputs[0].ConvertToMatrix();
    double width = 1.0;

    if (nargin == 2)
    {
        if (!inputs[1].IsScalar())
        {
            throw OML_Error(OML_ERR_SCALAR, 2, OML_VAR_TYPE);
        }

        width = inputs[1].Scalar();
    }

    hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = RectPulse(*time, width, *waveForm);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(waveForm);
    return true;
}
//------------------------------------------------------------------------------
//! Computes triangular pulse signal [tripuls command]
//------------------------------------------------------------------------------
bool OmlTriPuls(EvaluatorInterface           eval,
                const std::vector<Currency>& inputs,
                std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    const hwMatrix* time = inputs[0].ConvertToMatrix();
    double width = 1.0;
    double skew = 0.0;

    if (nargin > 1)
    {
        if (inputs[1].IsScalar())
        {
            width = inputs[1].Scalar();
        }
        else if (!(inputs[1].IsMatrix() && inputs[1].Matrix()->Is0x0()))
        {
            throw OML_Error(OML_ERR_SCALAR, 2, OML_VAR_VALUE);
        }
    }

    if (nargin > 2)
    {
        if (inputs[2].IsScalar())
        {
            skew = inputs[2].Scalar();
        }
        else if (!(inputs[2].IsMatrix() && inputs[2].Matrix()->Is0x0()))
        {
            throw OML_Error(OML_ERR_SCALAR, 3, OML_VAR_VALUE);
        }
    }

    hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = TriPulse(*time, width, skew, *waveForm);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(waveForm);
    return true;
}
//------------------------------------------------------------------------------
//! Computes Gaussian pulse signal [gauspuls command]
//------------------------------------------------------------------------------
bool OmlGausPuls(EvaluatorInterface           eval,
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 5)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar() && !inputs[0].IsString())
    {
        throw OML_Error(OML_ERR_STRINGVECTOR, 1, OML_VAR_TYPE);
    }

    double fc = 1000.0;
    double bw = 0.5;
    double bwr = -6.0;

    if (nargin > 1)
    {
        if (inputs[1].IsScalar())
        {
            fc = inputs[1].Scalar();
        }
        else if (!(inputs[1].IsMatrix() && inputs[1].Matrix()->Is0x0()))
        {
            throw OML_Error(OML_ERR_SCALAR, 2, OML_VAR_VALUE);
        }
    }

    if (nargin > 2)
    {
        if (inputs[2].IsScalar())
        {
            bw = inputs[2].Scalar();
        }
        else if (!(inputs[2].IsMatrix() && inputs[2].Matrix()->Is0x0()))
        {
            throw OML_Error(OML_ERR_SCALAR, 3, OML_VAR_VALUE);
        }
    }

    if (nargin > 3)
    {
        if (inputs[3].IsScalar())
        {
            bwr = inputs[3].Scalar();
        }
        else if (!(inputs[3].IsMatrix() && inputs[3].Matrix()->Is0x0()))
        {
            throw OML_Error(OML_ERR_SCALAR, 4, OML_VAR_VALUE);
        }
    }

    if (inputs[0].IsMatrix() || inputs[0].IsScalar())
    {
        if (nargin == 5)
        {
            throw OML_Error(OML_ERR_NUMARGIN);
        }

        const hwMatrix* time = inputs[0].ConvertToMatrix();
        hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

        hwMathStatus status = GausPulse(*time, fc, bw, bwr, *waveForm);
        BuiltInFuncsUtils::CheckMathStatus(eval, status);
        outputs.push_back(waveForm);
    }
    else if (inputs[0].IsString())
    {
        if (inputs[0].StringVal() == "cutoff")
        {
            double tc;
            double tpr = -60.0;

            if (nargin > 4)
            {
                if (inputs[4].IsScalar())
                {
                    tpr = inputs[4].Scalar();
                }
                else if (!(inputs[4].IsMatrix() && inputs[4].Matrix()->Is0x0()))
                {
                    throw OML_Error(OML_ERR_SCALAR, 5, OML_VAR_VALUE);
                }
            }

            hwMathStatus status = GausPulse(fc, bw, bwr, tpr, tc);

            if (!status.IsOk())
            {
                status.SetArg1(status.GetArg1() + 1);
            }

            outputs.push_back(tc);
        }
        else
        {
            throw OML_Error(OML_ERR_BAD_STRING, 1, OML_VAR_VALUE);
        }
    }

    return true;
}
//------------------------------------------------------------------------------
//! Computes pulse train signal [gauspuls command]
//------------------------------------------------------------------------------
bool OmlPulsTran(EvaluatorInterface           eval,
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    int nargin = static_cast<int> (inputs.size());

    if (nargin < 3 || nargin > 8)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_VECTOR, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsMatrix() && !inputs[1].IsScalar())
    {
        throw OML_Error(OML_ERR_VECTOR, 2, OML_VAR_TYPE);
    }

    const hwMatrix* time  = inputs[0].ConvertToMatrix();
    const hwMatrix* delay = inputs[1].ConvertToMatrix();
    const hwMatrix* pulse = nullptr;
    std::string func;
    hwMathStatus status;
    hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

    if (inputs[2].IsString())
    {
        func = inputs[2].StringVal();

        hwMatrix args(nargin - 3, hwMatrix::REAL);

        for (int i = 3; i < nargin; ++i)
        {
            if (!inputs[i].Scalar())
                throw OML_Error(OML_ERR_SCALAR, i + 1, OML_VAR_TYPE);

            args(i - 3) = inputs[i].Scalar();
        }

        status = PulsTran(*time, *delay, func, args, *waveForm);
    }
    else if (inputs[2].IsMatrix() || inputs[2].IsScalar())
    {
        pulse = inputs[2].ConvertToMatrix();

        double fs = 1.0;

        if (nargin > 3)
        {
            if (!inputs[3].IsScalar())
                throw OML_Error(OML_ERR_SCALAR, 4, OML_VAR_TYPE);

            fs = inputs[3].Scalar();
        }

        std::string method = "linear";

        if (nargin > 4)
        {
            if (!inputs[4].IsString())
                throw OML_Error(OML_ERR_STRING, 5, OML_VAR_TYPE);

            method = inputs[4].StringVal();
        }

        status = PulsTran(*time, *delay, *pulse, fs, method, *waveForm);
    }
    else
    {
        throw OML_Error(OML_ERR_STRINGVECTOR, 3, OML_VAR_TYPE);
    }

    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(waveForm);
    return true;
}
//------------------------------------------------------------------------------
//! Computes square wave signal [square command]
//------------------------------------------------------------------------------
bool OmlSquare(EvaluatorInterface           eval,
               const std::vector<Currency>& inputs,
               std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    const hwMatrix* time = inputs[0].ConvertToMatrix();
    double duty = 50.0;

    if (nargin == 2)
    {
        if (!inputs[1].IsScalar())
        {
            throw OML_Error(OML_ERR_SCALAR, 2, OML_VAR_TYPE);
        }

        duty = inputs[1].Scalar();
    }

    hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = SquarePulse(*time, duty, *waveForm);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(waveForm);
    return true;
}
//------------------------------------------------------------------------------
//! Computes sawtooth wave signal [sawtooth command]
//------------------------------------------------------------------------------
bool OmlSawtooth(EvaluatorInterface           eval,
                 const std::vector<Currency>& inputs,
                 std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    const hwMatrix* time = inputs[0].ConvertToMatrix();
    double width = 1.0;

    if (nargin == 2)
    {
        if (!inputs[1].IsScalar())
        {
            throw OML_Error(OML_ERR_SCALAR, 2, OML_VAR_TYPE);
        }

        width = inputs[1].Scalar();
    }

    hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = SawToothPulse(*time, width, *waveForm);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(waveForm);
    return true;
}
//------------------------------------------------------------------------------
//! Computes Dirichlet wave signal [diric command]
//------------------------------------------------------------------------------
bool OmlDiric(EvaluatorInterface           eval,
              const std::vector<Currency>& inputs,
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin != 2)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    if (!inputs[1].IsPositiveInteger())
    {
        throw OML_Error(OML_ERR_POSINTEGER, 2, OML_VAR_VALUE);
    }

    const hwMatrix* time = inputs[0].ConvertToMatrix();
    int n = static_cast<int> (inputs[1].Scalar());

    hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = Diric(*time, n, *waveForm);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(waveForm);
    return true;
}
//------------------------------------------------------------------------------
//! Computes chirp pulse function [chirp command]
//------------------------------------------------------------------------------
bool OmlChirp(EvaluatorInterface           eval,
              const std::vector<Currency>& inputs,
              std::vector<Currency>&       outputs)
{
    size_t nargin = inputs.size();

    if (nargin < 1 || nargin > 6)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    if (!inputs[0].IsMatrix() && !inputs[0].IsScalar())
    {
        throw OML_Error(OML_ERR_MATRIX, 1, OML_VAR_TYPE);
    }

    const hwMatrix* time = inputs[0].ConvertToMatrix();
    double f0 = 0.0;
    double t1 = 1.0;
    double f1 = 100.0;
    std::string shape = "linear";
    double phase = 0.0;

    if (nargin > 1)
    {
        if (inputs[1].IsScalar())
            f0 = inputs[1].Scalar();
        else if (!inputs[1].IsMatrix() || !inputs[1].Matrix()->Is0x0())
            throw OML_Error(OML_ERR_SCALAR, 2, OML_VAR_TYPE);
    }

    if (nargin > 2)
    {
        if (inputs[2].IsScalar())
            t1 = inputs[2].Scalar();
        else if (!inputs[2].IsMatrix() || !inputs[2].Matrix()->Is0x0())
            throw OML_Error(OML_ERR_SCALAR, 3, OML_VAR_TYPE);
    }

    if (nargin > 3)
    {
        if (inputs[3].IsScalar())
            f1 = inputs[3].Scalar();
        else if (!inputs[3].IsMatrix() || !inputs[3].Matrix()->Is0x0())
            throw OML_Error(OML_ERR_SCALAR, 4, OML_VAR_TYPE);
    }

    if (nargin > 4)
    {
        if (inputs[4].IsString())
            shape = inputs[4].StringVal();
        else if (!inputs[4].IsMatrix() || !inputs[4].Matrix()->Is0x0())
            throw OML_Error(OML_ERR_STRING, 5, OML_VAR_TYPE);
    }

    if (nargin > 5)
    {
        if (inputs[5].IsScalar())
            phase = inputs[5].Scalar();
        else if (!inputs[5].IsMatrix() || !inputs[5].Matrix()->Is0x0())
            throw OML_Error(OML_ERR_SCALAR, 6, OML_VAR_TYPE);
    }

    hwMatrix* waveForm = EvaluatorInterface::allocateMatrix();

    hwMathStatus status = ChirpPulse(*time, f0, t1, f1, shape.c_str(), phase, *waveForm);

    BuiltInFuncsUtils::CheckMathStatus(eval, status);

    outputs.push_back(waveForm);
    return true;
}
//------------------------------------------------------------------------------
// Returns toolbox version
//------------------------------------------------------------------------------
double GetToolboxVersion(EvaluatorInterface eval)
{
    return TBOXVERSION;
}

