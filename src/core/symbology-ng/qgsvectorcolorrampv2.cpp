/***************************************************************************
    qgsvectorcolorrampv2.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorcolorrampv2.h"
#include "qgscolorrampv2data.h"
#include "qgscptcityarchive.h"

#include "qgssymbollayerv2utils.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <stdlib.h> // for random()
#include <QTime>

QgsVectorGradientColorRampV2::QgsVectorGradientColorRampV2( QColor color1, QColor color2 )
    : mColor1( color1 ), mColor2( color2 )
{
}

QgsVectorColorRampV2* QgsVectorGradientColorRampV2::create( const QgsStringMap& props )
{
  QColor color1 = DEFAULT_GRADIENT_COLOR1;
  QColor color2 = DEFAULT_GRADIENT_COLOR2;
  if ( props.contains( "color1" ) )
    color1 = QgsSymbolLayerV2Utils::decodeColor( props["color1"] );
  if ( props.contains( "color2" ) )
    color2 = QgsSymbolLayerV2Utils::decodeColor( props["color2"] );

  StopsMap stops;
  if ( props.contains( "stops" ) )
  {
    foreach ( QString stop, props["stops"].split( ':' ) )
    {
      int i = stop.indexOf( ';' );
      if ( i == -1 )
        continue;

      QColor c = QgsSymbolLayerV2Utils::decodeColor( stop.mid( i + 1 ) );
      stops.insert( stop.left( i ).toDouble(), c );
    }
  }

  QgsVectorGradientColorRampV2* r = new QgsVectorGradientColorRampV2( color1, color2 );
  r->setStops( stops );
  return r;
}

static QColor _interpolate( QColor c1, QColor c2, double value )
{
  int r = ( int )( c1.red() + value * ( c2.red() - c1.red() ) );
  int g = ( int )( c1.green() + value * ( c2.green() - c1.green() ) );
  int b = ( int )( c1.blue() + value * ( c2.blue() - c1.blue() ) );
  int a = ( int )( c1.alpha() + value * ( c2.alpha() - c1.alpha() ) );

  return QColor::fromRgb( r, g, b, a );
}

QColor QgsVectorGradientColorRampV2::color( double value ) const
{
  if ( mStops.isEmpty() )
  {
    return _interpolate( mColor1, mColor2, value );
  }
  else
  {
    double lower = 0, upper;
    QColor c1 = mColor1, c2;
    for ( StopsMap::const_iterator it = mStops.begin(); it != mStops.end(); ++it )
    {
      if ( it.key() >= value )
      {
        upper = it.key();
        c2 = it.value();

        return upper == lower ? c1 : _interpolate( c1, c2, ( value - lower ) / ( upper - lower ) );
      }
      lower = it.key();
      c1 = it.value();
    }

    upper = 1;
    c2 = mColor2;
    return upper == lower ? c1 : _interpolate( c1, c2, ( value - lower ) / ( upper - lower ) );
  }
}

QgsVectorColorRampV2* QgsVectorGradientColorRampV2::clone() const
{
  QgsVectorGradientColorRampV2* r = new QgsVectorGradientColorRampV2( mColor1, mColor2 );
  r->setStops( mStops );
  return r;
}

QgsStringMap QgsVectorGradientColorRampV2::properties() const
{
  QgsStringMap map;
  map["color1"] = QgsSymbolLayerV2Utils::encodeColor( mColor1 );
  map["color2"] = QgsSymbolLayerV2Utils::encodeColor( mColor2 );
  if ( !mStops.isEmpty() )
  {
    QStringList lst;
    for ( StopsMap::const_iterator it = mStops.begin(); it != mStops.end(); ++it )
    {
      lst.append( QString( "%1;%2" ).arg( it.key() ).arg( QgsSymbolLayerV2Utils::encodeColor( it.value() ) ) );
    }
    map["stops"] = lst.join( ":" );
  }
  return map;
}

//////////////


QgsVectorRandomColorRampV2::QgsVectorRandomColorRampV2( int count, int hueMin, int hueMax,
    int satMin, int satMax, int valMin, int valMax )
    : mCount( count ), mHueMin( hueMin ), mHueMax( hueMax ),
    mSatMin( satMin ), mSatMax( satMax ), mValMin( valMin ), mValMax( valMax )
{
  updateColors();
}

QgsVectorColorRampV2* QgsVectorRandomColorRampV2::create( const QgsStringMap& props )
{
  int count = DEFAULT_RANDOM_COUNT;
  int hueMin = DEFAULT_RANDOM_HUE_MIN, hueMax = DEFAULT_RANDOM_HUE_MAX;
  int satMin = DEFAULT_RANDOM_SAT_MIN, satMax = DEFAULT_RANDOM_SAT_MAX;
  int valMin = DEFAULT_RANDOM_VAL_MIN, valMax = DEFAULT_RANDOM_VAL_MAX;

  if ( props.contains( "count" ) ) count = props["count"].toInt();
  if ( props.contains( "hueMin" ) ) hueMin = props["hueMin"].toInt();
  if ( props.contains( "hueMax" ) ) hueMax = props["hueMax"].toInt();
  if ( props.contains( "satMin" ) ) satMin = props["satMin"].toInt();
  if ( props.contains( "satMax" ) ) satMax = props["satMax"].toInt();
  if ( props.contains( "valMin" ) ) valMin = props["valMin"].toInt();
  if ( props.contains( "valMax" ) ) valMax = props["valMax"].toInt();

  return new QgsVectorRandomColorRampV2( count, hueMin, hueMax, satMin, satMax, valMin, valMax );
}

QColor QgsVectorRandomColorRampV2::color( double value ) const
{
  int colorCnt = mColors.count();
  int colorIdx = ( int )( value * colorCnt );

  if ( colorIdx >= 0 && colorIdx < colorCnt )
    return mColors.at( colorIdx );

  return QColor();
}

QgsVectorColorRampV2* QgsVectorRandomColorRampV2::clone() const
{
  return new QgsVectorRandomColorRampV2( mCount, mHueMin, mHueMax, mSatMin, mSatMax, mValMin, mValMax );
}

QgsStringMap QgsVectorRandomColorRampV2::properties() const
{
  QgsStringMap map;
  map["count"] = QString::number( mCount );
  map["hueMin"] = QString::number( mHueMin );
  map["hueMax"] = QString::number( mHueMax );
  map["satMin"] = QString::number( mSatMin );
  map["satMax"] = QString::number( mSatMax );
  map["valMin"] = QString::number( mValMin );
  map["valMax"] = QString::number( mValMax );
  return map;
}

void QgsVectorRandomColorRampV2::updateColors()
{
  int h, s, v;

  mColors.clear();
  for ( int i = 0; i < mCount; i++ )
  {
    h = ( rand() % ( mHueMax - mHueMin + 1 ) ) + mHueMin;
    s = ( rand() % ( mSatMax - mSatMin + 1 ) ) + mSatMin;
    v = ( rand() % ( mValMax - mValMin + 1 ) ) + mValMin;
    mColors.append( QColor::fromHsv( h, s, v ) );
  }
}


////////////

QgsVectorColorBrewerColorRampV2::QgsVectorColorBrewerColorRampV2( QString schemeName, int colors )
    : mSchemeName( schemeName ), mColors( colors )
{
  loadPalette();
}

QgsVectorColorRampV2* QgsVectorColorBrewerColorRampV2::create( const QgsStringMap& props )
{
  QString schemeName = DEFAULT_COLORBREWER_SCHEMENAME;
  int colors = DEFAULT_COLORBREWER_COLORS;

  if ( props.contains( "schemeName" ) )
    schemeName = props["schemeName"];
  if ( props.contains( "colors" ) )
    colors = props["colors"].toInt();

  return new QgsVectorColorBrewerColorRampV2( schemeName, colors );
}

void QgsVectorColorBrewerColorRampV2::loadPalette()
{
  mPalette = QgsColorBrewerPalette::listSchemeColors( mSchemeName, mColors );
}

QStringList QgsVectorColorBrewerColorRampV2::listSchemeNames()
{
  return QgsColorBrewerPalette::listSchemes();
}

QList<int> QgsVectorColorBrewerColorRampV2::listSchemeVariants( QString schemeName )
{
  return QgsColorBrewerPalette::listSchemeVariants( schemeName );
}

QColor QgsVectorColorBrewerColorRampV2::color( double value ) const
{
  if ( mPalette.isEmpty() || value < 0 || value > 1 )
    return QColor( 255, 0, 0 ); // red color as a warning :)

  int paletteEntry = ( int )( value * mPalette.count() );
  if ( paletteEntry >= mPalette.count() )
    paletteEntry = mPalette.count() - 1;
  return mPalette.at( paletteEntry );
}

QgsVectorColorRampV2* QgsVectorColorBrewerColorRampV2::clone() const
{
  return new QgsVectorColorBrewerColorRampV2( mSchemeName, mColors );
}

QgsStringMap QgsVectorColorBrewerColorRampV2::properties() const
{
  QgsStringMap map;
  map["schemeName"] = mSchemeName;
  map["colors"] = QString::number( mColors );
  return map;
}


////////////

/*
TODO
- return a suitable name (scheme_variant) ??
- re-organize and rename colorramp classes and widgets
 */

