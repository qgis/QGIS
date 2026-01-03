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

#include <iostream>
#include <zip.h>

#include "qgsarcgisrestutils.h"
#include "qgsjsonutils.h"
#include "qgslayermetadata.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgsziputils.h"

#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QTextDocumentFragment>

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
    const int count = zip_get_num_entries( mZip, ZIP_FL_UNCHANGED );
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
    if ( file )
      zip_fclose( file );
    file = nullptr;
    QgsMessageLog::logMessage( QObject::tr( "Error reading metadata: '%1'" ).arg( zip_strerror( mZip ) ) );
  }

  mTileMapPath = mMetadata.value( u"tileMap"_s ).toString();

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
    if ( file )
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
    const QString spriteFileCandidate = u"p12/resources/sprites/sprite%1.json"_s.arg( resolution > 1 ? u"@%1x"_s.arg( resolution ) : QString() );
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
      if ( file )
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
    const QString spriteFileCandidate = u"p12/resources/sprites/sprite%1.png"_s.arg( resolution > 1 ? u"@%1x"_s.arg( resolution ) : QString() );
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
      if ( file )
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
      metadata.setType( u"dataset"_s );

      const QDomElement infoElement = doc.firstChildElement( u"ESRI_ItemInformation"_s );

      metadata.setLanguage( infoElement.attribute( u"Culture"_s ) );

      const QDomElement guidElement = infoElement.firstChildElement( u"guid"_s );
      metadata.setIdentifier( guidElement.text() );

      const QDomElement nameElement = infoElement.firstChildElement( u"name"_s );
      metadata.setTitle( nameElement.text() );

      const QDomElement descriptionElement = infoElement.firstChildElement( u"description"_s );
      metadata.setAbstract( QTextDocumentFragment::fromHtml( descriptionElement.text() ).toPlainText() );

      const QDomElement tagsElement = infoElement.firstChildElement( u"tags"_s );

      const QStringList rawTags = tagsElement.text().split( ',' );
      QStringList tags;
      tags.reserve( rawTags.size() );
      for ( const QString &tag : rawTags )
        tags.append( tag.trimmed() );
      metadata.addKeywords( u"keywords"_s, tags );

      const QDomElement accessInformationElement = infoElement.firstChildElement( u"accessinformation"_s );
      metadata.setRights( { accessInformationElement.text() } );

      const QDomElement licenseInfoElement = infoElement.firstChildElement( u"licenseinfo"_s );
      metadata.setLicenses( { QTextDocumentFragment::fromHtml( licenseInfoElement.text() ).toPlainText() } );

      const QDomElement extentElement = infoElement.firstChildElement( u"extent"_s );
      const double xMin = extentElement.firstChildElement( u"xmin"_s ).text().toDouble();
      const double xMax = extentElement.firstChildElement( u"xmax"_s ).text().toDouble();
      const double yMin = extentElement.firstChildElement( u"ymin"_s ).text().toDouble();
      const double yMax = extentElement.firstChildElement( u"ymax"_s ).text().toDouble();

      QgsCoordinateReferenceSystem crs = matrixSet().crs();

      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.bounds = QgsBox3D( QgsRectangle( xMin, yMin, xMax, yMax ) );
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
    if ( file )
      zip_fclose( file );
    file = nullptr;
    QgsMessageLog::logMessage( QObject::tr( "Error reading layer metadata: '%1'" ).arg( zip_strerror( mZip ) ) );
  }
  return metadata;
}

QVariantMap QgsVtpkTiles::rootTileMap() const
{
  // make sure metadata has been read already
  ( void )metadata();

  if ( mHasReadTileMap || mTileMapPath.isEmpty() )
    return mRootTileMap;

  if ( !mZip )
    return QVariantMap();

  const QString tileMapPath = u"p12/%1/root.json"_s.arg( mTileMapPath );
  struct zip_stat stat;
  zip_stat_init( &stat );
  zip_stat( mZip, tileMapPath.toLocal8Bit().constData(), 0, &stat );

  const size_t len = stat.size;
  const std::unique_ptr< char[] > buf( new char[len + 1] );

  //Read the compressed file
  zip_file *file = zip_fopen( mZip, tileMapPath.toLocal8Bit().constData(), 0 );
  if ( !file )
  {
    QgsDebugError( u"Tilemap %1 was not found in vtpk archive"_s.arg( tileMapPath ) );
    mTileMapPath.clear();
    return mRootTileMap;
  }

  if ( zip_fread( file, buf.get(), len ) != -1 )
  {
    buf[ len ] = '\0';
    std::string jsonString( buf.get( ) );
    mRootTileMap = QgsJsonUtils::parseJson( jsonString ).toMap();
    zip_fclose( file );
    file = nullptr;
  }
  else
  {
    if ( file )
      zip_fclose( file );
    file = nullptr;
    QgsDebugError( u"Tilemap %1 could not be read from vtpk archive"_s.arg( tileMapPath ) );
    mTileMapPath.clear();
  }
  mHasReadTileMap = true;
  return mRootTileMap;
}

