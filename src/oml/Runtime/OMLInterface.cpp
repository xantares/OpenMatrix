/**
* @file OMLInterface.cpp
* @date January 2017
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

#include "OMLInterface.h"

#include "Currency.h"
#include "StructData.h"
#include "FunctionInfo.h"
#include "hwComplex.h"
#include "hwMatrix_NMKL.h"
#include "hwMatrixN_NMKL.h"
#include "hwMatrixS_NMKL.h"
#include "OML_Error.h"

OMLImplBase::OMLImplBase(EvaluatorInterface* in_eval)
{ 
	_eval = in_eval;
	_eval->CacheBCIPointer(this);
}

OMLImplBase::~OMLImplBase()
{
}

OMLInterfaceImpl::OMLInterfaceImpl(EvaluatorInterface* in_eval) : OMLImplBase(in_eval)
{
}

OMLInterfaceImpl::~OMLInterfaceImpl()
{
}

void OMLInterfaceImpl::RegisterFunction(const char* name, ALT_FUNCPTR fp)
{
	_eval->RegisterBuiltInFunction(name, fp);
}

void OMLInterfaceImpl::RegisterHiddenFunction(const char* name, ALT_FUNCPTR fp)
{
	_eval->RegisterBuiltInFunction(name, fp);
	_eval->LockBuiltInFunction(name);
}

void OMLInterfaceImpl::RegisterFunctionWithMetadata(const char* name, ALT_FUNCPTR fp, const char* module,
	                                                int in_vals, int out_vals)
{
	_eval->RegisterBuiltInFunction(name, fp, FunctionMetaData(in_vals, out_vals, module));
}

void OMLInterfaceImpl::RegisterFunctionWithMetadata(const char* name, ALT_FUNCPTR fp, const char* module,
	int in_vals, int out_vals, bool thread_safe)
{
	_eval->RegisterBuiltInFunction(name, fp, FunctionMetaData(in_vals, out_vals, module), thread_safe);
}

void OMLInterfaceImpl::ThrowError(const char* message)
{
	throw OML_Error(message);
}

int OMLInterfaceImpl::Nargout() const
{
	return _eval->GetNargoutValue();
}

OMLCurrencyList* OMLInterfaceImpl::CreateCurrencyList()
{
	return new OMLCurrencyListImpl(_eval);
}

const OMLCurrency* OMLInterfaceImpl::CallFunction(const OMLFunctionHandle* handle, OMLCurrencyList* inputs)
{
	std::vector<Currency> ins;

	for (int j=0; j<inputs->Size(); j++)
	{
		const OMLCurrency* temp = inputs->Get(j);
		OMLCurrencyImpl* impl = (OMLCurrencyImpl*)temp;

		ins.push_back(impl->GetCurrency());
	}

	OMLFunctionHandleImpl* fh = (OMLFunctionHandleImpl*)handle;
	Currency result = _eval->CallInternalFunction(fh->GetFunctionInfo(), ins);
	return new OMLCurrencyImpl(_eval, result);
}

const OMLCurrency* OMLInterfaceImpl::CallFunction(const char* name, OMLCurrencyList* inputs)
{
	std::vector<Currency> ins;

	for (int j = 0; j < inputs->Size(); j++)
	{
		const OMLCurrency* temp = inputs->Get(j);
		OMLCurrencyImpl* impl = (OMLCurrencyImpl*)temp;

		ins.push_back(impl->GetCurrency());
	}

	Currency result = _eval->CallFunction(name, ins);
	return new OMLCurrencyImpl(_eval, result);
}

const OMLCurrency* OMLInterfaceImpl::GetGlobalValue(const char* varname)
{
	Currency result = _eval->GetGlobalValue(varname);
	return new OMLCurrencyImpl(_eval, result);
}

OMLCurrencyImpl::OMLCurrencyImpl(EvaluatorInterface* in_eval, Currency in_cur) : OMLImplBase(in_eval)
{
	_cur = in_cur;
}

OMLCurrencyImpl::~OMLCurrencyImpl()
{
}

bool OMLCurrencyImpl::IsScalar() const
{
	return _cur.IsScalar();
}

bool OMLCurrencyImpl::IsComplex() const
{
	return _cur.IsComplex();
}

bool OMLCurrencyImpl::IsString() const
{
	return _cur.IsString();
}

bool OMLCurrencyImpl::IsMatrix() const
{
	return _cur.IsMatrix();
}

bool OMLCurrencyImpl::IsNDMatrix() const
{
	return _cur.IsNDMatrix();
}

bool OMLCurrencyImpl::IsSparseMatrix() const
{
	return _cur.IsSparse();
}

bool OMLCurrencyImpl::IsCellArray() const
{
	return _cur.IsCellArray();
}

bool OMLCurrencyImpl::IsNDCellArray() const
{
	return _cur.IsNDCellArray();
}

bool OMLCurrencyImpl::IsStruct() const
{
	return _cur.IsStruct();
}

bool OMLCurrencyImpl::IsFunctionHandle() const
{
	return _cur.IsFunctionHandle();
}

double OMLCurrencyImpl::GetScalar() const
{
	return _cur.Scalar();
}

bool OMLCurrencyImpl::IsLogical() const
{
	return _cur.IsLogical();
}

bool OMLCurrencyImpl::GetLogical() const
{
	return _cur.Scalar() ? true : false;
}

const char* OMLCurrencyImpl::GetString() const
{
	std::string temp = _cur.StringVal();
	size_t      temp_len = temp.size();

	char* ret = new char [temp_len+1];
	strcpy(ret, temp.c_str());
	ret[temp_len] = 0;
	return ret;
}

const OMLCellArray* OMLCurrencyImpl::GetCellArray() const
{
	return new OMLCellArrayImpl(_eval, _cur.CellArray());
}

const OMLStruct* OMLCurrencyImpl::GetStruct() const
{
	return new OMLStructImpl(_eval, _cur.Struct());
}

const OMLComplex* OMLCurrencyImpl::GetComplex() const
{
	hwComplex cplx = _cur.Complex();
	return new OMLComplexImpl(_eval, cplx.Real(), cplx.Imag());
}

const OMLMatrix* OMLCurrencyImpl::GetMatrix() const
{
	return new OMLMatrixImpl(_eval, _cur.Matrix());
}

const OMLNDMatrix* OMLCurrencyImpl::GetNDMatrix() const
{
	return new OMLNDMatrixImpl(_eval, _cur.MatrixN());
}

const OMLFunctionHandle* OMLCurrencyImpl::GetFunctionHandle() const
{
	return new OMLFunctionHandleImpl(_eval, _cur.FunctionHandle());
}

OMLMatrixImpl::OMLMatrixImpl(EvaluatorInterface* in_eval, const hwMatrix* in_mtx) : OMLImplBase(in_eval)
{
	_mtx = (hwMatrix*)in_mtx;
}

OMLMatrixImpl::~OMLMatrixImpl()
{
}

bool OMLMatrixImpl::IsReal() const
{
	return _mtx->IsReal();
}

int OMLMatrixImpl::GetRows() const
{
	return _mtx->M();
}

int OMLMatrixImpl::GetCols() const
{
	return _mtx->N();
}

const double* OMLMatrixImpl::GetRealData() const
{
	if (_mtx->IsReal())
		return _mtx->GetRealData();
	
	hwTMatrix<double, hwComplex> temp;
	hwMathStatus stat = _mtx->UnpackComplex(&temp, NULL);
	temp.OwnData(false);
	return temp.GetRealData();
}

const double* OMLMatrixImpl::GetImaginaryData() const
{
	if (_mtx->IsReal())
		return NULL;

	hwTMatrix<double, hwComplex> temp;
	hwMathStatus stat = _mtx->UnpackComplex(NULL, &temp);
	temp.OwnData(false);
	return temp.GetRealData();
}

OMLCurrency* OMLMatrixImpl::GetCurrency() const
{
	return (new OMLCurrencyImpl(_eval, _mtx));
}

hwMatrix* OMLMatrixImpl::GetMatrixPointer() const
{
	return _mtx;
}

OMLNDMatrixImpl::OMLNDMatrixImpl(EvaluatorInterface* in_eval, const hwMatrixN* in_mtx) : OMLImplBase(in_eval)
{
	_mtx = (hwMatrixN*)in_mtx;
}

OMLNDMatrixImpl::~OMLNDMatrixImpl()
{
}

bool OMLNDMatrixImpl::IsReal() const
{
	return _mtx->IsReal();
}

int OMLNDMatrixImpl::GetNumDimension() const
{
	std::vector<int> dims = _mtx->Dimensions();
	return (int)dims.size();
}

int OMLNDMatrixImpl::GetDimension(int num) const
{
	std::vector<int> dims = _mtx->Dimensions();
	
	return dims[num];
}

const double* OMLNDMatrixImpl::GetRealData() const
{
	return &((*_mtx)(0));
}

const double* OMLNDMatrixImpl::GetImaginaryData() const
{
	return NULL;
}

OMLCurrency* OMLNDMatrixImpl::GetCurrency() const
{
	return (new OMLCurrencyImpl(_eval, _mtx));
}

hwMatrixN* OMLNDMatrixImpl::GetMatrixPointer() const
{
	return _mtx;
}

OMLSparseMatrixImpl::OMLSparseMatrixImpl(EvaluatorInterface* in_eval, const hwMatrixS* in_mtx) : OMLImplBase(in_eval)
{
	_mtxs = (hwMatrixS*)in_mtx;
}

OMLSparseMatrixImpl::~OMLSparseMatrixImpl()
{
}

bool OMLSparseMatrixImpl::IsReal() const
{
	return _mtxs->IsReal();
}

int OMLSparseMatrixImpl::GetRows() const
{
	return _mtxs->M();
}

int OMLSparseMatrixImpl::GetCols() const
{
	return _mtxs->N();
}

const double* OMLSparseMatrixImpl::GetRealData() const
{
	if (_mtxs->IsReal())
		return _mtxs->GetRealData();

	hwMatrixS temp;
	_mtxs->UnpackComplex(&temp, NULL);
	return temp.GetRealData();
}

const double* OMLSparseMatrixImpl::GetImaginaryData() const
{
	if (_mtxs->IsReal())
		return NULL;

	hwMatrixS temp;
	_mtxs->UnpackComplex(NULL, &temp);
	return temp.GetRealData();
}

const int* OMLSparseMatrixImpl::GetRowVector() const
{
	int num_vals = _mtxs->NNZ();
	//	void NZinfo(int first, int last, std::vector<int> & row,
	//	std::vector<int> & col, hwTMatrix<T1, T2> & value) const;
		
	static std::vector<int> row_vec;
	std::vector<int> col_vec;

	hwMatrix val_mat;

	_mtxs->NZinfo(0, num_vals, row_vec, col_vec, val_mat);

	return row_vec.data();
}

const int* OMLSparseMatrixImpl::GetColumnVector() const
{
	int num_vals = _mtxs->NNZ();

	std::vector<int> row_vec;
	static std::vector<int> col_vec;

	hwMatrix val_mat;

	_mtxs->NZinfo(0, num_vals-1, row_vec, col_vec, val_mat);

	return col_vec.data();
}

OMLCurrency* OMLSparseMatrixImpl::GetCurrency() const
{
	return (new OMLCurrencyImpl(_eval, _mtxs));
}

hwMatrixS* OMLSparseMatrixImpl::GetMatrixPointer() const
{
	return _mtxs;
}

OMLCellArrayImpl::OMLCellArrayImpl(EvaluatorInterface* in_eval, HML_CELLARRAY* in_cells) : OMLImplBase(in_eval)
{
	_cells = in_cells;
	_is_temp = false;
}
OMLCellArrayImpl::OMLCellArrayImpl(EvaluatorInterface* in_eval, HML_CELLARRAY* in_cells, bool temp) : OMLImplBase(in_eval)
{
	_cells = in_cells;
	_is_temp = temp;
}

OMLCellArrayImpl::~OMLCellArrayImpl()
{
	if (_is_temp) 
		delete _cells;
}

OMLCurrency* OMLCellArrayImpl::GetValue(int index1) const
{
	return new OMLCurrencyImpl(_eval, (*_cells)(index1));
}

OMLCurrency* OMLCellArrayImpl::GetValue(int index1, int index2) const
{
	return new OMLCurrencyImpl(_eval, (*_cells)(index1, index2));
}

void OMLCellArrayImpl::SetValue(int index1, OMLCurrency* val)
{
	OMLCurrencyImpl* ci = (OMLCurrencyImpl*)val;
	(*_cells)(index1)   = ci->GetCurrency();
}

void OMLCellArrayImpl::SetValue(int index1, int index2, OMLCurrency* val)
{
	OMLCurrencyImpl* ci = (OMLCurrencyImpl*)val;
	(*_cells)(index1, index2)   = ci->GetCurrency();
}

int OMLCellArrayImpl::GetRows() const
{
	return _cells->M();
}

int OMLCellArrayImpl::GetCols() const
{
	return _cells->N();
}

OMLCurrency* OMLCellArrayImpl::GetCurrency() const
{
	return (new OMLCurrencyImpl(_eval, _cells));
}

HML_CELLARRAY* OMLCellArrayImpl::GetCells() const
{
	return _cells;
}

OMLNDCellArrayImpl::OMLNDCellArrayImpl(EvaluatorInterface* in_eval, HML_ND_CELLARRAY* in_cells) : OMLImplBase(in_eval)
{
	_cells = in_cells;
}

OMLNDCellArrayImpl::~OMLNDCellArrayImpl()
{
}

OMLCurrency* OMLNDCellArrayImpl::GetValue(int index1) const
{
	return new OMLCurrencyImpl(_eval, (*_cells)(index1));
}

void OMLNDCellArrayImpl::SetValue(int index1, OMLCurrency* val)
{
	OMLCurrencyImpl* ci = (OMLCurrencyImpl*)val;
	(*_cells)(index1) = ci->GetCurrency();
}

int OMLNDCellArrayImpl::GetNumDimension() const
{
	std::vector<int> dims = _cells->Dimensions();

	return (int)dims.size();
}

int OMLNDCellArrayImpl::GetDimension(int num) const
{
	std::vector<int> dims = _cells->Dimensions();

	return dims[num];
}

OMLCurrency* OMLNDCellArrayImpl::GetCurrency() const
{
	return (new OMLCurrencyImpl(_eval, _cells));
}

HML_ND_CELLARRAY* OMLNDCellArrayImpl::GetCells() const
{
	return _cells;
}

OMLFunctionHandleImpl::OMLFunctionHandleImpl(EvaluatorInterface* in_eval, const FunctionInfo* in_fh) : OMLImplBase(in_eval)
{
	_fi = (FunctionInfo*)in_fh;
}

OMLFunctionHandleImpl::~OMLFunctionHandleImpl()
{
}

FunctionInfo* OMLFunctionHandleImpl::GetFunctionInfo() const 
{ 
	return _fi; 
}

OMLCurrencyListImpl::OMLCurrencyListImpl(EvaluatorInterface* in_eval) : OMLImplBase(in_eval)
{
	_list = nullptr;
	_count = 0;
}

OMLCurrencyListImpl::~OMLCurrencyListImpl()
{
	delete [] _list;
}

int OMLCurrencyListImpl::Size() const
{
	return _count;
}

const OMLCurrency* OMLCurrencyListImpl::Get(int idx) const
{
	if (idx < 0)
		return NULL;

	if (idx >= _count)
		return NULL;

	return _list[idx];
}

void OMLCurrencyListImpl::Expand()
{
	OMLCurrency** new_list = new OMLCurrency* [_count+1];

	if (_list)
	{
		memcpy(new_list, _list, sizeof(OMLCurrency*)*_count);
		delete [] _list;
	}
	
	_list = new_list;
	_count++;
}

void OMLCurrencyListImpl::AddScalar(double val)
{
	Expand();
	_list[_count-1] = new OMLCurrencyImpl(_eval, val);
}

void OMLCurrencyListImpl::AddString(const char* str)
{
	Expand();
	_list[_count-1] = new OMLCurrencyImpl(_eval, str);
}

void OMLCurrencyListImpl::AddLogical(bool logical_val)
{
	Expand();
	_list[_count - 1] = new OMLCurrencyImpl(_eval, logical_val);
}

void OMLCurrencyListImpl::AddMatrix(OMLMatrix* mtx)
{
	Expand();
	_list[_count-1] = mtx->GetCurrency();
}

void OMLCurrencyListImpl::AddMatrix(const hwMatrix* mtx)
{
	Expand();

	hwMatrix* cheat = (hwMatrix*)mtx;
	cheat->IncrRefCount(); 

	_list[_count-1] = new OMLCurrencyImpl(_eval, (hwMatrix*)mtx);
}

void OMLCurrencyListImpl::AddNDMatrix(OMLNDMatrix* mtx)
{
	Expand();
	_list[_count-1] = mtx->GetCurrency();
}

void OMLCurrencyListImpl::AddNDMatrix(const hwMatrixN* mtx)
{
	Expand();

	hwMatrixN* cheat = (hwMatrixN*)mtx;
	cheat->IncrRefCount();

	_list[_count-1] = new OMLCurrencyImpl(_eval, (hwMatrixN*)mtx);
}

void OMLCurrencyListImpl::AddComplex(OMLComplex* cplx)
{
	Expand();
	_list[_count-1] = cplx->GetCurrency();
}

void OMLCurrencyListImpl::AddComplex(hwComplex cplx)
{
	Expand();
	_list[_count-1] = new OMLCurrencyImpl(_eval, cplx);
}

void OMLCurrencyListImpl::AddCellArray(HML_CELLARRAY* cells)
{
	Expand();
	
	cells->IncrRefCount();

	_list[_count-1] = new OMLCurrencyImpl(_eval, cells);
}

void OMLCurrencyListImpl::AddCellArray(OMLCellArray* array)
{
	Expand();
	_list[_count-1] = array->GetCurrency();
}

void OMLCurrencyListImpl::AddStruct(OMLStruct* in_struct)
{
	Expand();
	_list[_count-1] = in_struct->GetCurrency();
}

void OMLCurrencyListImpl::AddStruct(StructData* in_sd)
{
	Expand();
	_list[_count-1] = new OMLCurrencyImpl(_eval, new StructData(*in_sd));
}

void OMLCurrencyListImpl::AddFunctionHandle(FunctionInfo* fi)
{
	Expand();
	fi->IncrRefCount();
	_list[_count-1] = new OMLCurrencyImpl(_eval, fi);
}

double* OMLCurrencyListImpl::AllocateData(int size)
{
	return (new double [size]);
}

OMLCellArray* OMLCurrencyListImpl::CreateCellArray(int rows, int cols)
{
	HML_CELLARRAY* cells = new HML_CELLARRAY(rows, cols, HML_CELLARRAY::REAL);
	return new OMLCellArrayImpl(_eval, cells);
}

OMLCellArray* OMLCurrencyListImpl::CreateTemporaryCellArray(int rows, int cols)
{
	HML_CELLARRAY* cells = new HML_CELLARRAY(rows, cols, HML_CELLARRAY::REAL);
	return new OMLCellArrayImpl(_eval, cells, true);
}

OMLStruct* OMLCurrencyListImpl::CreateStruct(int rows, int cols)
{
	StructData* sd = new StructData();
	sd->DimensionNew(rows, cols);
	return new OMLStructImpl(_eval, sd);
}

OMLMatrix* OMLCurrencyListImpl::CreateMatrix(int rows, int cols, double* data)
{
	hwMatrix* mtx = new hwMatrix(rows, cols, data, hwMatrix::REAL);
	mtx->OwnData(true);
	return new OMLMatrixImpl(_eval, mtx);
}

OMLNDMatrix* OMLCurrencyListImpl::CreateNDMatrix(int num_dims, int* dims, double* data)
{
	std::vector<int> dim_vec;

	for (int j=0; j<num_dims; j++)
		dim_vec.push_back(dims[j]);

	hwMatrixN* mtx = new hwMatrixN(dim_vec, data, hwMatrixN::REAL);
	mtx->OwnData(1);
	return new OMLNDMatrixImpl(_eval, mtx);
}

OMLNDMatrix* OMLCurrencyListImpl::CreateNDMatrix(int num_dims, int* dims, double* real, double* imag)
{
	std::vector<int> dim_vec;

	for (int j=0; j<num_dims; j++)
		dim_vec.push_back(dims[j]);

	hwMatrixN mtx_1(dim_vec, real, hwMatrixN::REAL);
	hwMatrixN mtx_2(dim_vec, imag, hwMatrixN::REAL);
	mtx_1.OwnData(1);
	mtx_2.OwnData(1);

	hwMatrixN* result = new hwMatrixN;

	result->PackComplex(mtx_1, &mtx_2);

	return new OMLNDMatrixImpl(_eval, result);
}

OMLMatrix* OMLCurrencyListImpl::CreateMatrix(int rows, int cols, double* real, double* imag)
{
	hwMatrix mtx_1(rows, cols, real, hwMatrix::REAL);
	hwMatrix mtx_2(rows, cols, imag, hwMatrix::REAL);
	
	hwMatrix* result = new hwMatrix;

	result->PackComplex(mtx_1, &mtx_2);

	return new OMLMatrixImpl(_eval, result);
}

OMLCurrency* OMLCurrencyListImpl::CreateCurrencyFromDouble(double dbl)
{
	return new OMLCurrencyImpl(_eval, dbl);
}

OMLCurrency* OMLCurrencyListImpl::CreateCurrencyFromString(const char* str)
{
	return new OMLCurrencyImpl(_eval, str);
}

OMLComplex* OMLCurrencyListImpl::CreateComplex(double real, double imag)
{
	return NULL;
}

void OMLCurrencyListImpl::AddNDCellArray(HML_ND_CELLARRAY* cells)
{
	Expand();

	cells->IncrRefCount();

	_list[_count - 1] = new OMLCurrencyImpl(_eval, cells);
}

void OMLCurrencyListImpl::AddNDCellArray(OMLNDCellArray* array)
{
	Expand();
	_list[_count - 1] = array->GetCurrency();
}

void OMLCurrencyListImpl::AddSparseMatrix(OMLSparseMatrix* mtx)
{
	Expand();
	_list[_count - 1] = mtx->GetCurrency();
}

void OMLCurrencyListImpl::AddSparseMatrix(const hwMatrixS* mtx)
{
	Expand();

	hwMatrix* cheat = (hwMatrix*)mtx;
	cheat->IncrRefCount();

	_list[_count - 1] = new OMLCurrencyImpl(_eval, (hwMatrix*)mtx);
}

OMLNDCellArray*  OMLCurrencyListImpl::CreateNDCellArray(int num_dims, int* dims)
{
	std::vector<int> dim_vec;

	for (int j = 0; j<num_dims; j++)
		dim_vec.push_back(dims[j]);

	HML_ND_CELLARRAY* cells = new HML_ND_CELLARRAY(dim_vec, HML_ND_CELLARRAY::REAL);
	return new OMLNDCellArrayImpl(_eval, cells);
}

OMLSparseMatrix* OMLCurrencyListImpl::CreateSparseMatrix(int num_vals, int* ivec, int* jvec, double* vals, int rows, int cols)
{
	// expecting to create a sparse matrix of the specified size with all zero-values
	// the user can then fill in the non-zero values one-at-a-time
	std::vector<int> ivector(num_vals);
	std::vector<int> jvector(num_vals);

	hwMatrix* temp_mtx = new hwMatrix;

	for (int j = 0; j < num_vals; ++j)
	{
		ivector[j]     = ivec[j];
		jvector[j]     = jvec[j];
		(*temp_mtx)(j) = vals[j];
	}

	hwMatrixS* temp =  new hwMatrixS(ivector, jvector, *temp_mtx, rows, cols);
	return new OMLSparseMatrixImpl(_eval, temp);
}

OMLComplexImpl::OMLComplexImpl(EvaluatorInterface* in_eval, double real, double imag) : OMLImplBase(in_eval), cplx(real, imag)
{
}

OMLComplexImpl::~OMLComplexImpl()
{
}

double OMLComplexImpl::GetReal() const
{
	return cplx.Real();
}

double OMLComplexImpl::GetImag() const
{
	return cplx.Imag();
}

OMLCurrency* OMLComplexImpl::GetCurrency() const
{
	return (new OMLCurrencyImpl(_eval, cplx));
}

OMLStructImpl::OMLStructImpl(EvaluatorInterface* in_eval, StructData* in_sd) : OMLImplBase(in_eval)
{
	_sd = in_sd;
}

OMLStructImpl::~OMLStructImpl()
{
}

OMLCurrency* OMLStructImpl::GetValue(int index1, const char* field) const
{
	Currency cur = _sd->GetValue(index1, -1, field);
	return new OMLCurrencyImpl(_eval, cur);
}

OMLCurrency* OMLStructImpl::GetValue(int index1, int index2, const char* field) const
{
	Currency cur = _sd->GetValue(index1, index2, field);
	return new OMLCurrencyImpl(_eval, cur);
}

int OMLStructImpl::GetRows() const
{
	return _sd->M();
}

int OMLStructImpl::GetCols() const
{
	return _sd->N();
}

void OMLStructImpl::SetValue(int index, const char* field, OMLCurrency* val)
{
	OMLCurrencyImpl* ci = (OMLCurrencyImpl*)val;
	_sd->SetValue(index, -1, field, ci->GetCurrency());
}

void OMLStructImpl::SetValue(int index1, int index2, const char* field, OMLCurrency* val)
{
	OMLCurrencyImpl* ci = (OMLCurrencyImpl*)val;
	_sd->SetValue(index1, index2, field, ci->GetCurrency());
}

OMLCurrency* OMLStructImpl::GetCurrency() const
{
	return (new OMLCurrencyImpl(_eval, _sd));
}

StructData* OMLStructImpl::GetStructData() const
{
	return _sd;
}