// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#pragma once

#include <string>
#include <vector>

namespace gte
{
    class Command
    {
    public:
        // Construction and destruction.
        ~Command() = default;
        Command(int numArguments, char const* arguments[]);
        Command(int numArguments, char* arguments[]);

        // The return value is the index of the first excess argument.
        int ExcessArguments() const;

        // Set bounds for numerical arguments.  If bounds are required, they
        // must be set for each argument.
        Command& Min(double value);
        Command& Max(double value);
        Command& Inf(double value);
        Command& Sup(double value);

        // The return value of each of the following methods is the option
        // index within the argument array.

        // Use the bool methods for options which take no argument, for
        // example in
        //     myprogram -debug -x 10 -y 20 filename
        // the option -debug has no argument.  The first GetBoolean function
        // returns only the existence of the option.
        int GetBoolean(std::string const& name);
        int GetBoolean(std::string const& name, bool& value);
        int GetInteger(std::string const& name, int& value);
        int GetFloat(std::string const& name, float& value);
        int GetDouble(std::string const& name, double& value);
        int GetString(std::string const& name, std::string& value);
        int GetFilename(std::string& value, int startArgIndex = 1);

        // Last-error reporting.
        inline std::string GetLastError() const
        {
            return mLastError;
        }

    private:
        // The member mArguments contains the array of input strings.  The
        // member mProcessed indicates those arguments that have been
        // processed by the parser.
        std::vector<std::string> mArguments;
        std::vector<bool> mProcessed;

        // Parameters for bounds checking.
        double mSmall;  // lower bound for numerical argument (min or inf)
        double mLarge;  // upper bound for numerical argument (max or sup)
        bool mMinSet;   // if true, compare:  small <= arg
        bool mMaxSet;   // if true, compare:  arg <= large
        bool mInfSet;   // if true, compare:  small < arg
        bool mSupSet;   // if true, compare:  arg < large

        // Last-error strings.
        std::string mLastError;
        static std::string const msOptNotFound;
        static std::string const msArgRequired;
        static std::string const msArgOutOfRange;
        static std::string const msFilenameNotFound;
        static std::string const msDash;
    };
}
