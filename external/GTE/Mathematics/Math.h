// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.08

#pragma once

// This file extends the <cmath> support to include convenient constants and
// functions.  The shared constants for CPU, Intel SSE and GPU lead to
// correctly rounded approximations of the constants when using 'float' or
// 'double'.  The file also includes a type trait, is_arbitrary_precision,
// to support selecting between floating-point arithmetic (float, double,
//long double) or arbitrary-precision arithmetic (BSNumber<T>, BSRational<T>)
// in an implementation using std::enable_if.  There is also a type trait,
// has_division_operator, to support selecting between numeric types that
// have a division operator (BSRational<T>) and those that do not have a
// division operator (BSNumber<T>).

#include <cfenv>
#include <cmath>
#include <limits>
#include <type_traits>

// Maximum number of iterations for bisection before a subinterval
// degenerates to a single point. TODO: Verify these.  I used the formula:
// 3 + std::numeric_limits<T>::digits - std::numeric_limits<T>::min_exponent.
//   IEEEBinary16:  digits = 11, min_exponent = -13
//   float:         digits = 27, min_exponent = -125
//   double:        digits = 53, min_exponent = -1021
// BSNumber and BSRational use std::numeric_limits<unsigned int>::max(),
// but maybe these should be set to a large number or be user configurable.
// The MAX_BISECTIONS_GENERIC is an arbitrary choice for now and is used
// in template code where Real is the template parameter.
#define GTE_C_MAX_BISECTIONS_FLOAT16    27u
#define GTE_C_MAX_BISECTIONS_FLOAT32    155u
#define GTE_C_MAX_BISECTIONS_FLOAT64    1077u
#define GTE_C_MAX_BISECTIONS_BSNUMBER   0xFFFFFFFFu
#define GTE_C_MAX_BISECTIONS_BSRATIONAL 0xFFFFFFFFu
#define GTE_C_MAX_BISECTIONS_GENERIC    2048u

// Constants involving pi.
#define GTE_C_PI 3.1415926535897931
#define GTE_C_HALF_PI 1.5707963267948966
#define GTE_C_QUARTER_PI 0.7853981633974483
#define GTE_C_TWO_PI 6.2831853071795862
#define GTE_C_INV_PI 0.3183098861837907
#define GTE_C_INV_TWO_PI 0.1591549430918953
#define GTE_C_INV_HALF_PI 0.6366197723675813

// Conversions between degrees and radians.
#define GTE_C_DEG_TO_RAD 0.0174532925199433
#define GTE_C_RAD_TO_DEG 57.295779513082321

// Common constants.
#define GTE_C_SQRT_2 1.4142135623730951
#define GTE_C_INV_SQRT_2 0.7071067811865475
#define GTE_C_LN_2 0.6931471805599453
#define GTE_C_INV_LN_2 1.4426950408889634
#define GTE_C_LN_10 2.3025850929940459
#define GTE_C_INV_LN_10 0.43429448190325176

// Constants for minimax polynomial approximations to sqrt(x).
// The algorithm minimizes the maximum absolute error on [1,2].
#define GTE_C_SQRT_DEG1_C0 +1.0
#define GTE_C_SQRT_DEG1_C1 +4.1421356237309505e-01
#define GTE_C_SQRT_DEG1_MAX_ERROR 1.7766952966368793e-2

#define GTE_C_SQRT_DEG2_C0 +1.0
#define GTE_C_SQRT_DEG2_C1 +4.8563183076125260e-01
#define GTE_C_SQRT_DEG2_C2 -7.1418268388157458e-02
#define GTE_C_SQRT_DEG2_MAX_ERROR 1.1795695163108744e-3

#define GTE_C_SQRT_DEG3_C0 +1.0
#define GTE_C_SQRT_DEG3_C1 +4.9750045320242231e-01
#define GTE_C_SQRT_DEG3_C2 -1.0787308044477850e-01
#define GTE_C_SQRT_DEG3_C3 +2.4586189615451115e-02
#define GTE_C_SQRT_DEG3_MAX_ERROR 1.1309620116468910e-4

#define GTE_C_SQRT_DEG4_C0 +1.0
#define GTE_C_SQRT_DEG4_C1 +4.9955939832918816e-01
#define GTE_C_SQRT_DEG4_C2 -1.2024066151943025e-01
#define GTE_C_SQRT_DEG4_C3 +4.5461507257698486e-02
#define GTE_C_SQRT_DEG4_C4 -1.0566681694362146e-02
#define GTE_C_SQRT_DEG4_MAX_ERROR 1.2741170151556180e-5

