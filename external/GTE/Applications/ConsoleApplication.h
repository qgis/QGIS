// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Application.h>

namespace gte
{
    class ConsoleApplication : public Application
    {
    public:
        struct Parameters : public Application::Parameters
        {
            Parameters();

            Parameters(std::wstring const& inTitle);

            std::wstring title;
            bool created;
        };

    public:
        // Abstract base class.
        virtual ~ConsoleApplication() = default;
    protected:
        ConsoleApplication(Parameters const& parameters);

    public:
        // Member access.
        virtual void SetTitle(std::wstring const& title)
        {
            mTitle = title;
        }

        inline std::wstring GetTitle() const
        {
            return mTitle;
        }

        // Classes derived from ConsoleApplication can do whatever they wish.
        virtual void Execute() = 0;

        std::wstring mTitle;
    };
}
