// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.11.21

#include <Mathematics/ArbitraryPrecision.h>
using namespace gte;

// sizeof(BSNumber<UIntegerFP32<N>>) = 4 * (N + 4)

int PrimalQuery2Determinant2(BSPrecision::Type type, bool forBSNumber)
{
    // Real det2 = a00 * a11 - a01 * a10
    BSPrecision input(type);
    BSPrecision prod = input * input;
    BSPrecision det2 = prod - prod;
    return (forBSNumber ? det2.bsn.maxWords : det2.bsr.maxWords);
}

int PrimalQuery2Determinant3(BSPrecision::Type type, bool forBSNumber)
{
    // Real c0 = a11 * a22 - a12 * a21;
    // Real c1 = a10 * a22 - a12 * a20;
    // Real c2 = a10 * a21 - a11 * a20;
    // Real det3 = a00 * c0 - a01 * c1 + a02 * c2;
    BSPrecision input(type);
    BSPrecision prod = input * input;
    BSPrecision det2 = prod - prod;
    BSPrecision term1 = input * det2;
    BSPrecision term2 = term1 + term1;
    BSPrecision det3 = term1 + term2;
    return (forBSNumber ? det3.bsn.maxWords : det3.bsr.maxWords);
}

int PrimalQuery2Determinant4(BSPrecision::Type type, bool forBSNumber)
{
    // Real u0 = a00 * a11 - a01 * a10, v0 = a20 * a31 - a21 * a30;
    // Real u1 = a00 * a12 - a02 * a10, v1 = a20 * a32 - a22 * a30;
    // Real u2 = a00 * a13 - a02 * a10, v2 = a20 * a33 - a23 * a30;
    // Real u3 = a01 * a12 - a02 * a11, v3 = a21 * a32 - a22 * a31;
    // Real u4 = a01 * a13 - a03 * a11, v4 = a21 * a33 - a23 * a31;
    // Real u5 = a02 * a13 - a03 * a12, v5 = a22 * a33 - a23 * a32;
    // Real det = (u0 * v5 - u1 * v4) + (u2 * v3 + u3 * v2) + (- u4 * v1 + u5 * v0);
    BSPrecision input(type);
    BSPrecision prod = input * input;
    BSPrecision det2 = prod - prod;
    BSPrecision term1 = det2 * det2;
    BSPrecision term2 = term1 + term1;
    BSPrecision det4 = term2 + term2 + term2;
    return (forBSNumber ? det4.bsn.maxWords : det4.bsr.maxWords);
}

int PrimalQuery2ToLine(BSPrecision::Type type, bool forBSNumber)
{
    // ToLine (no order parameter):
    //Real x0 = test[0] - vec0[0];
    //Real y0 = test[1] - vec0[1];
    //Real x1 = vec1[0] - vec0[0];
    //Real y1 = vec1[1] - vec0[1];
    //Real x0y1 = x0*y1;
    //Real x1y0 = x1*y0;
    //Real det = x0y1 - x1y0;
    //
    // ToLine (with order parameter), worst-case path:
    //Real x0 = test[0] - vec0[0];
    //Real y0 = test[1] - vec0[1];
    //Real x1 = vec1[0] - vec0[0];
    //Real y1 = vec1[1] - vec0[1];
    //Real x0y1 = x0*y1;
    //Real x1y0 = x1*y0;
    //Real det = x0y1 - x1y0;
    //Real x0x1 = x0*x1;
    //Real y0y1 = y0*y1;
    //Real dot = x0x1 + y0y1;
    //Real x0x0 = x0*x0;
    //Real y0y0 = y0*y0;
    //Real sqrlen = x0x0 + y0y0;
    //
    // ToLineExtended, worst-case path:
    //Real x0 = Q1[0] - Q0[0];
    //Real y0 = Q1[1] - Q0[1];
    //Real x1 = P[0] - Q0[0];
    //Real y1 = P[1] - Q0[1];
    //Real x2 = P[0] - Q1[0];
    //Real y2 = P[1] - Q1[1];
    //Real x0y1 = x0 * y1;
    //Real x1y0 = x1 * y0;
    //Real det = x0y1 - x1y0;
    //Real x0x1 = x0 * x1;
    //Real y0y1 = y0 * y1;
    //Real dot = x0x1 + y0y1;
    //Real x0x0 = x0 * x0;
    //Real y0y0 = y0 * y0;
    //Real sqrLength = x0x0 + y0y0;

    // test[.], vec0[.], vec1[.]
    BSPrecision u(type);
    // x0, y0, x1, y1
    BSPrecision add0 = u + u;
    // x0y1, x1y0, x0x1, y0y1, x0x0, y0y0
    BSPrecision mul = add0 * add0;
    // det, dot, sqrLength
    BSPrecision add1 = mul + mul;
    return (forBSNumber ? add1.bsn.maxWords : add1.bsr.maxWords);
}

