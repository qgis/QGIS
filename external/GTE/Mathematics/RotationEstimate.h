// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

// Rotation matrices can be constructed using estimates of the coefficients
// that involve trigonometric and polynomial terms. See
// https://www.geometrictools.com/Documentation/RotationEstimation.pdf
// for the length details.

#pragma once

#include <Mathematics/Matrix.h>
#include <array>

namespace gte
{
    // Constants for rotc0(t) = sin(t)/t.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC0_EST_COEFF =
    { {
        {   // degree 4
            +1.00000000000000000e+00,
            -1.58971650732578684e-01,
            +5.84121356311684790e-03
        },
        {   // degree 6
            +1.00000000000000000e+00,
            -1.66218398161274539e-01,
            +8.06129151017077016e-03,
            -1.50545944866583496e-04
        },
        {   // degree 8
            +1.00000000000000000e+00,
            -1.66651290458553397e-01,
            +8.31836205080888937e-03,
            -1.93853969255209339e-04,
            +2.19921657358978346e-06
        },
        {   // degree 10
            +1.00000000000000000e+00,
            -1.66666320608302304e-01,
            +8.33284074932796014e-03,
            -1.98184457544372085e-04,
            +2.70931602688878442e-06,
            -2.07033154672609224e-08
        },
        {   // degree 12
            +1.00000000000000000e+00,
            -1.66666661172424985e-01,
            +8.33332258782319701e-03,
            -1.98405693280704135e-04,
            +2.75362742468406608e-06,
            -2.47308402190765123e-08,
            +1.36149932075244694e-10
        },
        {   // degree 14
            +1.00000000000000000e+00,
            -1.66666666601880786e-01,
            +8.33333316679120591e-03,
            -1.98412553530683797e-04,
            +2.75567210003238900e-06,
            -2.50388692626200884e-08,
            +1.58972932135933544e-10,
            -6.61111627233688785e-13
        },
        {   // degree 16
            +1.00000000000000000e+00,
            -1.66666666666648478e-01,
            +8.33333333318112164e-03,
            -1.98412698077537775e-04,
            +2.75573162083557394e-06,
            -2.50519743096581360e-08,
            +1.60558314470477309e-10,
            -7.60488921303402553e-13,
            +2.52255089807125025e-15
        }
    } };

    std::array<double, 7> constexpr C_ROTC0_EST_MAX_ERROR =
    {
        6.9656371186750e-03,    // degree 4
        2.2379506089580e-04,    // degree 6
        4.8670096434722e-06,    // degree 8
        7.5654711606532e-08,    // degree 10
        8.7939172610518e-10,    // degree 12
        7.9199615615755e-12,    // degree 14
        6.8001160258291e-16     // degree 16
    };

    // Constants for rotc1(t) = (1-cos(t))/t^2.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC1_EST_COEFF =
    { {
        {   // degree 4
            +5.00000000000000000e-01,
            -4.06593520914583922e-02,
            +1.06698549928666312e-03
        },
        {   // degree 6
            +5.00000000000000000e-01,
            -4.16202835017619524e-02,
            +1.36087417563353699e-03,
            -1.99122437404000405e-05
        },
        {   // degree 8
            +5.00000000000000000e-01,
            -4.16653520191245796e-02,
            +1.38761160375298095e-03,
            -2.44138380330618480e-05,
            +2.28499434819148172e-07
        },
        {   // degree 10
            +5.00000000000000000e-01,
            -4.16666414534321572e-02,
            +1.38885303988537192e-03,
            -2.47850001122705350e-05,
            +2.72207208413898425e-07,
            -1.77358008600681907e-09
        },
        {   // degree 12
            +5.00000000000000000e-01,
            -4.16666663178411334e-02,
            +1.38888820709641924e-03,
            -2.48011431705518285e-05,
            +2.75439902962340229e-07,
            -2.06736081122602257e-09,
            +9.93003618302030503e-12
        },
        {   // degree 14
            +5.00000000000000000e-01,
            -4.16666666664263635e-02,
            +1.38888888750799658e-03,
            -2.48015851902670717e-05,
            +2.75571871163332658e-07,
            -2.08727380201649381e-09,
            +1.14076763269827225e-11,
            -4.28619236995285237e-14
        },
        {   // degree 16
            +5.00000000000000000e-01,
            -4.16666666666571719e-02,
            +1.38888888885105744e-03,
            -2.48015872513761947e-05,
            +2.75573160474227648e-07,
            -2.08766469798137579e-09,
            +1.14685460418668139e-11,
            -4.75415775440997119e-14,
            +1.40555891469552795e-16
        }
    } };

