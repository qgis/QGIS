/***************************************************************************
  qgisexiftools.cpp
  -----------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexiftools.h"
#include "qgspoint.h"
#include <exiv2/exiv2.hpp>
#include <QRegularExpression>
#include <QFileInfo>

#if 0 // needs further work on the correct casting of tag values to QVariant values!
QVariantMap QgsExifTools::readTags( const QString &imagePath )
{
  std::unique_ptr< Exiv2::Image > image( Exiv2::ImageFactory::open( imagePath.toStdString() ) );
  if ( !image )
    return QVariantMap();

  image->readMetadata();
  Exiv2::ExifData &exifData = image->exifData();
  if ( exifData.empty() )
  {
    return QVariantMap();
  }

  QVariantMap res;
  Exiv2::ExifData::const_iterator end = exifData.end();
  for ( Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i )
  {
    const QString key = QString::fromStdString( i->key() );
    QVariant val;
    switch ( i->typeId() )
    {
      case Exiv2::asciiString:
      case Exiv2::string:
      case Exiv2::comment:
      case Exiv2::directory:
      case Exiv2::xmpText:
        val = QString::fromStdString( i->toString() );
        break;

      case Exiv2::unsignedLong:
      case Exiv2::signedLong:
        val = QVariant::fromValue( i->toLong() );
        break;

      case Exiv2::tiffDouble:
      case Exiv2::tiffFloat:
        val = QVariant::fromValue( i->toFloat() );
        break;

      case Exiv2::unsignedShort:
      case Exiv2::signedShort:
        val = QVariant::fromValue( static_cast< int >( i->toLong() ) );
        break;

      case Exiv2::unsignedRational:
      case Exiv2::signedRational:
      case Exiv2::unsignedByte:
      case Exiv2::signedByte:
      case Exiv2::undefined:
      case Exiv2::tiffIfd:
      case Exiv2::date:
      case Exiv2::time:
      case Exiv2::xmpAlt:
      case Exiv2::xmpBag:
      case Exiv2::xmpSeq:
      case Exiv2::langAlt:
      case Exiv2::invalidTypeId:
      case Exiv2::lastTypeId:
        val = QString::fromStdString( i->toString() );
        break;

    }

    res.insert( key, val );
  }
  return res;
}
#endif

QString doubleToExifCoordinate( const double val )
{
  double d = std::abs( val );
  int degrees = static_cast< int >( std::floor( d ) );
  double m = 60 * ( d - degrees );
  int minutes = static_cast< int >( std::floor( m ) );
  double s = 60 * ( m - minutes );
  int seconds = static_cast< int >( std::floor( s * 1000 ) );
  return QStringLiteral( "%1/1 %2/1 %3/1000" ).arg( degrees ).arg( minutes ).arg( seconds );
}

QgsPoint QgsExifTools::getGeoTag( const QString &imagePath, bool &ok )
{
  ok = false;
  if ( !QFileInfo::exists( imagePath ) )
    return QgsPoint();
  try
  {
    std::unique_ptr< Exiv2::Image > image( Exiv2::ImageFactory::open( imagePath.toStdString() ) );
    if ( !image )
      return QgsPoint();

    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    if ( exifData.empty() )
      return QgsPoint();

    Exiv2::ExifData::iterator itLatRef = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLatitudeRef" ) );
    Exiv2::ExifData::iterator itLatVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLatitude" ) );
    Exiv2::ExifData::iterator itLonRef = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLongitudeRef" ) );
    Exiv2::ExifData::iterator itLonVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLongitude" ) );

    if ( itLatRef == exifData.end() || itLatVal == exifData.end() ||
         itLonRef == exifData.end() || itLonVal == exifData.end() )
      return QgsPoint();

    auto readCoord = []( const QString & coord )->double
    {
      double res = 0;
      double div = 1;
      const QStringList parts = coord.split( QRegularExpression( QStringLiteral( "\\s+" ) ) );
      for ( const QString &rational : parts )
      {
        const QStringList pair = rational.split( '/' );
        if ( pair.size() != 2 )
          break;
        res += ( pair[0].toDouble() / pair[1].toDouble() ) / div;
        div *= 60;
      }
      return res;
    };

    auto readRationale = []( const QString & rational )->double
    {
      const QStringList pair = rational.split( '/' );
      if ( pair.size() != 2 )
        return std::numeric_limits< double >::quiet_NaN();
      return pair[0].toDouble() / pair[1].toDouble();
    };

    double lat = readCoord( QString::fromStdString( itLatVal->value().toString() ) );
    double lon = readCoord( QString::fromStdString( itLonVal->value().toString() ) );

    const QString latRef = QString::fromStdString( itLatRef->value().toString() );
    const QString lonRef = QString::fromStdString( itLonRef->value().toString() );
    if ( latRef.compare( QLatin1String( "S" ), Qt::CaseInsensitive ) == 0 )
    {
      lat *= -1;
    }
    if ( lonRef.compare( QLatin1String( "W" ), Qt::CaseInsensitive ) == 0 )
    {
      lon *= -1;
    }

    ok = true;

    Exiv2::ExifData::iterator itElevVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSAltitude" ) );
    Exiv2::ExifData::iterator itElevRefVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSAltitudeRef" ) );
    if ( itElevVal != exifData.end() )
    {
      double elev = readRationale( QString::fromStdString( itElevVal->value().toString() ) );
      if ( itElevRefVal != exifData.end() )
      {
        const QString elevRef = QString::fromStdString( itElevRefVal->value().toString() );
        if ( elevRef.compare( QLatin1String( "1" ), Qt::CaseInsensitive ) == 0 )
        {
          elev *= -1;
        }
      }
      return QgsPoint( lon, lat, elev );
    }
    else
    {
      return QgsPoint( lon, lat );
    }
  }
  catch ( ... )
  {
    return QgsPoint();
  }
}

bool QgsExifTools::geoTagImage( const QString &imagePath, const QgsPointXY &location, const GeoTagDetails &details )
{
  try
  {
    std::unique_ptr< Exiv2::Image > image( Exiv2::ImageFactory::open( imagePath.toStdString() ) );
    if ( !image )
      return false;

    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    exifData["Exif.GPSInfo.GPSVersionID"] = "2 0 0 0";
    exifData["Exif.GPSInfo.GPSMapDatum"] = "WGS-84";
    exifData["Exif.GPSInfo.GPSLatitude"] = doubleToExifCoordinate( location.y() ).toStdString();
    exifData["Exif.GPSInfo.GPSLongitude"] = doubleToExifCoordinate( location.x() ).toStdString();
    if ( !std::isnan( details.elevation ) )
    {
      const QString elevationString = QStringLiteral( "%1/1000" ).arg( static_cast< int>( std::floor( std::abs( details.elevation ) * 1000 ) ) );
      exifData["Exif.GPSInfo.GPSAltitude"] = elevationString.toStdString();
      exifData["Exif.GPSInfo.GPSAltitudeRef"] = details.elevation < 0.0 ? "1" : "0";
    }
    exifData["Exif.GPSInfo.GPSLatitudeRef"] = location.y() > 0 ? "N" : "S";
    exifData["Exif.GPSInfo.GPSLongitudeRef"] = location.x() > 0 ? "E" : "W";
    exifData["Exif.Image.GPSTag"] = 4908;
    image->writeMetadata();
  }
  catch ( ... )
  {
    return false;
  }
  return true;
}