int PrimalQuery2ToCircumcircle(BSPrecision::Type type, bool forBSNumber)
{
    //Real x0 = vec0[0] - test[0];
    //Real y0 = vec0[1] - test[1];
    //Real s00 = vec0[0] + test[0];
    //Real s01 = vec0[1] + test[1];
    //Real t00 = s00*x0;
    //Real t01 = s01*y0;
    //Real z0 = t00 + t01;

    //Real x1 = vec1[0] - test[0];
    //Real y1 = vec1[1] - test[1];
    //Real s10 = vec1[0] + test[0];
    //Real s11 = vec1[1] + test[1];
    //Real t10 = s10*x1;
    //Real t11 = s11*y1;
    //Real z1 = t10 + t11;

    //Real x2 = vec2[0] - test[0];
    //Real y2 = vec2[1] - test[1];
    //Real s20 = vec2[0] + test[0];
    //Real s21 = vec2[1] + test[1];
    //Real t20 = s20*x2;
    //Real t21 = s21*y2;
    //Real z2 = t20 + t21;

    //Real y0z1 = y0*z1;
    //Real y0z2 = y0*z2;
    //Real y1z0 = y1*z0;
    //Real y1z2 = y1*z2;
    //Real y2z0 = y2*z0;
    //Real y2z1 = y2*z1;
    //Real c0 = y1z2 - y2z1;
    //Real c1 = y2z0 - y0z2;
    //Real c2 = y0z1 - y1z0;
    //Real x0c0 = x0*c0;
    //Real x1c1 = x1*c1;
    //Real x2c2 = x2*c2;
    //Real term = x0c0 + x1c1;
    //Real det = term + x2c2;

    // test[.], vec0[.], vec1[.], vec2[.]
    BSPrecision u(type);
    // x0, y0, s00, s01, x1, y1, s10, s11, x2, y2, s20, s21
    BSPrecision add0 = u + u;
    // t00, t01, t10, t11, t20, t21
    BSPrecision mul0 = add0 * add0;
    // z0, z1, z2
    BSPrecision add1 = mul0 + mul0;
    // y0z1, y0z2, y1z0, y1z2, y2z0, y2z1
    BSPrecision mul1 = add0 * add1;
    // c0, c1, c2
    BSPrecision add2 = mul1 + mul1;
    // x0c0, x1c1, x2c2
    BSPrecision mul2 = add0 * add2;
    // term
    BSPrecision add3 = mul2 + mul2;
    // det
    BSPrecision add4 = add3 + mul2;
    return (forBSNumber ? add4.bsn.maxWords : add4.bsr.maxWords);
}