/*
TODO load schemes
- don't show empty dirs? (e.g. jjg/hatch)

- better grouping:
  - cl/fs2????
  - cw
  - dca
  - dirs with one scheme ( cl/ es/ ma/ occ/ wkp/
      jjg/neo10/env/green-crystal jjg/neo10/face/eyes/blue )

- fix rendering:
  - ibcao

*/

QgsCptCityColorRampV2::QgsCptCityColorRampV2( QString schemeName, QString variantName )
    : mSchemeName( schemeName ), mVariantName( variantName ),
    mGradientType( Continuous ), mFileLoaded( false )
{
  // TODO replace this with hard-coded data in the default case
  // don't load file if variant is missing
  if ( variantName != QString() || mVariantList.isEmpty() )
    loadFile();
}

QgsCptCityColorRampV2::QgsCptCityColorRampV2( QString schemeName,  QStringList variantList,
    QString variantName )
    : mSchemeName( schemeName ), mVariantName( variantName ),
    mGradientType( Continuous ), mFileLoaded( false )
{
  mVariantList = variantList;

  // TODO replace this with hard-coded data in the default case
  // don't load file if variant is missing
  if ( variantName != QString() || mVariantList.isEmpty() )
    loadFile();
}

QgsVectorColorRampV2* QgsCptCityColorRampV2::create( const QgsStringMap& props )
{
  QString schemeName = DEFAULT_CPTCITY_SCHEMENAME;
  QString variantName = DEFAULT_CPTCITY_VARIANTNAME;

  if ( props.contains( "schemeName" ) )
    schemeName = props["schemeName"];
  if ( props.contains( "variantName" ) )
    variantName = props["variantName"];

  return new QgsCptCityColorRampV2( schemeName, variantName );
}

