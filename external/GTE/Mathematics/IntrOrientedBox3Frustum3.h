// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/Frustum3.h>
#include <Mathematics/OrientedBox.h>

// The test-intersection query uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The potential separating axes include the 3 box face normals, the 5
// distinct frustum normals (near and far plane have the same normal), and
// cross products of normals, one from the box and one from the frustum.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox3<Real>, Frustum3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(OrientedBox3<Real> const& box, Frustum3<Real> const& frustum)
        {
            Result result;

            // Convenience variables.
            Vector3<Real> const* axis = &box.axis[0];
            Vector3<Real> const& extent = box.extent;

            Vector3<Real> diff = box.center - frustum.origin;  // C-E

            Real A[3];      // Dot(R,A[i])
            Real B[3];      // Dot(U,A[i])
            Real C[3];      // Dot(D,A[i])
            Real D[3];      // (Dot(R,C-E),Dot(U,C-E),Dot(D,C-E))
            Real NA[3];     // dmin*Dot(R,A[i])
            Real NB[3];     // dmin*Dot(U,A[i])
            Real NC[3];     // dmin*Dot(D,A[i])
            Real ND[3];     // dmin*(Dot(R,C-E),Dot(U,C-E),?)
            Real RC[3];     // rmax*Dot(D,A[i])
            Real RD[3];     // rmax*(?,?,Dot(D,C-E))
            Real UC[3];     // umax*Dot(D,A[i])
            Real UD[3];     // umax*(?,?,Dot(D,C-E))
            Real NApRC[3];  // dmin*Dot(R,A[i]) + rmax*Dot(D,A[i])
            Real NAmRC[3];  // dmin*Dot(R,A[i]) - rmax*Dot(D,A[i])
            Real NBpUC[3];  // dmin*Dot(U,A[i]) + umax*Dot(D,A[i])
            Real NBmUC[3];  // dmin*Dot(U,A[i]) - umax*Dot(D,A[i])
            Real RBpUA[3];  // rmax*Dot(U,A[i]) + umax*Dot(R,A[i])
            Real RBmUA[3];  // rmax*Dot(U,A[i]) - umax*Dot(R,A[i])
            Real DdD, radius, p, fmin, fmax, MTwoUF, MTwoRF, tmp;
            int i, j;

            // M = D
            D[2] = Dot(diff, frustum.dVector);
            for (i = 0; i < 3; ++i)
            {
                C[i] = Dot(axis[i], frustum.dVector);
            }
            radius =
                extent[0] * std::fabs(C[0]) +
                extent[1] * std::fabs(C[1]) +
                extent[2] * std::fabs(C[2]);
            if (D[2] + radius < frustum.dMin
                || D[2] - radius > frustum.dMax)
            {
                result.intersect = false;
                return result;
            }

            // M = n*R - r*D
            for (i = 0; i < 3; ++i)
            {
                A[i] = Dot(axis[i], frustum.rVector);
                RC[i] = frustum.rBound * C[i];
                NA[i] = frustum.dMin * A[i];
                NAmRC[i] = NA[i] - RC[i];
            }
            D[0] = Dot(diff, frustum.rVector);
            radius =
                extent[0] * std::fabs(NAmRC[0]) +
                extent[1] * std::fabs(NAmRC[1]) +
                extent[2] * std::fabs(NAmRC[2]);
            ND[0] = frustum.dMin * D[0];
            RD[2] = frustum.rBound * D[2];
            DdD = ND[0] - RD[2];
            MTwoRF = frustum.GetMTwoRF();
            if (DdD + radius < MTwoRF || DdD > radius)
            {
                result.intersect = false;
                return result;
            }

            // M = -n*R - r*D
            for (i = 0; i < 3; ++i)
            {
                NApRC[i] = NA[i] + RC[i];
            }
            radius =
                extent[0] * std::fabs(NApRC[0]) +
                extent[1] * std::fabs(NApRC[1]) +
                extent[2] * std::fabs(NApRC[2]);
            DdD = -(ND[0] + RD[2]);
            if (DdD + radius < MTwoRF || DdD > radius)
            {
                result.intersect = false;
                return result;
            }

            // M = n*U - u*D
            for (i = 0; i < 3; ++i)
            {
                B[i] = Dot(axis[i], frustum.uVector);
                UC[i] = frustum.uBound * C[i];
                NB[i] = frustum.dMin * B[i];
                NBmUC[i] = NB[i] - UC[i];
            }
            D[1] = Dot(diff, frustum.uVector);
            radius =
                extent[0] * std::fabs(NBmUC[0]) +
                extent[1] * std::fabs(NBmUC[1]) +
                extent[2] * std::fabs(NBmUC[2]);
            ND[1] = frustum.dMin * D[1];
            UD[2] = frustum.uBound * D[2];
            DdD = ND[1] - UD[2];
            MTwoUF = frustum.GetMTwoUF();
            if (DdD + radius < MTwoUF || DdD > radius)
            {
                result.intersect = false;
                return result;
            }

            // M = -n*U - u*D
            for (i = 0; i < 3; ++i)
            {
                NBpUC[i] = NB[i] + UC[i];
            }
            radius =
                extent[0] * std::fabs(NBpUC[0]) +
                extent[1] * std::fabs(NBpUC[1]) +
                extent[2] * std::fabs(NBpUC[2]);
            DdD = -(ND[1] + UD[2]);
            if (DdD + radius < MTwoUF || DdD > radius)
            {
                result.intersect = false;
                return result;
            }

            // M = A[i]
            for (i = 0; i < 3; ++i)
            {
                p = frustum.rBound * std::fabs(A[i]) +
                    frustum.uBound * std::fabs(B[i]);
                NC[i] = frustum.dMin * C[i];
                fmin = NC[i] - p;
                if (fmin < (Real)0)
                {
                    fmin *= frustum.GetDRatio();
                }
                fmax = NC[i] + p;
                if (fmax > (Real)0)
                {
                    fmax *= frustum.GetDRatio();
                }
                DdD = A[i] * D[0] + B[i] * D[1] + C[i] * D[2];
                if (DdD + extent[i] < fmin || DdD - extent[i] > fmax)
                {
                    result.intersect = false;
                    return result;
                }
            }

            // M = Cross(R,A[i])
            for (i = 0; i < 3; ++i)
            {
                p = frustum.uBound * std::fabs(C[i]);
                fmin = -NB[i] - p;
                if (fmin < (Real)0)
                {
                    fmin *= frustum.GetDRatio();
                }
                fmax = -NB[i] + p;
                if (fmax > (Real)0)
                {
                    fmax *= frustum.GetDRatio();
                }
                DdD = C[i] * D[1] - B[i] * D[2];
                radius =
                    extent[0] * std::fabs(B[i] * C[0] - B[0] * C[i]) +
                    extent[1] * std::fabs(B[i] * C[1] - B[1] * C[i]) +
                    extent[2] * std::fabs(B[i] * C[2] - B[2] * C[i]);
                if (DdD + radius < fmin || DdD - radius > fmax)
                {
                    result.intersect = false;
                    return result;
                }
            }

            // M = Cross(U,A[i])
            for (i = 0; i < 3; ++i)
            {
                p = frustum.rBound * std::fabs(C[i]);
                fmin = NA[i] - p;
                if (fmin < (Real)0)
                {
                    fmin *= frustum.GetDRatio();
                }
                fmax = NA[i] + p;
                if (fmax > (Real)0)
                {
                    fmax *= frustum.GetDRatio();
                }
                DdD = -C[i] * D[0] + A[i] * D[2];
                radius =
                    extent[0] * std::fabs(A[i] * C[0] - A[0] * C[i]) +
                    extent[1] * std::fabs(A[i] * C[1] - A[1] * C[i]) +
                    extent[2] * std::fabs(A[i] * C[2] - A[2] * C[i]);
                if (DdD + radius < fmin || DdD - radius > fmax)
                {
                    result.intersect = false;
                    return result;
                }
            }

            // M = Cross(n*D+r*R+u*U,A[i])
            for (i = 0; i < 3; ++i)
            {
                Real fRB = frustum.rBound * B[i];
                Real fUA = frustum.uBound * A[i];
                RBpUA[i] = fRB + fUA;
                RBmUA[i] = fRB - fUA;
            }
            for (i = 0; i < 3; ++i)
            {
                p = frustum.rBound * std::fabs(NBmUC[i]) +
                    frustum.uBound * std::fabs(NAmRC[i]);
                tmp = -frustum.dMin * RBmUA[i];
                fmin = tmp - p;
                if (fmin < (Real)0)
                {
                    fmin *= frustum.GetDRatio();
                }
                fmax = tmp + p;
                if (fmax > (Real)0)
                {
                    fmax *= frustum.GetDRatio();
                }
                DdD = D[0] * NBmUC[i] - D[1] * NAmRC[i] - D[2] * RBmUA[i];
                radius = (Real)0;
                for (j = 0; j < 3; j++)
                {
                    radius += extent[j] * std::fabs(A[j] * NBmUC[i] -
                        B[j] * NAmRC[i] - C[j] * RBmUA[i]);
                }
                if (DdD + radius < fmin || DdD - radius > fmax)
                {
                    result.intersect = false;
                    return result;
                }
            }

            // M = Cross(n*D+r*R-u*U,A[i])
            for (i = 0; i < 3; ++i)
            {
                p = frustum.rBound * std::fabs(NBpUC[i]) +
                    frustum.uBound * std::fabs(NAmRC[i]);
                tmp = -frustum.dMin * RBpUA[i];
                fmin = tmp - p;
                if (fmin < (Real)0)
                {
                    fmin *= frustum.GetDRatio();
                }
                fmax = tmp + p;
                if (fmax > (Real)0)
                {
                    fmax *= frustum.GetDRatio();
                }
                DdD = D[0] * NBpUC[i] - D[1] * NAmRC[i] - D[2] * RBpUA[i];
                radius = (Real)0;
                for (j = 0; j < 3; ++j)
                {
                    radius += extent[j] * std::fabs(A[j] * NBpUC[i] -
                        B[j] * NAmRC[i] - C[j] * RBpUA[i]);
                }
                if (DdD + radius < fmin || DdD - radius > fmax)
                {
                    result.intersect = false;
                    return result;
                }
            }

            // M = Cross(n*D-r*R+u*U,A[i])
            for (i = 0; i < 3; ++i)
            {
                p = frustum.rBound * std::fabs(NBmUC[i]) +
                    frustum.uBound * std::fabs(NApRC[i]);
                tmp = frustum.dMin * RBpUA[i];
                fmin = tmp - p;
                if (fmin < (Real)0)
                {
                    fmin *= frustum.GetDRatio();
                }
                fmax = tmp + p;
                if (fmax > (Real)0)
                {
                    fmax *= frustum.GetDRatio();
                }
                DdD = D[0] * NBmUC[i] - D[1] * NApRC[i] + D[2] * RBpUA[i];
                radius = (Real)0;
                for (j = 0; j < 3; ++j)
                {
                    radius += extent[j] * std::fabs(A[j] * NBmUC[i] -
                        B[j] * NApRC[i] + C[j] * RBpUA[i]);
                }
                if (DdD + radius < fmin || DdD - radius > fmax)
                {
                    result.intersect = false;
                    return result;
                }
            }

            // M = Cross(n*D-r*R-u*U,A[i])
            for (i = 0; i < 3; ++i)
            {
                p = frustum.rBound * std::fabs(NBpUC[i]) +
                    frustum.uBound * std::fabs(NApRC[i]);
                tmp = frustum.dMin * RBmUA[i];
                fmin = tmp - p;
                if (fmin < (Real)0)
                {
                    fmin *= frustum.GetDRatio();
                }
                fmax = tmp + p;
                if (fmax > (Real)0)
                {
                    fmax *= frustum.GetDRatio();
                }
                DdD = D[0] * NBpUC[i] - D[1] * NApRC[i] + D[2] * RBmUA[i];
                radius = (Real)0;
                for (j = 0; j < 3; ++j)
                {
                    radius += extent[j] * std::fabs(A[j] * NBpUC[i] -
                        B[j] * NApRC[i] + C[j] * RBmUA[i]);
                }
                if (DdD + radius < fmin || DdD - radius > fmax)
                {
                    result.intersect = false;
                    return result;
                }
            }

            result.intersect = true;
            return result;
        }
    };
}