#define GTE_C_SQRT_DEG5_C0 +1.0
#define GTE_C_SQRT_DEG5_C1 +4.9992197660031912e-01
#define GTE_C_SQRT_DEG5_C2 -1.2378506719245053e-01
#define GTE_C_SQRT_DEG5_C3 +5.6122776972699739e-02
#define GTE_C_SQRT_DEG5_C4 -2.3128836281145482e-02
#define GTE_C_SQRT_DEG5_C5 +5.0827122737047148e-03
#define GTE_C_SQRT_DEG5_MAX_ERROR 1.5725568940708201e-6

#define GTE_C_SQRT_DEG6_C0 +1.0
#define GTE_C_SQRT_DEG6_C1 +4.9998616695784914e-01
#define GTE_C_SQRT_DEG6_C2 -1.2470733323278438e-01
#define GTE_C_SQRT_DEG6_C3 +6.0388587356982271e-02
#define GTE_C_SQRT_DEG6_C4 -3.1692053551807930e-02
#define GTE_C_SQRT_DEG6_C5 +1.2856590305148075e-02
#define GTE_C_SQRT_DEG6_C6 -2.6183954624343642e-03
#define GTE_C_SQRT_DEG6_MAX_ERROR 2.0584155535630089e-7

#define GTE_C_SQRT_DEG7_C0 +1.0
#define GTE_C_SQRT_DEG7_C1 +4.9999754817809228e-01
#define GTE_C_SQRT_DEG7_C2 -1.2493243476353655e-01
#define GTE_C_SQRT_DEG7_C3 +6.1859954146370910e-02
#define GTE_C_SQRT_DEG7_C4 -3.6091595023208356e-02
#define GTE_C_SQRT_DEG7_C5 +1.9483946523450868e-02
#define GTE_C_SQRT_DEG7_C6 -7.5166134568007692e-03
#define GTE_C_SQRT_DEG7_C7 +1.4127567687864939e-03
#define GTE_C_SQRT_DEG7_MAX_ERROR 2.8072302919734948e-8

#define GTE_C_SQRT_DEG8_C0 +1.0
#define GTE_C_SQRT_DEG8_C1 +4.9999956583056759e-01
#define GTE_C_SQRT_DEG8_C2 -1.2498490369914350e-01
#define GTE_C_SQRT_DEG8_C3 +6.2318494667579216e-02
#define GTE_C_SQRT_DEG8_C4 -3.7982961896432244e-02
#define GTE_C_SQRT_DEG8_C5 +2.3642612312869460e-02
#define GTE_C_SQRT_DEG8_C6 -1.2529377587270574e-02
#define GTE_C_SQRT_DEG8_C7 +4.5382426960713929e-03
#define GTE_C_SQRT_DEG8_C8 -7.8810995273670414e-04
#define GTE_C_SQRT_DEG8_MAX_ERROR 3.9460605685825989e-9

// Constants for minimax polynomial approximations to 1/sqrt(x).
// The algorithm minimizes the maximum absolute error on [1,2].
#define GTE_C_INVSQRT_DEG1_C0 +1.0
#define GTE_C_INVSQRT_DEG1_C1 -2.9289321881345254e-01
#define GTE_C_INVSQRT_DEG1_MAX_ERROR 3.7814314552701983e-2

#define GTE_C_INVSQRT_DEG2_C0 +1.0
#define GTE_C_INVSQRT_DEG2_C1 -4.4539812104566801e-01
#define GTE_C_INVSQRT_DEG2_C2 +1.5250490223221547e-01
#define GTE_C_INVSQRT_DEG2_MAX_ERROR 4.1953446330581234e-3

#define GTE_C_INVSQRT_DEG3_C0 +1.0
#define GTE_C_INVSQRT_DEG3_C1 -4.8703230993068791e-01
#define GTE_C_INVSQRT_DEG3_C2 +2.8163710486669835e-01
#define GTE_C_INVSQRT_DEG3_C3 -8.7498013749463421e-02
#define GTE_C_INVSQRT_DEG3_MAX_ERROR 5.6307702007266786e-4

#define GTE_C_INVSQRT_DEG4_C0 +1.0
#define GTE_C_INVSQRT_DEG4_C1 -4.9710061558048779e-01
#define GTE_C_INVSQRT_DEG4_C2 +3.4266247597676802e-01
#define GTE_C_INVSQRT_DEG4_C3 -1.9106356536293490e-01
#define GTE_C_INVSQRT_DEG4_C4 +5.2608486153198797e-02
#define GTE_C_INVSQRT_DEG4_MAX_ERROR 8.1513919987605266e-5

