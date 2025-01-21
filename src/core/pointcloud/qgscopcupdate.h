/***************************************************************************
    qgscopcupdate.h
    ---------------------
    begin                : January 2025
    copyright            : (C) 2025 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOPCUPDATE_H
#define QGSCOPCUPDATE_H

#include "qgis_core.h"
#include "qgspointcloudindex.h"

#include <lazperf/header.hpp>
#include <lazperf/vlr.hpp>

#define SIP_NO_FILE


/**
 * \ingroup core
 *
 * This class takes an existing COPC file and a list of chunks that should be modified,
 * and outputs an updated COPC file where the modified chunks replace the original chunks.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsCopcUpdate
{
  public:

    //! Keeps information how points of a single chunk has been modified
    struct UpdatedChunk
    {
      //! Number of points in the updated chunk
      int32_t pointCount;
      //! Data of the chunk (compressed already with LAZ compressor)
      QByteArray chunkData;
    };

    //! Reads input COPC file and initializes all the members
    bool read( const QString &inputFilename );

    //! Writes a COPC file with updated chunks
    bool write( const QString &outputFilename, const QHash<QgsPointCloudNodeId, UpdatedChunk> &updatedChunks );

    //! Returns error message
    QString errorMessage() const { return mErrorMessage; }

    /**
     * Convenience function to do the whole process in one go:
     * load a COPC file, then write a new COPC file with updated
     * chunks. Returns TRUE on success. If errorMessage is not
     * a null pointer, it will be set to an error message in case
     * of failure (i.e. FALSE is returned).
     */
    static bool writeUpdatedFile( const QString &inputFilename,
                                  const QString &outputFilename,
                                  const QHash<QgsPointCloudNodeId, UpdatedChunk> &updatedChunks,
                                  QString *errorMessage = nullptr );

  private:
    bool readHeader();
    void readChunkTable();
    void readHierarchy();

  private:
    QString mInputFilename;
    std::ifstream mFile;
    lazperf::header14 mHeader;
    lazperf::copc_info_vlr mCopcVlr;
    std::vector<lazperf::chunk> mChunks;
    uint32_t mChunkCount = 0;
    uint64_t mHierarchyOffset = 0;
    std::vector<char> mHierarchyBlob;
    std::vector<lazperf::evlr_header> mEvlrHeaders;
    std::vector<std::vector<char>> mEvlrData;
    QHash<uint64_t, QgsPointCloudNodeId> mOffsetToVoxel;

    QString mErrorMessage;
};

#endif // QGSCOPCUPDATE_H