QgsVectorTileMatrixSet QgsVtpkTiles::matrixSet() const
{
  if ( !mMatrixSet.isEmpty() )
    return mMatrixSet;

  mMatrixSet.fromEsriJson( metadata(), rootTileMap() );
  return mMatrixSet;
}

QgsCoordinateReferenceSystem QgsVtpkTiles::crs() const
{
  return matrixSet().crs();
}

QgsRectangle QgsVtpkTiles::extent( const QgsCoordinateTransformContext &context ) const
{
  const QVariantMap md = metadata();

  const QVariantMap fullExtent = md.value( u"fullExtent"_s ).toMap();
  if ( !fullExtent.isEmpty() )
  {
    QgsRectangle fullExtentRect(
      fullExtent.value( u"xmin"_s ).toDouble(),
      fullExtent.value( u"ymin"_s ).toDouble(),
      fullExtent.value( u"xmax"_s ).toDouble(),
      fullExtent.value( u"ymax"_s ).toDouble()
    );

    const QgsCoordinateReferenceSystem fullExtentCrs = QgsArcGisRestUtils::convertSpatialReference( fullExtent.value( u"spatialReference"_s ).toMap() );
    const QgsCoordinateTransform extentTransform( fullExtentCrs, crs(), context );
    try
    {
      return extentTransform.transformBoundingBox( fullExtentRect );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( u"Could not transform layer fullExtent to layer CRS"_s );
    }
  }

  return QgsRectangle();
}

QByteArray QgsVtpkTiles::tileData( int z, int x, int y )
{
  if ( !mZip )
  {
    QgsDebugError( u"VTPK tile package not open: "_s + mFilename );
    return QByteArray();
  }
  if ( mPacketSize < 0 )
    mPacketSize = metadata().value( u"resourceInfo"_s ).toMap().value( u"cacheInfo"_s ).toMap().value( u"storageInfo"_s ).toMap().value( u"packetSize"_s ).toInt();

  const int fileRow = mPacketSize * static_cast< int >( std::floor( y / static_cast< double>( mPacketSize ) ) );
  const int fileCol = mPacketSize * static_cast< int >( std::floor( x / static_cast< double>( mPacketSize ) ) );

  const QString tileName = u"R%1C%2"_s
                           .arg( fileRow, 4, 16, '0'_L1 )
                           .arg( fileCol, 4, 16, '0'_L1 );

  const QString fileName = u"p12/tile/L%1/%2.bundle"_s
                           .arg( z, 2, 10, '0'_L1 ).arg( tileName );
  struct zip_stat stat;
  zip_stat_init( &stat );
  zip_stat( mZip, fileName.toLocal8Bit().constData(), 0, &stat );

  const size_t tileIndexOffset = 64 + 8 * ( mPacketSize * ( y % mPacketSize ) + ( x % mPacketSize ) );

  QByteArray res;
  const size_t len = stat.size;
  if ( len <= tileIndexOffset )
  {
    // seems this should be treated as "no content" here, rather then a broken VTPK
    res = QByteArray( "" );
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
      if ( tileSize == 0 )
      {
        // construct a non-null bytearray
        res = QByteArray( "" );
      }
      else if ( !QgsZipUtils::decodeGzip( buf.get() + tileOffset, tileSize, res ) )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error extracting bundle contents as gzip: %1" ).arg( fileName ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "Error reading tile: '%1'" ).arg( zip_strerror( mZip ) ) );
    }
    if ( file )
      zip_fclose( file );
    file = nullptr;
  }

  return res;
}