#define GTE_C_INVSQRT_DEG5_C0 +1.0
#define GTE_C_INVSQRT_DEG5_C1 -4.9937760586004143e-01
#define GTE_C_INVSQRT_DEG5_C2 +3.6508741295133973e-01
#define GTE_C_INVSQRT_DEG5_C3 -2.5884890281853501e-01
#define GTE_C_INVSQRT_DEG5_C4 +1.3275782221320753e-01
#define GTE_C_INVSQRT_DEG5_C5 -3.2511945299404488e-02
#define GTE_C_INVSQRT_DEG5_MAX_ERROR 1.2289367475583346e-5

#define GTE_C_INVSQRT_DEG6_C0 +1.0
#define GTE_C_INVSQRT_DEG6_C1 -4.9987029229547453e-01
#define GTE_C_INVSQRT_DEG6_C2 +3.7220923604495226e-01
#define GTE_C_INVSQRT_DEG6_C3 -2.9193067713256937e-01
#define GTE_C_INVSQRT_DEG6_C4 +1.9937605991094642e-01
#define GTE_C_INVSQRT_DEG6_C5 -9.3135712130901993e-02
#define GTE_C_INVSQRT_DEG6_C6 +2.0458166789566690e-02
#define GTE_C_INVSQRT_DEG6_MAX_ERROR 1.9001451223750465e-6

#define GTE_C_INVSQRT_DEG7_C0 +1.0
#define GTE_C_INVSQRT_DEG7_C1 -4.9997357250704977e-01
#define GTE_C_INVSQRT_DEG7_C2 +3.7426216884998809e-01
#define GTE_C_INVSQRT_DEG7_C3 -3.0539882498248971e-01
#define GTE_C_INVSQRT_DEG7_C4 +2.3976005607005391e-01
#define GTE_C_INVSQRT_DEG7_C5 -1.5410326351684489e-01
#define GTE_C_INVSQRT_DEG7_C6 +6.5598809723041995e-02
#define GTE_C_INVSQRT_DEG7_C7 -1.3038592450470787e-02
#define GTE_C_INVSQRT_DEG7_MAX_ERROR 2.9887724993168940e-7

#define GTE_C_INVSQRT_DEG8_C0 +1.0
#define GTE_C_INVSQRT_DEG8_C1 -4.9999471066120371e-01
#define GTE_C_INVSQRT_DEG8_C2 +3.7481415745794067e-01
#define GTE_C_INVSQRT_DEG8_C3 -3.1023804387422160e-01
#define GTE_C_INVSQRT_DEG8_C4 +2.5977002682930106e-01
#define GTE_C_INVSQRT_DEG8_C5 -1.9818790717727097e-01
#define GTE_C_INVSQRT_DEG8_C6 +1.1882414252613671e-01
#define GTE_C_INVSQRT_DEG8_C7 -4.6270038088550791e-02
#define GTE_C_INVSQRT_DEG8_C8 +8.3891541755747312e-03
#define GTE_C_INVSQRT_DEG8_MAX_ERROR 4.7596926146947771e-8

// Constants for minimax polynomial approximations to sin(x).
// The algorithm minimizes the maximum absolute error on [-pi/2,pi/2].
#define GTE_C_SIN_DEG3_C0 +1.0
#define GTE_C_SIN_DEG3_C1 -1.4727245910375519e-01
#define GTE_C_SIN_DEG3_MAX_ERROR 1.3481903639145865e-2

#define GTE_C_SIN_DEG5_C0 +1.0
#define GTE_C_SIN_DEG5_C1 -1.6600599923812209e-01
#define GTE_C_SIN_DEG5_C2 +7.5924178409012000e-03
#define GTE_C_SIN_DEG5_MAX_ERROR 1.4001209384639779e-4

#define GTE_C_SIN_DEG7_C0 +1.0
#define GTE_C_SIN_DEG7_C1 -1.6665578084732124e-01
#define GTE_C_SIN_DEG7_C2 +8.3109378830028557e-03
#define GTE_C_SIN_DEG7_C3 -1.8447486103462252e-04
#define GTE_C_SIN_DEG7_MAX_ERROR 1.0205878936686563e-6

#define GTE_C_SIN_DEG9_C0 +1.0
#define GTE_C_SIN_DEG9_C1 -1.6666656235308897e-01
#define GTE_C_SIN_DEG9_C2 +8.3329962509886002e-03
#define GTE_C_SIN_DEG9_C3 -1.9805100675274190e-04
#define GTE_C_SIN_DEG9_C4 +2.5967200279475300e-06
#define GTE_C_SIN_DEG9_MAX_ERROR 5.2010746265374053e-9

#define GTE_C_SIN_DEG11_C0 +1.0
#define GTE_C_SIN_DEG11_C1 -1.6666666601721269e-01
#define GTE_C_SIN_DEG11_C2 +8.3333303183525942e-03
#define GTE_C_SIN_DEG11_C3 -1.9840782426250314e-04
#define GTE_C_SIN_DEG11_C4 +2.7521557770526783e-06
#define GTE_C_SIN_DEG11_C5 -2.3828544692960918e-08
#define GTE_C_SIN_DEG11_MAX_ERROR 1.9295870457014530e-11

