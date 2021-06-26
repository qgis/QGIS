// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.15

#pragma once

#include <Applications/Environment.h>
#include <Graphics/Graphics.h>

// The Application class is an abstract base class with two derived classes,
// ConsoleApplication and WindowApplication. All parameters for constructing
// Application objects are in the Parameters structure or in nested structures
// derived from Parameters.

namespace gte
{
    class Application
    {
    public:
        struct Parameters
        {
            // Window applications using the GPU must set these. Console
            // applications that do not use the GPU may are not required to
            // set these, in which case 'engine' and 'factory' are null.
            std::shared_ptr<BaseEngine> engine;
            std::shared_ptr<ProgramFactory> factory;
        };

    public:
        // Abstract base class.
        virtual ~Application() = default;
    protected:
        Application(Parameters const& parameters);

    public:
        // Get the value of the GTE_PATH environment variable. Derived
        // classes may use this variable to ensure the existence of input
        // data sets that are required by an application. If the function
        // returns "", the GTE_PATH variable has not been set.
        inline std::string GetGTEPath() const
        {
            return mEnvironment.GetGTEPath();
        }

    protected:
        // Support for access to environment variables and paths.
        Environment mEnvironment;

        // The graphics engine and program factory are stored as base
        // class pointers to allow Application to be independent of the
        // corresponding graphics API subsystems.
        std::shared_ptr<BaseEngine> mBaseEngine;
        std::shared_ptr<ProgramFactory> mProgramFactory;
    };
}
