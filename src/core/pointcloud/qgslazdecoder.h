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


#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"

#include <string>
#include <QString>

///@cond PRIVATE
#define SIP_NO_FILE


class QgsPointCloudExpression;
class QgsLazInfo;
class QgsRectangle;

template <typename T>
bool lazStoreToStream_( char *s, size_t position, QgsPointCloudAttribute::DataType type, T value );
bool lazSerialize_( char *data, size_t outputPosition, QgsPointCloudAttribute::DataType outputType,
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

    static QgsPointCloudBlock *decompressLaz( const QString &filename, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect );
    static QgsPointCloudBlock *decompressLaz( const QByteArray &data, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect );
    static QgsPointCloudBlock *decompressCopc( const QByteArray &data, QgsLazInfo &lazInfo, int32_t pointCount, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect );

#if defined(_MSC_VER)

    /**
     * Converts Unicode path to MSVC's wide string (file streams in MSVC c++ library
     * expect paths in the active code page, not UTF-8, but they provide variants
     * with std::wstring to deal with unicode paths)
     */
    static std::wstring toNativePath( const QString &filename );
#else
    //! Converts Unicode path to UTF-8 encoded string
    static std::string toNativePath( const QString &filename );
#endif

};

///@endcond
#endif // QGSLAZDECODER_H