int PrimalQuery3ToPlane(BSPrecision::Type type, bool forBSNumber)
{
    //Real x0 = test[0] - vec0[0];
    //Real y0 = test[1] - vec0[1];
    //Real z0 = test[2] - vec0[2];
    //Real x1 = vec1[0] - vec0[0];
    //Real y1 = vec1[1] - vec0[1];
    //Real z1 = vec1[2] - vec0[2];
    //Real x2 = vec2[0] - vec0[0];
    //Real y2 = vec2[1] - vec0[1];
    //Real z2 = vec2[2] - vec0[2];
    //Real y1z2 = y1*z2;
    //Real y2z1 = y2*z1;
    //Real y2z0 = y2*z0;
    //Real y0z2 = y0*z2;
    //Real y0z1 = y0*z1;
    //Real y1z0 = y1*z0;
    //Real c0 = y1z2 - y2z1;
    //Real c1 = y2z0 - y0z2;
    //Real c2 = y0z1 - y1z0;
    //Real x0c0 = x0*c0;
    //Real x1c1 = x1*c1;
    //Real x2c2 = x2*c2;
    //Real term = x0c0 + x1c1;
    //Real det = term + x2c2;

    // test[.], vec0[.], vec1[.], vec2[.]
    BSPrecision u(type);
    // x0, y0, z0, x1, y1, z1, x2, y2, z2
    BSPrecision add0 = u + u;
    // y1z2, y2z1, y2z0, y0z2, y0z1, y1z0
    BSPrecision mul0 = add0 * add0;
    // c0, c1, c2
    BSPrecision add1 = mul0 + mul0;
    // x0c0, x1c1, x2c2
    BSPrecision mul1 = add0 * add1;
    // term
    BSPrecision add2 = mul1 + mul1;
    // det
    BSPrecision add3 = add2 + mul1;
    return (forBSNumber ? add3.bsn.maxWords : add3.bsr.maxWords);
}