// Constants for minimax polynomial approximations to cos(x).
// The algorithm minimizes the maximum absolute error on [-pi/2,pi/2].
#define GTE_C_COS_DEG2_C0 +1.0
#define GTE_C_COS_DEG2_C1 -4.0528473456935105e-01
#define GTE_C_COS_DEG2_MAX_ERROR 5.4870946878404048e-2

#define GTE_C_COS_DEG4_C0 +1.0
#define GTE_C_COS_DEG4_C1 -4.9607181958647262e-01
#define GTE_C_COS_DEG4_C2 +3.6794619653489236e-02
#define GTE_C_COS_DEG4_MAX_ERROR 9.1879932449712154e-4

#define GTE_C_COS_DEG6_C0 +1.0
#define GTE_C_COS_DEG6_C1 -4.9992746217057404e-01
#define GTE_C_COS_DEG6_C2 +4.1493920348353308e-02
#define GTE_C_COS_DEG6_C3 -1.2712435011987822e-03
#define GTE_C_COS_DEG6_MAX_ERROR 9.2028470133065365e-6

#define GTE_C_COS_DEG8_C0 +1.0
#define GTE_C_COS_DEG8_C1 -4.9999925121358291e-01
#define GTE_C_COS_DEG8_C2 +4.1663780117805693e-02
#define GTE_C_COS_DEG8_C3 -1.3854239405310942e-03
#define GTE_C_COS_DEG8_C4 +2.3154171575501259e-05
#define GTE_C_COS_DEG8_MAX_ERROR 5.9804533020235695e-8

#define GTE_C_COS_DEG10_C0 +1.0
#define GTE_C_COS_DEG10_C1 -4.9999999508695869e-01
#define GTE_C_COS_DEG10_C2 +4.1666638865338612e-02
#define GTE_C_COS_DEG10_C3 -1.3888377661039897e-03
#define GTE_C_COS_DEG10_C4 +2.4760495088926859e-05
#define GTE_C_COS_DEG10_C5 -2.6051615464872668e-07
#define GTE_C_COS_DEG10_MAX_ERROR 2.7006769043325107e-10

// Constants for minimax polynomial approximations to tan(x).
// The algorithm minimizes the maximum absolute error on [-pi/4,pi/4].
#define GTE_C_TAN_DEG3_C0 1.0
#define GTE_C_TAN_DEG3_C1 4.4295926544736286e-01
#define GTE_C_TAN_DEG3_MAX_ERROR 1.1661892256204731e-2

#define GTE_C_TAN_DEG5_C0 1.0
#define GTE_C_TAN_DEG5_C1 3.1401320403542421e-01
#define GTE_C_TAN_DEG5_C2 2.0903948109240345e-01
#define GTE_C_TAN_DEG5_MAX_ERROR 5.8431854390143118e-4

#define GTE_C_TAN_DEG7_C0 1.0
#define GTE_C_TAN_DEG7_C1 3.3607213284422555e-01
#define GTE_C_TAN_DEG7_C2 1.1261037305184907e-01
#define GTE_C_TAN_DEG7_C3 9.8352099470524479e-02
#define GTE_C_TAN_DEG7_MAX_ERROR 3.5418688397723108e-5

#define GTE_C_TAN_DEG9_C0 1.0
#define GTE_C_TAN_DEG9_C1 3.3299232843941784e-01
#define GTE_C_TAN_DEG9_C2 1.3747843432474838e-01
#define GTE_C_TAN_DEG9_C3 3.7696344813028304e-02
#define GTE_C_TAN_DEG9_C4 4.6097377279281204e-02
#define GTE_C_TAN_DEG9_MAX_ERROR 2.2988173242199927e-6

#define GTE_C_TAN_DEG11_C0 1.0
#define GTE_C_TAN_DEG11_C1 3.3337224456224224e-01
#define GTE_C_TAN_DEG11_C2 1.3264516053824593e-01
#define GTE_C_TAN_DEG11_C3 5.8145237645931047e-02
#define GTE_C_TAN_DEG11_C4 1.0732193237572574e-02
#define GTE_C_TAN_DEG11_C5 2.1558456793513869e-02
#define GTE_C_TAN_DEG11_MAX_ERROR 1.5426257940140409e-7

#define GTE_C_TAN_DEG13_C0 1.0
#define GTE_C_TAN_DEG13_C1 3.3332916426394554e-01
#define GTE_C_TAN_DEG13_C2 1.3343404625112498e-01
#define GTE_C_TAN_DEG13_C3 5.3104565343119248e-02
#define GTE_C_TAN_DEG13_C4 2.5355038312682154e-02
#define GTE_C_TAN_DEG13_C5 1.8253255966556026e-03
#define GTE_C_TAN_DEG13_C6 1.0069407176615641e-02
#define GTE_C_TAN_DEG13_MAX_ERROR 1.0550264249037378e-8

