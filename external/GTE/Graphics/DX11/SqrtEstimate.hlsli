// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#define GT1_USE_IN_HLSL
#include "Gt1Constants.h"
#include "Gt1DoubleFunction.hlsli"

// Minimax polynomial approximations to sqrt(x).  The polynomial p(x) of
// degree D minimizes the quantity maximum{|sqrt(x) - p(x)| : x in [1,2]}
// over all polynomials of degree D.

//----------------------------------------------------------------------------
#define TSqrtEstimateReduceF(Dim)\
    void SqrtEstimateReduce(vector<float,Dim> x, out vector<float,Dim> adj,\
        out vector<float,Dim> y, out vector<int,Dim> p)\
    {\
        y = frexp(x, p);\
        y = 2.0*y;\
        --p;\
        adj = (1 & p)*GT1_C_SQRT_2 + (1 & ~p)*1.0;\
        p >>= 1;\
    }

#define TSqrtEstimateReduceD(Dim)\
    void SqrtEstimateReduce(vector<double,Dim> x, out vector<double,Dim> adj,\
        out vector<double,Dim> y, out vector<int,Dim> p)\
    {\
        y = frexp_d(x, p);\
        y = 2.0*y;\
        --p;\
        adj = (1 & p)*GT1_C_SQRT_2 + (1 & ~p)*1.0;\
        p >>= 1;\
    }

TSqrtEstimateReduceF(1)
TSqrtEstimateReduceF(2)
TSqrtEstimateReduceF(3)
TSqrtEstimateReduceF(4)
//TSqrtEstimateReduceD(1)
//TSqrtEstimateReduce(double,2)
//TSqrtEstimateReduce(double,3)
//TSqrtEstimateReduce(double,4)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#define TSqrtEstimateCombine(Real,Dim)\
    vector<Real,Dim> SqrtEstimateCombine(vector<Real,Dim> adj,\
        vector<Real,Dim> y, vector<int,Dim> p)\
    {\
        return adj*ldexp(y, p);\
    }

TSqrtEstimateCombine(float,1)
TSqrtEstimateCombine(float,2)
TSqrtEstimateCombine(float,3)
TSqrtEstimateCombine(float,4)
//TSqrtEstimateCombine(double,1)
//TSqrtEstimateCombine(double,2)
//TSqrtEstimateCombine(double,3)
//TSqrtEstimateCombine(double,4)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#define TSqrtEstimateDegree1Evaluate(Real,Dim)\
    vector<Real,Dim> SqrtEstimateDegree1Evaluate(vector<Real,Dim> t)\
    {\
        vector<Real,Dim> poly;\
        poly = GT1_C_SQRT_DEG1_C1;\
        poly = GT1_C_SQRT_DEG1_C0 + poly * t;\
        return poly;\
    }

TSqrtEstimateDegree1Evaluate(float,1)
TSqrtEstimateDegree1Evaluate(float,2)
TSqrtEstimateDegree1Evaluate(float,3)
TSqrtEstimateDegree1Evaluate(float,4)
//TSqrtEstimateDegree1Evaluate(double,1)
//TSqrtEstimateDegree1Evaluate(double,2)
//TSqrtEstimateDegree1Evaluate(double,3)
//TSqrtEstimateDegree1Evaluate(double,4)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#define TSqrtEstimateDegree1(Real,Dim)\
    vector<Real,Dim> SqrtEstimateDegree1(vector<Real,Dim> x)\
    {\
        vector<Real,Dim> t = x - 1.0;\
        return SqrtEstimateDegree1Evaluate(t);\
    }

TSqrtEstimateDegree1(float,1)
TSqrtEstimateDegree1(float,2)
TSqrtEstimateDegree1(float,3)
TSqrtEstimateDegree1(float,4)
//TSqrtEstimateDegree1(double,1)
//TSqrtEstimateDegree1(double,2)
//TSqrtEstimateDegree1(double,3)
//TSqrtEstimateDegree1(double,4)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#define TSqrtEstimateDegree1RR(Real,Dim)\
    vector<Real,Dim> SqrtEstimateDegree1RR(vector<Real,Dim> x)\
    {\
        vector<Real,Dim> adj, y;\
        vector<int,Dim> p;\
        SqrtEstimateReduce(x, adj, y, p);\
        vector<Real,Dim> poly = SqrtEstimateDegree1Evaluate(y);\
        vector<Real,Dim> result = SqrtEstimateCombine(adj, poly, p);\
        return result;\
    }

TSqrtEstimateDegree1RR(float, 1)
TSqrtEstimateDegree1RR(float, 2)
TSqrtEstimateDegree1RR(float, 3)
TSqrtEstimateDegree1RR(float, 4)
TSqrtEstimateDegree1RR(double, 1)
//TSqrtEstimateDegree1RR(double, 2)
//TSqrtEstimateDegree1RR(double, 3)
//TSqrtEstimateDegree1RR(double, 4)
//----------------------------------------------------------------------------