int PrimalQuery3ToCircumsphere(BSPrecision::Type type, bool forBSNumber)
{
    //Real x0 = vec0[0] - test[0];
    //Real y0 = vec0[1] - test[1];
    //Real z0 = vec0[2] - test[2];
    //Real s00 = vec0[0] + test[0];
    //Real s01 = vec0[1] + test[1];
    //Real s02 = vec0[2] + test[2];
    //Real t00 = s00*x0;
    //Real t01 = s01*y0;
    //Real t02 = s02*z0;
    //Real t00pt01 = t00 + t01;
    //Real w0 = t00pt01 + t02;

    //Real x1 = vec1[0] - test[0];
    //Real y1 = vec1[1] - test[1];
    //Real z1 = vec1[2] - test[2];
    //Real s10 = vec1[0] + test[0];
    //Real s11 = vec1[1] + test[1];
    //Real s12 = vec1[2] + test[2];
    //Real t10 = s10*x1;
    //Real t11 = s11*y1;
    //Real t12 = s12*z1;
    //Real t10pt11 = t10 + t11;
    //Real w1 = t10pt11 + t12;

    //Real x2 = vec2[0] - test[0];
    //Real y2 = vec2[1] - test[1];
    //Real z2 = vec2[2] - test[2];
    //Real s20 = vec2[0] + test[0];
    //Real s21 = vec2[1] + test[1];
    //Real s22 = vec2[2] + test[2];
    //Real t20 = s20*x2;
    //Real t21 = s21*y2;
    //Real t22 = s22*z2;
    //Real t20pt21 = t20 + t21;
    //Real w2 = t20pt21 + t22;

    //Real x3 = vec3[0] - test[0];
    //Real y3 = vec3[1] - test[1];
    //Real z3 = vec3[2] - test[2];
    //Real s30 = vec3[0] + test[0];
    //Real s31 = vec3[1] + test[1];
    //Real s32 = vec3[2] + test[2];
    //Real t30 = s30*x3;
    //Real t31 = s31*y3;
    //Real t32 = s32*z3;
    //Real t30pt31 = t30 + t31;
    //Real w3 = t30pt31 + t32;

    //Real x0y1 = x0*y1;
    //Real x0y2 = x0*y2;
    //Real x0y3 = x0*y3;
    //Real x1y0 = x1*y0;
    //Real x1y2 = x1*y2;
    //Real x1y3 = x1*y3;
    //Real x2y0 = x2*y0;
    //Real x2y1 = x2*y1;
    //Real x2y3 = x2*y3;
    //Real x3y0 = x3*y0;
    //Real x3y1 = x3*y1;
    //Real x3y2 = x3*y2;
    //Real a0 = x0y1 - x1y0;
    //Real a1 = x0y2 - x2y0;
    //Real a2 = x0y3 - x3y0;
    //Real a3 = x1y2 - x2y1;
    //Real a4 = x1y3 - x3y1;
    //Real a5 = x2y3 - x3y2;

    //Real z0w1 = z0*w1;
    //Real z0w2 = z0*w2;
    //Real z0w3 = z0*w3;
    //Real z1w0 = z1*w0;
    //Real z1w2 = z1*w2;
    //Real z1w3 = z1*w3;
    //Real z2w0 = z2*w0;
    //Real z2w1 = z2*w1;
    //Real z2w3 = z2*w3;
    //Real z3w0 = z3*w0;
    //Real z3w1 = z3*w1;
    //Real z3w2 = z3*w2;
    //Real b0 = z0w1 - z1w0;
    //Real b1 = z0w2 - z2w0;
    //Real b2 = z0w3 - z3w0;
    //Real b3 = z1w2 - z2w1;
    //Real b4 = z1w3 - z3w1;
    //Real b5 = z2w3 - z3w2;
    //Real a0b5 = a0*b5;
    //Real a1b4 = a1*b4;
    //Real a2b3 = a2*b3;
    //Real a3b2 = a3*b2;
    //Real a4b1 = a4*b1;
    //Real a5b0 = a5*b0;
    //Real term0 = a0b5 - a1b4;
    //Real term1 = term0 + a2b3;
    //Real term2 = term1 + a3b2;
    //Real term3 = term2 - a4b1;
    //Real det = term3 + a5b0;

    // test[.], vec0[.], vec1[.], vec2[.], vec3[.]
    BSPrecision u(type);
    // x0, y0, z0, s00, s01, s02
    // x1, y1, z1, s10, s11, s12
    // x2, y2, z2, s20, s21, s22
    // x3, y3, z3, s30, s31, s32
    BSPrecision add0 = u + u;
    // t00, t01, t02, t10, t11, t12, t20, t21, t22, t30, t31, t32
    BSPrecision mul0 = add0 * add0;
    // t00pt01, t10pt11, t20pt21, t30pt31
    BSPrecision add1 = mul0 + mul0;
    // w0, w1, w2, w3
    BSPrecision add2 = add1 + mul0;
    // x0y1, x0y2, x0y3, x1y0, x1y2, x1y3, x2y0, x2y1, x2y3, x3y0, x3y1, x3y2
    BSPrecision mul1 = add0 * add0;
    // a0, a1, a2, a3, a4, a5
    BSPrecision add3 = mul1 + mul1;
    // z0w1, z0w2, z0w3, z1w0, z1w2, z1w3, z2w0, z2w1, z2w3, z3w0, z3w1, z3w2
    BSPrecision mul2 = add0 * add2;
    // b0, b1, b2, b3, b4, b4
    BSPrecision add4 = mul2 + mul2;
    // a0b5, a1b4, a2b3, a3b2, a4b1, a5b0
    BSPrecision mul3 = add3 * add4;
    // term0
    BSPrecision add5 = mul3 + mul3;
    // term1
    BSPrecision add6 = add5 + mul3;
    // term2
    BSPrecision add7 = add6 + mul3;
    // term3
    BSPrecision add8 = add7 + mul3;
    // det
    BSPrecision add9 = add8 + mul3;
    return (forBSNumber ? add9.bsn.maxWords : add9.bsr.maxWords);
}