// Constants for minimax polynomial approximations to acos(x), where the
// approximation is of the form acos(x) = sqrt(1 - x)*p(x) with p(x) a
// polynomial.  The algorithm minimizes the maximum error
// |acos(x)/sqrt(1-x) - p(x)| on [0,1].  At the same time we get an
// approximation for asin(x) = pi/2 - acos(x).
#define GTE_C_ACOS_DEG1_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG1_C1 -1.5658276442180141e-01
#define GTE_C_ACOS_DEG1_MAX_ERROR 1.1659002803738105e-2

#define GTE_C_ACOS_DEG2_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG2_C1 -2.0347053865798365e-01
#define GTE_C_ACOS_DEG2_C2 +4.6887774236182234e-02
#define GTE_C_ACOS_DEG2_MAX_ERROR 9.0311602490029258e-4

#define GTE_C_ACOS_DEG3_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG3_C1 -2.1253291899190285e-01
#define GTE_C_ACOS_DEG3_C2 +7.4773789639484223e-02
#define GTE_C_ACOS_DEG3_C3 -1.8823635069382449e-02
#define GTE_C_ACOS_DEG3_MAX_ERROR 9.3066396954288172e-5

#define GTE_C_ACOS_DEG4_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG4_C1 -2.1422258835275865e-01
#define GTE_C_ACOS_DEG4_C2 +8.4936675142844198e-02
#define GTE_C_ACOS_DEG4_C3 -3.5991475120957794e-02
#define GTE_C_ACOS_DEG4_C4 +8.6946239090712751e-03
#define GTE_C_ACOS_DEG4_MAX_ERROR 1.0930595804481413e-5

#define GTE_C_ACOS_DEG5_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG5_C1 -2.1453292139805524e-01
#define GTE_C_ACOS_DEG5_C2 +8.7973089282889383e-02
#define GTE_C_ACOS_DEG5_C3 -4.5130266382166440e-02
#define GTE_C_ACOS_DEG5_C4 +1.9467466687281387e-02
#define GTE_C_ACOS_DEG5_C5 -4.3601326117634898e-03
#define GTE_C_ACOS_DEG5_MAX_ERROR 1.3861070257241426-6

#define GTE_C_ACOS_DEG6_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG6_C1 -2.1458939285677325e-01
#define GTE_C_ACOS_DEG6_C2 +8.8784960563641491e-02
#define GTE_C_ACOS_DEG6_C3 -4.8887131453156485e-02
#define GTE_C_ACOS_DEG6_C4 +2.7011519960012720e-02
#define GTE_C_ACOS_DEG6_C5 -1.1210537323478320e-02
#define GTE_C_ACOS_DEG6_C6 +2.3078166879102469e-03
#define GTE_C_ACOS_DEG6_MAX_ERROR 1.8491291330427484e-7

#define GTE_C_ACOS_DEG7_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG7_C1 -2.1459960076929829e-01
#define GTE_C_ACOS_DEG7_C2 +8.8986946573346160e-02
#define GTE_C_ACOS_DEG7_C3 -5.0207843052845647e-02
#define GTE_C_ACOS_DEG7_C4 +3.0961594977611639e-02
#define GTE_C_ACOS_DEG7_C5 -1.7162031184398074e-02
#define GTE_C_ACOS_DEG7_C6 +6.7072304676685235e-03
#define GTE_C_ACOS_DEG7_C7 -1.2690614339589956e-03
#define GTE_C_ACOS_DEG7_MAX_ERROR 2.5574620927948377e-8

#define GTE_C_ACOS_DEG8_C0 +1.5707963267948966
#define GTE_C_ACOS_DEG8_C1 -2.1460143648688035e-01
#define GTE_C_ACOS_DEG8_C2 +8.9034700107934128e-02
#define GTE_C_ACOS_DEG8_C3 -5.0625279962389413e-02
#define GTE_C_ACOS_DEG8_C4 +3.2683762943179318e-02
#define GTE_C_ACOS_DEG8_C5 -2.0949278766238422e-02
#define GTE_C_ACOS_DEG8_C6 +1.1272900916992512e-02
#define GTE_C_ACOS_DEG8_C7 -4.1160981058965262e-03
#define GTE_C_ACOS_DEG8_C8 +7.1796493341480527e-04
#define GTE_C_ACOS_DEG8_MAX_ERROR 3.6340015129032732e-9