QColor QgsCptCityColorRampV2::color( double value ) const
{
  int numStops = mPalette.count();
  if ( mPalette.isEmpty() || value < 0 || value > 1 || numStops < 2 )
    return QColor( 255, 0, 0 ); // red color as a warning :)

  double lower = 0, upper = 0;
  QColor c1, c2;
  c1 = mPalette[0].second;
  for ( int i = 0; i < numStops; i++ )
  {
    if ( mPalette[i].first > value )
    {
      if ( mGradientType == Discrete )
        return c1;

      upper = mPalette[i].first;
      c2 = mPalette[i].second;

      return upper == lower ? c1 : _interpolate( c1, c2, ( value - lower ) / ( upper - lower ) );
    }

    lower = mPalette[i].first;
    c1 = mPalette[i].second;
  }

  if ( mGradientType == Discrete )
    return c1;

  upper = 1;
  c2 = mPalette[ numStops - 1 ].second;

  return upper == lower ? c1 : _interpolate( c1, c2, ( value - lower ) / ( upper - lower ) );
}

QgsVectorColorRampV2* QgsCptCityColorRampV2::clone() const
{
  return new QgsCptCityColorRampV2( mSchemeName, mVariantName );
}

QgsStringMap QgsCptCityColorRampV2::properties() const
{
  QgsStringMap map;
  map["schemeName"] = mSchemeName;
  map["variantName"] = mVariantName;
  return map;
}


QString QgsCptCityColorRampV2::fileName() const
{
  if ( mSchemeName == "" )
    return QString();
  else
  {
    return QgsCptCityArchive::defaultBaseDir() + QDir::separator() + mSchemeName + mVariantName + ".svg";
  }
}

QString QgsCptCityColorRampV2::copyingFileName() const
{
  return QgsCptCityArchive::findFileName( "COPYING.xml", QFileInfo( fileName() ).dir().path(),
                                          QgsCptCityArchive::defaultBaseDir() );
}

QString QgsCptCityColorRampV2::descFileName() const
{
  return QgsCptCityArchive::findFileName( "DESC.xml", QFileInfo( fileName() ).dir().path(),
                                          QgsCptCityArchive::defaultBaseDir() );
}

QMap< QString, QString > QgsCptCityColorRampV2::copyingInfo( ) const
{
  return QgsCptCityArchive::copyingInfo( copyingFileName() );
}

bool QgsCptCityColorRampV2::loadFile()
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

  QgsDebugMsg( QString( "filename= %1 loaded=%2" ).arg( filename ).arg( mFileLoaded ) );

  // get color ramp from svg file
  QMap< double, QPair<QColor, QColor> > colorMap =
    QgsCptCityArchive::gradientColorMap( filename );

  // add colors to palette
  mFileLoaded = false;
  mPalette.clear();
  QMap<double, QPair<QColor, QColor> >::const_iterator it, prev;
  // first detect if file is gradient is continuous or dicrete
  // discrete: stop contains 2 colors and first color is identical to previous second
  // multi: stop contains 2 colors and no relation with previous stop
  mGradientType = Continuous;
  it = prev = colorMap.constBegin();
  while ( it != colorMap.constEnd() )
  {
    // look for stops that contain multiple values
    if ( it != colorMap.constBegin() && ( it.value().first != it.value().second ) )
    {
      if ( it.value().first == prev.value().second )
      {
        mGradientType = Discrete;
        break;
      }
      else
      {
        mGradientType = ContinuousMulti;
        break;
      }
    }
    prev = it;
    ++it;
  }

  it = prev = colorMap.constBegin();
  while ( it != colorMap.constEnd() )
  {
    if ( mGradientType == Discrete )
    {
      mPalette << qMakePair( it.key(), it.value().second );
    }
    else
    {
      mPalette << qMakePair( it.key(), it.value().first );
      if (( mGradientType == ContinuousMulti ) &&
          ( it.key() != 0.0 && it.key() != 1.0 ) )
      {
        mPalette << qMakePair( it.key(), it.value().second );
      }
    }
    prev = it;
    ++it;
  }

  mFileLoaded = true;
  return true;
}

