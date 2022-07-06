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
#include <QDomDocument>
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
    colorElement.setAttribute( QStringLiteral( "invalid" ), QStringLiteral( "1" ) );
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
        spec = QStringLiteral( "rgb" );
        qreal red = 1;
        qreal green = 1;
        qreal blue = 1;
        color.getRgbF( &red, &green, &blue );
        colorElement.setAttribute( QStringLiteral( "red" ), qgsDoubleToString( red ) );
        colorElement.setAttribute( QStringLiteral( "green" ), qgsDoubleToString( green ) );
        colorElement.setAttribute( QStringLiteral( "blue" ), qgsDoubleToString( blue ) );
        break;
      }

      case QColor::Hsv:
      {
        spec = QStringLiteral( "hsv" );
        qreal h = 1;
        qreal s = 1;
        qreal v = 1;
        color.getHsvF( &h, &s, &v );
        colorElement.setAttribute( QStringLiteral( "hue" ), qgsDoubleToString( h ) );
        colorElement.setAttribute( QStringLiteral( "saturation" ), qgsDoubleToString( s ) );
        colorElement.setAttribute( QStringLiteral( "value" ), qgsDoubleToString( v ) );
        break;
      }

      case QColor::Hsl:
      {
        spec = QStringLiteral( "hsl" );
        qreal h = 1;
        qreal s = 1;
        qreal l = 1;
        color.getHslF( &h, &s, &l );
        colorElement.setAttribute( QStringLiteral( "hue" ), qgsDoubleToString( h ) );
        colorElement.setAttribute( QStringLiteral( "saturation" ), qgsDoubleToString( s ) );
        colorElement.setAttribute( QStringLiteral( "lightness" ), qgsDoubleToString( l ) );
        break;
      }

      case QColor::Cmyk:
      {
        spec = QStringLiteral( "cmyk" );
        qreal c = 1;
        qreal m = 1;
        qreal y = 1;
        qreal k = 1;
        color.getCmykF( &c, &y, &m, &k );
        colorElement.setAttribute( QStringLiteral( "c" ), qgsDoubleToString( c ) );
        colorElement.setAttribute( QStringLiteral( "m" ), qgsDoubleToString( m ) );
        colorElement.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( y ) );
        colorElement.setAttribute( QStringLiteral( "k" ), qgsDoubleToString( k ) );
        break;
      }
    }
    colorElement.setAttribute( QStringLiteral( "spec" ), spec );
    if ( color.alphaF() < 1.0 )
    {
      colorElement.setAttribute( QStringLiteral( "alpha" ), qgsDoubleToString( color.alphaF() ) );
    }
  }
  element.appendChild( colorElement );
}

QColor QgsColorUtils::readXml( const QDomElement &element, const QString &identifier, const QgsReadWriteContext & )
{
  const QDomElement colorElement = element.firstChildElement( identifier );
  if ( colorElement.isNull() )
    return QColor();

  const bool invalid = colorElement.attribute( QStringLiteral( "invalid" ), QStringLiteral( "0" ) ).toInt();
  if ( invalid )
    return QColor();

  QColor res;
  const QString spec = colorElement.attribute( QStringLiteral( "spec" ) );
  if ( spec == QLatin1String( "rgb" ) )
  {
    // QColor will automatically adapt between extended rgb/rgb based on value of red/green/blue components
    const double red = colorElement.attribute( QStringLiteral( "red" ) ).toDouble();
    const double green = colorElement.attribute( QStringLiteral( "green" ) ).toDouble();
    const double blue = colorElement.attribute( QStringLiteral( "blue" ) ).toDouble();
    res = QColor::fromRgbF( red, green, blue );
  }
  else if ( spec == QLatin1String( "hsv" ) )
  {
    const double hue = colorElement.attribute( QStringLiteral( "hue" ) ).toDouble();
    const double saturation = colorElement.attribute( QStringLiteral( "saturation" ) ).toDouble();
    const double value = colorElement.attribute( QStringLiteral( "value" ) ).toDouble();
    res = QColor::fromHsvF( hue, saturation, value );
  }
  else if ( spec == QLatin1String( "hsl" ) )
  {
    const double hue = colorElement.attribute( QStringLiteral( "hue" ) ).toDouble();
    const double saturation = colorElement.attribute( QStringLiteral( "saturation" ) ).toDouble();
    const double value = colorElement.attribute( QStringLiteral( "lightness" ) ).toDouble();
    res = QColor::fromHslF( hue, saturation, value );
  }
  else if ( spec == QLatin1String( "cmyk" ) )
  {
    const double cyan = colorElement.attribute( QStringLiteral( "c" ) ).toDouble();
    const double magenta = colorElement.attribute( QStringLiteral( "m" ) ).toDouble();
    const double yellow = colorElement.attribute( QStringLiteral( "y" ) ).toDouble();
    const double black = colorElement.attribute( QStringLiteral( "k" ) ).toDouble();
    res = QColor::fromCmykF( cyan, magenta, yellow, black );
  }

  {
    const double alpha = colorElement.attribute( QStringLiteral( "alpha" ), QStringLiteral( "1" ) ).toDouble();
    res.setAlphaF( alpha );
  }

  return res;
}

