// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ProgramDefines.h>
#include <Graphics/VisualProgram.h>
#include <Graphics/ComputeProgram.h>
#include <fstream>
#include <stack>

namespace gte
{
    class ProgramFactory
    {
    public:
        // Abstract base class.
        virtual ~ProgramFactory() = default;
        ProgramFactory();

        // Disallow copy and assignment.
        ProgramFactory(ProgramFactory const&) = delete;
        ProgramFactory& operator=(ProgramFactory const&) = delete;

        // All members are in public scope, because there are no side effects
        // when the values are modified.  The current values are used in the
        // the Create(...) functions.
        std::string version;
        std::string vsEntry, psEntry, gsEntry, csEntry;
        ProgramDefines defines;
        unsigned int flags;

        // The returned value is used as a lookup index into arrays of strings
        // corresponding to shader programs.  Currently, GLSLProgramFactory
        // returns PF_GLSL and HLSLProgramFactory returns PF_HLSL.
        enum
        {
            PF_GLSL,
            PF_HLSL,
            PF_NUM_API
        };

        virtual int GetAPI() const = 0;

        // Create a program for GPU display.  The files are loaded, converted
        // to strings, and passed to CreateFromNamedSources.  The filenames
        // are passed as the 'xsName' parameters in case the shader compiler
        // needs this for #include path searches.
        std::shared_ptr<VisualProgram> CreateFromFiles(std::string const& vsFile,
            std::string const& psFile, std::string const& gsFile);

        std::shared_ptr<VisualProgram> CreateFromSources(
            std::string const& vsSource, std::string const& psSource,
            std::string const& gsSource);

        // Create a program for GPU computing.  The file is loaded, converted
        // to a string, and passed to CreateFromNamedSource.  The filename is
        // passed the 'csName' parameters in case the shader compiler needs
        // this for #include path searches.
        std::shared_ptr<ComputeProgram> CreateFromFile(std::string const& csFile);

        std::shared_ptr<ComputeProgram> CreateFromSource(std::string const& csSource);

        // In public scope in case an effect needs a source-code string from
        // a file; for example, OverlayEffect can use this.
        static std::string GetStringFromFile(std::string const& filename);

        // Support for passing ProgramFactory objects to a function that
        // potentially modifies 'defines' or 'flags' but then needs to
        // restore the previous state on return.  The PushDefines() function
        // saves the current 'defines' on a stack and then clears 'defines'.
        // The PushFlags() function saves the current 'flags' on a stack and
        // then sets 'flags' to zero.  If you need to modify subelements of
        // either member, you will have to manage that on your own.
        void PushDefines();
        void PopDefines();
        void PushFlags();
        void PopFlags();

    protected:
        virtual std::shared_ptr<VisualProgram> CreateFromNamedSources(
            std::string const& vsName, std::string const& vsSource,
            std::string const& psName, std::string const& psSource,
            std::string const& gsName, std::string const& gsSource) = 0;

        virtual std::shared_ptr<ComputeProgram> CreateFromNamedSource(
            std::string const& csName, std::string const& csSource) = 0;

    private:
        std::stack<ProgramDefines> mDefinesStack;
        std::stack<unsigned int> mFlagsStack;
    };

    typedef std::array<std::string const*, ProgramFactory::PF_NUM_API> ProgramSources;
}
