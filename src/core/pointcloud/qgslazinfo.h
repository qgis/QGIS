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

#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudindex.h"

#include "lazperf/lazperf.hpp"
#include "lazperf/readers.hpp"

#define SIP_NO_FILE

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

    //! Constructor for reading informations from a local COPC file
    QgsLazInfo( std::ifstream &file );

    //! Constructor for reading informations from a remote COPC file
    QgsLazInfo( QUrl &url );

    uint64_t pointCount() const { return mPointCount; }
    QgsVector3D scale() const { return mScale; }
    QgsVector3D offset() const { return mOffset; }
    QPair<uint16_t, uint16_t> creationYearDay() const { return mCreationYearDay; }
    QPair<uint8_t, uint8_t> version() const { return mVersion; }
    int pointFormat() const { return mPointFormat; }
    QString projectId() const { return mProjectId; }
    QString systemId() const { return mSystemId; }
    QString softwareId() const { return mSoftwareId; }
    QgsVector3D minCoords() const { return mMinCoords; }
    QgsVector3D maxCoords() const { return mMaxCoords; }

    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    QVariantMap toMetadata() const;

    QByteArray vlrData( QString userId, int recordId );

    lazperf::header14 lazHeader() const { return mHeader; }

    QVector<ExtraBytesAttributeDetails> extrabytes();

    QgsPointCloudAttributeCollection attributes() const { return mAttributes; }

    static QVector<ExtraBytesAttributeDetails> parseExtrabytes( char *rawData, int length, int pointRecordLength );
  private:
    void parseHeader( lazperf::header14 &header );
    void parseVlrData( char *vlrData, uint32_t size );
    void parseCrs();
    void parseAttributes();
  private:
    bool mIsValid = false;

    lazperf::header14 mHeader;

    uint64_t mPointCount = 0;
    QgsVector3D mScale, mOffset;
    QPair<uint16_t, uint16_t> mCreationYearDay;
    QPair<uint8_t, uint8_t> mVersion;
    int mPointFormat;
    QString mProjectId;
    QString mSystemId;
    QString mSoftwareId;

    QgsCoordinateReferenceSystem mCrs;

    QgsVector3D mMinCoords, mMaxCoords;

    int mVlrCount;
    uint32_t mPointRecordsOffset;
    int mPointRecordLength;

    QVector<LazVlr> mVlrVector;

    QgsPointCloudAttributeCollection mAttributes;
};

#endif // QGSLAZINFO_H
