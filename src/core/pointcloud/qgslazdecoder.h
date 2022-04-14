/***************************************************************************
                         qgslazdecoder.h
                         --------------------
    begin                : March 2022
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

#ifndef QGSLAZDECODER_H
#define QGSLAZDECODER_H


#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"

#include "lazperf/lazperf.hpp"
#include "lazperf/readers.hpp"

///@cond PRIVATE
#define SIP_NO_FILE

#include <QString>

class QgsPointCloudExpression;
class QgsLazInfo;

template <typename T>
bool _lazStoreToStream( char *s, size_t position, QgsPointCloudAttribute::DataType type, T value );
bool _lazSerialize( char *data, size_t outputPosition, QgsPointCloudAttribute::DataType outputType,
                    const char *input, QgsPointCloudAttribute::DataType inputType, int inputSize, size_t inputPosition );

class QgsLazDecoder
{
  public:

    enum class LazAttribute
    {
      X,
      Y,
      Z,
      Classification,
      Intensity,
      ReturnNumber,
      NumberOfReturns,
      ScanDirectionFlag,
      EdgeOfFlightLine,
      ScanAngleRank,
      UserData,
      PointSourceId,
      GpsTime,
      Red,
      Green,
      Blue,
      ScannerChannel,
      ClassificationFlags,
      NIR,
      ExtraBytes,
      MissingOrUnknown
    };

    struct RequestedAttributeDetails
    {
      RequestedAttributeDetails( LazAttribute attribute, QgsPointCloudAttribute::DataType type, int size, int offset = -1 )
        : attribute( attribute )
        , type( type )
        , size( size )
        , offset( offset )
      {}

      LazAttribute attribute;
      QgsPointCloudAttribute::DataType type;
      int size;
      int offset; // Used in case the attribute is an extra byte attribute
    };

    static QgsPointCloudBlock *decompressLaz( const QString &filename, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression );
    static QgsPointCloudBlock *decompressLaz( const QByteArray &data, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression );
    static QgsPointCloudBlock *decompressCopc( const QString &filename, QgsLazInfo &lazInfo, uint64_t blockOffset, uint64_t blockSize, int32_t pointCount, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression );
    static QgsPointCloudBlock *decompressCopc( const QByteArray &data, QgsLazInfo &lazInfo, int32_t pointCount, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression );
};

///@endcond
#endif // QGSLAZDECODER_H
