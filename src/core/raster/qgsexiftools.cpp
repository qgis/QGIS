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
#include "moc_qgsexiftools.cpp"
#include "qgspoint.h"

#include <exiv2/exiv2.hpp>

#include <QDate>
#include <QRegularExpression>
#include <QFileInfo>
#include <QTime>

double readRational( const Exiv2::Value &value, long n = 0 )
{
  const Exiv2::Rational rational = value.toRational( n );
  const auto numerator = rational.first;
  const auto denominator = rational.second;
  double res = 0;
  if ( value.typeId() == Exiv2::unsignedRational )
  {
    res = static_cast< double >( static_cast<uint32_t>( numerator ) ) / static_cast<uint32_t>( denominator );
  }
  else
  {
    res = static_cast< double >( numerator ) / denominator;
  }
  return res;
};

double readCoordinate( const Exiv2::Value &value )
{
  double res = 0;
  double div = 1;
  for ( int i = 0; i < 3; i++ )
  {
    res += readRational( value, i ) / div;
    div *= 60;
  }
  return res;
};

QVariant decodeXmpData( const QString &key, Exiv2::XmpData::const_iterator &it )
{
  QVariant val;
  if ( key == QLatin1String( "Xmp.xmp.MetadataDate" ) ||
       key == QLatin1String( "Xmp.xmp.CreateDate" ) ||
       key == QLatin1String( "Xmp.xmp.ModifyDate" ) )
  {
    val = QVariant::fromValue( QDateTime::fromString( QString::fromStdString( it->toString() ), Qt::ISODate ) );
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
#if EXIV2_TEST_VERSION (0, 28, 0)
        val = QVariant::fromValue( it->toUint32() );
#else
        val = QVariant::fromValue( it->toLong() );
#endif
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
#if EXIV2_TEST_VERSION (0, 28, 0)
        val = QVariant::fromValue( static_cast< int >( it->toUint32() ) );
#else
        val = QVariant::fromValue( static_cast< int >( it->toLong() ) );
#endif
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
          val = QVariant::fromValue( readRational( it->value() ) );
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
      const int hour = std::max( 0, std::min( 23, static_cast< int >( readRational( it->value(), 0 ) ) ) );
      const int minute = std::max( 0, std::min( 59, static_cast< int >( readRational( it->value(), 1 ) ) ) );
      const int second = std::max( 0, std::min( 59, static_cast< int >( readRational( it->value(), 2 ) ) ) );

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
#if EXIV2_TEST_VERSION (0, 28, 0)
        val = QVariant::fromValue( it->toUint32() );
#else
        val = QVariant::fromValue( it->toLong() );
#endif
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
#if EXIV2_TEST_VERSION (0, 28, 0)
        val = QVariant::fromValue( static_cast< int >( it->toUint32() ) );
#else
        val = QVariant::fromValue( static_cast< int >( it->toLong() ) );
#endif
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
          val = QVariant::fromValue( readRational( it->value() ) );
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

    if ( key.startsWith( QLatin1String( "Xmp." ) ) )
    {
      Exiv2::XmpData &xmpData = image->xmpData();
      if ( xmpData.empty() )
      {
        return QVariant();
      }
      Exiv2::XmpData::const_iterator i = xmpData.findKey( Exiv2::XmpKey( key.toUtf8().constData() ) );
      return i != xmpData.end() ? decodeXmpData( key, i ) : QVariant();
    }
    else
    {
      Exiv2::ExifData &exifData = image->exifData();
      if ( exifData.empty() )
      {
        return QVariant();
      }
      Exiv2::ExifData::const_iterator i = exifData.findKey( Exiv2::ExifKey( key.toUtf8().constData() ) );
      return i != exifData.end() ? decodeExifData( key, i ) : QVariant();
    }
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
    QVariantMap res;
    std::unique_ptr< Exiv2::Image > image( Exiv2::ImageFactory::open( imagePath.toStdString() ) );
    if ( !image )
      return QVariantMap();
    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    if ( !exifData.empty() )
    {
      const Exiv2::ExifData::const_iterator end = exifData.end();
      for ( Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i )
      {
        const QString key = QString::fromStdString( i->key() );
        res.insert( key, decodeExifData( key, i ) );
      }
    }

    Exiv2::XmpData &xmpData = image->xmpData();
    if ( !xmpData.empty() )
    {
      const Exiv2::XmpData::const_iterator end = xmpData.end();
      for ( Exiv2::XmpData::const_iterator i = xmpData.begin(); i != end; ++i )
      {
        const QString key = QString::fromStdString( i->key() );
        res.insert( key, decodeXmpData( key, i ) );
      }
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
      double elev = readRational( itElevVal->value() );
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

bool QgsExifTools::tagImage( const QString &imagePath, const QString &tag, const QVariant &value )
{
  try
  {
    std::unique_ptr< Exiv2::Image > image( Exiv2::ImageFactory::open( imagePath.toStdString() ) );
    if ( !image )
      return false;

    QVariant actualValue;
    bool actualValueIsUShort = false;
    if ( tag == QLatin1String( "Exif.GPSInfo.GPSLatitude" ) ||
         tag == QLatin1String( "Exif.GPSInfo.GPSLongitude" ) ||
         tag == QLatin1String( "Exif.GPSInfo.GPSDestLatitude" ) ||
         tag == QLatin1String( "Exif.GPSInfo.GPSDestLongitude" ) )
    {
      actualValue = doubleToExifCoordinateString( value.toDouble() );
    }
    else if ( tag == QLatin1String( "Exif.GPSInfo.GPSAltitude" ) )
    {
      actualValue = QStringLiteral( "%1/1000" ).arg( static_cast< int>( std::floor( std::abs( value.toDouble() ) * 1000 ) ) );
    }
    else if ( tag == QLatin1String( "Exif.Image.Orientation" ) )
    {
      actualValueIsUShort = true;
      actualValue = value;
    }
    else if ( value.userType() == QMetaType::Type::QDateTime )
    {
      const QDateTime dateTime = value.toDateTime();
      if ( tag == QLatin1String( "Exif.Image.DateTime" ) ||
           tag == QLatin1String( "Exif.Image.DateTime" ) ||
           tag == QLatin1String( "Exif.Photo.DateTimeDigitized" ) ||
           tag == QLatin1String( "Exif.Photo.DateTimeOriginal" ) )
      {
        actualValue = dateTime.toString( QStringLiteral( "yyyy:MM:dd hh:mm:ss" ) );
      }
      else
      {
        actualValue = dateTime.toString( Qt::ISODate );
      }
    }
    else if ( value.userType() == QMetaType::Type::QDate )
    {
      const QDate date = value.toDate();
      if ( tag == QLatin1String( "Exif.GPSInfo.GPSDateStamp" ) )
      {
        actualValue = date.toString( QStringLiteral( "yyyy:MM:dd" ) );
      }
      else
      {
        actualValue = date.toString( QStringLiteral( "yyyy-MM-dd" ) );
      }
    }
    else if ( value.userType() == QMetaType::Type::QTime )
    {
      const QTime time = value.toTime();
      if ( tag == QLatin1String( "Exif.GPSInfo.GPSTimeStamp" ) )
      {
        actualValue = QStringLiteral( "%1/1 %2/1 %3/1" ).arg( time.hour() ).arg( time.minute() ).arg( time.second() );
      }
      else
      {
        actualValue = time.toString( QStringLiteral( "HH:mm:ss" ) );
      }
    }
    else
    {
      actualValue = value;
    }

    const bool isXmp = tag.startsWith( QLatin1String( "Xmp." ) );
    image->readMetadata();
    if ( actualValueIsUShort )
    {
      if ( isXmp )
      {
        Exiv2::XmpData &xmpData = image->xmpData();
        xmpData[tag.toStdString()] = static_cast<ushort>( actualValue.toLongLong() );
      }
      else
      {
        Exiv2::ExifData &exifData = image->exifData();
        exifData[tag.toStdString()] = static_cast<ushort>( actualValue.toLongLong() );
      }
    }
    else if ( actualValue.userType() == QMetaType::Type::Int ||
              actualValue.userType() == QMetaType::Type::LongLong )
    {
      if ( isXmp )
      {
        Exiv2::XmpData &xmpData = image->xmpData();
        xmpData[tag.toStdString()] = static_cast<uint32_t>( actualValue.toLongLong() );
      }
      else
      {
        Exiv2::ExifData &exifData = image->exifData();
        exifData[tag.toStdString()] = static_cast<uint32_t>( actualValue.toLongLong() );
      }
    }
    else if ( actualValue.userType() == QMetaType::Type::UInt ||
              actualValue.userType() ==  QMetaType::Type::ULongLong )
    {
      if ( isXmp )
      {
        Exiv2::XmpData &xmpData = image->xmpData();
        xmpData[tag.toStdString()] = static_cast<int32_t>( actualValue.toULongLong() );
      }
      else
      {
        Exiv2::ExifData &exifData = image->exifData();
        exifData[tag.toStdString()] = static_cast<int32_t>( actualValue.toULongLong() );
      }
    }
    else if ( actualValue.userType() == QMetaType::Type::Double )
    {
      if ( isXmp )
      {
        Exiv2::XmpData &xmpData = image->xmpData();
        xmpData[tag.toStdString()] = Exiv2::floatToRationalCast( actualValue.toFloat() );
      }
      else
      {
        Exiv2::ExifData &exifData = image->exifData();
        exifData[tag.toStdString()] = Exiv2::floatToRationalCast( actualValue.toFloat() );
      }
    }
    else
    {
      if ( isXmp )
      {
        Exiv2::XmpData &xmpData = image->xmpData();
        xmpData[tag.toStdString()] = actualValue.toString().toStdString();
      }
      else
      {
        Exiv2::ExifData &exifData = image->exifData();
        exifData[tag.toStdString()] = actualValue.toString().toStdString();
      }
    }
    image->writeMetadata();
  }
  catch ( ... )
  {
    return false;
  }
  return true;
}