int ConstrainedDelaunay2ComputePSD(BSPrecision::Type type, bool forBSNumber)
{
    // Precompute some common values that are used in all calls
    // to ComputePSD.
    //Vector2<ComputeType> const& ctv0 = this->mComputeVertices[v0];
    //Vector2<ComputeType> const& ctv1 = this->mComputeVertices[v1];
    //Vector2<ComputeType> V1mV0 = ctv1 - ctv0;
    //ComputeType sqrlen10 = Dot(V1mV0, V1mV0);
    // :
    //ComputeType const zero = static_cast<ComputeType>(0);
    //Vector2<ComputeType> const& ctv0 = this->mComputeVertices[v0];
    //Vector2<ComputeType> const& ctv1 = this->mComputeVertices[v1];
    //Vector2<ComputeType> const& ctv2 = this->mComputeVertices[v2];
    //Vector2<ComputeType> V2mV0 = ctv2 - ctv0;
    //ComputeType dot1020 = Dot(V1mV0, V2mV0);
    //ComputeType psd;
    //if (dot1020 <= zero)
    //{
    //    ComputeType sqrlen20 = Dot(V2mV0, V2mV0);
    //    psd = sqrlen10 * sqrlen20;
    //}
    //else
    //{
    //    Vector2<ComputeType> V2mV1 = ctv2 - ctv1;
    //    ComputeType dot1021 = Dot(V1mV0, V2mV1);
    //    if (dot1021 >= zero)
    //    {
    //        ComputeType sqrlen21 = Dot(V2mV1, V2mV1);
    //        psd = sqrlen10 * sqrlen21;
    //    }
    //    else
    //    {
    //        ComputeType sqrlen20 = Dot(V2mV0, V2mV0);
    //        psd = sqrlen10 * sqrlen20 - dot1020 * dot1020;
    //    }
    //}
    //return psd;

    // The longest computational path is
    // psd = sqrlen10 * sqrlen20 - dot1020 * dot1020;
    BSPrecision u(type);
    BSPrecision vdiff = u * u - u * u;
    BSPrecision dotvdiff = vdiff * vdiff + vdiff * vdiff;
    BSPrecision psd = dotvdiff * dotvdiff - dotvdiff * dotvdiff;
    return (forBSNumber ? psd.bsn.maxWords : psd.bsr.maxWords);
}

int PrimalQuery3Colinear(BSPrecision::Type type, bool forBSNumber)
{
    // delta1 = v1 - v0
    // delta2 = v2 - v0
    // cross = Cross(diff1, diff2)
    //       = (diff1.y * diff2.z - diff1.z * diff2.y, *, *)
    // cross[0] = cross[1] = cross[2] = 0;
    BSPrecision vcomponent(type);
    BSPrecision vdelta = vcomponent - vcomponent;
    BSPrecision product = vdelta * vdelta;
    BSPrecision ddiff = product - product;
    return (forBSNumber ? ddiff.bsn.maxWords : ddiff.bsr.maxWords);
}

int PrimalQuery3Coplanar(BSPrecision::Type type, bool forBSNumber)
{
    // delta1 = v1 - v0
    // delta2 = v2 - v0
    // delta3 = v3 - v0
    // dotCross = Dot(Cross(delta1, delta2, delta3))
    // return dotCross == 0
    BSPrecision vcomponent(type);
    BSPrecision vdelta = vcomponent - vcomponent;
    BSPrecision product = vdelta * vdelta;
    BSPrecision det2 = product - product;
    BSPrecision term = vdelta * det2;
    BSPrecision det3 = term + term + term;
    return (forBSNumber ? det3.bsn.maxWords : det3.bsr.maxWords);
}

int SumOfTwoSquares(BSPrecision::Type type, bool forBSNumber)
{
    // z = x * x + y * y
    BSPrecision u(type);
    BSPrecision product = u * u;
    BSPrecision sum = product + product;
    return (forBSNumber ? sum.bsn.maxWords : sum.bsr.maxWords);
}