    std::array<double, 7> constexpr C_ROTC1_EST_MAX_ERROR =
    {
        9.2119010150538e-04,    // degree 4
        2.3251261806301e-05,    // degree 6
        4.1693160884870e-07,    // degree 8
        5.5177887536839e-09,    // degree 10
        5.5865700954172e-11,    // degree 12
        7.1609385088323e-15,    // degree 14
        7.2164496600635e-16     // degree 16
    };

    // Constants for rotc2(t) = (sin(t) - t*cos(t))/t^3.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC2_EST_COEFF =
    { {
        {   // degree 4
            +3.33333333333333315e-01,
            -3.24417271573718483e-02,
            +9.05201583387763454e-04
        },
        {   // degree 6
            +3.33333333333333315e-01,
            -3.32912781805089902e-02,
            +1.16506615743456146e-03,
            -1.76083105011587047e-05
        },
        {   // degree 8
            +3.33333333333333315e-01,
            -3.33321218985461534e-02,
            +1.18929901553194335e-03,
            -2.16884239911580259e-05,
            +2.07111898922214621e-07
        },
        {   // degree 10
            +3.33333333333333315e-01,
            -3.33333098285273563e-02,
            +1.19044276839748377e-03,
            -2.20303898188601926e-05,
            +2.47382309397892291e-07,
            -1.63412179599052932e-09
        },
        {   // degree 12
            +3.33333333333333315e-01,
            -3.33333330053029661e-02,
            +1.19047554930589209e-03,
            -2.20454376925152508e-05,
            +2.50395723787030737e-07,
            -1.90797721719554658e-09,
            +9.25661051509749896e-12
        },
        {   // degree 14
            +3.33333333333333315e-01,
            -3.33333333331133561e-02,
            +1.19047618918715682e-03,
            -2.20458533943125258e-05,
            +2.50519837811549507e-07,
            -1.92670551155064303e-09,
            +1.06463697865186991e-11,
            -4.03135292145519115e-14
        },
        {   // degree 16
            +3.33333333333333315e-01,
            -3.33333333333034956e-02,
            +1.19047619036920628e-03,
            -2.20458552540489507e-05,
            +2.50521015434838418e-07,
            -1.92706504721931338e-09,
            +1.07026043656398707e-11,
            -4.46498739610373537e-14,
            +1.30526089083317312e-16
        }
    } };

    std::array<double, 7> constexpr C_ROTC2_EST_MAX_ERROR =
    {
        8.1461508460229e-04,    // degree 4
        2.1075025784856e-05,    // degree 6
        3.8414838612888e-07,    // degree 8
        5.1435967152180e-09,    // degree 10
        5.2533588590364e-11,    // degree 12
        7.7715611723761e-15,    // degree 14
        2.2759572004816e-15     // degree 16
    };

    // Constants for rotc3(t) = (2*(1-cos(t)) - t*sin(t))/t^4.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC3_EST_COEFF =
    { {
        {   // degree 4
            +8.33333333333333287e-02,
            -5.46357009138465424e-03,
            +1.19638433962248889e-04
        },
        {   // degree 6
            +8.33333333333333287e-02,
            -5.55196372993948303e-03,
            +1.46646667516630680e-04,
            -1.82905866698780768e-06
        },
        {   // degree 8
            +8.33333333333333287e-02,
            -5.55546733314307706e-03,
            +1.48723933698110248e-04,
            -2.17865651989456709e-06,
            +1.77408035681006169e-08
        },
        {   // degree 10
            +8.33333333333333287e-02,
            -5.55555406357728914e-03,
            +1.48807404153008735e-04,
            -2.20360578108261882e-06,
            +2.06782449582308932e-08,
            -1.19178562817913197e-10
        },
        {   // degree 12
            +8.33333333333333287e-02,
            -5.55555555324832757e-03,
            +1.48809514798423797e-04,
            -2.20457622072950518e-06,
            +2.08728631685852690e-08,
            -1.36888190776165574e-10,
            +5.99292681875750821e-13
        },
        {   // degree 14
            +8.33333333333333287e-02,
            -5.55555555528319030e-03,
            +1.48809523101214977e-04,
            -2.20458493798151629e-06,
            +2.08765224186559757e-08,
            -1.37600800115177215e-10,
            +6.63762129016229865e-13,
            -2.19044013684859942e-15
        },
        {   // degree 16
            +8.33333333333333287e-02,
            -5.55555555501025672e-03,
            +1.48809521898935978e-04,
            -2.20458342827337994e-06,
            +2.08757075326674457e-08,
            -1.37379825035843510e-10,
            +6.32209097599974706e-13,
            +7.39204014316007136e-17,
            -6.43236558920699052e-17
        }
    } };

