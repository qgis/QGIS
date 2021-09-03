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

#include <QDate>
#include <QRegularExpression>
#include <QFileInfo>
#include <QTime>


double readRationale( const Exiv2::Value &value, long n = 0 )
{
  const Exiv2::Rational rational = value.toRational( n );
  return static_cast< double >( rational.first ) / rational.second;
};

double readCoordinate( const Exiv2::Value &value )
{
  double res = 0;
  double div = 1;
  for ( int i = 0; i < 3; i++ )
  {
    res += readRationale( value, i ) / div;
    div *= 60;
  }
  return res;
};

QVariant decodeExifData( const QString &key, Exiv2::ExifData::const_iterator &it )
{
  QVariant val;

  if ( key == QLatin1String( "Exif.GPSInfo.GPSLatitude" ) ||
       key == QLatin1String( "Exif.GPSInfo.GPSLongitude" ) ||
       key == QLatin1String( "Exif.GPSInfo.GPSDestLatitude" ) ||
       key == QLatin1String( "Exif.GPSInfo.GPSDestLongitude" ) )
  {
    val = readCoordinate( it->value() );
  }
  else if ( key == QLatin1String( "Exif.GPSInfo.GPSTimeStamp" ) )
  {
    const QStringList parts = QString::fromStdString( it->toString() ).split( QRegularExpression( QStringLiteral( "\\s+" ) ) );
    if ( parts.size() == 3 )
    {
      const int hour = readRationale( it->value(), 0 );
      const int minute = readRationale( it->value(), 1 );
      const int second = readRationale( it->value(), 2 );
      val = QVariant::fromValue( QTime::fromString( QStringLiteral( "%1:%2:%3" )
                                 .arg( QString::number( hour ).rightJustified( 2, '0' ) )
                                 .arg( QString::number( minute ).rightJustified( 2, '0' ) )
                                 .arg( QString::number( second ).rightJustified( 2, '0' ) ), QLatin1String( "hh:mm:ss" ) ) );
    }
  }
  else if ( key == QLatin1String( "Exif.GPSInfo.GPSDateStamp" ) )
  {
    val = QVariant::fromValue( QDate::fromString( QString::fromStdString( it->toString() ), QLatin1String( "yyyy:MM:dd" ) ) );
  }
  else if ( key == QLatin1String( "Exif.Image.DateTime" ) ||
            key == QLatin1String( "Exif.Image.DateTime" ) ||
            key == QLatin1String( "Exif.Photo.DateTimeDigitized" ) ||
            key == QLatin1String( "Exif.Photo.DateTimeOriginal" ) )
  {
    val = QVariant::fromValue( QDateTime::fromString( QString::fromStdString( it->toString() ), QLatin1String( "yyyy:MM:dd hh:mm:ss" ) ) );
  }
  else
  {
    switch ( it->typeId() )
    {
      case Exiv2::asciiString:
      case Exiv2::string:
      case Exiv2::comment:
      case Exiv2::directory:
      case Exiv2::xmpText:
        val = QString::fromStdString( it->toString() );
        break;

      case Exiv2::unsignedLong:
      case Exiv2::signedLong:
      case Exiv2::unsignedLongLong:
      case Exiv2::signedLongLong:
        val = QVariant::fromValue( it->toLong() );
        break;

      case Exiv2::tiffDouble:
      case Exiv2::tiffFloat:
        val = QVariant::fromValue( it->toFloat() );
        break;

      case Exiv2::unsignedShort:
      case Exiv2::signedShort:
      case Exiv2::unsignedByte:
      case Exiv2::signedByte:
      case Exiv2::tiffIfd:
      case Exiv2::tiffIfd8:
        val = QVariant::fromValue( static_cast< int >( it->toLong() ) );
        break;

      case Exiv2::date:
      {
        const Exiv2::DateValue::Date date = static_cast< const Exiv2::DateValue *>( &it->value() )->getDate();
        val = QVariant::fromValue( QDate::fromString( QStringLiteral( "%1-%2-%3" ).arg( date.year )
                                   .arg( QString::number( date.month ).rightJustified( 2, '0' ) )
                                   .arg( QString::number( date.day ).rightJustified( 2, '0' ) ), QLatin1String( "yyyy-MM-dd" ) ) );
        break;
      }

      case Exiv2::time:
      {
        const Exiv2::TimeValue::Time time = static_cast< const Exiv2::TimeValue *>( &it->value() )->getTime();
        val = QVariant::fromValue( QTime::fromString( QStringLiteral( "%1:%2:%3" ).arg( QString::number( time.hour ).rightJustified( 2, '0' ) )
                                   .arg( QString::number( time.minute ).rightJustified( 2, '0' ) )
                                   .arg( QString::number( time.second ).rightJustified( 2, '0' ) ), QLatin1String( "hh:mm:ss" ) ) );
        break;
      }

      case Exiv2::unsignedRational:
      case Exiv2::signedRational:
      {
        if ( it->count() == 1 )
        {
          val = QVariant::fromValue( readRationale( it->value() ) );
        }
        else
        {
          val = QString::fromStdString( it->toString() );
        }
        break;
      }

      case Exiv2::undefined:
      case Exiv2::xmpAlt:
      case Exiv2::xmpBag:
      case Exiv2::xmpSeq:
      case Exiv2::langAlt:
      case Exiv2::invalidTypeId:
      case Exiv2::lastTypeId:
        val = QString::fromStdString( it->toString() );
        break;

    }
  }
  return val;
}

