/***************************************************************************
                         qgslazinfo.h
                         --------------------
    begin                : April 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAZINFO_H
#define QGSLAZINFO_H

#include <fstream>

#include "qgspointcloudattribute.h"
#include "qgscoordinatereferencesystem.h"

#include "lazperf/header.hpp"

#define SIP_NO_FILE

/**
 * \ingroup core
 *
 * \brief Class for extracting information contained in LAZ file such as the public header block
 * and variable length records
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsLazInfo
{
  public:
    struct LazVlr
    {
      QString userId;
      int recordId;
      QByteArray data;
    };

    struct ExtraBytesAttributeDetails
    {
      QString attribute;
      QgsPointCloudAttribute::DataType type;
      int size;
      int offset;
    };

    //! Constructor for an empty laz info parser
    QgsLazInfo();

    //! Returns whether the LAZ header data passed to this class is from a valid LAZ file
    bool isValid() const { return mIsValid; }

    //! Returns an error string detailing what went wrong with reading the LAZ info
    QString error() const { return mError; }

    //! Parses the raw header data loaded from a LAZ file
    void parseRawHeader( char *data, uint64_t length );

    //! Parses the variable length records found in the array \a data of length \a length
    void parseRawVlrEntries( char *data, uint64_t length );

    //! Returns the number of points contained in the LAZ file
    uint64_t pointCount() const
    {
      return mHeader.version.major == 1 && mHeader.version.minor == 4 ? mHeader.point_count_14 : mHeader.point_count;
    }
    //! Returns the scale of the points coordinates
    QgsVector3D scale() const { return mScale; }
    //! Returns the offset of the points coordinates
    QgsVector3D offset() const { return mOffset; }
    //! Returns the pair ( creation_year, creation_day ) extracted from the LAZ file public header block
    QPair<uint16_t, uint16_t> creationYearDay() const { return mCreationYearDay; }
    //! Returns the LAZ specification version of the LAZ file
    QPair<uint8_t, uint8_t> version() const { return mVersion; }
    //! Returns the point format of the point records contained in the LAZ file
    int pointFormat() const { return mPointFormat; }
    //! Returns the project identifier contained in the LAZ file public header block (Optional field)
    QString projectId() const { return mProjectId; }
    //! Returns the system identifier contained in the LAZ file public header block
    QString systemId() const { return mSystemId; }
    //! Returns the identifier of the software used to generate the LAZ file public header block
    QString softwareId() const { return mSoftwareId; }
    //! Returns the minimum coordinate across X, Y and Z axis
    QgsVector3D minCoords() const { return mMinCoords; }
    //! Returns the maximum coordinate across X, Y and Z axis
    QgsVector3D maxCoords() const { return mMaxCoords; }
    //! Returns the absolute offset to the first point record in the LAZ file
    uint32_t firstPointRecordOffset() const { return mHeader.point_offset; }
    //! Returns the absolute offset to the first variable length record in the LAZ file
    uint32_t firstVariableLengthRecord() const;
    //! Returns the length of each point record in bytes
    int pointRecordLength() const { return mHeader.point_record_length; }
    //! Returns the number of extrabytes contained in the LAZ dataset
    int extrabytesCount() const { return mHeader.ebCount(); }

    //! Returns the absolute offset to the first extended point record in the LAZ file
    uint64_t firstEvlrOffset() const { return mHeader.evlr_offset; }
    //! Returns the absolute offset to the first variable length record in the LAZ file
    uint32_t evlrCount() const { return mHeader.evlr_count; }

    //! Returns the coordinate system stored in the LAZ file
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    //! Returns a map containing various metadata extracted from the LAZ file
    QVariantMap toMetadata() const;

    //! Returns the binary data of the variable length record with the user identifier \a userId and record identifier \a recordId
    QByteArray vlrData( QString userId, int recordId );

    //! Returns the list of attributes contained in the LAZ file
    QgsPointCloudAttributeCollection attributes() const { return mAttributes; }

    //! Returns the list of extrabytes contained in the LAZ file
    QVector<ExtraBytesAttributeDetails> extrabytes() const { return mExtrabyteAttributes; }

#ifndef SIP_RUN
    //! Returns the LAZPERF header object
    lazperf::header14 header() const { return mHeader; }
#endif

    //! Static function to parse the raw extrabytes VLR into a list of recognizable extrabyte attributes
    static QVector<ExtraBytesAttributeDetails> parseExtrabytes( char *rawData, int length, int pointRecordLength );

    //! Static function to create a QgsLazInfo class from a file
    static QgsLazInfo fromFile( std::ifstream &file );
    //! Static function to create a QgsLazInfo class from a file over network
    static QgsLazInfo fromUrl( QUrl &url );

    //! Static function to check whether the server of URL \a url supports range queries
    static bool supportsRangeQueries( QUrl &url );

  private:
    void parseHeader( lazperf::header14 &header );
    void parseCrs();
    void parseLazAttributes();
    void parseExtrabyteAttributes();
  private:
    bool mIsValid = false;
    QString mError;

    lazperf::header14 mHeader;

    QgsVector3D mScale, mOffset;
    QPair<uint16_t, uint16_t> mCreationYearDay;
    QPair<uint8_t, uint8_t> mVersion;
    int mPointFormat = -1;
    QString mProjectId;
    QString mSystemId;
    QString mSoftwareId;

    QgsCoordinateReferenceSystem mCrs;

    QgsVector3D mMinCoords, mMaxCoords;

    int mVlrCount = 0;

    QVector<LazVlr> mVlrVector;

    QgsPointCloudAttributeCollection mAttributes;
    QVector<ExtraBytesAttributeDetails> mExtrabyteAttributes;
};

#endif // QGSLAZINFO_H
