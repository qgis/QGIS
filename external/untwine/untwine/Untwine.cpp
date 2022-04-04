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

#include "Common.hpp"
#include "Config.hpp"
#include "ProgressWriter.hpp"

#include "../epf/Epf.hpp"
#include "../bu/BuPyramid.hpp"

namespace untwine
{

void addArgs(pdal::ProgramArgs& programArgs, Options& options, pdal::Arg * &tempArg)
{
    programArgs.add("output_dir,o", "Output directory/filename for single-file output",
        options.outputName).setPositional();
    programArgs.add("files,i", "Input files/directory", options.inputFiles).setPositional();
    programArgs.add("single_file,s", "Create a single output file", options.singleFile);
    tempArg = &(programArgs.add("temp_dir", "Temp directory", options.tempDir));
    programArgs.add("preserve_temp_dir", "Remove files from the temp directory",
        options.preserveTempDir);
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
            std::cout << "Usage: untwine [output file/directory] <options>\n";
            programArgs.dump(std::cout, 2, 80);
        }
        if (help || version)
            return false;

        programArgs.parse(arglist);

        if (!tempArg->set())
        {
            if (options.singleFile)
                options.tempDir = options.outputName + "_tmp";
            else
                options.tempDir = options.outputName + "/temp";
        }
        if (options.singleFile)
            options.stats = true;

        //
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

void createDirs(const Options& options)
{
    if (!options.singleFile)
    {
        if (!pdal::FileUtils::createDirectory(options.outputName))
            throw FatalError("Couldn't create output directory: " + options.outputName + "'.");
        pdal::FileUtils::deleteFile(options.outputName + "/ept.json");
        pdal::FileUtils::deleteDirectory(options.outputName + "/ept-data");
        pdal::FileUtils::deleteDirectory(options.outputName + "/ept-hierarchy");
        pdal::FileUtils::createDirectory(options.outputName + "/ept-data");
        pdal::FileUtils::createDirectory(options.outputName + "/ept-hierarchy");
    }

    bool tempExists = pdal::FileUtils::fileExists(options.tempDir);
    if (tempExists && !pdal::FileUtils::isDirectory(options.tempDir))
        throw FatalError("Can't use temp directory - exists as a regular or special file.");
    if (!options.preserveTempDir)
        pdal::FileUtils::deleteDirectory(options.tempDir);
    if (!tempExists && !pdal::FileUtils::createDirectory(options.tempDir))
        throw FatalError("Couldn't create temp directory: '" + options.tempDir + "'.");
}

} // namespace untwine


int main(int argc, char *argv[])
{
    std::vector<std::string> arglist;

    // Skip the program name.
    argv++;
    argc--;
    while (argc--)
        arglist.push_back(*argv++);

    using namespace untwine;

    BaseInfo common;
    Options& options = common.opts;
    ProgressWriter progress;

    try
    {
        if (!handleOptions(arglist, options))
            return 0;
        progress.init(options.progressFd, options.progressDebug);
        createDirs(options);

        epf::Epf preflight(common);
        preflight.run(progress);

        bu::BuPyramid builder(common);
        builder.run(progress);
    }
    catch (const char *s)
    {
        progress.writeErrorMessage(std::string("Error: ") + s + "\n");
        return -1;
    }
    catch (const pdal::pdal_error& err)
    {
        progress.writeErrorMessage(err.what());
        return -1;
    }
    catch (const untwine::FatalError& err)
    {
        progress.writeErrorMessage(err.what());
        return -1;
    }
    catch (const std::exception& ex)
    {
        progress.writeErrorMessage(ex.what());
        return -1;
    }
    catch (...)
    {
        progress.writeErrorMessage("Unknown/unexpected exception.");
        return -1;
    }

    return 0;
}

