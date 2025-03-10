/**
* @file fminunc.cxx
* @date September 2017
* Copyright (C) 2017-2018 Altair Engineering, Inc.  
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

#include "OmlOptimizationTboxFuncs.h"

#include "BuiltInFuncsUtils.h"
#include "FunctionInfo.h"
#include "OML_Error.h"
#include "StructData.h"
#include "hwMathException.h"

#include "hwOptimizationFuncs.h"

// File scope variables and functions
static std::vector<EvaluatorInterface*> FMINUNC_eval_ptr_stack;
static std::vector<FunctionInfo*>       FMINUNC_oml_func_stack;
static std::vector<FUNCPTR>             FMINUNC_oml_pntr_stack;
static std::vector<bool>                FMINUNC_oml_anon_stack;
static std::vector<bool>                FMINUNC_oml_Prow_stack; // row vector checks

//------------------------------------------------------------------------------
// Wrapper for the objective function called in OmlFminunc by oml scripts
//------------------------------------------------------------------------------
static hwMathStatus FMinUnconFunc(const hwMatrix& P,
                                  const hwMatrix* userData,
                                  double&         min)
{
    EvaluatorInterface* FMINUNC_eval_ptr        = FMINUNC_eval_ptr_stack.back();
    FunctionInfo*       FMINUNC_oml_func        = FMINUNC_oml_func_stack.back();
    FUNCPTR             FMINUNC_oml_pntr        = FMINUNC_oml_pntr_stack.back();
    bool                FMINUNC_oml_func_isanon = FMINUNC_oml_anon_stack.back();
    bool                FMINUNC_oml_func_isProw = FMINUNC_oml_Prow_stack.back();

    std::vector<Currency> inputs;
    std::vector<Currency> outputs;
    hwMatrix* P_temp = EvaluatorInterface::allocateMatrix(&P);

    if (FMINUNC_oml_func_isProw && P_temp->M() > 1)
        P_temp->Transpose();

    inputs.push_back(P_temp);

    try
    {
        FMINUNC_eval_ptr->Mark();

        if (FMINUNC_oml_func_isanon)
        {
            Currency result = FMINUNC_eval_ptr->CallInternalFunction(
                FMINUNC_oml_func, inputs);
            outputs.push_back(result);
        }
        else if (FMINUNC_oml_func)
        {
            outputs = FMINUNC_eval_ptr->DoMultiReturnFunctionCall(
                FMINUNC_oml_func, inputs, 1, 1, true);
        }
        else if (FMINUNC_oml_pntr)
        {
            outputs = FMINUNC_eval_ptr->DoMultiReturnFunctionCall(
                FMINUNC_oml_pntr, inputs, 1, 1, true);
        }
        else
        {
            return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
        }

        FMINUNC_eval_ptr->Unmark();
    }
    catch (OML_Error&)
    {
        FMINUNC_eval_ptr->Restore();
        return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
    }
    catch (hwMathException&)
    {
        FMINUNC_eval_ptr->Restore();
        return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
    }

    if (outputs.size() == 1 || outputs.size() == 2)
    {
        Currency obj = outputs[0];

        if (obj.IsScalar())
        {
            min = obj.Scalar();
        }
        else if (obj.IsMatrix())
        {
            const hwMatrix* minMatrix = obj.Matrix();

            if (minMatrix->Size() != 1)
            {
                return hwMathStatus(HW_MATH_ERR_USERFUNCSIZE, 111);
            }

            if (minMatrix->IsReal())
            {
                min = (*minMatrix)(0);
            }
            else if (minMatrix->z(0).IsReal())
            {
                min = minMatrix->z(0).Real();
            }
            else
            {
                return hwMathStatus(HW_MATH_ERR_USERFUNCREAL, 111);
            }
        }
        else
        {
            return hwMathStatus(HW_MATH_ERR_USERFUNCREAL, 111);
        }
    }
    else
    {
        return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
    }

    return hwMathStatus();
}
//------------------------------------------------------------------------------
// Gradient vector function wrapper
//------------------------------------------------------------------------------
static hwMathStatus FMinUnconGradient(const hwMatrix& P, 
                                      const hwMatrix* userData, 
                                      hwMatrix&       grad)
{
    EvaluatorInterface* FMINUNC_eval_ptr        = FMINUNC_eval_ptr_stack.back();
    FunctionInfo*       FMINUNC_oml_func        = FMINUNC_oml_func_stack.back();
    FUNCPTR             FMINUNC_oml_pntr        = FMINUNC_oml_pntr_stack.back();
    bool                FMINUNC_oml_func_isProw = FMINUNC_oml_Prow_stack.back();

    std::vector<Currency> inputs;
    std::vector<Currency> outputs;
    hwMatrix* P_temp = EvaluatorInterface::allocateMatrix(&P);

    if (FMINUNC_oml_func_isProw && P_temp->M() > 1)
        P_temp->Transpose();

    inputs.push_back(P_temp);

    try
    {
        FMINUNC_eval_ptr->Mark();

        if (FMINUNC_oml_func)
        {
            outputs = FMINUNC_eval_ptr->DoMultiReturnFunctionCall(FMINUNC_oml_func, 
                      inputs, 1, 2, true);
        }
        else if (FMINUNC_oml_pntr)
        {
            outputs = FMINUNC_eval_ptr->DoMultiReturnFunctionCall(FMINUNC_oml_pntr, 
                      inputs, 1, 2, true);
        }
        else
        {
            return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
        }
        
        FMINUNC_eval_ptr->Unmark();
    }
    catch (OML_Error&)
    {
        FMINUNC_eval_ptr->Restore();
        return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
    }
    catch (hwMathException&)
    {
        FMINUNC_eval_ptr->Restore();
        return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
    }

    if (outputs.size() != 2)
    {
        return hwMathStatus(HW_MATH_ERR_USERFUNCFAIL, 111);
    }

    Currency objGrad = outputs[1];

    if (objGrad.IsScalar())
    {
        if (P.Size() != 1 || grad.Size() != 1)
        {
            return hwMathStatus(HW_MATH_ERR_USERFUNCSIZE, 111);
        }
        grad(0) = objGrad.Scalar();
    }
    else if (objGrad.IsMatrix())
    {
        const hwMatrix* gradMatrix = objGrad.Matrix();

        if (!gradMatrix->IsReal())
        {
            return hwMathStatus(HW_MATH_ERR_USERFUNCREAL, 111);
        }
        if (gradMatrix->M() != grad.M() || gradMatrix->N() != grad.N())
        {
            return hwMathStatus(HW_MATH_ERR_USERFUNCSIZE, 111);
        }
        for (int i = 0; i < grad.M(); ++i)
        {
            grad(i) = (*gradMatrix)(i);
        }
    }
    else
    {
        return hwMathStatus(HW_MATH_ERR_USERFUNCREAL, 111);
    }
      
    return hwMathStatus();
}
//------------------------------------------------------------------------------
// Finds the unconstrained minimum of a real function [fminunc]
//------------------------------------------------------------------------------
bool OmlFminunc(EvaluatorInterface           eval, 
                const std::vector<Currency>& inputs, 
                std::vector<Currency>&       outputs)
{
    int nargin = eval.GetNarginValue();
    int nargout = eval.GetNargoutValue();

    if (nargin < 2 || nargin > 3)
    {
        throw OML_Error(OML_ERR_NUMARGIN);
    }

    Currency cur1 = inputs[0];

    if (!cur1.IsFunctionHandle())
    {
        throw OML_Error(OML_ERR_HANDLE, 1, OML_VAR_TYPE);
    }

    std::string   funcName = cur1.FunctionHandle()->FunctionName();
    FunctionInfo* funcInfo = nullptr;
    FUNCPTR       funcPntr = nullptr;

    if (funcName == "anonymous")
    {
        funcInfo = cur1.FunctionHandle();
        (funcInfo->RedirectedFunction().empty()) ?
            FMINUNC_oml_anon_stack.push_back(true):  // true anonymous function
            FMINUNC_oml_anon_stack.push_back(false); // handle to an inner function
    }
    else
    {
        FMINUNC_oml_anon_stack.push_back(false);

        if (!eval.FindFunctionByName(funcName, &funcInfo, &funcPntr, nullptr))
        {
            throw OML_Error(OML_ERR_FUNCNAME, 1);
        }
    }

    if (funcInfo && funcInfo->Parameters().size() != 1)
    {
        throw OML_Error(OML_ERR_NUMARGIN, 1);
    }

    Currency cur2 = inputs[1];

    if (!cur2.IsScalar() && !cur2.IsMatrix())
        throw OML_Error(OML_ERR_SCALARVECTOR, 2);

    hwMatrix* optPoint = EvaluatorInterface::
                         allocateMatrix(cur2.ConvertToMatrix());

    if (!optPoint->IsFinite())
        throw OML_Error(OML_ERR_FINITE, 2);

    bool   displayHist        = false;
    bool   analyticalGradient = false;
    int    maxIter            = 400;
    int    maxFuncEval        = 1000000;
    double tolf               = 1.0e-7;
    double tolx               = 1.0e-7;

    if (nargin > 2)
    {
        if (inputs[2].IsStruct())
        {
            StructData* opt = inputs[2].Struct();

            if (opt->N() != 1)
            {
                throw OML_Error(OML_ERR_OPTION, 3);
            }           

            Currency gradientC = opt->GetValue(0, -1, "GradObj");

            if (!gradientC.IsEmpty())
            {
                if (!gradientC.IsString())
                {
                    throw OML_Error(OML_ERR_FUNCSWITCH, 3, OML_VAR_JACOBIAN);
                }

                std::string val (gradientC.StringVal());

                if (val != "on" && val != "off")
                {
                    throw OML_Error(OML_ERR_FUNCSWITCH, 3, OML_VAR_JACOBIAN);
                }
                analyticalGradient = (val == "on") ? true : false;
            }

            Currency maxIterC = opt->GetValue(0, -1, "MaxIter");

            if (!maxIterC.IsEmpty())
            {
                if (!maxIterC.IsPositiveInteger())
                {
                    throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_MAXITER);
                }
                maxIter = static_cast<int>(maxIterC.Scalar());                   
            }

            Currency maxFunEvalC = opt->GetValue(0, -1, "MaxFunEvals");

            if (!maxFunEvalC.IsEmpty())
            {
                if (!maxFunEvalC.IsPositiveInteger())
                {
                    throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_MAXFUNEVALS);
                }
                maxFuncEval = static_cast<int>(maxFunEvalC.Scalar());
            }

            Currency tolfC = opt->GetValue(0, -1, "TolFun");

            if (!tolfC.IsEmpty())
            {
                if (!tolfC.IsScalar())
                {
                    throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_TOLFUN);
                }
                tolf = tolfC.Scalar();
            }

            Currency tolxC = opt->GetValue(0, -1, "TolX");

            if (!tolxC.IsEmpty())
            {
                if (!tolxC.IsScalar())
                {
                    throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_TOLX);
                }
                tolx = tolxC.Scalar();
            }

            Currency displayC = opt->GetValue(0, -1, "Display");

            if (!displayC.IsEmpty())
            {
                if (!displayC.IsString())
                {
                    throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_DISPLAY);
                }

                std::string val (displayC.StringVal());

                if (val == "iter")
                {
                    displayHist = true;
                }
                else if (val == "off")
                {
                    displayHist = false;
                }
                else
                {
                    throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_DISPLAY);
                }
            }
        }
        else if (inputs[2].IsMatrix())
        {
            if (inputs[2].Matrix()->M() != 0 || inputs[2].Matrix()->M() != 0)
            {
                throw OML_Error(OML_ERR_OPTIONVAL, 3, OML_VAR_DISPLAY);
            }
        }
        else
        {
            throw OML_Error(OML_ERR_HANDLE_EMPTY, 3);
        }
    }

    // set file scope variables
    FMINUNC_eval_ptr_stack.push_back(&eval);
    FMINUNC_oml_func_stack.push_back(funcInfo);
    FMINUNC_oml_pntr_stack.push_back(funcPntr);

    if (optPoint->N() > 1)
        FMINUNC_oml_Prow_stack.push_back(true);
    else
        FMINUNC_oml_Prow_stack.push_back(false);

    // call algorithm
    hwMatrix*    objHist    = (displayHist || nargout > 3) ?
                              EvaluatorInterface::allocateMatrix() : nullptr;
    hwMatrix*    designHist = (nargout > 3) ?
                              EvaluatorInterface::allocateMatrix() : nullptr;
    hwMatrix*    userData   = nullptr;
    hwMathStatus status;
    double       minVal;

    if (analyticalGradient)
    {
        status = FMinUncon(FMinUnconFunc, FMinUnconGradient, *optPoint, minVal,
            maxIter, maxFuncEval, tolf, tolx, objHist, designHist, userData);
    }
    else
    {
        status = FMinUncon(FMinUnconFunc, (UnConMinGradFunc) NULL, *optPoint, minVal,
            maxIter, maxFuncEval, tolf, tolx, objHist, designHist, userData);
    }

    // display history
    if (displayHist && (status.IsOk() || status.IsWarning() || status.IsInfoMsg()))
    {
        std::string line = "Iteration    f(x)\n";
        eval.PrintResult(line);

        for (int i = 0; i < objHist->Size(); ++i)
        {
            char intChar[8];
            char numChar[20];

#ifdef OS_WIN  // sprintf_s is a safer option
            sprintf_s(intChar, sizeof(intChar), "%5d",    i+1);
            sprintf_s(numChar, sizeof(numChar), "%12.5f", (*objHist)(i));
#else
            sprintf(intChar, "%5d", i+1);
            sprintf(numChar, "%12.5f", (*objHist)(i));
#endif
            line = std::string(intChar) + "  " + std::string(numChar) + "\n";

            eval.PrintResult(line);
        }

        line = "\n";
        eval.PrintResult(line);
    }

    if (!status.IsOk())
    {
        if (status == HW_MATH_WARN_MAXITERATE ||
            status == HW_MATH_WARN_MAXFUNCEVAL ||
            status == HW_MATH_WARN_NOTCONVERGE ||
            status.IsInfoMsg())
        {
            // warning message has been replaced by a return value
        }
        else
        {
            if (status.GetArg1() == 111)
            {
                if (funcName == "anonymous")
                {
                    funcName = funcInfo->RedirectedFunction();

                    if (!funcName.size())
                        funcName = "anonymous";
                }

                status.SetUserFuncName(funcName);
                status.SetArg1(1);
            }
            else if (status.GetArg1() == 3)
            {
                status.SetArg1(2);
            }
            else if (status.GetArg1() > 4 && status.GetArg1() < 9)
            {
                status.SetArg1(3);
            }
            else
            {
                status.ResetArgs();
            }

            if (status.IsWarning())
            {
                BuiltInFuncsUtils::SetWarning(eval, status.GetMessageString());
            }
            else
            {
                FMINUNC_eval_ptr_stack.clear();
                FMINUNC_oml_func_stack.clear();
                FMINUNC_oml_pntr_stack.clear();
                FMINUNC_oml_anon_stack.clear();
                FMINUNC_oml_Prow_stack.clear();

                if (optPoint)
                {
                    delete optPoint;
                    optPoint = nullptr;
                }

                if (objHist)
                {
                    delete objHist;
                    objHist = nullptr;
                }

                if (designHist)
                {
                    delete designHist;
                    designHist = nullptr;
                }

                throw OML_Error(status);
            }
        }
    }

    // pack outputs
    outputs.push_back(optPoint);

    if (nargout > 1)
        outputs.push_back(minVal);

    if (nargout > 2)
    {
        if (status == HW_MATH_INFO_TOLFCONV)
            outputs.push_back(1.0);
        else if (status == HW_MATH_INFO_TOLXCONV)
            outputs.push_back(2.0);
        else if (status == HW_MATH_INFO_TOLFCONV_R)
            outputs.push_back(3.0);
        else if (status == HW_MATH_INFO_TOLXCONV_R)
            outputs.push_back(4.0);
        else if (status == HW_MATH_WARN_MAXITERATE ||
                 status == HW_MATH_WARN_MAXFUNCEVAL ||
                 status == HW_MATH_WARN_NOTCONVERGE)
            outputs.push_back(0.0);
        else if (status == HW_MATH_INFO_SMALLTRUST)
            outputs.push_back(-3.0);
    }

    if (nargout > 3)
    {
        objHist->Transpose();
        designHist->Transpose();

        Currency out = EvaluatorInterface::allocateStruct();
        out.Struct()->SetValue(0, -1, "iterations", maxIter);
        out.Struct()->SetValue(0, -1, "nfev",       maxFuncEval);
        out.Struct()->SetValue(0, -1, "xiter",      designHist);
        out.Struct()->SetValue(0, -1, "fvaliter",   objHist);
        outputs.push_back(out);
    }
    else
    {
        if (objHist)
        {
            delete objHist;
            objHist = nullptr;
        }

        if (designHist)
        {
            delete designHist;
            designHist = nullptr;
        }
    }

    FMINUNC_eval_ptr_stack.pop_back();
    FMINUNC_oml_func_stack.pop_back();
    FMINUNC_oml_pntr_stack.pop_back();
    FMINUNC_oml_anon_stack.pop_back();
    FMINUNC_oml_Prow_stack.pop_back();

    return true;
}
