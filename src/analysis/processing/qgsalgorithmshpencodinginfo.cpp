/***************************************************************************
                         qgsalgorithmshpencodinginfo.cpp
                         -----------------------------
    begin                : February 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmshpencodinginfo.h"

#include "qgsogrutils.h"

///@cond PRIVATE

QString QgsShapefileEncodingInfoAlgorithm::name() const
{
  return u"shpencodinginfo"_s;
}

QString QgsShapefileEncodingInfoAlgorithm::displayName() const
{
  return QObject::tr( "Extract Shapefile encoding" );
}

QStringList QgsShapefileEncodingInfoAlgorithm::tags() const
{
  return QObject::tr( "shp,codepage,cpg,ldid,information,list,show" ).split( ',' );
}

QString QgsShapefileEncodingInfoAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsShapefileEncodingInfoAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsShapefileEncodingInfoAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the attribute encoding information embedded in a Shapefile.\n\n"
                      "Both the encoding specified by an optional .cpg file and any encoding details present in the "
                      ".dbf LDID header block are considered." );
}

QString QgsShapefileEncodingInfoAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts the attribute encoding information embedded in a Shapefile." );
}

QgsShapefileEncodingInfoAlgorithm *QgsShapefileEncodingInfoAlgorithm::createInstance() const
{
  return new QgsShapefileEncodingInfoAlgorithm();
}

void QgsShapefileEncodingInfoAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( u"INPUT"_s, QObject::tr( "Input layer" ), Qgis::ProcessingFileParameterBehavior::File, QString(), QVariant(), false, QObject::tr( "Shapefiles (%1)" ).arg( "*.shp *.SHP"_L1 ) ) );

  addOutput( new QgsProcessingOutputString( u"ENCODING"_s, QObject::tr( "Shapefile Encoding" ) ) );
  addOutput( new QgsProcessingOutputString( u"CPG_ENCODING"_s, QObject::tr( "CPG Encoding" ) ) );
  addOutput( new QgsProcessingOutputString( u"LDID_ENCODING"_s, QObject::tr( "LDID Encoding" ) ) );
}

bool QgsShapefileEncodingInfoAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString path = parameterAsFile( parameters, u"INPUT"_s, context );

  mCpgEncoding = QgsOgrUtils::readShapefileEncodingFromCpg( path );
  if ( mCpgEncoding.isEmpty() )
    feedback->pushInfo( QObject::tr( "No encoding information present in CPG file" ) );
  else
    feedback->pushInfo( QObject::tr( "Detected encoding from CPG file: %1" ).arg( mCpgEncoding ) );

  mLdidEncoding = QgsOgrUtils::readShapefileEncodingFromLdid( path );
  if ( mLdidEncoding.isEmpty() )
    feedback->pushInfo( QObject::tr( "No encoding information present in DBF LDID header" ) );
  else
    feedback->pushInfo( QObject::tr( "Detected encoding from DBF LDID header: %1" ).arg( mLdidEncoding ) );

  return true;
}


QVariantMap QgsShapefileEncodingInfoAlgorithm::processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( u"ENCODING"_s, mCpgEncoding.isEmpty() ? mLdidEncoding : mCpgEncoding );
  outputs.insert( u"CPG_ENCODING"_s, mCpgEncoding );
  outputs.insert( u"LDID_ENCODING"_s, mLdidEncoding );
  return outputs;
}

///@endcond
