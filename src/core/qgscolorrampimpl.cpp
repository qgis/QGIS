/***************************************************************************
    qgscolorrampimpl.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorrampimpl.h"
#include "qgscolorbrewerpalette.h"
#include "qgscptcityarchive.h"

#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <algorithm>
#include <random>

#include <QTime>
#include <QDir>
#include <QFileInfo>

//////////////


static QColor _interpolateRgb( const QColor &c1, const QColor &c2, const double value, const Qgis::AngularDirection )
{
  if ( std::isnan( value ) )
    return c2;

  const qreal red1 = c1.redF();
  const qreal red2 = c2.redF();
  const qreal red = ( red1 + value * ( red2 - red1 ) );

  const qreal green1 = c1.greenF();
  const qreal green2 = c2.greenF();
  const qreal green = ( green1 + value * ( green2 - green1 ) );

  const qreal blue1 = c1.blueF();
  const qreal blue2 = c2.blueF();
  const qreal blue = ( blue1 + value * ( blue2 - blue1 ) );

  const qreal alpha1 = c1.alphaF();
  const qreal alpha2 = c2.alphaF();
  const qreal alpha = ( alpha1 + value * ( alpha2 - alpha1 ) );

  return QColor::fromRgbF( red, green, blue, alpha );
}

static QColor _interpolateHsv( const QColor &c1, const QColor &c2, const double value, const Qgis::AngularDirection direction )
{
  if ( std::isnan( value ) )
    return c2;

  qreal hue1 = c1.hsvHueF();
  qreal hue2 = c2.hsvHueF();
  qreal hue = 0;
  if ( hue1 == -1 )
    hue = hue2;
  else if ( hue2 == -1 )
    hue = hue1;
  else
  {
    switch ( direction )
    {
      case Qgis::AngularDirection::Clockwise:
      {
        if ( hue1 < hue2 )
          hue1 += 1;

        hue = hue1 - value * ( hue1 - hue2 );
        if ( hue < 0 )
          hue += 1;
        if ( hue > 1 )
          hue -= 1;
        break;
      }

      case Qgis::AngularDirection::CounterClockwise:
      {
        if ( hue2 < hue1 )
          hue2 += 1;

        hue = hue1 + value * ( hue2 - hue1 );
        if ( hue > 1 )
          hue -= 1;
        break;
      }
    }
  }

  const qreal saturation1 = c1.hsvSaturationF();
  const qreal saturation2 = c2.hsvSaturationF();
  const qreal saturation = ( saturation1 + value * ( saturation2 - saturation1 ) );

  const qreal value1 = c1.valueF();
  const qreal value2 = c2.valueF();
  const qreal valueOut = ( value1 + value * ( value2 - value1 ) );

  const qreal alpha1 = c1.alphaF();
  const qreal alpha2 = c2.alphaF();
  const qreal alpha = ( alpha1 + value * ( alpha2 - alpha1 ) );

  return QColor::fromHsvF( hue > 1 ? hue - 1 : hue, saturation, valueOut, alpha );
}

static QColor _interpolateHsl( const QColor &c1, const QColor &c2, const double value, const Qgis::AngularDirection direction )
{
  if ( std::isnan( value ) )
    return c2;

  qreal hue1 = c1.hslHueF();
  qreal hue2 = c2.hslHueF();
  qreal hue = 0;
  if ( hue1 == -1 )
    hue = hue2;
  else if ( hue2 == -1 )
    hue = hue1;
  else
  {
    switch ( direction )
    {
      case Qgis::AngularDirection::Clockwise:
      {
        if ( hue1 < hue2 )
          hue1 += 1;

        hue = hue1 - value * ( hue1 - hue2 );
        if ( hue < 0 )
          hue += 1;
        if ( hue > 1 )
          hue -= 1;
        break;
      }

      case Qgis::AngularDirection::CounterClockwise:
      {
        if ( hue2 < hue1 )
          hue2 += 1;

        hue = hue1 + value * ( hue2 - hue1 );
        if ( hue > 1 )
          hue -= 1;
        break;
      }
    }
  }

  const qreal saturation1 = c1.hslSaturationF();
  const qreal saturation2 = c2.hslSaturationF();
  const qreal saturation = ( saturation1 + value * ( saturation2 - saturation1 ) );

  const qreal lightness1 = c1.lightnessF();
  const qreal lightness2 = c2.lightnessF();
  const qreal lightness = ( lightness1 + value * ( lightness2 - lightness1 ) );

  const qreal alpha1 = c1.alphaF();
  const qreal alpha2 = c2.alphaF();
  const qreal alpha = ( alpha1 + value * ( alpha2 - alpha1 ) );

  return QColor::fromHslF( hue > 1 ? hue - 1 : hue, saturation, lightness, alpha );
}

//////////////


QgsGradientStop::QgsGradientStop( double offset, const QColor &color )
  : offset( offset )
  , color( color )
  , mFunc( _interpolateRgb )
{

}

void QgsGradientStop::setColorSpec( QColor::Spec spec )
{
  mColorSpec = spec;

  switch ( mColorSpec )
  {
    case QColor::Rgb:
    case QColor::Invalid:
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    case QColor::ExtendedRgb:
#endif
    case QColor::Cmyk:
      mFunc = _interpolateRgb;
      break;
    case QColor::Hsv:
      mFunc = _interpolateHsv;
      break;
    case QColor::Hsl:
      mFunc = _interpolateHsl;
      break;
  }
}

QgsGradientColorRamp::QgsGradientColorRamp( const QColor &color1, const QColor &color2,
    bool discrete, const QgsGradientStopsList &stops )
  : mColor1( color1 )
  , mColor2( color2 )
  , mDiscrete( discrete )
  , mStops( stops )
  , mFunc( _interpolateRgb )
{
}

QgsColorRamp *QgsGradientColorRamp::create( const QVariantMap &props )
{
  // color1 and color2
  QColor color1 = DEFAULT_GRADIENT_COLOR1;
  QColor color2 = DEFAULT_GRADIENT_COLOR2;
  if ( props.contains( QStringLiteral( "color1" ) ) )
    color1 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color1" )].toString() );
  if ( props.contains( QStringLiteral( "color2" ) ) )
    color2 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color2" )].toString() );

  //stops
  QgsGradientStopsList stops;
  if ( props.contains( QStringLiteral( "stops" ) ) )
  {
    const auto constSplit = props[QStringLiteral( "stops" )].toString().split( ':' );
    for ( const QString &stop : constSplit )
    {
      const QStringList parts = stop.split( ';' );
      if ( parts.size() != 2 && parts.size() != 4 )
        continue;

      QColor c = QgsSymbolLayerUtils::decodeColor( parts.at( 1 ) );
      stops.append( QgsGradientStop( parts.at( 0 ).toDouble(), c ) );

      if ( parts.size() == 4 )
      {
        if ( parts.at( 2 ).compare( QLatin1String( "rgb" ) ) == 0 )
          stops.last().setColorSpec( QColor::Spec::Rgb );
        else if ( parts.at( 2 ).compare( QLatin1String( "hsv" ) ) == 0 )
          stops.last().setColorSpec( QColor::Spec::Hsv );
        else if ( parts.at( 2 ).compare( QLatin1String( "hsl" ) ) == 0 )
          stops.last().setColorSpec( QColor::Spec::Hsl );

        if ( parts.at( 3 ).compare( QLatin1String( "cw" ) ) == 0 )
          stops.last().setDirection( Qgis::AngularDirection::Clockwise );
        else if ( parts.at( 3 ).compare( QLatin1String( "ccw" ) ) == 0 )
          stops.last().setDirection( Qgis::AngularDirection::CounterClockwise );
      }
    }
  }

  // discrete vs. continuous
  bool discrete = false;
  if ( props.contains( QStringLiteral( "discrete" ) ) )
  {
    if ( props[QStringLiteral( "discrete" )] == QLatin1String( "1" ) )
      discrete = true;
  }

  // search for information keys starting with "info_"
  QgsStringMap info;
  for ( QVariantMap::const_iterator it = props.constBegin();
        it != props.constEnd(); ++it )
  {
    if ( it.key().startsWith( QLatin1String( "info_" ) ) )
      info[ it.key().mid( 5 )] = it.value().toString();
  }

  QgsGradientColorRamp *r = new QgsGradientColorRamp( color1, color2, discrete, stops );
  r->setInfo( info );

  if ( props.contains( QStringLiteral( "spec" ) ) )
  {
    const QString spec = props.value( QStringLiteral( "spec" ) ).toString().trimmed();
    if ( spec.compare( QLatin1String( "rgb" ) ) == 0 )
      r->setColorSpec( QColor::Spec::Rgb );
    else if ( spec.compare( QLatin1String( "hsv" ) ) == 0 )
      r->setColorSpec( QColor::Spec::Hsv );
    else if ( spec.compare( QLatin1String( "hsl" ) ) == 0 )
      r->setColorSpec( QColor::Spec::Hsl );
  }

  if ( props.contains( QStringLiteral( "direction" ) ) )
  {
    const QString direction = props.value( QStringLiteral( "direction" ) ).toString().trimmed();
    if ( direction.compare( QLatin1String( "ccw" ) ) == 0 )
      r->setDirection( Qgis::AngularDirection::CounterClockwise );
    else if ( direction.compare( QLatin1String( "cw" ) ) == 0 )
      r->setDirection( Qgis::AngularDirection::Clockwise );
  }

  return r;
}

double QgsGradientColorRamp::value( int index ) const
{
  if ( index <= 0 )
  {
    return 0;
  }
  else if ( index >= mStops.size() + 1 )
  {
    return 1;
  }
  else
  {
    return mStops[index - 1].offset;
  }
}

QColor QgsGradientColorRamp::color( double value ) const
{
  if ( qgsDoubleNear( value, 0.0 ) || value < 0.0 )
  {
    return mColor1;
  }
  else if ( qgsDoubleNear( value, 1.0 ) || value > 1.0 )
  {
    return mColor2;
  }
  else if ( mStops.isEmpty() )
  {
    if ( mDiscrete )
      return mColor1;

    return mFunc( mColor1, mColor2, value, mDirection );
  }
  else
  {
    double lower = 0, upper = 0;
    QColor c1 = mColor1, c2;
    for ( QgsGradientStopsList::const_iterator it = mStops.begin(); it != mStops.end(); ++it )
    {
      if ( it->offset > value )
      {
        if ( mDiscrete )
          return c1;

        upper = it->offset;
        c2 = it->color;

        return qgsDoubleNear( upper, lower ) ? c1 : it->mFunc( c1, c2, ( value - lower ) / ( upper - lower ), it->mDirection );
      }
      lower = it->offset;
      c1 = it->color;
    }

    if ( mDiscrete )
      return c1;

    upper = 1;
    c2 = mColor2;
    return qgsDoubleNear( upper, lower ) ? c1 : mFunc( c1, c2, ( value - lower ) / ( upper - lower ), mDirection );
  }
}

QString QgsGradientColorRamp::type() const
{
  return QgsGradientColorRamp::typeString();
}

void QgsGradientColorRamp::invert()
{
  QgsGradientStopsList newStops;
  newStops.reserve( mStops.size() );

  if ( mDiscrete )
  {
    mColor2 = mColor1;
    mColor1 = mStops.at( mStops.size() - 1 ).color;
    for ( int k = mStops.size() - 1; k >= 1; k-- )
    {
      newStops << QgsGradientStop( 1 - mStops.at( k ).offset, mStops.at( k - 1 ).color );
    }
    newStops << QgsGradientStop( 1 - mStops.at( 0 ).offset, mColor2 );
  }
  else
  {
    QColor tmpColor = mColor2;
    mColor2 = mColor1;
    mColor1 = tmpColor;
    for ( int k = mStops.size() - 1; k >= 0; k-- )
    {
      newStops << QgsGradientStop( 1 - mStops.at( k ).offset, mStops.at( k ).color );
    }
  }

  // transfer color spec, invert directions
  if ( mStops.empty() )
  {
    // reverse direction
    mDirection = mDirection == Qgis::AngularDirection::Clockwise ? Qgis::AngularDirection::CounterClockwise : Qgis::AngularDirection::Clockwise;
  }
  else
  {
    newStops[0].setColorSpec( mColorSpec );
    newStops[0].setDirection( mDirection == Qgis::AngularDirection::Clockwise ? Qgis::AngularDirection::CounterClockwise : Qgis::AngularDirection::Clockwise );
    for ( int i = 1, j = mStops.size() - 1; i < mStops.size(); ++i, --j )
    {
      newStops[i].setColorSpec( mStops.at( j ).colorSpec() );
      newStops[i].setDirection( mStops.at( j ).direction() == Qgis::AngularDirection::Clockwise ? Qgis::AngularDirection::CounterClockwise : Qgis::AngularDirection::Clockwise );
    }
    mColorSpec = mStops.at( 0 ).colorSpec();
    mDirection = mStops.at( 0 ).direction() == Qgis::AngularDirection::Clockwise ? Qgis::AngularDirection::CounterClockwise : Qgis::AngularDirection::Clockwise;
  }

  mStops = newStops;
}

QgsGradientColorRamp *QgsGradientColorRamp::clone() const
{
  QgsGradientColorRamp *r = new QgsGradientColorRamp( mColor1, mColor2,
      mDiscrete, mStops );
  r->setInfo( mInfo );
  r->setColorSpec( mColorSpec );
  r->setDirection( mDirection );
  return r;
}

QVariantMap QgsGradientColorRamp::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "color1" )] = QgsSymbolLayerUtils::encodeColor( mColor1 );
  map[QStringLiteral( "color2" )] = QgsSymbolLayerUtils::encodeColor( mColor2 );
  if ( !mStops.isEmpty() )
  {
    QStringList lst;
    lst.reserve( mStops.size() );
    for ( const QgsGradientStop &stop : mStops )
    {
      lst.append( QStringLiteral( "%1;%2;%3;%4" ).arg( stop.offset ).arg( QgsSymbolLayerUtils::encodeColor( stop.color ),
                  stop.colorSpec() == QColor::Rgb ? QStringLiteral( "rgb" )
                  : stop.colorSpec() == QColor::Hsv ? QStringLiteral( "hsv" )
                  : stop.colorSpec() == QColor::Hsl ? QStringLiteral( "hsl" ) : QString(),
                  stop.direction() == Qgis::AngularDirection::CounterClockwise ? QStringLiteral( "ccw" ) : QStringLiteral( "cw" ) ) );
    }
    map[QStringLiteral( "stops" )] = lst.join( QLatin1Char( ':' ) );
  }

  map[QStringLiteral( "discrete" )] = mDiscrete ? "1" : "0";

  for ( QgsStringMap::const_iterator it = mInfo.constBegin();
        it != mInfo.constEnd(); ++it )
  {
    map["info_" + it.key()] = it.value();
  }

  switch ( mColorSpec )
  {
    case QColor::Rgb:
      map[QStringLiteral( "spec" ) ] = QStringLiteral( "rgb" );
      break;
    case QColor::Hsv:
      map[QStringLiteral( "spec" ) ] = QStringLiteral( "hsv" );
      break;
    case QColor::Hsl:
      map[QStringLiteral( "spec" ) ] = QStringLiteral( "hsl" );
      break;
    case QColor::Cmyk:
    case QColor::Invalid:
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    case QColor::ExtendedRgb:
#endif
      break;
  }

  switch ( mDirection )
  {
    case Qgis::AngularDirection::Clockwise:
      map[QStringLiteral( "direction" ) ] = QStringLiteral( "cw" );
      break;
    case Qgis::AngularDirection::CounterClockwise:
      map[QStringLiteral( "direction" ) ] = QStringLiteral( "ccw" );
      break;
  }

  map[QStringLiteral( "rampType" )] = type();
  return map;
}
void QgsGradientColorRamp::convertToDiscrete( bool discrete )
{
  if ( discrete == mDiscrete )
    return;

  // if going to/from Discrete, re-arrange stops
  // this will only work when stops are equally-spaced
  QgsGradientStopsList newStops;
  if ( discrete )
  {
    // re-arrange stops offset
    int numStops = mStops.count() + 2;
    int i = 1;
    for ( QgsGradientStopsList::const_iterator it = mStops.constBegin();
          it != mStops.constEnd(); ++it )
    {
      newStops.append( QgsGradientStop( static_cast< double >( i ) / numStops, it->color ) );
      if ( i == numStops - 1 )
        break;
      i++;
    }
    // replicate last color
    newStops.append( QgsGradientStop( static_cast< double >( i ) / numStops, mColor2 ) );
  }
  else
  {
    // re-arrange stops offset, remove duplicate last color
    int numStops = mStops.count() + 2;
    int i = 1;
    for ( QgsGradientStopsList::const_iterator it = mStops.constBegin();
          it != mStops.constEnd(); ++it )
    {
      newStops.append( QgsGradientStop( static_cast< double >( i ) / ( numStops - 2 ), it->color ) );
      if ( i == numStops - 3 )
        break;
      i++;
    }
  }
  mStops = newStops;
  mDiscrete = discrete;
}

bool stopLessThan( const QgsGradientStop &s1, const QgsGradientStop &s2 )
{
  return s1.offset < s2.offset;
}

void QgsGradientColorRamp::setStops( const QgsGradientStopsList &stops )
{
  mStops = stops;

  //sort stops by offset
  std::sort( mStops.begin(), mStops.end(), stopLessThan );
}

void QgsGradientColorRamp::addStopsToGradient( QGradient *gradient, double opacity ) const
{
  //copy color ramp stops to a QGradient
  QColor color1 = mColor1;
  QColor color2 = mColor2;
  if ( opacity < 1 )
  {
    color1.setAlpha( color1.alpha() * opacity );
    color2.setAlpha( color2.alpha() * opacity );
  }
  gradient->setColorAt( 0, color1 );
  gradient->setColorAt( 1, color2 );

  double lastOffset = 0;
  for ( const QgsGradientStop &stop : mStops )
  {

    QColor rampColor = stop.color;
    if ( opacity < 1 )
    {
      rampColor.setAlpha( rampColor.alpha() * opacity );
    }
    gradient->setColorAt( stop.offset, rampColor );

    if ( stop.colorSpec() != QColor::Rgb )
    {
      // QGradient only supports RGB interpolation. For other color specs we have
      // to "fake" things by populating the gradient with additional stops
      for ( double offset = lastOffset + 0.05; offset < stop.offset; offset += 0.05 )
      {
        QColor midColor = color( offset );
        if ( opacity < 1 )
        {
          midColor.setAlpha( midColor.alpha() * opacity );
        }
        gradient->setColorAt( offset, midColor );
      }
    }
    lastOffset = stop.offset;
  }

  if ( mColorSpec != QColor::Rgb )
  {
    for ( double offset = lastOffset + 0.05; offset < 1; offset += 0.05 )
    {
      QColor midColor = color( offset );
      if ( opacity < 1 )
      {
        midColor.setAlpha( midColor.alpha() * opacity );
      }
      gradient->setColorAt( offset, midColor );
    }
  }
}

void QgsGradientColorRamp::setColorSpec( QColor::Spec spec )
{
  mColorSpec = spec;
  switch ( mColorSpec )
  {
    case QColor::Rgb:
    case QColor::Invalid:
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    case QColor::ExtendedRgb:
#endif
    case QColor::Cmyk:
      mFunc = _interpolateRgb;
      break;
    case QColor::Hsv:
      mFunc = _interpolateHsv;
      break;
    case QColor::Hsl:
      mFunc = _interpolateHsl;
      break;
  }
}


//////////////


QgsLimitedRandomColorRamp::QgsLimitedRandomColorRamp( int count, int hueMin, int hueMax,
    int satMin, int satMax, int valMin, int valMax )
  : mCount( count )
  , mHueMin( hueMin ), mHueMax( hueMax )
  , mSatMin( satMin ), mSatMax( satMax )
  , mValMin( valMin ), mValMax( valMax )
{
  updateColors();
}

QgsColorRamp *QgsLimitedRandomColorRamp::create( const QVariantMap &props )
{
  int count = DEFAULT_RANDOM_COUNT;
  int hueMin = DEFAULT_RANDOM_HUE_MIN, hueMax = DEFAULT_RANDOM_HUE_MAX;
  int satMin = DEFAULT_RANDOM_SAT_MIN, satMax = DEFAULT_RANDOM_SAT_MAX;
  int valMin = DEFAULT_RANDOM_VAL_MIN, valMax = DEFAULT_RANDOM_VAL_MAX;

  if ( props.contains( QStringLiteral( "count" ) ) ) count = props[QStringLiteral( "count" )].toInt();
  if ( props.contains( QStringLiteral( "hueMin" ) ) ) hueMin = props[QStringLiteral( "hueMin" )].toInt();
  if ( props.contains( QStringLiteral( "hueMax" ) ) ) hueMax = props[QStringLiteral( "hueMax" )].toInt();
  if ( props.contains( QStringLiteral( "satMin" ) ) ) satMin = props[QStringLiteral( "satMin" )].toInt();
  if ( props.contains( QStringLiteral( "satMax" ) ) ) satMax = props[QStringLiteral( "satMax" )].toInt();
  if ( props.contains( QStringLiteral( "valMin" ) ) ) valMin = props[QStringLiteral( "valMin" )].toInt();
  if ( props.contains( QStringLiteral( "valMax" ) ) ) valMax = props[QStringLiteral( "valMax" )].toInt();

  return new QgsLimitedRandomColorRamp( count, hueMin, hueMax, satMin, satMax, valMin, valMax );
}

double QgsLimitedRandomColorRamp::value( int index ) const
{
  if ( mColors.empty() )
    return 0;
  return static_cast< double >( index ) / ( mColors.size() - 1 );
}

QColor QgsLimitedRandomColorRamp::color( double value ) const
{
  if ( value < 0 || value > 1 )
    return QColor();

  int colorCnt = mColors.count();
  int colorIdx = std::min( static_cast< int >( value * colorCnt ), colorCnt - 1 );

  if ( colorIdx >= 0 && colorIdx < colorCnt )
    return mColors.at( colorIdx );

  return QColor();
}

QString QgsLimitedRandomColorRamp::type() const
{
  return QgsLimitedRandomColorRamp::typeString();
}

QgsLimitedRandomColorRamp *QgsLimitedRandomColorRamp::clone() const
{
  return new QgsLimitedRandomColorRamp( mCount, mHueMin, mHueMax, mSatMin, mSatMax, mValMin, mValMax );
}

QVariantMap QgsLimitedRandomColorRamp::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "count" )] = QString::number( mCount );
  map[QStringLiteral( "hueMin" )] = QString::number( mHueMin );
  map[QStringLiteral( "hueMax" )] = QString::number( mHueMax );
  map[QStringLiteral( "satMin" )] = QString::number( mSatMin );
  map[QStringLiteral( "satMax" )] = QString::number( mSatMax );
  map[QStringLiteral( "valMin" )] = QString::number( mValMin );
  map[QStringLiteral( "valMax" )] = QString::number( mValMax );
  map[QStringLiteral( "rampType" )] = type();
  return map;
}

QList<QColor> QgsLimitedRandomColorRamp::randomColors( int count,
    int hueMax, int hueMin, int satMax, int satMin, int valMax, int valMin )
{
  int h, s, v;
  QList<QColor> colors;

  //normalize values
  int safeHueMax = std::max( hueMin, hueMax );
  int safeHueMin = std::min( hueMin, hueMax );
  int safeSatMax = std::max( satMin, satMax );
  int safeSatMin = std::min( satMin, satMax );
  int safeValMax = std::max( valMin, valMax );
  int safeValMin = std::min( valMin, valMax );

  //start hue at random angle
  double currentHueAngle = 360.0 * static_cast< double >( std::rand() ) / RAND_MAX;

  colors.reserve( count );
  for ( int i = 0; i < count; ++i )
  {
    //increment hue by golden ratio (approx 137.507 degrees)
    //as this minimizes hue nearness as count increases
    //see http://basecase.org/env/on-rainbows for more details
    currentHueAngle += 137.50776;
    //scale hue to between hueMax and hueMin
    h = std::clamp( std::round( ( std::fmod( currentHueAngle, 360.0 ) / 360.0 ) * ( safeHueMax - safeHueMin ) + safeHueMin ), 0.0, 359.0 );
    s = std::clamp( ( static_cast<int>( std::rand() ) % ( safeSatMax - safeSatMin + 1 ) ) + safeSatMin, 0, 255 );
    v = std::clamp( ( static_cast<int>( std::rand() ) % ( safeValMax - safeValMin + 1 ) ) + safeValMin, 0, 255 );
    colors.append( QColor::fromHsv( h, s, v ) );
  }
  return colors;
}

void QgsLimitedRandomColorRamp::updateColors()
{
  mColors = QgsLimitedRandomColorRamp::randomColors( mCount, mHueMax, mHueMin, mSatMax, mSatMin, mValMax, mValMin );
}

/////////////

int QgsRandomColorRamp::count() const
{
  return -1;
}

double QgsRandomColorRamp::value( int index ) const
{
  Q_UNUSED( index )
  return 0.0;
}

QColor QgsRandomColorRamp::color( double value ) const
{
  int minVal = 130;
  int maxVal = 255;

  //if value is nan, then use last precalculated color
  if ( std::isnan( value ) )
  {
    value = 1.0;
  }
  // Caller has converted an index into a value in [0.0, 1.0]
  // by doing "index / (mTotalColorCount - 1)"; retrieve the original index.
  int colorIndex = std::round( value * ( mTotalColorCount - 1 ) );
  if ( mTotalColorCount >= 1 && mPrecalculatedColors.length() > colorIndex )
  {
    //use precalculated hue
    return mPrecalculatedColors.at( colorIndex );
  }

  //can't use precalculated hues, use a totally random hue
  int h = static_cast< int >( 360.0 * std::rand() / ( RAND_MAX + 1.0 ) );
  int s = ( std::rand() % ( DEFAULT_RANDOM_SAT_MAX - DEFAULT_RANDOM_SAT_MIN + 1 ) ) + DEFAULT_RANDOM_SAT_MIN;
  int v = ( std::rand() % ( maxVal - minVal + 1 ) ) + minVal;
  return QColor::fromHsv( h, s, v );
}

void QgsRandomColorRamp::setTotalColorCount( const int colorCount )
{
  //calculate colors in advance, so that we can ensure they are more visually distinct than pure random colors
  mPrecalculatedColors.clear();
  mTotalColorCount = colorCount;

  //This works OK for low color counts, but for > 10 or so colors there's still a good chance of
  //similar colors being picked. TODO - investigate alternative "n-visually distinct color" routines

  //random offsets
  double hueOffset = ( 360.0 * std::rand() / ( RAND_MAX + 1.0 ) );

  //try to maximise difference between hues. this is not an ideal implementation, as constant steps
  //through the hue wheel are not visually perceived as constant changes in hue
  //(for instance, we are much more likely to get green hues than yellow hues)
  double hueStep = 359.0 / colorCount;
  double currentHue = hueOffset;

  //build up a list of colors
  for ( int idx = 0; idx < colorCount; ++ idx )
  {
    int h = static_cast< int >( std::round( currentHue ) ) % 360;
    int s = ( std::rand() % ( DEFAULT_RANDOM_SAT_MAX - DEFAULT_RANDOM_SAT_MIN + 1 ) ) + DEFAULT_RANDOM_SAT_MIN;
    int v = ( std::rand() % ( DEFAULT_RANDOM_VAL_MAX - DEFAULT_RANDOM_VAL_MIN + 1 ) ) + DEFAULT_RANDOM_VAL_MIN;
    mPrecalculatedColors << QColor::fromHsv( h, s, v );
    currentHue += hueStep;
  }

  //lastly, shuffle color list
  std::random_device rd;
  std::mt19937 g( rd() );
  std::shuffle( mPrecalculatedColors.begin(), mPrecalculatedColors.end(), g );
}

QString QgsRandomColorRamp::type() const
{
  return QgsRandomColorRamp::typeString();
}

QgsRandomColorRamp *QgsRandomColorRamp::clone() const
{
  return new QgsRandomColorRamp();
}

QVariantMap QgsRandomColorRamp::properties() const
{
  return QVariantMap();
}

////////////

QgsColorBrewerColorRamp::QgsColorBrewerColorRamp( const QString &schemeName, int colors, bool inverted )
  : mSchemeName( schemeName )
  , mColors( colors )
  , mInverted( inverted )
{
  loadPalette();
}

QgsColorRamp *QgsColorBrewerColorRamp::create( const QVariantMap &props )
{
  QString schemeName = DEFAULT_COLORBREWER_SCHEMENAME;
  int colors = DEFAULT_COLORBREWER_COLORS;
  bool inverted = false;

  if ( props.contains( QStringLiteral( "schemeName" ) ) )
    schemeName = props[QStringLiteral( "schemeName" )].toString();
  if ( props.contains( QStringLiteral( "colors" ) ) )
    colors = props[QStringLiteral( "colors" )].toInt();
  if ( props.contains( QStringLiteral( "inverted" ) ) )
    inverted = props[QStringLiteral( "inverted" )].toInt();

  return new QgsColorBrewerColorRamp( schemeName, colors, inverted );
}

void QgsColorBrewerColorRamp::loadPalette()
{
  mPalette = QgsColorBrewerPalette::listSchemeColors( mSchemeName, mColors );

  if ( mInverted )
  {
    QList<QColor> tmpPalette;

    for ( int k = mPalette.size() - 1; k >= 0; k-- )
    {
      tmpPalette << mPalette.at( k );
    }
    mPalette = tmpPalette;
  }
}

QStringList QgsColorBrewerColorRamp::listSchemeNames()
{
  return QgsColorBrewerPalette::listSchemes();
}

QList<int> QgsColorBrewerColorRamp::listSchemeVariants( const QString &schemeName )
{
  return QgsColorBrewerPalette::listSchemeVariants( schemeName );
}

double QgsColorBrewerColorRamp::value( int index ) const
{
  if ( mPalette.empty() )
    return 0;
  return static_cast< double >( index ) / ( mPalette.size() - 1 );
}

QColor QgsColorBrewerColorRamp::color( double value ) const
{
  if ( mPalette.isEmpty() || value < 0 || value > 1 || std::isnan( value ) )
    return QColor();

  int paletteEntry = static_cast< int >( value * mPalette.count() );
  if ( paletteEntry >= mPalette.count() )
    paletteEntry = mPalette.count() - 1;
  return mPalette.at( paletteEntry );
}

void QgsColorBrewerColorRamp::invert()
{
  mInverted = !mInverted;
  loadPalette();
}

QgsColorBrewerColorRamp *QgsColorBrewerColorRamp::clone() const
{
  return new QgsColorBrewerColorRamp( mSchemeName, mColors, mInverted );
}

QVariantMap QgsColorBrewerColorRamp::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "schemeName" )] = mSchemeName;
  map[QStringLiteral( "colors" )] = QString::number( mColors );
  map[QStringLiteral( "inverted" )] = QString::number( mInverted );
  map[QStringLiteral( "rampType" )] = type();
  return map;
}


////////////


QgsCptCityColorRamp::QgsCptCityColorRamp( const QString &schemeName, const QString &variantName,
    bool inverted, bool doLoadFile )
  : QgsGradientColorRamp()
  , mSchemeName( schemeName )
  , mVariantName( variantName )
  , mInverted( inverted )
{
  // TODO replace this with hard-coded data in the default case
  // don't load file if variant is missing
  if ( doLoadFile && ( variantName != QString() || mVariantList.isEmpty() ) )
    loadFile();
}

QgsCptCityColorRamp::QgsCptCityColorRamp( const QString &schemeName, const QStringList &variantList,
    const QString &variantName, bool inverted, bool doLoadFile )
  : QgsGradientColorRamp()
  , mSchemeName( schemeName )
  , mVariantName( variantName )
  , mVariantList( variantList )
  , mInverted( inverted )
{
  mVariantList = variantList;

  // TODO replace this with hard-coded data in the default case
  // don't load file if variant is missing
  if ( doLoadFile && ( variantName != QString() || mVariantList.isEmpty() ) )
    loadFile();
}

QgsColorRamp *QgsCptCityColorRamp::create( const QVariantMap &props )
{
  QString schemeName = DEFAULT_CPTCITY_SCHEMENAME;
  QString variantName = DEFAULT_CPTCITY_VARIANTNAME;
  bool inverted = false;

  if ( props.contains( QStringLiteral( "schemeName" ) ) )
    schemeName = props[QStringLiteral( "schemeName" )].toString();
  if ( props.contains( QStringLiteral( "variantName" ) ) )
    variantName = props[QStringLiteral( "variantName" )].toString();
  if ( props.contains( QStringLiteral( "inverted" ) ) )
    inverted = props[QStringLiteral( "inverted" )].toInt();

  return new QgsCptCityColorRamp( schemeName, variantName, inverted );
}

QString QgsCptCityColorRamp::type() const
{
  return QgsCptCityColorRamp::typeString();
}

void QgsCptCityColorRamp::invert()
{
  mInverted = !mInverted;
  QgsGradientColorRamp::invert();
}

QgsCptCityColorRamp *QgsCptCityColorRamp::clone() const
{
  QgsCptCityColorRamp *ramp = new QgsCptCityColorRamp( QString(), QString(), mInverted, false );
  ramp->copy( this );
  return ramp;
}

void QgsCptCityColorRamp::copy( const QgsCptCityColorRamp *other )
{
  if ( ! other )
    return;
  mColor1 = other->color1();
  mColor2 = other->color2();
  mDiscrete = other->isDiscrete();
  mStops = other->stops();
  mSchemeName = other->mSchemeName;
  mVariantName = other->mVariantName;
  mVariantList = other->mVariantList;
  mFileLoaded = other->mFileLoaded;
  mInverted = other->mInverted;
}

QgsGradientColorRamp *QgsCptCityColorRamp::cloneGradientRamp() const
{
  QgsGradientColorRamp *ramp =
    new QgsGradientColorRamp( mColor1, mColor2, mDiscrete, mStops );
  // add author and copyright information
  // TODO also add COPYING.xml file/link?
  QgsStringMap info = copyingInfo();
  info[QStringLiteral( "cpt-city-gradient" )] = "<cpt-city>/" + mSchemeName + mVariantName + ".svg";
  QString copyingFilename = copyingFileName();
  copyingFilename.remove( QgsCptCityArchive::defaultBaseDir() );
  info[QStringLiteral( "cpt-city-license" )] = "<cpt-city>" + copyingFilename;
  ramp->setInfo( info );
  return ramp;
}


QVariantMap QgsCptCityColorRamp::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "schemeName" )] = mSchemeName;
  map[QStringLiteral( "variantName" )] = mVariantName;
  map[QStringLiteral( "inverted" )] = QString::number( mInverted );
  map[QStringLiteral( "rampType" )] = type();
  return map;
}


QString QgsCptCityColorRamp::fileName() const
{
  if ( mSchemeName.isEmpty() )
    return QString();
  else
  {
    return QgsCptCityArchive::defaultBaseDir() + QDir::separator() + mSchemeName + mVariantName + ".svg";
  }
}

QString QgsCptCityColorRamp::copyingFileName() const
{
  return QgsCptCityArchive::findFileName( QStringLiteral( "COPYING.xml" ), QFileInfo( fileName() ).dir().path(),
                                          QgsCptCityArchive::defaultBaseDir() );
}

QString QgsCptCityColorRamp::descFileName() const
{
  return QgsCptCityArchive::findFileName( QStringLiteral( "DESC.xml" ), QFileInfo( fileName() ).dir().path(),
                                          QgsCptCityArchive::defaultBaseDir() );
}

QgsStringMap QgsCptCityColorRamp::copyingInfo() const
{
  return QgsCptCityArchive::copyingInfo( copyingFileName() );
}

bool QgsCptCityColorRamp::loadFile()
{
  if ( mFileLoaded )
  {
    QgsDebugMsg( "File already loaded for " + mSchemeName + mVariantName );
    return true;
  }

  // get filename
  QString filename = fileName();
  if ( filename.isNull() )
  {
    QgsDebugMsg( "Couldn't get fileName() for " + mSchemeName + mVariantName );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "filename= %1 loaded=%2" ).arg( filename ).arg( mFileLoaded ), 2 );

  // get color ramp from svg file
  QMap< double, QPair<QColor, QColor> > colorMap =
    QgsCptCityArchive::gradientColorMap( filename );

  // add colors to palette
  mFileLoaded = false;
  mStops.clear();
  QMap<double, QPair<QColor, QColor> >::const_iterator it, prev;
  // first detect if file is gradient is continuous or discrete
  // discrete: stop contains 2 colors and first color is identical to previous second
  // multi: stop contains 2 colors and no relation with previous stop
  mDiscrete = false;
  mMultiStops = false;
  it = prev = colorMap.constBegin();
  while ( it != colorMap.constEnd() )
  {
    // look for stops that contain multiple values
    if ( it != colorMap.constBegin() && ( it.value().first != it.value().second ) )
    {
      if ( it.value().first == prev.value().second )
      {
        mDiscrete = true;
        break;
      }
      else
      {
        mMultiStops = true;
        break;
      }
    }
    prev = it;
    ++it;
  }

  // fill all stops
  it = prev = colorMap.constBegin();
  while ( it != colorMap.constEnd() )
  {
    if ( mDiscrete )
    {
      // mPalette << qMakePair( it.key(), it.value().second );
      mStops.append( QgsGradientStop( it.key(), it.value().second ) );
    }
    else
    {
      // mPalette << qMakePair( it.key(), it.value().first );
      mStops.append( QgsGradientStop( it.key(), it.value().first ) );
      if ( ( mMultiStops ) &&
           ( it.key() != 0.0 && it.key() != 1.0 ) )
      {
        mStops.append( QgsGradientStop( it.key(), it.value().second ) );
      }
    }
    prev = it;
    ++it;
  }

  // remove first and last items (mColor1 and mColor2)
  if ( ! mStops.isEmpty() && mStops.at( 0 ).offset == 0.0 )
    mColor1 = mStops.takeFirst().color;
  if ( ! mStops.isEmpty() && mStops.last().offset == 1.0 )
    mColor2 = mStops.takeLast().color;

  if ( mInverted )
  {
    QgsGradientColorRamp::invert();
  }

  mFileLoaded = true;
  return true;
}


//
// QgsPresetColorRamp
//

QgsPresetSchemeColorRamp::QgsPresetSchemeColorRamp( const QList<QColor> &colors )
{
  const auto constColors = colors;
  for ( const QColor &color : constColors )
  {
    mColors << qMakePair( color, color.name() );
  }
  // need at least one color
  if ( mColors.isEmpty() )
    mColors << qMakePair( QColor( 250, 75, 60 ), QStringLiteral( "#fa4b3c" ) );
}

QgsPresetSchemeColorRamp::QgsPresetSchemeColorRamp( const QgsNamedColorList &colors )
  : mColors( colors )
{
  // need at least one color
  if ( mColors.isEmpty() )
    mColors << qMakePair( QColor( 250, 75, 60 ), QStringLiteral( "#fa4b3c" ) );
}

QgsColorRamp *QgsPresetSchemeColorRamp::create( const QVariantMap &properties )
{
  QgsNamedColorList colors;

  int i = 0;
  QString colorString = properties.value( QStringLiteral( "preset_color_%1" ).arg( i ), QString() ).toString();
  QString colorName = properties.value( QStringLiteral( "preset_color_name_%1" ).arg( i ), QString() ).toString();
  while ( !colorString.isEmpty() )
  {
    colors << qMakePair( QgsSymbolLayerUtils::decodeColor( colorString ), colorName );
    i++;
    colorString = properties.value( QStringLiteral( "preset_color_%1" ).arg( i ), QString() ).toString();
    colorName = properties.value( QStringLiteral( "preset_color_name_%1" ).arg( i ), QString() ).toString();
  }

  return new QgsPresetSchemeColorRamp( colors );
}

QList<QColor> QgsPresetSchemeColorRamp::colors() const
{
  QList< QColor > l;
  l.reserve( mColors.count() );
  for ( int i = 0; i < mColors.count(); ++i )
  {
    l << mColors.at( i ).first;
  }
  return l;
}

double QgsPresetSchemeColorRamp::value( int index ) const
{
  if ( mColors.empty() )
    return 0;
  return static_cast< double >( index ) / ( mColors.size() - 1 );
}

QColor QgsPresetSchemeColorRamp::color( double value ) const
{
  if ( value < 0 || value > 1 )
    return QColor();

  int colorCnt = mColors.count();
  int colorIdx = std::min( static_cast< int >( value * colorCnt ), colorCnt - 1 );

  if ( colorIdx >= 0 && colorIdx < colorCnt )
    return mColors.at( colorIdx ).first;

  return QColor();
}

QString QgsPresetSchemeColorRamp::type() const
{
  return QgsPresetSchemeColorRamp::typeString();
}

void QgsPresetSchemeColorRamp::invert()
{
  QgsNamedColorList tmpColors;

  for ( int k = mColors.size() - 1; k >= 0; k-- )
  {
    tmpColors << mColors.at( k );
  }
  mColors = tmpColors;
}

QgsPresetSchemeColorRamp *QgsPresetSchemeColorRamp::clone() const
{
  return new QgsPresetSchemeColorRamp( *this );
}

QVariantMap QgsPresetSchemeColorRamp::properties() const
{
  QVariantMap props;
  for ( int i = 0; i < mColors.count(); ++i )
  {
    props.insert( QStringLiteral( "preset_color_%1" ).arg( i ), QgsSymbolLayerUtils::encodeColor( mColors.at( i ).first ) );
    props.insert( QStringLiteral( "preset_color_name_%1" ).arg( i ), mColors.at( i ).second );
  }
  props[QStringLiteral( "rampType" )] = type();
  return props;
}

int QgsPresetSchemeColorRamp::count() const
{
  return mColors.count();
}

QgsNamedColorList QgsPresetSchemeColorRamp::fetchColors( const QString &, const QColor & )
{
  return mColors;
}
