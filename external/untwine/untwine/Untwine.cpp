/*****************************************************************************
 *   Copyright (c) 2020, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include <pdal/pdal_types.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include <regex>

#include "Common.hpp"
#include "Config.hpp"
#include "ProgressWriter.hpp"

#include "../epf/Epf.hpp"
#include "../bu/BuPyramid.hpp"

#include <dirlist.hpp>    // untwine/os
#include <stringconv.hpp> // untwine/os

namespace untwine
{

void addArgs(pdal::ProgramArgs& programArgs, Options& options, pdal::Arg * &tempArg)
{
    programArgs.add("output_dir,o", "Output filename", options.outputName).setPositional();
    programArgs.addSynonym("output_dir", "output_file");
    programArgs.add("files,i", "Input files/directory", options.inputFiles).setPositional();
    programArgs.add("single_file,s", "Deprecated and ingored.", options.dummy);
    tempArg = &(programArgs.add("temp_dir", "Temp directory", options.tempDir));
    programArgs.add("cube", "Make a cube, rather than a rectangular solid", options.doCube, true);
    programArgs.add("level", "Set an initial tree level, rather than guess based on data",
        options.level, -1);
    programArgs.add("file_limit", "Only load 'file_limit' files, even if more exist",
        options.fileLimit, (size_t)10000000);
    programArgs.add("progress_fd", "File descriptor on which to write progress messages.",
        options.progressFd, -1);
    programArgs.add("progress_debug", "Send progress info to stdout.", options.progressDebug);
    programArgs.add("dims", "Dimensions to load. Note that X, Y and Z are always "
        "loaded.", options.dimNames);
    programArgs.add("stats", "Generate statistics for dimensions in the manner of Entwine.",
        options.stats);
    programArgs.add("a_srs", "Assign output SRS",
        options.a_srs, "");
    programArgs.add("metadata", "Write PDAL metadata to VLR output",
        options.metadata, false);
    programArgs.add("no_srs", "PDAL readers.las.nosrs passthrough.",
        options.no_srs, false);
}

bool handleOptions(pdal::StringList& arglist, Options& options)
{
    pdal::ProgramArgs programArgs;
    pdal::Arg *tempArg;

    addArgs(programArgs, options, tempArg);
    try
    {
        bool version;
        bool help;
        pdal::ProgramArgs hargs;
        hargs.add("version", "Report the untwine version.", version);
        hargs.add("help", "Print some help.", help);

        hargs.parseSimple(arglist);
        if (version)
            std::cout << "untwine version (" << UNTWINE_VERSION << ")\n";
        if (help)
        {
            std::cout << "Usage: untwine output file <options>\n";
            programArgs.dump(std::cout, 2, 80);
        }
        if (help || version)
            return false;

        programArgs.parse(arglist);

        // Make sure the output file can be opened so that we can provide an early error if
        // there's a problem.
        std::ofstream tmp(os::toNative(options.outputName), std::ios::out | std::ios::binary);
        if (!tmp)
            throw FatalError("Can't open file '" + options.outputName + "' for output");
        tmp.close();
        pdal::FileUtils::deleteFile(options.outputName);

        if (!tempArg->set())
        {
            options.tempDir = options.outputName + "_tmp";
        }
        options.stats = true;

        if (options.progressFd == 1 && options.progressDebug)
        {
            std::cerr << "'--progress_fd' set to 1. Disabling '--progressDebug'.\n";
            options.progressDebug = false;
        }
    }
    catch (const pdal::arg_error& err)
    {
        throw FatalError(err.what());
    }
    return true;
}

bool createDirs(const Options& options)
{
    bool tempExists = pdal::FileUtils::fileExists(options.tempDir);
    if (tempExists && !pdal::FileUtils::isDirectory(options.tempDir))
        throw FatalError("Can't use temp directory - exists as a regular or special file.");
    if (!tempExists && !pdal::FileUtils::createDirectory(options.tempDir))
        throw FatalError("Couldn't create temp directory: '" + options.tempDir + "'.");
    return tempExists;
}

void cleanup(const std::string& dir, bool rmdir)
{
    std::regex re("[0-9]+-[0-9]+-[0-9]+-[0-9]+.bin");
    std::smatch sm;

    const std::vector<std::string>& files = os::directoryList(dir);
    for (const std::string& f : files)
        if (std::regex_match(f, sm, re))
            pdal::FileUtils::deleteFile(dir + "/" + f);
    if (rmdir)
        pdal::FileUtils::deleteDirectory(dir);
}

} // namespace untwine

#ifdef _MSC_VER // MSVC Compiler
int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] )
#else
int main(int argc, char *argv[])
#endif
{
    std::vector<std::string> arglist;

    // Skip the program name.
    argv++;
    argc--;
    while (argc--)
        arglist.push_back(untwine::os::fromNative(*argv++));

    using namespace untwine;

    BaseInfo common;
    Options& options = common.opts;
    ProgressWriter progress;
    bool tempDirExists = false;
    int status = 0;

    try
    {
        if (!handleOptions(arglist, options))
            return 0;
        progress.init(options.progressFd, options.progressDebug);
        tempDirExists = createDirs(options);

        epf::Epf preflight(common);
        preflight.run(progress);

        bu::BuPyramid builder(common);
        builder.run(progress);
    }
    catch (const char *s)
    {
        progress.writeErrorMessage(std::string("Error: ") + s + "\n");
        status = -1;
    }
    catch (const pdal::pdal_error& err)
    {
        progress.writeErrorMessage(err.what());
        status = -1;
    }
    catch (const untwine::FatalError& err)
    {
        progress.writeErrorMessage(err.what());
        status = -1;
    }
    catch (const std::exception& ex)
    {
        progress.writeErrorMessage(ex.what());
        status = -1;
    }
    catch (...)
    {
        progress.writeErrorMessage("Unknown/unexpected exception.");
        status = -1;
    }

    cleanup(common.opts.tempDir, !tempDirExists);

    return status;
}

