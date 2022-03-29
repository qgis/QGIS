/***************************************************************************
  qgsvtpktiles.cpp
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvtpktiles.h"

#include "qgslogger.h"
#include "qgsrectangle.h"
#include "qgsmessagelog.h"
#include "qgsjsonutils.h"
#include "qgsarcgisrestutils.h"
#include "qgsmbtiles.h"
#include "qgsziputils.h"
#include "qgslayermetadata.h"

#include <QFile>
#include <QImage>
#include <QDomDocument>
#include <QTextDocumentFragment>
#include "zip.h"
#include <iostream>


QgsVtpkTiles::QgsVtpkTiles( const QString &filename )
  : mFilename( filename )
{
}

QgsVtpkTiles::~QgsVtpkTiles()
{
  if ( mZip )
  {
    zip_close( mZip );
    mZip = nullptr;
  }
}

bool QgsVtpkTiles::open()
{
  if ( mZip )
    return true;  // already opened

  const QByteArray fileNamePtr = mFilename.toUtf8();
  int rc = 0;
  mZip = zip_open( fileNamePtr.constData(), ZIP_CHECKCONS, &rc );
  if ( rc == ZIP_ER_OK && mZip )
  {
    const int count = zip_get_num_files( mZip );
    if ( count != -1 )
    {
      return true;
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "Error getting files: '%1'" ).arg( zip_strerror( mZip ) ) );
      zip_close( mZip );
      mZip = nullptr;
      return false;
    }
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error opening zip archive: '%1' (Error code: %2)" ).arg( mZip ? zip_strerror( mZip ) : mFilename ).arg( rc ) );
    if ( mZip )
    {
      zip_close( mZip );
      mZip = nullptr;
    }
    return false;
  }
}

bool QgsVtpkTiles::isOpen() const
{
  return static_cast< bool >( mZip );
}

QVariantMap QgsVtpkTiles::metadata() const
{
  if ( !mMetadata.isEmpty() )
    return mMetadata;

  if ( !mZip )
    return QVariantMap();

  const char *name = "p12/root.json";
  struct zip_stat stat;
  zip_stat_init( &stat );
  zip_stat( mZip, name, 0, &stat );

  const size_t len = stat.size;
  const std::unique_ptr< char[] > buf( new char[len + 1] );

  //Read the compressed file
  zip_file *file = zip_fopen( mZip, name, 0 );
  if ( zip_fread( file, buf.get(), len ) != -1 )
  {
    buf[ len ] = '\0';
    std::string jsonString( buf.get( ) );
    mMetadata = QgsJsonUtils::parseJson( jsonString ).toMap();
    zip_fclose( file );
    file = nullptr;
  }
  else
  {
    zip_fclose( file );
    file = nullptr;
    QgsMessageLog::logMessage( QObject::tr( "Error reading metadata: '%1'" ).arg( zip_strerror( mZip ) ) );
  }
  return mMetadata;
}

QVariantMap QgsVtpkTiles::styleDefinition() const
{
  if ( !mZip )
    return QVariantMap();

  const char *name = "p12/resources/styles/root.json";
  struct zip_stat stat;
  zip_stat_init( &stat );
  zip_stat( mZip, name, 0, &stat );

  const size_t len = stat.size;
  const std::unique_ptr< char[] > buf( new char[len + 1] );

  QVariantMap style;
  //Read the compressed file
  zip_file *file = zip_fopen( mZip, name, 0 );
  if ( zip_fread( file, buf.get(), len ) != -1 )
  {
    buf[ len ] = '\0';
    std::string jsonString( buf.get( ) );
    style = QgsJsonUtils::parseJson( jsonString ).toMap();
    zip_fclose( file );
    file = nullptr;
  }
  else
  {
    zip_fclose( file );
    file = nullptr;
    QgsMessageLog::logMessage( QObject::tr( "Error reading style definition: '%1'" ).arg( zip_strerror( mZip ) ) );
  }
  return style;
}

QVariantMap QgsVtpkTiles::spriteDefinition() const
{
  if ( !mZip )
    return QVariantMap();

  for ( int resolution = 2; resolution > 0; resolution-- )
  {
    const QString spriteFileCandidate = QStringLiteral( "p12/resources/sprites/sprite%1.json" ).arg( resolution > 1 ? QStringLiteral( "@%1x" ).arg( resolution ) : QString() );
    const QByteArray spriteFileCandidateBa = spriteFileCandidate.toLocal8Bit();
    const char *name = spriteFileCandidateBa.constData();
    struct zip_stat stat;
    zip_stat_init( &stat );
    zip_stat( mZip, name, 0, &stat );

    if ( !stat.valid )
      continue;

    const size_t len = stat.size;
    const std::unique_ptr< char[] > buf( new char[len + 1] );

    QVariantMap definition;
    //Read the compressed file
    zip_file *file = zip_fopen( mZip, name, 0 );
    if ( zip_fread( file, buf.get(), len ) != -1 )
    {
      buf[ len ] = '\0';
      std::string jsonString( buf.get( ) );
      definition = QgsJsonUtils::parseJson( jsonString ).toMap();
      zip_fclose( file );
      file = nullptr;
    }
    else
    {
      zip_fclose( file );
      file = nullptr;
      QgsMessageLog::logMessage( QObject::tr( "Error reading sprite definition: '%1'" ).arg( zip_strerror( mZip ) ) );
    }
    return definition;
  }

  return QVariantMap();
}

QImage QgsVtpkTiles::spriteImage() const
{
  if ( !mZip )
    return QImage();

  for ( int resolution = 2; resolution > 0; resolution-- )
  {
    const QString spriteFileCandidate = QStringLiteral( "p12/resources/sprites/sprite%1.png" ).arg( resolution > 1 ? QStringLiteral( "@%1x" ).arg( resolution ) : QString() );
    const QByteArray spriteFileCandidateBa = spriteFileCandidate.toLocal8Bit();
    const char *name = spriteFileCandidateBa.constData();
    struct zip_stat stat;
    zip_stat_init( &stat );
    zip_stat( mZip, name, 0, &stat );

    if ( !stat.valid )
      continue;

    const size_t len = stat.size;
    const std::unique_ptr< char[] > buf( new char[len + 1] );

    QImage result;
    //Read the compressed file
    zip_file *file = zip_fopen( mZip, name, 0 );
    if ( zip_fread( file, buf.get(), len ) != -1 )
    {
      buf[ len ] = '\0';

      result = QImage::fromData( reinterpret_cast<const uchar *>( buf.get() ), len );

      zip_fclose( file );
      file = nullptr;
    }
    else
    {
      zip_fclose( file );
      file = nullptr;
      QgsMessageLog::logMessage( QObject::tr( "Error reading sprite image: '%1'" ).arg( zip_strerror( mZip ) ) );
    }
    return result;
  }

  return QImage();
}

QgsLayerMetadata QgsVtpkTiles::layerMetadata() const
{
  if ( !mZip )
    return QgsLayerMetadata();

  const char *name = "esriinfo/iteminfo.xml";
  struct zip_stat stat;
  zip_stat_init( &stat );
  zip_stat( mZip, name, 0, &stat );

  const size_t len = stat.size;
  QByteArray buf( len, Qt::Uninitialized );

  QgsLayerMetadata metadata;
  //Read the compressed file
  zip_file *file = zip_fopen( mZip, name, 0 );
  if ( zip_fread( file, buf.data(), len ) != -1 )
  {
    zip_fclose( file );
    file = nullptr;

    QDomDocument doc;
    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;
    if ( !doc.setContent( buf, false, &errorMessage, &errorLine, &errorColumn ) )
    {
      QgsMessageLog::logMessage( QObject::tr( "Error reading layer metadata (line %1, col %2): %3" ).arg( errorLine ).arg( errorColumn ).arg( errorMessage ) );
    }
    else
    {
      metadata.setType( QStringLiteral( "dataset" ) );

      const QDomElement infoElement = doc.firstChildElement( QStringLiteral( "ESRI_ItemInformation" ) );

      metadata.setLanguage( infoElement.attribute( QStringLiteral( "Culture" ) ) );

      const QDomElement guidElement = infoElement.firstChildElement( QStringLiteral( "guid" ) );
      metadata.setIdentifier( guidElement.text() );

      const QDomElement nameElement = infoElement.firstChildElement( QStringLiteral( "name" ) );
      metadata.setTitle( nameElement.text() );

      const QDomElement descriptionElement = infoElement.firstChildElement( QStringLiteral( "description" ) );
      metadata.setAbstract( QTextDocumentFragment::fromHtml( descriptionElement.text() ).toPlainText() );

      const QDomElement tagsElement = infoElement.firstChildElement( QStringLiteral( "tags" ) );

      const QStringList rawTags = tagsElement.text().split( ',' );
      QStringList tags;
      tags.reserve( rawTags.size() );
      for ( const QString &tag : rawTags )
        tags.append( tag.trimmed() );
      metadata.addKeywords( QStringLiteral( "keywords" ), tags );

      const QDomElement accessInformationElement = infoElement.firstChildElement( QStringLiteral( "accessinformation" ) );
      metadata.setRights( { accessInformationElement.text() } );

      const QDomElement licenseInfoElement = infoElement.firstChildElement( QStringLiteral( "licenseinfo" ) );
      metadata.setLicenses( { QTextDocumentFragment::fromHtml( licenseInfoElement.text() ).toPlainText() } );

      const QDomElement extentElement = infoElement.firstChildElement( QStringLiteral( "extent" ) );
      const double xMin = extentElement.firstChildElement( QStringLiteral( "xmin" ) ).text().toDouble();
      const double xMax = extentElement.firstChildElement( QStringLiteral( "xmax" ) ).text().toDouble();
      const double yMin = extentElement.firstChildElement( QStringLiteral( "ymin" ) ).text().toDouble();
      const double yMax = extentElement.firstChildElement( QStringLiteral( "ymax" ) ).text().toDouble();

      QgsCoordinateReferenceSystem crs = matrixSet().crs();

      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.bounds = QgsBox3d( QgsRectangle( xMin, yMin, xMax, yMax ) );
      spatialExtent.extentCrs = QgsCoordinateReferenceSystem( "EPSG:4326" );
      QgsLayerMetadata::Extent extent;
      extent.setSpatialExtents( { spatialExtent } );
      metadata.setExtent( extent );
      metadata.setCrs( crs );

      return metadata;
    }
  }
  else
  {
    zip_fclose( file );
    file = nullptr;
    QgsMessageLog::logMessage( QObject::tr( "Error reading layer metadata: '%1'" ).arg( zip_strerror( mZip ) ) );
  }
  return metadata;
}

QgsVectorTileMatrixSet QgsVtpkTiles::matrixSet() const
{
  if ( !mMatrixSet.isEmpty() )
    return mMatrixSet;

  mMatrixSet.fromEsriJson( metadata() );
  return mMatrixSet;
}

QgsCoordinateReferenceSystem QgsVtpkTiles::crs() const
{
  return matrixSet().crs();
}

QgsRectangle QgsVtpkTiles::extent( const QgsCoordinateTransformContext &context ) const
{
  const QVariantMap md = metadata();

  const QVariantMap fullExtent = md.value( QStringLiteral( "fullExtent" ) ).toMap();
  if ( !fullExtent.isEmpty() )
  {
    QgsRectangle fullExtentRect(
      fullExtent.value( QStringLiteral( "xmin" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "ymin" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "xmax" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "ymax" ) ).toDouble()
    );

    const QgsCoordinateReferenceSystem fullExtentCrs = QgsArcGisRestUtils::convertSpatialReference( fullExtent.value( QStringLiteral( "spatialReference" ) ).toMap() );
    const QgsCoordinateTransform extentTransform( fullExtentCrs, crs(), context );
    try
    {
      return extentTransform.transformBoundingBox( fullExtentRect );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform layer fullExtent to layer CRS" ) );
    }
  }

  return QgsRectangle();
}

QByteArray QgsVtpkTiles::tileData( int z, int x, int y )
{
  if ( !mZip )
  {
    QgsDebugMsg( QStringLiteral( "VTPK tile package not open: " ) + mFilename );
    return QByteArray();
  }
  if ( mPacketSize < 0 )
    mPacketSize = metadata().value( QStringLiteral( "resourceInfo" ) ).toMap().value( QStringLiteral( "cacheInfo" ) ).toMap().value( QStringLiteral( "storageInfo" ) ).toMap().value( QStringLiteral( "packetSize" ) ).toInt();

  const int fileRow = mPacketSize * static_cast< int >( std::floor( y / static_cast< double>( mPacketSize ) ) );
  const int fileCol = mPacketSize * static_cast< int >( std::floor( x / static_cast< double>( mPacketSize ) ) );

  const QString tileName = QStringLiteral( "R%1C%2" )
                           .arg( fileRow, 4, 16, QLatin1Char( '0' ) )
                           .arg( fileCol, 4, 16, QLatin1Char( '0' ) );

  const QString fileName = QStringLiteral( "p12/tile/L%1/%2.bundle" )
                           .arg( z, 2, 10, QLatin1Char( '0' ) ).arg( tileName );
  struct zip_stat stat;
  zip_stat_init( &stat );
  zip_stat( mZip, fileName.toLocal8Bit().constData(), 0, &stat );

  const size_t tileIndexOffset = 64 + 8 * ( mPacketSize * ( y % mPacketSize ) + ( x % mPacketSize ) );

  QByteArray res;
  const size_t len = stat.size;
  if ( len <= tileIndexOffset )
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot read gzip contents at offset %1: %2" ).arg( tileIndexOffset ).arg( fileName ) );
  }
  else
  {
    const std::unique_ptr< char[] > buf( new char[len] );

    //Read the compressed file
    zip_file *file = zip_fopen( mZip, fileName.toLocal8Bit().constData(), 0 );
    if ( zip_fread( file, buf.get(), len ) != -1 )
    {
      unsigned long long indexValue;
      memcpy( &indexValue, buf.get() + tileIndexOffset, 8 );

      const std::size_t tileOffset = indexValue % ( 2ULL << 39 );
      const std::size_t tileSize = static_cast< std::size_t>( std::floor( indexValue / ( 2ULL << 39 ) ) );

      // bundle is a gzip file;
      if ( !QgsZipUtils::decodeGzip( buf.get() + tileOffset, tileSize, res ) )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error extracting bundle contents as gzip: %1" ).arg( fileName ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "Error reading tile: '%1'" ).arg( zip_strerror( mZip ) ) );
    }
    zip_fclose( file );
    file = nullptr;
  }

  return res;
}

