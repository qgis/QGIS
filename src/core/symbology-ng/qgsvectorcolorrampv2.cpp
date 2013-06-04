/***************************************************************************
    qgsvectorcolorrampv2.cpp
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

#include "qgsvectorcolorrampv2.h"
#include "qgscolorbrewerpalette.h"
#include "qgscptcityarchive.h"

#include "qgssymbollayerv2utils.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <stdlib.h> // for random()
#include <QTime>

//////////////

static QColor _interpolate( QColor c1, QColor c2, double value )
{
  int r = ( int )( c1.red() + value * ( c2.red() - c1.red() ) );
  int g = ( int )( c1.green() + value * ( c2.green() - c1.green() ) );
  int b = ( int )( c1.blue() + value * ( c2.blue() - c1.blue() ) );
  int a = ( int )( c1.alpha() + value * ( c2.alpha() - c1.alpha() ) );

  return QColor::fromRgb( r, g, b, a );
}

//////////////

QgsVectorGradientColorRampV2::QgsVectorGradientColorRampV2( QColor color1, QColor color2,
    bool discrete, QgsGradientStopsList stops )
    : mColor1( color1 ), mColor2( color2 ), mDiscrete( discrete ), mStops( stops )
{
}

QgsVectorColorRampV2* QgsVectorGradientColorRampV2::create( const QgsStringMap& props )
{
  // color1 and color2
  QColor color1 = DEFAULT_GRADIENT_COLOR1;
  QColor color2 = DEFAULT_GRADIENT_COLOR2;
  if ( props.contains( "color1" ) )
    color1 = QgsSymbolLayerV2Utils::decodeColor( props["color1"] );
  if ( props.contains( "color2" ) )
    color2 = QgsSymbolLayerV2Utils::decodeColor( props["color2"] );

  //stops
  QgsGradientStopsList stops;
  if ( props.contains( "stops" ) )
  {
    foreach ( QString stop, props["stops"].split( ':' ) )
    {
      int i = stop.indexOf( ';' );
      if ( i == -1 )
        continue;

      QColor c = QgsSymbolLayerV2Utils::decodeColor( stop.mid( i + 1 ) );
      stops.append( QgsGradientStop( stop.left( i ).toDouble(), c ) );
    }
  }

  // discrete vs. continuous
  bool discrete = false;
  if ( props.contains( "discrete" ) )
  {
    if ( props["discrete"] == "1" )
      discrete = true;
  }

  // search for information keys starting with "info_"
  QgsStringMap info;
  for ( QgsStringMap::const_iterator it = props.constBegin();
        it != props.constEnd(); ++it )
  {
    if ( it.key().startsWith( "info_" ) )
      info[ it.key().mid( 5 )] = it.value();
  }

  QgsVectorGradientColorRampV2* r = new QgsVectorGradientColorRampV2( color1, color2, discrete, stops );
  r->setInfo( info );
  return r;
}

double QgsVectorGradientColorRampV2::value( int index ) const
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
    return mStops[index-1].offset;
  }
}

QColor QgsVectorGradientColorRampV2::color( double value ) const
{
  if ( mStops.isEmpty() )
  {
    if ( mDiscrete )
      return mColor1;
    return _interpolate( mColor1, mColor2, value );
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

        return upper == lower ? c1 : _interpolate( c1, c2, ( value - lower ) / ( upper - lower ) );
      }
      lower = it->offset;
      c1 = it->color;
    }

    if ( mDiscrete )
      return c1;

    upper = 1;
    c2 = mColor2;
    return upper == lower ? c1 : _interpolate( c1, c2, ( value - lower ) / ( upper - lower ) );
  }
}

QgsVectorColorRampV2* QgsVectorGradientColorRampV2::clone() const
{
  QgsVectorGradientColorRampV2* r = new QgsVectorGradientColorRampV2( mColor1, mColor2,
      mDiscrete, mStops );
  r->setInfo( mInfo );
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
    for ( QgsGradientStopsList::const_iterator it = mStops.begin(); it != mStops.end(); ++it )
    {
      lst.append( QString( "%1;%2" ).arg( it->offset ).arg( QgsSymbolLayerV2Utils::encodeColor( it->color ) ) );
    }
    map["stops"] = lst.join( ":" );
  }

  map["discrete"] = mDiscrete ? "1" : "0";

  for ( QgsStringMap::const_iterator it = mInfo.constBegin();
        it != mInfo.constEnd(); ++it )
  {
    map["info_" + it.key()] = it.value();
  }

  return map;
}
void QgsVectorGradientColorRampV2::convertToDiscrete( bool discrete )
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
    for ( QgsGradientStopsList::const_iterator it = mStops.begin();
          it != mStops.end(); ++it )
    {
      newStops.append( QgsGradientStop(( double ) i / numStops, it->color ) );
      if ( i == numStops - 1 )
        break;
      i++;
    }
    // replicate last color
    newStops.append( QgsGradientStop(( double ) i / numStops, mColor2 ) );
  }
  else
  {
    // re-arrange stops offset, remove duplicate last color
    int numStops = mStops.count() + 2;
    int i = 1;
    for ( QgsGradientStopsList::const_iterator it = mStops.begin();
          it != mStops.end(); ++it )
    {
      newStops.append( QgsGradientStop(( double ) i / ( numStops - 2 ), it->color ) );
      if ( i == numStops - 3 )
        break;
      i++;
    }
  }
  mStops = newStops;
  mDiscrete = discrete;
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

double QgsVectorRandomColorRampV2::value( int index ) const
{
  if ( mColors.size() < 1 ) return 0;
  return index / mColors.size() - 1;
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
  //start hue at random angle
  double currentHueAngle = 360.0 * ( double )rand() / RAND_MAX;

  for ( int i = 0; i < mCount; i++ )
  {
    //increment hue by golden ratio (approx 137.507 degrees)
    //as this minimises hue nearness as count increases
    //see http://basecase.org/env/on-rainbows for more details
    currentHueAngle += 137.50776;
    //scale hue to between mHueMax and mHueMin
    h = ( fmod( currentHueAngle, 360.0 ) / 360.0 ) * ( mHueMax - mHueMin ) + mHueMin;
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

double QgsVectorColorBrewerColorRampV2::value( int index ) const
{
  if ( mPalette.size() < 1 ) return 0;
  return index / mPalette.size() - 1;
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


QgsCptCityColorRampV2::QgsCptCityColorRampV2( QString schemeName, QString variantName,
    bool doLoadFile )
    : QgsVectorGradientColorRampV2(),
    mSchemeName( schemeName ), mVariantName( variantName ),
    mVariantList( QStringList() ), mFileLoaded( false ), mMultiStops( false )
{
  // TODO replace this with hard-coded data in the default case
  // don't load file if variant is missing
  if ( doLoadFile && ( variantName != QString() || mVariantList.isEmpty() ) )
    loadFile();
}

QgsCptCityColorRampV2::QgsCptCityColorRampV2( QString schemeName,  QStringList variantList,
    QString variantName, bool doLoadFile )
    : QgsVectorGradientColorRampV2(),
    mSchemeName( schemeName ), mVariantName( variantName ),
    mVariantList( variantList ), mFileLoaded( false ), mMultiStops( false )
{
  mVariantList = variantList;

  // TODO replace this with hard-coded data in the default case
  // don't load file if variant is missing
  if ( doLoadFile && ( variantName != QString() || mVariantList.isEmpty() ) )
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

QgsVectorColorRampV2* QgsCptCityColorRampV2::clone() const
{
  QgsCptCityColorRampV2* ramp = new QgsCptCityColorRampV2( "", "", false );
  ramp->copy( this );
  return ramp;
}

void QgsCptCityColorRampV2::copy( const QgsCptCityColorRampV2* other )
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
}

QgsVectorGradientColorRampV2* QgsCptCityColorRampV2::cloneGradientRamp() const
{
  QgsVectorGradientColorRampV2* ramp =
    new QgsVectorGradientColorRampV2( mColor1, mColor2, mDiscrete, mStops );
  // add author and copyright information
  // TODO also add COPYING.xml file/link?
  QgsStringMap info = copyingInfo();
  info["cpt-city-gradient"] = "<cpt-city>/" + mSchemeName + mVariantName + ".svg";
  QString copyingFilename = copyingFileName();
  copyingFilename.remove( QgsCptCityArchive::defaultBaseDir() );
  info["cpt-city-license"] = "<cpt-city>" + copyingFilename;
  ramp->setInfo( info );
  return ramp;
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

QgsStringMap QgsCptCityColorRampV2::copyingInfo( ) const
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
  mStops.clear();
  QMap<double, QPair<QColor, QColor> >::const_iterator it, prev;
  // first detect if file is gradient is continuous or dicrete
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
      if (( mMultiStops ) &&
          ( it.key() != 0.0 && it.key() != 1.0 ) )
      {
        mStops.append( QgsGradientStop( it.key(), it.value().second ) );
      }
    }
    prev = it;
    ++it;
  }

  // remove first and last items (mColor1 and mColor2)
  if ( ! mStops.isEmpty() && mStops.first().offset == 0.0 )
    mColor1 = mStops.takeFirst().color;
  if ( ! mStops.isEmpty() && mStops.last().offset == 1.0 )
    mColor2 = mStops.takeLast().color;

  mFileLoaded = true;
  return true;
}

