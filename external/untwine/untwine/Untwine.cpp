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

void fatal(const std::string& err)
{
    std::cerr << "untwine fatal error: " << err << "\n";
    exit(-1);
}


void addArgs(pdal::ProgramArgs& programArgs, Options& options, pdal::Arg * &tempArg)
{
    programArgs.add("files,i", "Input files/directory", options.inputFiles).setPositional();
    programArgs.add("output_dir,o", "Output directory", options.outputDir).setPositional();
    tempArg = &(programArgs.add("temp_dir", "Temp directory", options.tempDir));
    programArgs.add("cube", "Make a cube, rather than a rectangular solid", options.doCube, true);
    programArgs.add("level", "Set an initial tree leve, rather than guess based on data",
        options.level, -1);
    programArgs.add("file_limit", "Only load 'file_limit' files, even if more exist",
        options.fileLimit, (size_t)10000000);
    programArgs.add("progress_fd", "File descriptor on which to write progress messages.",
        options.progressFd, -1);
    programArgs.add("dims", "Dimensions to load. Note that X, Y and Z are always "
        "loaded.", options.dimNames);
    programArgs.add("stats", "Generate statistics for dimensions in the manner of Entwine.",
        options.stats);
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
        programArgs.add("version", "Report the untwine version.", version);
        programArgs.add("help", "Print some help.", help);

        programArgs.parseSimple(arglist);
        if (version)
            std::cout << "untwine version (" << UNTWINE_VERSION << ")\n";
        if (help)
        {
            std::cout << "Usage: untwine <options>\n";
            programArgs.dump(std::cout, 2, 80);
        }
        if (help || version)
            return false;

        programArgs.parse(arglist);
        if (!tempArg->set())
            options.tempDir = options.outputDir + "/temp";
    }
    catch (const pdal::arg_error& err)
    {
        fatal(err.what());
    }
    return true;
}

void createDirs(const Options& options)
{
    pdal::FileUtils::createDirectory(options.outputDir);
    pdal::FileUtils::createDirectory(options.tempDir);
    pdal::FileUtils::deleteFile(options.outputDir + "/ept.json");
    pdal::FileUtils::deleteDirectory(options.outputDir + "/ept-data");
    pdal::FileUtils::deleteDirectory(options.outputDir + "/ept-hierarchy");
    pdal::FileUtils::createDirectory(options.outputDir + "/ept-data");
    pdal::FileUtils::createDirectory(options.outputDir + "/ept-hierarchy");
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

    Options options;
    if (!handleOptions(arglist, options))
        return 0;
    createDirs(options);

    ProgressWriter progress(options.progressFd);

    try
    {
        BaseInfo common;

        epf::Epf preflight(common);
        preflight.run(options, progress);

        bu::BuPyramid builder(common);
        builder.run(options, progress);
    }
    catch (const char *s)
    {
        std::cerr << "Error: " << s << "\n";
    }

    return 0;
}