QString doubleToExifCoordinateString( const double val )
{
  const double d = std::abs( val );
  const int degrees = static_cast< int >( std::floor( d ) );
  const double m = 60 * ( d - degrees );
  const int minutes = static_cast< int >( std::floor( m ) );
  const double s = 60 * ( m - minutes );
  const int seconds = static_cast< int >( std::floor( s * 1000 ) );
  return QStringLiteral( "%1/1 %2/1 %3/1000" ).arg( degrees ).arg( minutes ).arg( seconds );
}

QVariant QgsExifTools::readTag( const QString &imagePath, const QString &key )
{
  if ( !QFileInfo::exists( imagePath ) )
    return QVariant();

  try
  {
    std::unique_ptr< Exiv2::Image > image( Exiv2::ImageFactory::open( imagePath.toStdString() ) );
    if ( !image || key.isEmpty() )
      return QVariant();

    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if ( exifData.empty() )
    {
      return QVariant();
    }

    Exiv2::ExifData::const_iterator i = exifData.findKey( Exiv2::ExifKey( key.toUtf8().constData() ) );
    return i != exifData.end() ? decodeExifData( key, i ) : QVariant();
  }
  catch ( ... )
  {
    return QVariant();
  }
}

QVariantMap QgsExifTools::readTags( const QString &imagePath )
{
  if ( !QFileInfo::exists( imagePath ) )
    return QVariantMap();

  try
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
    const Exiv2::ExifData::const_iterator end = exifData.end();
    for ( Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i )
    {
      const QString key = QString::fromStdString( i->key() );
      res.insert( key, decodeExifData( key, i ) );
    }
    return res;
  }
  catch ( ... )
  {
    return QVariantMap();
  }
}

bool QgsExifTools::hasGeoTag( const QString &imagePath )
{
  bool ok = false;
  QgsExifTools::getGeoTag( imagePath, ok );
  return ok;
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

    const Exiv2::ExifData::iterator itLatRef = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLatitudeRef" ) );
    const Exiv2::ExifData::iterator itLatVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLatitude" ) );
    const Exiv2::ExifData::iterator itLonRef = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLongitudeRef" ) );
    const Exiv2::ExifData::iterator itLonVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSLongitude" ) );

    if ( itLatRef == exifData.end() || itLatVal == exifData.end() ||
         itLonRef == exifData.end() || itLonVal == exifData.end() )
      return QgsPoint();

    double lat = readCoordinate( itLatVal->value() );
    double lon = readCoordinate( itLonVal->value() );

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

    const Exiv2::ExifData::iterator itElevVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSAltitude" ) );
    const Exiv2::ExifData::iterator itElevRefVal = exifData.findKey( Exiv2::ExifKey( "Exif.GPSInfo.GPSAltitudeRef" ) );
    if ( itElevVal != exifData.end() )
    {
      double elev = readRationale( itElevVal->value() );
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
    exifData["Exif.GPSInfo.GPSLatitude"] = doubleToExifCoordinateString( location.y() ).toStdString();
    exifData["Exif.GPSInfo.GPSLongitude"] = doubleToExifCoordinateString( location.x() ).toStdString();
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