QString QgsColorUtils::colorToString( const QColor &color )
{
  if ( !color.isValid() )
    return QString();

  switch ( color.spec() )
  {
    case QColor::Invalid:
      break; // not possible

    case QColor::Rgb:
    case QColor::ExtendedRgb:
    {
      // QColor will automatically adapt between extended rgb/rgb based on value of red/green/blue components
      qreal red = 1;
      qreal green = 1;
      qreal blue = 1;
      qreal alpha = 1;
      color.getRgbF( &red, &green, &blue, &alpha );
      return QStringLiteral( "rgb:%1,%2,%3,%4" ).arg( qgsDoubleToString( red ),
             qgsDoubleToString( green ),
             qgsDoubleToString( blue ),
             qgsDoubleToString( alpha ) );
    }

    case QColor::Hsv:
    {
      qreal h = 1;
      qreal s = 1;
      qreal v = 1;
      qreal alpha = 1;
      color.getHsvF( &h, &s, &v, &alpha );
      return QStringLiteral( "hsv:%1,%2,%3,%4" ).arg( qgsDoubleToString( h ),
             qgsDoubleToString( s ),
             qgsDoubleToString( v ),
             qgsDoubleToString( alpha ) );
    }

    case QColor::Hsl:
    {
      qreal h = 1;
      qreal s = 1;
      qreal l = 1;
      qreal alpha = 1;
      color.getHslF( &h, &s, &l, &alpha );
      return QStringLiteral( "hsl:%1,%2,%3,%4" ).arg( qgsDoubleToString( h ),
             qgsDoubleToString( s ),
             qgsDoubleToString( l ),
             qgsDoubleToString( alpha ) );
    }

    case QColor::Cmyk:
    {
      qreal c = 1;
      qreal m = 1;
      qreal y = 1;
      qreal k = 1;
      qreal alpha = 1;
      color.getCmykF( &c, &y, &m, &k, &alpha );
      return QStringLiteral( "cmyk:%1,%2,%3,%4,%5" ).arg( qgsDoubleToString( c ),
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

  const thread_local QRegularExpression rx( QStringLiteral( "^([a-z]+):([\\d\\.\\-]+),([\\d\\.\\-]+),([\\d\\.\\-]+),([\\d\\.\\-]+),?([\\d\\.\\-]*)$" ) );
  const QRegularExpressionMatch match = rx.match( string );
  if ( !match.hasMatch() )
    return QColor();

  const QString spec = match.captured( 1 );

  if ( spec == QLatin1String( "rgb" ) )
  {
    // QColor will automatically adapt between extended rgb/rgb based on value of red/green/blue components
    const double red = match.captured( 2 ).toDouble();
    const double green = match.captured( 3 ).toDouble();
    const double blue = match.captured( 4 ).toDouble();
    const double alpha = match.captured( 5 ).toDouble();
    return QColor::fromRgbF( red, green, blue, alpha );
  }
  else if ( spec == QLatin1String( "hsv" ) )
  {
    const double hue = match.captured( 2 ).toDouble();
    const double saturation = match.captured( 3 ).toDouble();
    const double value = match.captured( 4 ).toDouble();
    const double alpha = match.captured( 5 ).toDouble();
    return QColor::fromHsvF( hue, saturation, value, alpha );
  }
  else if ( spec == QLatin1String( "hsl" ) )
  {
    const double hue = match.captured( 2 ).toDouble();
    const double saturation = match.captured( 3 ).toDouble();
    const double lightness = match.captured( 4 ).toDouble();
    const double alpha = match.captured( 5 ).toDouble();
    return QColor::fromHslF( hue, saturation, lightness, alpha );
  }
  else if ( spec == QLatin1String( "cmyk" ) )
  {
    const double cyan = match.captured( 2 ).toDouble();
    const double magenta = match.captured( 3 ).toDouble();
    const double yellow = match.captured( 4 ).toDouble();
    const double black = match.captured( 5 ).toDouble();
    const double alpha = match.captured( 6 ).toDouble();
    return QColor::fromCmykF( cyan, magenta, yellow, black, alpha );
  }
  return QColor();
}
