// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

#pragma once

#include <Applications/Console.h>
#include "RemezConstrained.h"
using namespace gte;

class RotationApproximationConsole : public Console
{
public:
    RotationApproximationConsole(Parameters& parameters);

    virtual void Execute() override;

private:
    void DoEstimate(size_t select, RemezConstrained& estimator);
};