    std::array<double, 7> constexpr C_ROTC3_EST_MAX_ERROR =
    {
        8.4612036888886e-05,    // degree 4
        1.8051973185995e-06,    // degree 6
        2.8016103950645e-08,    // degree 8
        3.2675415151395e-10,    // degree 10
        1.3714029911682e-13,    // degree 12
        3.2078506517763e-14,    // degree 14
        4.7774284528401e-14     // degree 16
    };
}

namespace gte
{
    // Estimate rotc0(t) = sin(t)/t for t in [0,pi]. For example, a degree-6
    // estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC0Estimate<float, 6>(t);
    template <typename Real, size_t Degree>
    inline Real RotC0Estimate(Real t)
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC0_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        Real tsqr = t * t;
        Real poly = static_cast<Real>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<Real>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    // Estimate rotc1(t) = (1 - cos(t))/t^2 for t in [0,pi]. For example,
    // a degree-6 estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC1Estimate<float, 6>(t);
    template <typename Real, size_t Degree>
    inline Real RotC1Estimate(Real t)
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC1_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        Real tsqr = t * t;
        Real poly = static_cast<Real>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<Real>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    // Estimate rotc2(t) = (sin(t) - t*cos(t))/t^3 for t in [0,pi]. For
    // example, a degree-6 estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC2Estimate<float, 6>(t);
    template <typename Real, size_t Degree>
    inline Real RotC2Estimate(Real t)
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC2_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        Real tsqr = t * t;
        Real poly = static_cast<Real>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<Real>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    // Estimate rotc3(t) = (2*(1-cos(t)) - t*sin(t))/t^4 for t in
    // [0,pi]. For example, a degree-6 estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC3Estimate<float, 6>(t);
    template <typename Real, size_t Degree>
    inline Real RotC3Estimate(Real t)
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC3_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        Real tsqr = t * t;
        Real poly = static_cast<Real>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<Real>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    template <typename Real, size_t Degree>
    Real constexpr GetRotC0EstimateMaxError()
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");
        return static_cast<Real>(C_ROTC0_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    template <typename Real, size_t Degree>
    Real constexpr GetRotC1EstimateMaxError()
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");
        return static_cast<Real>(C_ROTC1_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    template <typename Real, size_t Degree>
    Real constexpr GetRotC2EstimateMaxError()
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");
        return static_cast<Real>(C_ROTC2_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    template <typename Real, size_t Degree>
    Real constexpr GetRotC3EstimateMaxError()
    {
        static_assert((Degree & 1) == 0 && 4 <= Degree && Degree <= 16, "Invalid degree.");
        return static_cast<Real>(C_ROTC3_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    // Construct the estimate for the rotation matrix
    //   R = exp(S) = I + rotc0(t) * S + rotc1(t) * S^2
    // from a vector (p0,p1,p2) with length t = |(p0,p1,p2)| and
    // skew-symmetric matrix S = {{0,-p2,p1},{p2,0,-p0},{-p1,p0,0}}.
    template <typename Real, size_t Degree>
    void RotationEstimate(Vector<3, Real> const& p,
        Matrix<3, 3, Real>& R)
    {
        Real const zero(0), one(1);

        Matrix<3, 3, Real> I{
            one, zero, zero,
            zero, one, zero,
            zero, zero, one
        };

        Matrix<3, 3, Real> S{
            zero, -p[2], p[1],
            p[2], zero, -p[0],
            -p[1], p[0], zero
        };

        Real p0p0 = p[0] * p[0], p0p1 = p[0] * p[1], p0p2 = p[0] * p[2];
        Real p1p1 = p[1] * p[1], p1p2 = p[1] * p[2], p2p2 = p[2] * p[2];
        Matrix<3, 3, Real> Ssqr{
            -(p1p1 + p2p2), p0p1, p0p2,
            p0p1, -(p0p0 + p2p2), p1p2,
            p0p2, p1p2, -(p0p0 + p1p1)
        };

        Real t = Length(p);
        Real a = RotC0Estimate<Real, Degree>(t);
        Real b = RotC1Estimate<Real, Degree>(t);
        R = I + a * S + b * Ssqr;
    };

    template <typename Real, size_t Degree>
    void RotationDerivativeEstimate(Vector<3, Real> const& p,
        std::array<Matrix<3, 3, Real>, 3>& Rder)
    {
        Real const zero(0), one(1), negOne(-1);

        std::array<Matrix<3, 3, Real>, 3> skewE =
        {
            Matrix<3, 3, Real>{
                zero, zero, zero,
                zero, zero, negOne,
                zero, one, zero
            },

            Matrix<3, 3, Real>{
                zero, zero, one,
                zero, zero, zero,
                negOne, zero, zero
            },

            Matrix<3, 3, Real>{
                zero, negOne, zero,
                one, zero, zero,
                zero, zero, zero
            }
        };

        Matrix<3, 3, Real> S{
            zero, -p[2], p[1],
            p[2], zero, -p[0],
            -p[1], p[0], zero
        };

        Real p0p0 = p[0] * p[0], p0p1 = p[0] * p[1], p0p2 = p[0] * p[2];
        Real p1p1 = p[1] * p[1], p1p2 = p[1] * p[2], p2p2 = p[2] * p[2];
        Matrix<3, 3, Real> Ssqr{
            -(p1p1 + p2p2), p0p1, p0p2,
            p0p1, -(p0p0 + p2p2), p1p2,
            p0p2, p1p2, -(p0p0 + p1p1)
        };

        Real t = Length(p);
        Real a = RotC0Estimate<Real, Degree>(t);
        Real b = RotC1Estimate<Real, Degree>(t);
        Real c = RotC2Estimate<Real, Degree>(t);
        Real d = RotC3Estimate<Real, Degree>(t);
        for (int32_t i = 0; i < 3; ++i)
        {
            Rder[i] = a * skewE[i] + b * (S * skewE[i] + skewE[i] * S) - p[i] * (c * S + d * Ssqr);
        }
    }

    template <typename Real, size_t Degree>
    void RotationAndDerivativeEstimate(Vector<3, Real> const& p,
        Matrix<3, 3, Real>& R, std::array<Matrix<3, 3, Real>, 3>& Rder)
    {
        Real const zero(0), one(1), negOne(-1);

        Matrix<3, 3, Real> I{
            one, zero, zero,
            zero, one, zero,
            zero, zero, one
        };

        std::array<Matrix<3, 3, Real>, 3> skewE =
        {
            Matrix<3, 3, Real>{
                zero, zero, zero,
                zero, zero, negOne,
                zero, one, zero
            },

            Matrix<3, 3, Real>{
                zero, zero, one,
                zero, zero, zero,
                negOne, zero, zero
            },

            Matrix<3, 3, Real>{
                zero, negOne, zero,
                one, zero, zero,
                zero, zero, zero
            }
        };

        Matrix<3, 3, Real> S{
            zero, -p[2], p[1],
            p[2], zero, -p[0],
            -p[1], p[0], zero
        };

        Real p0p0 = p[0] * p[0], p0p1 = p[0] * p[1], p0p2 = p[0] * p[2];
        Real p1p1 = p[1] * p[1], p1p2 = p[1] * p[2], p2p2 = p[2] * p[2];
        Matrix<3, 3, Real> Ssqr{
            -(p1p1 + p2p2), p0p1, p0p2,
            p0p1, -(p0p0 + p2p2), p1p2,
            p0p2, p1p2, -(p0p0 + p1p1)
        };

        Real t = Length(p);
        Real a = RotC0Estimate<Real, Degree>(t);
        Real b = RotC1Estimate<Real, Degree>(t);
        Real c = RotC2Estimate<Real, Degree>(t);
        Real d = RotC3Estimate<Real, Degree>(t);

        R = I + a * S + b * Ssqr;
        for (int32_t i = 0; i < 3; ++i)
        {
            Rder[i] = a * skewE[i] + b * (S * skewE[i] + skewE[i] * S) - p[i] * (c * S + d * Ssqr);
        }
    }
}