// Constants for minimax polynomial approximations to atan(x).
// The algorithm minimizes the maximum absolute error on [-1,1].
#define GTE_C_ATAN_DEG3_C0 +1.0
#define GTE_C_ATAN_DEG3_C1 -2.1460183660255172e-01
#define GTE_C_ATAN_DEG3_MAX_ERROR 1.5970326392614240e-2

#define GTE_C_ATAN_DEG5_C0 +1.0
#define GTE_C_ATAN_DEG5_C1 -3.0189478312144946e-01
#define GTE_C_ATAN_DEG5_C2 +8.7292946518897740e-02
#define GTE_C_ATAN_DEG5_MAX_ERROR 1.3509832247372636e-3

#define GTE_C_ATAN_DEG7_C0 +1.0
#define GTE_C_ATAN_DEG7_C1 -3.2570157599356531e-01
#define GTE_C_ATAN_DEG7_C2 +1.5342994884206673e-01
#define GTE_C_ATAN_DEG7_C3 -4.2330209451053591e-02
#define GTE_C_ATAN_DEG7_MAX_ERROR 1.5051227215514412e-4

#define GTE_C_ATAN_DEG9_C0 +1.0
#define GTE_C_ATAN_DEG9_C1 -3.3157878236439586e-01
#define GTE_C_ATAN_DEG9_C2 +1.8383034738018011e-01
#define GTE_C_ATAN_DEG9_C3 -8.9253037587244677e-02
#define GTE_C_ATAN_DEG9_C4 +2.2399635968909593e-02
#define GTE_C_ATAN_DEG9_MAX_ERROR 1.8921598624582064e-5

#define GTE_C_ATAN_DEG11_C0 +1.0
#define GTE_C_ATAN_DEG11_C1 -3.3294527685374087e-01
#define GTE_C_ATAN_DEG11_C2 +1.9498657165383548e-01
#define GTE_C_ATAN_DEG11_C3 -1.1921576270475498e-01
#define GTE_C_ATAN_DEG11_C4 +5.5063351366968050e-02
#define GTE_C_ATAN_DEG11_C5 -1.2490720064867844e-02
#define GTE_C_ATAN_DEG11_MAX_ERROR 2.5477724974187765e-6

#define GTE_C_ATAN_DEG13_C0 +1.0
#define GTE_C_ATAN_DEG13_C1 -3.3324998579202170e-01
#define GTE_C_ATAN_DEG13_C2 +1.9856563505717162e-01
#define GTE_C_ATAN_DEG13_C3 -1.3374657325451267e-01
#define GTE_C_ATAN_DEG13_C4 +8.1675882859940430e-02
#define GTE_C_ATAN_DEG13_C5 -3.5059680836411644e-02
#define GTE_C_ATAN_DEG13_C6 +7.2128853633444123e-03
#define GTE_C_ATAN_DEG13_MAX_ERROR 3.5859104691865484e-7

// Constants for minimax polynomial approximations to exp2(x) = 2^x.
// The algorithm minimizes the maximum absolute error on [0,1].
#define GTE_C_EXP2_DEG1_C0 1.0
#define GTE_C_EXP2_DEG1_C1 1.0
#define GTE_C_EXP2_DEG1_MAX_ERROR 8.6071332055934313e-2

#define GTE_C_EXP2_DEG2_C0 1.0
#define GTE_C_EXP2_DEG2_C1 6.5571332605741528e-01
#define GTE_C_EXP2_DEG2_C2 3.4428667394258472e-01
#define GTE_C_EXP2_DEG2_MAX_ERROR 3.8132476831060358e-3

#define GTE_C_EXP2_DEG3_C0 1.0
#define GTE_C_EXP2_DEG3_C1 6.9589012084456225e-01
#define GTE_C_EXP2_DEG3_C2 2.2486494900110188e-01
#define GTE_C_EXP2_DEG3_C3 7.9244930154334980e-02
#define GTE_C_EXP2_DEG3_MAX_ERROR 1.4694877755186408e-4

#define GTE_C_EXP2_DEG4_C0 1.0
#define GTE_C_EXP2_DEG4_C1 6.9300392358459195e-01
#define GTE_C_EXP2_DEG4_C2 2.4154981722455560e-01
#define GTE_C_EXP2_DEG4_C3 5.1744260331489045e-02
#define GTE_C_EXP2_DEG4_C4 1.3701998859367848e-02
#define GTE_C_EXP2_DEG4_MAX_ERROR 4.7617792624521371e-6

#define GTE_C_EXP2_DEG5_C0 1.0
#define GTE_C_EXP2_DEG5_C1 6.9315298010274962e-01
#define GTE_C_EXP2_DEG5_C2 2.4014712313022102e-01
#define GTE_C_EXP2_DEG5_C3 5.5855296413199085e-02
#define GTE_C_EXP2_DEG5_C4 8.9477503096873079e-03
#define GTE_C_EXP2_DEG5_C5 1.8968500441332026e-03
#define GTE_C_EXP2_DEG5_MAX_ERROR 1.3162098333463490e-7

