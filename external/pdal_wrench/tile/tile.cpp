/*****************************************************************************
 *   Copyright (c) 2023, Lutra Consulting Ltd. and Hobu, Inc.                *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include <iostream>
#include <filesystem>
#include <unordered_set>

#include <pdal/pdal_features.hpp>
#include <pdal/SpatialReference.hpp>
#include <pdal/util/Algorithm.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/io/BufferReader.hpp>

#include "Common.hpp"
#include "TileGrid.hpp"
#include "FileDimInfo.hpp"
#include "EpfTypes.hpp"
#include "Writer.hpp"
#include "FileProcessor.hpp"
#include "Las.hpp"

#include "../utils.hpp"
#include "../vpc.hpp"

namespace fs = std::filesystem;


// PDAL's directoryList had a bug, so we've imported a working
// version here so that we can still use older PDAL releases.

#ifndef __APPLE_CC__
std::vector<std::string> directoryList(const std::string& dir)
{
    namespace fs = std::filesystem;

    std::vector<std::string> files;

    try
    {
        fs::directory_iterator it(untwine::toNative(dir));
        fs::directory_iterator end;
        while (it != end)
        {
#ifndef __MINGW32__
            files.push_back(untwine::fromNative(it->path()));
#else
            files.push_back(untwine::fromNative(it->path().string()));
#endif
            it++;
        }
    }
    catch (fs::filesystem_error&)
    {
        files.clear();
    }
    return files;
}
#else

#include <dirent.h>

// Provide simple opendir/readdir solution for OSX because directory_iterator is
// not available until OSX 10.15
std::vector<std::string> directoryList(const std::string& dir)
{

    DIR *dpdf;
    struct dirent *epdf;

    std::vector<std::string> files;
    dpdf = opendir(dir.c_str());
    if (dpdf != NULL){
       while ((epdf = readdir(dpdf))){
            std::string name = untwine::fromNative(epdf->d_name);
            // Skip paths
            if (pdal::Utils::iequals(name, ".") ||
                pdal::Utils::iequals(name, ".."))
            {
                continue;
            }
            else
            {
                // we expect the path + name
                std::string p = dir + "/" + untwine::fromNative(epdf->d_name);
                files.push_back(p);
           }
       }
    }
    closedir(dpdf);

    return files;

}
#endif


using namespace untwine::epf;
using namespace pdal;

using StringList = std::vector<std::string>;


struct BaseInfo
{
public:

    struct Options
    {
        std::string outputDir;
        StringList inputFiles;
        std::string tempDir;
        bool preserveTempDir;
        double tileLength;     // length of the square tile
        StringList dimNames;
        std::string a_srs;
        bool metadata;

        int max_threads;
        std::string outputFormat;   // las or laz (for now)
        bool buildVpc = false;
        std::string inputFileList; // file list with input files
    } opts;

    pdal::BOX3D trueBounds;
    size_t pointSize;
    untwine::DimInfoList dimInfo;
    pdal::SpatialReference srs;
    int pointFormatId;

    using d3 = std::array<double, 3>;
    d3 scale { -1.0, -1.0, -1.0 };
    d3 offset {};
};



static PointCount createFileInfo(const StringList& input, StringList dimNames,
    std::vector<FileInfo>& fileInfos, BaseInfo &m_b, TileGrid &m_grid, FileInfo &m_srsFileInfo)
{
    using namespace pdal;

    std::vector<FileInfo> tempFileInfos;
    std::vector<std::string> filenames;
    PointCount totalPoints = 0;

    // If there are some dim names specified, make sure they contain X, Y and Z and that
    // they're all uppercase.
    if (!dimNames.empty())
    {
        for (std::string& d : dimNames)
            d = Utils::toupper(d);
        for (const std::string xyz : { "X", "Y", "Z" })
            if (!Utils::contains(dimNames, xyz))
                dimNames.push_back(xyz);
    }

    // If any of the specified input files is a directory, get the names of the files
    // in the directory and add them.
    for (const std::string& filename : input)
    {
        if (FileUtils::isDirectory(filename))
        {
            std::vector<std::string> dirfiles = directoryList(filename);
            filenames.insert(filenames.end(), dirfiles.begin(), dirfiles.end());
        }
        else if (ends_with(filename, ".vpc"))
        {
            VirtualPointCloud vpc;
            if (!vpc.read(filename))
            {
                throw FatalError("Unable to read virtual point cloud: " + filename);
            }
            for (const VirtualPointCloud::File &vpcFile : vpc.files)
            {
                filenames.push_back(vpcFile.filename);
            }
        }
        else
            filenames.push_back(filename);
    }

    std::vector<double> xOffsets;
    std::vector<double> yOffsets;
    std::vector<double> zOffsets;

    // Determine a driver for each file and get a preview of the file.  If we couldn't
    // Create a FileInfo object containing the file bounds, dimensions, filename and
    // associated driver.  Expand our grid by the bounds and file point count.
    for (std::string& filename : filenames)
    {
        StageFactory factory;
        std::string driver = factory.inferReaderDriver(filename);
        if (driver.empty())
            throw FatalError("Can't infer reader for '" + filename + "'.");
        Stage *s = factory.createStage(driver);
        pdal::Options opts;
        opts.add("filename", filename);
        s->setOptions(opts);

        QuickInfo qi = s->preview();

        if (!qi.valid())
            throw FatalError("Couldn't get quick info for '" + filename + "'.");

        // Get scale values from the reader if they exist.
        pdal::MetadataNode root = s->getMetadata();
        pdal::MetadataNode m = root.findChild("scale_x");
        if (m.valid())
            m_b.scale[0] = (std::max)(m_b.scale[0], m.value<double>());
        m = root.findChild("scale_y");
        if (m.valid())
            m_b.scale[1] = (std::max)(m_b.scale[1], m.value<double>());
        m = root.findChild("scale_z");
        if (m.valid())
            m_b.scale[2] = (std::max)(m_b.scale[2], m.value<double>());
        m = root.findChild("offset_x");
        if (m.valid())
            xOffsets.push_back(m.value<double>());
        m = root.findChild("offset_y");
        if (m.valid())
            yOffsets.push_back(m.value<double>());
        m = root.findChild("offset_z");
        if (m.valid())
            zOffsets.push_back(m.value<double>());

        FileInfo fi;
        fi.bounds = qi.m_bounds;
        fi.numPoints = qi.m_pointCount;
        fi.filename = filename;
        fi.driver = driver;

        // Accept dimension names if there are no limits or this name is in the list
        // of desired dimensions.
        for (const std::string& name : qi.m_dimNames)
            if (dimNames.empty() || Utils::contains(dimNames, Utils::toupper(name)))
                fi.dimInfo.push_back(untwine::FileDimInfo(name));

        if (m_srsFileInfo.valid() && m_srsFileInfo.srs != qi.m_srs)
            std::cerr << "Files have mismatched SRS values. Using SRS from '" <<
                m_srsFileInfo.filename << "'.\n";
        fi.srs = qi.m_srs;
        tempFileInfos.push_back(fi);
        if (!m_srsFileInfo.valid() && qi.m_srs.valid())
            m_srsFileInfo = fi;

        m_grid.expand(qi.m_bounds, qi.m_pointCount);
        totalPoints += fi.numPoints;
    }

    // If we had an offset from the input, choose one in the middle of the list of offsets.
    if (xOffsets.size())
    {
        std::sort(xOffsets.begin(), xOffsets.end());
        m_b.offset[0] = xOffsets[xOffsets.size() / 2];
    }
    if (yOffsets.size())
    {
        std::sort(yOffsets.begin(), yOffsets.end());
        m_b.offset[1] = yOffsets[yOffsets.size() / 2];
    }
    if (zOffsets.size())
    {
        std::sort(zOffsets.begin(), zOffsets.end());
        m_b.offset[2] = zOffsets[zOffsets.size() / 2];
    }

    // If we have LAS start capability, break apart file infos into chunks of size 5 million.
#ifdef PDAL_LAS_START
    PointCount ChunkSize = 5'000'000;
    for (const FileInfo& fi : tempFileInfos)
    {
        if (fi.driver != "readers.las" || fi.numPoints < ChunkSize)
        {
            fileInfos.push_back(fi);
            continue;
        }
        PointCount remaining = fi.numPoints;
        pdal::PointId start = 0;
        while (remaining)
        {
            FileInfo lasFi(fi);
            lasFi.numPoints = (std::min)(ChunkSize, remaining);
            lasFi.start = start;
            fileInfos.push_back(lasFi);

            start += ChunkSize;
            remaining -= lasFi.numPoints;
        }
    }
#else
    fileInfos = std::move(tempFileInfos);
#endif

    return totalPoints;
}




static void fillMetadata(const pdal::PointLayoutPtr layout, BaseInfo &m_b, const TileGrid &m_grid, FileInfo &m_srsFileInfo)
{
    using namespace pdal;

    // Info to be passed to sampler.
    //m_b.bounds = m_grid.processingBounds();
    m_b.trueBounds = m_grid.conformingBounds();
    if (m_srsFileInfo.valid())
        m_b.srs = m_srsFileInfo.srs;
    m_b.pointSize = 0;

    // Set the pointFormatId based on whether or not colors exist in the file
    if (layout->hasDim(Dimension::Id::Infrared))
        m_b.pointFormatId = 8;
    else if (layout->hasDim(Dimension::Id::Red) ||
             layout->hasDim(Dimension::Id::Green) ||
             layout->hasDim(Dimension::Id::Blue))
        m_b.pointFormatId = 7;
    else
        m_b.pointFormatId = 6;

    const Dimension::IdList& lasDims = untwine::pdrfDims(m_b.pointFormatId);
    for (Dimension::Id id : layout->dims())
    {
        untwine::FileDimInfo di;
        di.name = layout->dimName(id);
        di.type = layout->dimType(id);
        di.offset = layout->dimOffset(id);
        di.dim = id;
        di.extraDim = !Utils::contains(lasDims, id);
        m_b.pointSize += pdal::Dimension::size(di.type);
        m_b.dimInfo.push_back(di);
    }
    auto calcScale = [](double scale, double low, double high)
    {
        if (scale > 0)
            return scale;

        // 2 billion is a little less than the int limit.  We center the data around 0 with the
        // offset, so we're applying the scale to half the range of the data.
        double val = high / 2 - low / 2;
        double power = std::ceil(std::log10(val / 2000000000.0));

        // Set an arbitrary limit on scale of 1e10-4.
        return std::pow(10, (std::max)(power, -4.0));
    };

    m_b.scale[0] = calcScale(m_b.scale[0], m_b.trueBounds.minx, m_b.trueBounds.maxx);
    m_b.scale[1] = calcScale(m_b.scale[1], m_b.trueBounds.miny, m_b.trueBounds.maxy);
    m_b.scale[2] = calcScale(m_b.scale[2], m_b.trueBounds.minz, m_b.trueBounds.maxz);

    // Find an offset such that (offset - min) / scale is close to an integer. This helps
    // to eliminate warning messages in lasinfo that complain because of being unable
    // to write nominal double values precisely using a 32-bit integer.
    // The hope is also that raw input values are written as the same raw values
    // on output. This may not be possible if the input files have different scaling or
    // incompatible offsets.
    auto calcOffset = [](double minval, double maxval, double scale)
    {
        double interval = maxval - minval;
        double spacings = interval / scale;  // Number of quantized values in our range.
        double halfspacings = spacings / 2;  // Half of that number.
        double offset = (int32_t)halfspacings * scale; // Round to an int value and scale down.
        return minval + offset;              // Add the base (min) value.
    };

    m_b.offset[0] = calcOffset(m_b.trueBounds.minx, m_b.trueBounds.maxx, m_b.scale[0]);
    m_b.offset[1] = calcOffset(m_b.trueBounds.miny, m_b.trueBounds.maxy, m_b.scale[1]);
    m_b.offset[2] = calcOffset(m_b.trueBounds.minz, m_b.trueBounds.maxz, m_b.scale[2]);

//    std::cout << "fill metadata: " << m_b.scale[0] << " " << m_b.scale[1] << " " << m_b.scale[2]
//              << m_b.offset[0] << " " << m_b.offset[1] << " " << m_b.offset[2] << std::endl;
}




static void writeOutputFile(const std::string& filename, pdal::PointViewPtr view, const BaseInfo &m_b)
{
    using namespace pdal;

    StageFactory factory;

    BufferReader r;
    r.addView(view);

    Stage *prev = &r;

    if (view->layout()->hasDim(Dimension::Id::GpsTime))
    {
        Stage *f = factory.createStage("filters.sort");
        pdal::Options fopts;
        fopts.add("dimension", "gpstime");
        f->setOptions(fopts);
        f->setInput(*prev);
        prev = f;
    }

    Stage *w = factory.createStage("writers.las");
    pdal::Options wopts;
    wopts.add("extra_dims", "all");
    wopts.add("software_id", "Entwine 1.0");
    if (m_b.opts.outputFormat == "laz")
      wopts.add("compression", "laszip");
    wopts.add("filename", filename);
    wopts.add("offset_x", m_b.offset[0]);
    wopts.add("offset_y", m_b.offset[1]);
    wopts.add("offset_z", m_b.offset[2]);
    wopts.add("scale_x", m_b.scale[0]);
    wopts.add("scale_y", m_b.scale[1]);
    wopts.add("scale_z", m_b.scale[2]);
    wopts.add("minor_version", 4);
    wopts.add("dataformat_id", m_b.pointFormatId);
    if (m_b.opts.a_srs.size())
      wopts.add("a_srs", m_b.opts.a_srs);
    if (m_b.opts.metadata)
        wopts.add("pdal_metadata", m_b.opts.metadata);
    w->setOptions(wopts);
    w->setInput(*prev);

    w->prepare(view->table());
    w->execute(view->table());
}


#include <pdal/util/ProgramArgs.hpp>


void addArgs(pdal::ProgramArgs& programArgs, BaseInfo::Options& options, pdal::Arg * &tempArg, pdal::Arg * &threadsArg)
{
    programArgs.add("output,o", "Output directory/filename", options.outputDir);
    programArgs.add("files,i", "Input files/directory", options.inputFiles).setPositional();
    programArgs.add("input-file-list", "Read input files from a text file", options.inputFileList);
    programArgs.add("length,l", "Tile length", options.tileLength, 1000.);
    tempArg = &(programArgs.add("temp_dir", "Temp directory", options.tempDir));
    programArgs.add("output-format", "Output format (las/laz)", options.outputFormat);

    // for debugging
    programArgs.add("preserve_temp_dir", "Do not remove the temp directory before and after processing (for debugging)",
        options.preserveTempDir);

    programArgs.add("dims", "Dimensions to load. Note that X, Y and Z are always "
        "loaded.", options.dimNames);
    programArgs.add("a_srs", "Assign output SRS",
        options.a_srs, "");
    programArgs.add("metadata", "Write PDAL metadata to VLR output",
        options.metadata, false);

    threadsArg = &(programArgs.add("threads", "Max number of concurrent threads for parallel runs", options.max_threads));
}

bool handleOptions(pdal::StringList& arglist, BaseInfo::Options& options)
{
    pdal::ProgramArgs programArgs;
    pdal::Arg *tempArg;
    pdal::Arg *threadsArg;

    addArgs(programArgs, options, tempArg, threadsArg);
    try
    {
        programArgs.parseSimple(arglist);
    }
    catch (const pdal::arg_error& err)
    {
        throw FatalError(err.what());
    }

    if (ends_with(options.outputDir, ".vpc"))
    {
        options.outputDir = options.outputDir.substr(0, options.outputDir.size()-4);
        options.buildVpc = true;
    }

    if (!tempArg->set())
    {
        options.tempDir = options.outputDir + "/temp";
    }
    if (!threadsArg->set())
    {
        // use number of cores if not specified by the user
        options.max_threads = std::thread::hardware_concurrency();
        if (options.max_threads == 0)
        {
            // in case the value can't be detected, use something reasonable...
            options.max_threads = 4;
        }
    }
    if (options.outputFormat.empty())
    {
        options.outputFormat = "las";  // uncompressed by default
    }
    else
    {
        if (options.outputFormat != "las" && options.outputFormat != "laz")
            throw FatalError("Unknown output format: " + options.outputFormat);
    }

    if (!options.inputFileList.empty())
    {
        std::ifstream inputFile(options.inputFileList);
        std::string line;
        if(!inputFile)
        {
            throw FatalError("Failed to open input file list: " + options.inputFileList);
        }

        while (std::getline(inputFile, line))
        {
            options.inputFiles.push_back(line);
        }
    }


    return true;
}


void createDirs(const BaseInfo::Options& options)
{
    if ( !pdal::FileUtils::isDirectory(options.outputDir) )
    {
        if (!pdal::FileUtils::createDirectory(options.outputDir))
            throw FatalError("Couldn't create output directory: " + options.outputDir + "'.");
    }

    if (pdal::FileUtils::fileExists(options.tempDir) && !pdal::FileUtils::isDirectory(options.tempDir))
        throw FatalError("Can't use temp directory - exists as a regular or special file.");
    if (!options.preserveTempDir)
        pdal::FileUtils::deleteDirectory(options.tempDir);
    if (!pdal::FileUtils::fileExists(options.tempDir) && !pdal::FileUtils::createDirectory(options.tempDir))
        throw FatalError("Couldn't create temp directory: '" + options.tempDir + "'.");
}



static void tilingPass1(BaseInfo &m_b, TileGrid &m_grid, FileInfo &m_srsFileInfo)
{
  //---------
  // pass 1: read input files and write temporary files with raw point data

  std::unique_ptr<untwine::epf::Writer> m_writer;
  untwine::ThreadPool m_pool(m_b.opts.max_threads);

  // Create the file infos. As each info is created, the N x N x N grid is expanded to
  // hold all the points. If the number of points seems too large, N is expanded to N + 1.
  // The correct N is often wrong, especially for some areas where things are more dense.
  std::vector<untwine::epf::FileInfo> fileInfos;
  point_count_t totalPoints = createFileInfo(m_b.opts.inputFiles, m_b.opts.dimNames, fileInfos, m_b, m_grid, m_srsFileInfo);

  // Stick all the dimension names from each input file in a set.
  std::unordered_set<std::string> allDimNames;
  for (const FileInfo& fi : fileInfos)
      for (const untwine::FileDimInfo& fdi : fi.dimInfo)
          allDimNames.insert(fdi.name);

  // Register the dimensions, either as the default type or double if we don't know
  // what it is.
  PointLayoutPtr layout(new PointLayout());
  for (const std::string& dimName : allDimNames)
  {
      Dimension::Type type;
      try
      {
          type = Dimension::defaultType(Dimension::id(dimName));
      }
      catch (pdal::pdal_error&)
      {
          type = Dimension::Type::Double;
      }
      layout->registerOrAssignDim(dimName, type);
  }
  layout->finalize();

  // Fill in dim info now that the layout is finalized.
  for (FileInfo& fi : fileInfos)
  {
      for (untwine::FileDimInfo& di : fi.dimInfo)
      {
          di.dim = layout->findDim(di.name);
          di.type = layout->dimType(di.dim);
          di.offset = layout->dimOffset(di.dim);
      }
  }

  fillMetadata( layout, m_b, m_grid, m_srsFileInfo );

  // Check if we have enough disk space at all
  size_t bytesNeeded = totalPoints * m_b.pointSize;
  std::filesystem::space_info space_info = std::filesystem::space(m_b.opts.tempDir);
  std::cout << "Total points:     " << totalPoints/1000000. << " M" << std::endl;
  std::cout << "Space needed:     " << bytesNeeded/1000./1000./1000. << " GB" << std::endl;
  std::cout << "Space available:  " << space_info.available/1000./1000./1000. << " GB" << std::endl;
  if (space_info.available < bytesNeeded)
    throw FatalError("Not enough space available for processing");

  // TODO: ideally we should check also for space in the output directory (but that's harder to estimate)

  // Make a writer with NumWriters threads.
  m_writer.reset(new untwine::epf::Writer(m_b.opts.tempDir, NumWriters, layout->pointSize()));

  // Sort file infos so the largest files come first. This helps to make sure we don't delay
  // processing big files that take the longest (use threads more efficiently).
  std::sort(fileInfos.begin(), fileInfos.end(), [](const FileInfo& f1, const FileInfo& f2)
      { return f1.numPoints > f2.numPoints; });

  ProgressBar progressBar;
  progressBar.init(totalPoints);

  // Add the files to the processing pool
  m_pool.trap(true, "Unknown error in FileProcessor");
  for (const FileInfo& fi : fileInfos)
  {
      int pointSize = layout->pointSize();
      m_pool.add([&fi, &progressBar, pointSize, &m_grid, &m_writer]()
      {
          untwine::epf::FileProcessor fp(fi, pointSize, m_grid, m_writer.get(), progressBar);
          fp.run();
      });
  }

  // Wait for  all the processors to finish and restart.
  m_pool.join();
  // Tell the writer that it can exit. stop() will block until the writer threads
  // are finished.  stop() will throw if an error occurred during writing.
  m_writer->stop();

  progressBar.done();

  // If the FileProcessors had an error, throw.
  std::vector<std::string> errors = m_pool.clearErrors();
  if (errors.size())
      throw FatalError(errors.front());
}


static void tilingPass2(BaseInfo &m_b, TileGrid &m_grid, FileInfo &m_srsFileInfo)
{
  (void)m_grid;
  (void)m_srsFileInfo;

  //---------
  // pass 2: write the final LAS/LAZ tiles

  untwine::ThreadPool m_pool2(m_b.opts.max_threads);
  m_pool2.trap(true);

  std::vector<std::string> lstBinFiles = directoryList(m_b.opts.tempDir);
  std::vector<std::string> outFiles;

  ProgressBar progressBar;
  progressBar.init(lstBinFiles.size());

  for ( const std::string &binFile : lstBinFiles )
  {
      std::string fileStem = fs::path(binFile).stem().string();
      std::string outFilename = m_b.opts.outputDir + "/" + fileStem + "." + m_b.opts.outputFormat;
      outFiles.push_back(outFilename);

      m_pool2.add([binFile, outFilename, &m_b, &progressBar]()
      {
          PointTable table;

          //ABELL - fixme
          // For now we copy the dimension list so we're sure that it matches the layout, though
          // there's no reason why it should change. We should modify things to use a single
          // layout.

          Dimension::IdList lasDims = untwine::pdrfDims(m_b.pointFormatId);
          untwine::DimInfoList dims = m_b.dimInfo;
          for (untwine::FileDimInfo& fdi : dims)
          {
              fdi.dim = table.layout()->registerOrAssignDim(fdi.name, fdi.type);
          }
          table.finalize();

          PointViewPtr view(new pdal::PointView(table));

          std::size_t ptCnt = 0;
          std::ifstream fileReader(binFile,std::ios::binary|std::ios::ate);
          if (!fileReader)
          {
            throw FatalError("Unable to open temporary file: " + binFile);
          }
          auto fileSize = fileReader.tellg();
          fileReader.seekg(std::ios::beg);

          ptCnt = fileSize / m_b.pointSize;

          const size_t readChunkSize = 10'000;
          std::string content(readChunkSize*m_b.pointSize, 0);
          char *contentPtr = content.data();

          pdal::PointId pointId = view->size();
          for (size_t i = 0; i < ptCnt; i+= readChunkSize)
          {
              size_t nPointsToRead = (std::min)(readChunkSize, ptCnt-i);
              fileReader.read(contentPtr, nPointsToRead*m_b.pointSize);
              for (std::size_t a = 0; a < nPointsToRead; ++a)
              {
                  char *base = contentPtr + a * m_b.pointSize;
                  for (const untwine::FileDimInfo& fdi : dims)
                      view->setField(fdi.dim, fdi.type, pointId,
                          reinterpret_cast<void *>(base + fdi.offset));
                  pointId++;
              }
          }

          writeOutputFile( outFilename, view, m_b);

          progressBar.add();
      });
  }

  m_pool2.join();

  std::vector<std::string> errors = m_pool2.clearErrors();
  if (errors.size())
      throw FatalError(errors.front());

  progressBar.done();

  if (m_b.opts.buildVpc)
  {
      std::vector<std::string> args;
      args.push_back("--output=" + m_b.opts.outputDir + ".vpc");
      args.insert(args.end(), outFiles.begin(), outFiles.end());
      buildVpc(args);
  }
}


int runTile(std::vector<std::string> arglist)
{
  // originally member vars
  untwine::epf::TileGrid m_grid;
  BaseInfo m_b;
  FileInfo m_srsFileInfo;

  try
  {
      // parse arguments
      if (!handleOptions(arglist, m_b.opts))
          return 0;

      std::cout << "max threads " << m_b.opts.max_threads << std::endl;

      m_grid.setTileLength(m_b.opts.tileLength);

      createDirs(m_b.opts);

      auto start = std::chrono::high_resolution_clock::now();

      tilingPass1(m_b, m_grid, m_srsFileInfo);
      tilingPass2(m_b, m_grid, m_srsFileInfo);

      // clean up temp files
      if (!m_b.opts.preserveTempDir)
        pdal::FileUtils::deleteDirectory(m_b.opts.tempDir);

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      std::cout << "time " << duration.count()/1000. << " s" << std::endl;
  }
  catch (const FatalError& err)
  {
      std::cerr << "FATAL ERROR: " << err.what() << std::endl;
      return -1;
  }
  catch (const std::exception& ex)
  {
      std::cerr << "Exception: " << ex.what() << std::endl;
      return -1;
  }
  catch (...)
  {
      std::cerr << "Unknown/unexpected exception." << std::endl;
      return -1;
  }

  return 0;
}
