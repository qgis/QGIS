/***************************************************************************
                             qgscolorutils.cpp
                             ---------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
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

#include "qgscolorutils.h"

#include <QColor>
#include <QColorSpace>
#include <QDomDocument>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

void QgsColorUtils::writeXml( const QColor &color, const QString &identifier, QDomDocument &document, QDomElement &element, const QgsReadWriteContext & )
{
  {
    const QDomElement oldElement = element.firstChildElement( identifier );
    if ( !oldElement.isNull() )
      element.removeChild( oldElement );
  }

  QDomElement colorElement = document.createElement( identifier );
  if ( !color.isValid() )
  {
    colorElement.setAttribute( u"invalid"_s, u"1"_s );
  }
  else
  {
    QString spec;
    switch ( color.spec() )
    {
      case QColor::Invalid:
        break; // not possible

      case QColor::Rgb:
      case QColor::ExtendedRgb:
      {
        // QColor will automatically adapt between extended rgb/rgb based on value of red/green/blue components
        spec = u"rgb"_s;
        float red = 1;
        float green = 1;
        float blue = 1;

        color.getRgbF( &red, &green, &blue );
        colorElement.setAttribute( u"red"_s, qgsDoubleToString( red ) );
        colorElement.setAttribute( u"green"_s, qgsDoubleToString( green ) );
        colorElement.setAttribute( u"blue"_s, qgsDoubleToString( blue ) );
        break;
      }

      case QColor::Hsv:
      {
        spec = u"hsv"_s;

        float h = 1;
        float s = 1;
        float v = 1;

        color.getHsvF( &h, &s, &v );
        colorElement.setAttribute( u"hue"_s, qgsDoubleToString( h ) );
        colorElement.setAttribute( u"saturation"_s, qgsDoubleToString( s ) );
        colorElement.setAttribute( u"value"_s, qgsDoubleToString( v ) );
        break;
      }

      case QColor::Hsl:
      {
        spec = u"hsl"_s;

        float h = 1;
        float s = 1;
        float l = 1;

        color.getHslF( &h, &s, &l );
        colorElement.setAttribute( u"hue"_s, qgsDoubleToString( h ) );
        colorElement.setAttribute( u"saturation"_s, qgsDoubleToString( s ) );
        colorElement.setAttribute( u"lightness"_s, qgsDoubleToString( l ) );
        break;
      }

      case QColor::Cmyk:
      {
        spec = u"cmyk"_s;

        float c = 1;
        float m = 1;
        float y = 1;
        float k = 1;

        color.getCmykF( &c, &y, &m, &k );
        colorElement.setAttribute( u"c"_s, qgsDoubleToString( c ) );
        colorElement.setAttribute( u"m"_s, qgsDoubleToString( m ) );
        colorElement.setAttribute( u"y"_s, qgsDoubleToString( y ) );
        colorElement.setAttribute( u"k"_s, qgsDoubleToString( k ) );
        break;
      }
    }
    colorElement.setAttribute( u"spec"_s, spec );
    if ( color.alphaF() < 1.0 )
    {
      colorElement.setAttribute( u"alpha"_s, qgsDoubleToString( color.alphaF() ) );
    }
  }
  element.appendChild( colorElement );
}

QColor QgsColorUtils::readXml( const QDomElement &element, const QString &identifier, const QgsReadWriteContext & )
{
  const QDomElement colorElement = element.firstChildElement( identifier );
  if ( colorElement.isNull() )
    return QColor();

  const bool invalid = colorElement.attribute( u"invalid"_s, u"0"_s ).toInt();
  if ( invalid )
    return QColor();

  QColor res;
  const QString spec = colorElement.attribute( u"spec"_s );
  if ( spec == "rgb"_L1 )
  {
    // QColor will automatically adapt between extended rgb/rgb based on value of red/green/blue components
    const double red = colorElement.attribute( u"red"_s ).toDouble();
    const double green = colorElement.attribute( u"green"_s ).toDouble();
    const double blue = colorElement.attribute( u"blue"_s ).toDouble();
    res = QColor::fromRgbF( red, green, blue );
  }
  else if ( spec == "hsv"_L1 )
  {
    const double hue = colorElement.attribute( u"hue"_s ).toDouble();
    const double saturation = colorElement.attribute( u"saturation"_s ).toDouble();
    const double value = colorElement.attribute( u"value"_s ).toDouble();
    res = QColor::fromHsvF( hue, saturation, value );
  }
  else if ( spec == "hsl"_L1 )
  {
    const double hue = colorElement.attribute( u"hue"_s ).toDouble();
    const double saturation = colorElement.attribute( u"saturation"_s ).toDouble();
    const double value = colorElement.attribute( u"lightness"_s ).toDouble();
    res = QColor::fromHslF( hue, saturation, value );
  }
  else if ( spec == "cmyk"_L1 )
  {
    const double cyan = colorElement.attribute( u"c"_s ).toDouble();
    const double magenta = colorElement.attribute( u"m"_s ).toDouble();
    const double yellow = colorElement.attribute( u"y"_s ).toDouble();
    const double black = colorElement.attribute( u"k"_s ).toDouble();
    res = QColor::fromCmykF( cyan, magenta, yellow, black );
  }

  {
    const double alpha = colorElement.attribute( u"alpha"_s, u"1"_s ).toDouble();
    res.setAlphaF( alpha );
  }

  return res;
}

QString QgsColorUtils::colorToString( const QColor &color )
{
  if ( !color.isValid() )
    return QString();

  // this is the pre 3.28 deprecated string format -- we prefix the lossless encoded color with this so that older QGIS versions
  // can still recover the lossy color via QgsSymbolLayerUtils::decodeColor
  const QString compatString = u"%1,%2,%3,%4,"_s.arg( color.red() ).arg( color.green() ).arg( color.blue() ).arg( color.alpha() );

  switch ( color.spec() )
  {
    case QColor::Invalid:
      break; // not possible

    case QColor::Rgb:
    case QColor::ExtendedRgb:
    {
      // QColor will automatically adapt between extended rgb/rgb based on value of red/green/blue components

      float red = 1;
      float green = 1;
      float blue = 1;
      float alpha = 1;

      color.getRgbF( &red, &green, &blue, &alpha );
      return compatString + u"rgb:%1,%2,%3,%4"_s.arg( qgsDoubleToString( red, 7 ),
             qgsDoubleToString( green, 7 ),
             qgsDoubleToString( blue, 7 ),
             qgsDoubleToString( alpha, 7 ) );
    }

    case QColor::Hsv:
    {
      float h = 1;
      float s = 1;
      float v = 1;
      float alpha = 1;

      color.getHsvF( &h, &s, &v, &alpha );
      return compatString + u"hsv:%1,%2,%3,%4"_s.arg( qgsDoubleToString( h ),
             qgsDoubleToString( s ),
             qgsDoubleToString( v ),
             qgsDoubleToString( alpha ) );
    }

    case QColor::Hsl:
    {
      float h = 1;
      float s = 1;
      float l = 1;
      float alpha = 1;

      color.getHslF( &h, &s, &l, &alpha );
      return compatString + u"hsl:%1,%2,%3,%4"_s.arg( qgsDoubleToString( h ),
             qgsDoubleToString( s ),
             qgsDoubleToString( l ),
             qgsDoubleToString( alpha ) );
    }

    case QColor::Cmyk:
    {
      float c = 1;
      float m = 1;
      float y = 1;
      float k = 1;
      float alpha = 1;

      color.getCmykF( &c, &m, &y, &k, &alpha );
      return compatString + u"cmyk:%1,%2,%3,%4,%5"_s.arg( qgsDoubleToString( c ),
             qgsDoubleToString( m ),
             qgsDoubleToString( y ),
             qgsDoubleToString( k ),
             qgsDoubleToString( alpha ) );
    }
  }
  return QString();
}

QColor QgsColorUtils::colorFromString( const QString &string )
{
  if ( string.isEmpty() )
    return QColor();

  const thread_local QRegularExpression rx( u"^(.*),([a-z]+):([\\d\\.\\-]+),([\\d\\.\\-]+),([\\d\\.\\-]+),([\\d\\.\\-]+),?([\\d\\.\\-]*)$"_s );
  const QRegularExpressionMatch match = rx.match( string );
  if ( !match.hasMatch() )
  {
    // try reading older color format and hex strings
    const QStringList lst = string.split( ',' );
    if ( lst.count() < 3 )
    {
      return QColor( string );
    }
    int red, green, blue, alpha;
    red = lst[0].toInt();
    green = lst[1].toInt();
    blue = lst[2].toInt();
    alpha = lst.count() > 3 ? lst[3].toInt() : 255;
    return QColor( red, green, blue, alpha );
  }

  const QString spec = match.captured( 2 );

  if ( spec == "rgb"_L1 )
  {
    // QColor will automatically adapt between extended rgb/rgb based on value of red/green/blue components
    const double red = match.captured( 3 ).toDouble();
    const double green = match.captured( 4 ).toDouble();
    const double blue = match.captured( 5 ).toDouble();
    const double alpha = match.captured( 6 ).toDouble();
    return QColor::fromRgbF( red, green, blue, alpha );
  }
  else if ( spec == "hsv"_L1 )
  {
    const double hue = match.captured( 3 ).toDouble();
    const double saturation = match.captured( 4 ).toDouble();
    const double value = match.captured( 5 ).toDouble();
    const double alpha = match.captured( 6 ).toDouble();
    return QColor::fromHsvF( hue, saturation, value, alpha );
  }
  else if ( spec == "hsl"_L1 )
  {
    const double hue = match.captured( 3 ).toDouble();
    const double saturation = match.captured( 4 ).toDouble();
    const double lightness = match.captured( 5 ).toDouble();
    const double alpha = match.captured( 6 ).toDouble();
    return QColor::fromHslF( hue, saturation, lightness, alpha );
  }
  else if ( spec == "cmyk"_L1 )
  {
    const double cyan = match.captured( 3 ).toDouble();
    const double magenta = match.captured( 4 ).toDouble();
    const double yellow = match.captured( 5 ).toDouble();
    const double black = match.captured( 6 ).toDouble();
    const double alpha = match.captured( 7 ).toDouble();
    return QColor::fromCmykF( cyan, magenta, yellow, black, alpha );
  }
  return QColor();
}

QColorSpace QgsColorUtils::iccProfile( const QString &iccProfileFilePath, QString &errorMsg )
{
  if ( iccProfileFilePath.isEmpty() )
    return QColorSpace();

  QFile file( iccProfileFilePath );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    errorMsg = QObject::tr( "Failed to open ICC Profile: %1" ).arg( iccProfileFilePath );
    return QColorSpace();
  }

  QColorSpace colorSpace = QColorSpace::fromIccProfile( file.readAll() );
  if ( !colorSpace.isValid() )
  {
    errorMsg = QObject::tr( "Invalid ICC Profile: %1" ).arg( iccProfileFilePath );
    return colorSpace;
  }

  return colorSpace;
}


QString QgsColorUtils::saveIccProfile( const QColorSpace &colorSpace, const QString &iccProfileFilePath )
{
  if ( !colorSpace.isValid() )
    return QObject::tr( "Invalid ICC profile" );

  QFile iccProfile( iccProfileFilePath );
  if ( !iccProfile.open( QIODevice::WriteOnly ) )
    return QObject::tr( "File access error '%1'" ).arg( iccProfileFilePath );

  if ( iccProfile.write( colorSpace.iccProfile() ) < 0 )
    return QObject::tr( "Error while writing to file '%1'" ).arg( iccProfileFilePath );

  return QString();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)

Qgis::ColorModel QgsColorUtils::toColorModel( QColorSpace::ColorModel colorModel, bool *ok )
{
  bool lok = false;
  Qgis::ColorModel res;
  switch ( colorModel )
  {
    case QColorSpace::ColorModel::Cmyk:
      lok = true;
      res = Qgis::ColorModel::Cmyk;
      break;

    case QColorSpace::ColorModel::Rgb:
      lok = true;
      res = Qgis::ColorModel::Rgb;
      break;

    case QColorSpace::ColorModel::Undefined:
    case QColorSpace::ColorModel::Gray: // not supported
      lok = false;
      res = Qgis::ColorModel::Rgb;
  }

  if ( ok )
    *ok = lok;

  return res;
}

#endif