int Delaunay2ToPlane(BSPrecision::Type type, bool forBSNumber)
{
    // Real x0 = P[0] - V0[0];
    // Real y0 = P[1] - V0[1];
    // Real z0 = P[2] - V0[2];
    // Real x1 = V1[0] - V0[0];
    // Real y1 = V1[1] - V0[1];
    // Real z1 = V1[2] - V0[2];
    // Real x2 = V2[0] - V0[0];
    // Real y2 = V2[1] - V0[1];
    // Real z2 = V2[2] - V0[2];
    // [det = z0*(x1*y2-x2*y1) + z1*(x2*y0-x0*y2) + z2*(x0*y1-x1*y0)]
    // Real x1y2 = x1*y2;
    // Real x2y1 = x2*y1;
    // Real x2y0 = x2*y0;
    // Real x0y2 = x0*y2;
    // Real x0y1 = x0*y1;
    // Real x1y0 = x1*y0;
    // Real c0 = x1y2 - x2y1;
    // Real c1 = x2y0 - x0y2;
    // Real c2 = x0y1 - x1y0;
    // Real z0c0 = z0*c0;
    // Real z1c1 = z1*c1;
    // Real z2c2 = z2*c2;
    // Real term = z0c0 + z1c1;
    // Real det = term + z2c2;

    // P[0], P[1], V0[0], V0[1], V1[0], V1[1], V2[0], V2[1]
    BSPrecision x(type), y(type);

    // P[2], V0[2], V1[2], V2[2]
    BSPrecision xx = x * x, yy = y * y;
    BSPrecision z = xx + yy;

    // x0, y0, x1, y1, x2, y2
    BSPrecision xSub = x - x, ySub = y - y;

    // z0, z1, z2
    BSPrecision zSub = z - z;

    // x1y2, x2y1, x2y0, x0y2, x0y1, x1y0
    BSPrecision mul0 = xSub * ySub;

    // c0, c1, c2
    BSPrecision subXYXY = mul0 - mul0;

    // z0c0, z1c1, z2c2
    BSPrecision mul1 = zSub * subXYXY;

    // term
    BSPrecision add0 = mul1 + mul1;

    // det
    BSPrecision add1 = add0 + mul1;
    return (forBSNumber ? add1.bsn.maxWords : add1.bsr.maxWords);
}

