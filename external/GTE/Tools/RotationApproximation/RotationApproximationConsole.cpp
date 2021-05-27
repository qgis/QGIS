// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

#include "RotationApproximationConsole.h"
#include "RemezRotC0.h"
#include "RemezRotC1.h"
#include "RemezRotC2.h"
#include "RemezRotC3.h"
#include <iostream>
#include <iomanip>
using namespace gte;

RotationApproximationConsole::RotationApproximationConsole(Parameters& parameters)
    :
    Console(parameters)
{
}

void RotationApproximationConsole::Execute()
{
    RemezRotC0 estimator0;
    RemezRotC1 estimator1;
    RemezRotC2 estimator2;
    RemezRotC3 estimator3;
    DoEstimate(0, estimator0);
    DoEstimate(1, estimator1);
    DoEstimate(2, estimator2);
    DoEstimate(3, estimator3);
}

void RotationApproximationConsole::DoEstimate(size_t select, RemezConstrained& estimator)
{
    double const tMin = 0.0, tMax = GTE_C_PI;
    size_t const maxRemez = 16;
    size_t const maxBisect = 128;
    size_t iterations;
    std::string order = std::to_string(select);
    std::ofstream output("RotC" + order + "Info.txt");
    output << std::scientific << std::showpos << std::setprecision(17);

    // For all executions: t[0] = 0, t[degree+1] = pi, e[0] = e[degree+1] = 0.
    for (size_t degree = 2; degree <= 8; ++degree)
    {
        iterations = estimator.Execute(tMin, tMax, degree, maxRemez, maxBisect);

        output << "degree " << degree << std::endl;
        output << "iterations = " << iterations << std::endl;
        for (size_t i = 0; i <= degree; ++i)
        {
            output << "p[" << i << "] = " << estimator.GetCoefficients()[i] << std::endl;
        }
        output << "erro = " << std::fabs(estimator.GetEstimatedMaxError()) << std::endl;
        for (size_t i = 1; i <= degree; ++i)
        {
            output << "t[" << i << "] = " << estimator.GetTNodes()[i] << std::endl;
        }
        for (size_t i = 1; i <= degree; ++i)
        {
            output << "e[" << i << "] = " << estimator.GetErrors()[i] << std::endl;
        }
        output << std::endl;
    }
    output.close();

    std::vector<double> maxError(7);
    output.open("RotC" + order + "GTL.txt");
    output << std::scientific << std::showpos << std::setprecision(17);
    output << "    std::array<std::array<double, 9>, 7> constexpr C_ROTC" << select << "_EST_COEFF =" << std::endl;
    output << "    { {" << std::endl;
    for (size_t degree = 2; degree <= 8; ++degree)
    {
        iterations = estimator.Execute(tMin, tMax, degree, maxRemez, maxBisect);
        maxError[degree - 2] = estimator.GetEstimatedMaxError();
        output << "        {   // degree " << 2 * degree << std::endl;
        for (size_t i = 0; i <= degree; ++i)
        {
            output << "            " << estimator.GetCoefficients()[i];
            if (i < degree)
            {
                output << ",";
            }
            output << std::endl;
        }
        output << "        }";
        if (degree < 8)
        {
            output << ",";
        }
        output << std::endl;

        std::vector<double> errors = estimator.GetErrors();
        for (auto& e : errors)
        {
            e = std::fabs(e);
        }
        std::sort(errors.begin(), errors.end());
        maxError[degree - 2] = errors.back();
    }
    output << "    } };" << std::endl;
    output << std::endl;

    output << std::noshowpos << std::setprecision(13);
    output << "    std::array<double, 7> constexpr C_ROTC" << select << "_EST_MAX_ERROR =" << std::endl;
    output << "    {" << std::endl;
    for (size_t i = 0; i < maxError.size(); ++i)
    {
        output << "        " << maxError[i];
        if (i + 1 < maxError.size())
        {
            output << ",";
        }
        else
        {
            output << " ";
        }
        output << "    // degree " << 2 * (i + 2) << std::endl;
    }
    output << "    };" << std::endl;
    output.close();
}