#define GTE_C_EXP2_DEG6_C0 1.0
#define GTE_C_EXP2_DEG6_C1 6.9314698914837525e-01
#define GTE_C_EXP2_DEG6_C2 2.4023013440952923e-01
#define GTE_C_EXP2_DEG6_C3 5.5481276898206033e-02
#define GTE_C_EXP2_DEG6_C4 9.6838443037086108e-03
#define GTE_C_EXP2_DEG6_C5 1.2388324048515642e-03
#define GTE_C_EXP2_DEG6_C6 2.1892283501756538e-04
#define GTE_C_EXP2_DEG6_MAX_ERROR 3.1589168225654163e-9

#define GTE_C_EXP2_DEG7_C0 1.0
#define GTE_C_EXP2_DEG7_C1 6.9314718588750690e-01
#define GTE_C_EXP2_DEG7_C2 2.4022637363165700e-01
#define GTE_C_EXP2_DEG7_C3 5.5505235570535660e-02
#define GTE_C_EXP2_DEG7_C4 9.6136265387940512e-03
#define GTE_C_EXP2_DEG7_C5 1.3429234504656051e-03
#define GTE_C_EXP2_DEG7_C6 1.4299202757683815e-04
#define GTE_C_EXP2_DEG7_C7 2.1662892777385423e-05
#define GTE_C_EXP2_DEG7_MAX_ERROR 6.6864513925679603e-11

// Constants for minimax polynomial approximations to log2(x).
// The algorithm minimizes the maximum absolute error on [1,2].
// The polynomials all have constant term zero.
#define GTE_C_LOG2_DEG1_C1 +1.0
#define GTE_C_LOG2_DEG1_MAX_ERROR 8.6071332055934202e-2

#define GTE_C_LOG2_DEG2_C1 +1.3465553856377803 
#define GTE_C_LOG2_DEG2_C2 -3.4655538563778032e-01
#define GTE_C_LOG2_DEG2_MAX_ERROR 7.6362868906658110e-3

#define GTE_C_LOG2_DEG3_C1 +1.4228653756681227
#define GTE_C_LOG2_DEG3_C2 -5.8208556916449616e-01
#define GTE_C_LOG2_DEG3_C3 +1.5922019349637218e-01
#define GTE_C_LOG2_DEG3_MAX_ERROR 8.7902902652883808e-4

#define GTE_C_LOG2_DEG4_C1 +1.4387257478171547
#define GTE_C_LOG2_DEG4_C2 -6.7778401359918661e-01
#define GTE_C_LOG2_DEG4_C3 +3.2118898377713379e-01
#define GTE_C_LOG2_DEG4_C4 -8.2130717995088531e-02
#define GTE_C_LOG2_DEG4_MAX_ERROR 1.1318551355360418e-4

#define GTE_C_LOG2_DEG5_C1 +1.4419170408633741
#define GTE_C_LOG2_DEG5_C2 -7.0909645927612530e-01
#define GTE_C_LOG2_DEG5_C3 +4.1560609399164150e-01
#define GTE_C_LOG2_DEG5_C4 -1.9357573729558908e-01
#define GTE_C_LOG2_DEG5_C5 +4.5149061716699634e-02
#define GTE_C_LOG2_DEG5_MAX_ERROR 1.5521274478735858e-5

#define GTE_C_LOG2_DEG6_C1 +1.4425449435950917
#define GTE_C_LOG2_DEG6_C2 -7.1814525675038965e-01
#define GTE_C_LOG2_DEG6_C3 +4.5754919692564044e-01
#define GTE_C_LOG2_DEG6_C4 -2.7790534462849337e-01
#define GTE_C_LOG2_DEG6_C5 +1.2179791068763279e-01
#define GTE_C_LOG2_DEG6_C6 -2.5841449829670182e-02
#define GTE_C_LOG2_DEG6_MAX_ERROR 2.2162051216689793e-6

#define GTE_C_LOG2_DEG7_C1 +1.4426664401536078
#define GTE_C_LOG2_DEG7_C2 -7.2055423726162360e-01
#define GTE_C_LOG2_DEG7_C3 +4.7332419162501083e-01
#define GTE_C_LOG2_DEG7_C4 -3.2514018752954144e-01
#define GTE_C_LOG2_DEG7_C5 +1.9302965529095673e-01
#define GTE_C_LOG2_DEG7_C6 -7.8534970641157997e-02
#define GTE_C_LOG2_DEG7_C7 +1.5209108363023915e-02
#define GTE_C_LOG2_DEG7_MAX_ERROR 3.2546531700261561e-7

