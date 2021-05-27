// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.15

#pragma once

#include <cstdarg>
#include <string>
#include <vector>

namespace gte
{
    class Environment
    {
    public:
        // Construction and destruction.
        ~Environment() = default;
        Environment() = default;

        // Get the string corresponding to an environment variable.
        std::string GetVariable(std::string const& name) const;

        // Support for paths to locate files.  For platform independence, use
        // "/" for the path separator.  The input 'directory' will
        // automatically be appended with a trailing "/" if it does not end
        // in '/' or '\\'.  The Insert*/Remove* functions return 'true' iff
        // the operation was successful.
        int GetNumDirectories() const;
        std::string Get(int i) const;
        bool Insert(std::string const& directory);
        bool Remove(std::string const& directory);
        void RemoveAll();

        // The GetPath function searches the list of directories and returns
        // the fully decorated file name if the file exists and can be opened
        // for reading.
        std::string GetPath(std::string const& name) const;

        // Get the path to the GTE folder.  This is obtained from the
        // GTE_PATH environment variable, and it is required when you want
        // a guaranteed way of finding application data/files in the GTE
        // subtree.  If you call this function and the environment variable
        // does not exist, the returned string is "".  The function calls
        // LogWarning(...) so that any listeners are informed about the
        // problem.
        std::string GetGTEPath() const;

    private:
        // The list of directories for GetPath to search.
        std::vector<std::string> mDirectories;
    };
}