int main()
{
    int32_t bsNumberFloatWords, bsNumberDoubleWords;
    int32_t bsRationalFloatWords, bsRationalDoubleWords;

    bsNumberFloatWords = PrimalQuery2Determinant2(BSPrecision::IS_FLOAT, true);  // 18
    bsNumberDoubleWords = PrimalQuery2Determinant2(BSPrecision::IS_DOUBLE, true);  // 132
    bsRationalFloatWords = PrimalQuery2Determinant2(BSPrecision::IS_FLOAT, false);  // 35
    bsRationalDoubleWords = PrimalQuery2Determinant2(BSPrecision::IS_DOUBLE, false);  // 263

    bsNumberFloatWords = PrimalQuery2Determinant3(BSPrecision::IS_FLOAT, true);  // 27
    bsNumberDoubleWords = PrimalQuery2Determinant3(BSPrecision::IS_DOUBLE, true);  // 197
    bsRationalFloatWords = PrimalQuery2Determinant3(BSPrecision::IS_FLOAT, false);  // 130
    bsRationalDoubleWords = PrimalQuery2Determinant3(BSPrecision::IS_DOUBLE, false);  // 984

    bsNumberFloatWords = PrimalQuery2Determinant4(BSPrecision::IS_FLOAT, true);  // 35
    bsNumberDoubleWords = PrimalQuery2Determinant4(BSPrecision::IS_DOUBLE, true);  // 263
    bsRationalFloatWords = PrimalQuery2Determinant4(BSPrecision::IS_FLOAT, false);  // 417
    bsRationalDoubleWords = PrimalQuery2Determinant4(BSPrecision::IS_DOUBLE, false);  // 3148

    bsNumberFloatWords = PrimalQuery2ToLine(BSPrecision::IS_FLOAT, true);  // 18
    bsNumberDoubleWords = PrimalQuery2ToLine(BSPrecision::IS_DOUBLE, true);  // 132
    bsRationalFloatWords = PrimalQuery2ToLine(BSPrecision::IS_FLOAT, false);  // 70
    bsRationalDoubleWords = PrimalQuery2ToLine(BSPrecision::IS_DOUBLE, false);  // 525

    bsNumberFloatWords = PrimalQuery2ToCircumcircle(BSPrecision::IS_FLOAT, true);  // 35
    bsNumberDoubleWords = PrimalQuery2ToCircumcircle(BSPrecision::IS_DOUBLE, true);  // 263
    bsRationalFloatWords = PrimalQuery2ToCircumcircle(BSPrecision::IS_FLOAT, false);  // 573
    bsRationalDoubleWords = PrimalQuery2ToCircumcircle(BSPrecision::IS_DOUBLE, false);  // 4329

    bsNumberFloatWords = PrimalQuery3ToPlane(BSPrecision::IS_FLOAT, true);  // 27
    bsNumberDoubleWords = PrimalQuery3ToPlane(BSPrecision::IS_DOUBLE, true);  // 197
    bsRationalFloatWords = PrimalQuery3ToPlane(BSPrecision::IS_FLOAT, false);  // 261
    bsRationalDoubleWords = PrimalQuery3ToPlane(BSPrecision::IS_DOUBLE, false);  // 1968

    bsNumberFloatWords = PrimalQuery3ToCircumsphere(BSPrecision::IS_FLOAT, true);  // 44
    bsNumberDoubleWords = PrimalQuery3ToCircumsphere(BSPrecision::IS_DOUBLE, true);  // 329
    bsRationalFloatWords = PrimalQuery3ToCircumsphere(BSPrecision::IS_FLOAT, false);  // 1875
    bsRationalDoubleWords = PrimalQuery3ToCircumsphere(BSPrecision::IS_DOUBLE, false);  // 14167

    bsNumberFloatWords = ConstrainedDelaunay2ComputePSD(BSPrecision::IS_FLOAT, true);  // 70
    bsNumberDoubleWords = ConstrainedDelaunay2ComputePSD(BSPrecision::IS_DOUBLE, true);  // 525
    bsRationalFloatWords = ConstrainedDelaunay2ComputePSD(BSPrecision::IS_FLOAT, false);  // 555
    bsRationalDoubleWords = ConstrainedDelaunay2ComputePSD(BSPrecision::IS_DOUBLE, false);  // 4197

    bsNumberFloatWords = PrimalQuery3Colinear(BSPrecision::IS_FLOAT, true);  // 18
    bsNumberDoubleWords = PrimalQuery3Colinear(BSPrecision::IS_DOUBLE, true);  // 132
    bsRationalFloatWords = PrimalQuery3Colinear(BSPrecision::IS_FLOAT, false);  // 70
    bsRationalDoubleWords = PrimalQuery3Colinear(BSPrecision::IS_DOUBLE, false);  // 525

    bsNumberFloatWords = PrimalQuery3Coplanar(BSPrecision::IS_FLOAT, true);  // 27
    bsNumberDoubleWords = PrimalQuery3Coplanar(BSPrecision::IS_DOUBLE, true);  // 197
    bsRationalFloatWords = PrimalQuery3Coplanar(BSPrecision::IS_FLOAT, false);  // 361
    bsRationalDoubleWords = PrimalQuery3Coplanar(BSPrecision::IS_DOUBLE, false);  // 1968

    bsNumberFloatWords = SumOfTwoSquares(BSPrecision::IS_FLOAT, true);  // 18
    bsNumberDoubleWords = SumOfTwoSquares(BSPrecision::IS_DOUBLE, true);  // 132
    bsRationalFloatWords = SumOfTwoSquares(BSPrecision::IS_FLOAT, false);  // 35
    bsRationalDoubleWords = SumOfTwoSquares(BSPrecision::IS_DOUBLE, false);  // 263

    bsNumberFloatWords = Delaunay2ToPlane(BSPrecision::IS_FLOAT, true);  // 35
    bsNumberDoubleWords = Delaunay2ToPlane(BSPrecision::IS_DOUBLE, true);  // 263
    bsRationalFloatWords = Delaunay2ToPlane(BSPrecision::IS_FLOAT, false);  // 417
    bsRationalDoubleWords = Delaunay2ToPlane(BSPrecision::IS_DOUBLE, false);  // 3148

    return 0;
}