#define GTE_C_LOG2_DEG8_C1 +1.4426896453621882
#define GTE_C_LOG2_DEG8_C2 -7.2115893912535967e-01
#define GTE_C_LOG2_DEG8_C3 +4.7861716616785088e-01
#define GTE_C_LOG2_DEG8_C4 -3.4699935395019565e-01
#define GTE_C_LOG2_DEG8_C5 +2.4114048765477492e-01
#define GTE_C_LOG2_DEG8_C6 -1.3657398692885181e-01
#define GTE_C_LOG2_DEG8_C7 +5.1421382871922106e-02
#define GTE_C_LOG2_DEG8_C8 -9.1364020499895560e-03
#define GTE_C_LOG2_DEG8_MAX_ERROR 4.8796219218050219e-8

// These functions are convenient for some applications.  The classes
// BSNumber, BSRational and IEEEBinary16 have implementations that
// (for now) use typecasting to call the 'float' or 'double' versions.
namespace gte
{
    inline float atandivpi(float x)
    {
        return std::atan(x) * (float)GTE_C_INV_PI;
    }

    inline float atan2divpi(float y, float x)
    {
        return std::atan2(y, x) * (float)GTE_C_INV_PI;
    }

    inline float clamp(float x, float xmin, float xmax)
    {
        return (x <= xmin ? xmin : (x >= xmax ? xmax : x));
    }

    inline float cospi(float x)
    {
        return std::cos(x * (float)GTE_C_PI);
    }

    inline float exp10(float x)
    {
        return std::exp(x * (float)GTE_C_LN_10);
    }

    inline float invsqrt(float x)
    {
        return 1.0f / std::sqrt(x);
    }

    inline int isign(float x)
    {
        return (x > 0.0f ? 1 : (x < 0.0f ? -1 : 0));
    }

    inline float saturate(float x)
    {
        return (x <= 0.0f ? 0.0f : (x >= 1.0f ? 1.0f : x));
    }

    inline float sign(float x)
    {
        return (x > 0.0f ? 1.0f : (x < 0.0f ? -1.0f : 0.0f));
    }

    inline float sinpi(float x)
    {
        return std::sin(x * (float)GTE_C_PI);
    }

    inline float sqr(float x)
    {
        return x * x;
    }


    inline double atandivpi(double x)
    {
        return std::atan(x) * GTE_C_INV_PI;
    }

    inline double atan2divpi(double y, double x)
    {
        return std::atan2(y, x) * GTE_C_INV_PI;
    }

    inline double clamp(double x, double xmin, double xmax)
    {
        return (x <= xmin ? xmin : (x >= xmax ? xmax : x));
    }

    inline double cospi(double x)
    {
        return std::cos(x * GTE_C_PI);
    }

    inline double exp10(double x)
    {
        return std::exp(x * GTE_C_LN_10);
    }

    inline double invsqrt(double x)
    {
        return 1.0 / std::sqrt(x);
    }

    inline double sign(double x)
    {
        return (x > 0.0 ? 1.0 : (x < 0.0 ? -1.0 : 0.0f));
    }

    inline int isign(double x)
    {
        return (x > 0.0 ? 1 : (x < 0.0 ? -1 : 0));
    }

    inline double saturate(double x)
    {
        return (x <= 0.0 ? 0.0 : (x >= 1.0 ? 1.0 : x));
    }

    inline double sinpi(double x)
    {
        return std::sin(x * GTE_C_PI);
    }

    inline double sqr(double x)
    {
        return x * x;
    }
}

// Type traits to support std::enable_if conditional compilation for
// numerical computations.
namespace gte
{
    // The trait is_arbitrary_precision<T> for type T of float, double or
    // long double generates is_arbitrary_precision<T>::value of false.  The
    // implementations for arbitrary-precision arithmetic are found in
    // GteArbitraryPrecision.h.
    template <typename T>
    struct is_arbitrary_precision_internal : std::false_type {};

    template <typename T>
    struct is_arbitrary_precision : is_arbitrary_precision_internal<T>::type {};

    // The trait has_division_operator<T> for type T of float, double or
    // long double generates has_division_operator<T>::value of true.  The
    // implementations for arbitrary-precision arithmetic are found in
    // ArbitraryPrecision.h.
    template <typename T>
    struct has_division_operator_internal : std::false_type {};

    template <typename T>
    struct has_division_operator : has_division_operator_internal<T>::type {};

    template <>
    struct has_division_operator_internal<float> : std::true_type {};

    template <>
    struct has_division_operator_internal<double> : std::true_type {};

    template <>
    struct has_division_operator_internal<long double> : std::true_type {};
}
