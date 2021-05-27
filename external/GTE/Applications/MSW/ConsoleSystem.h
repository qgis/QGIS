// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Console.h>
#include <memory>

namespace gte
{
    class ConsoleSystem
    {
    public:
        ConsoleSystem() = default;
        ~ConsoleSystem() = default;

        // Create and destroy consoles.  Derived classes may extend the inputs
        // using a nested class derived from Console::Parameters
        template <typename ConsoleType>
        std::shared_ptr<ConsoleType> Create(typename ConsoleType::Parameters& parameters)
        {
            CreateEngineAndProgramFactory(parameters);
            if (parameters.created)
            {
                auto console = std::make_shared<ConsoleType>(parameters);
                if (parameters.created)
                {
                    return console;
                }
                Destroy(console);
            }
            return nullptr;
        }

        template <typename ConsoleType>
        void Destroy(std::shared_ptr<ConsoleType>& console)
        {
            console = nullptr;
        }

        template <typename ConsoleType>
        void Execute(std::shared_ptr<ConsoleType> const& console)
        {
            if (console)
            {
                console->Execute();
            }
        }

    protected:
        // The CreateEngineAndProgramFactory function has an implementation
        // for DX11 and an implementation for WGL. It is not possible to have
        // both DX11-based and WGL-based console creation in the same
        // application, although it is possible to have DX11-based and
        // WGL-based graphics engines in the same application.
        void CreateEngineAndProgramFactory(Console::Parameters& parameters);
    };

    extern ConsoleSystem TheConsoleSystem;
}
