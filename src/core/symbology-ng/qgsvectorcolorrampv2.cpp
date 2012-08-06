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

#include "qgssymbollayerv2utils.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <stdlib.h> // for random()

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
- return a suitable name (scheme_variant)
- use model/view for the treewidget
- fix ocal file parsing
- re-organize and rename colorramp classes and widgets
 */
QString QgsCptCityColorRampV2::mBaseDir;
QStringList QgsCptCityColorRampV2::mCollections;
QMap< QString, QStringList > QgsCptCityColorRampV2::mSchemeMap;
// QMap< QString, int > QgsCptCityColorRampV2::mSchemeNumColors;
QMap< QString, QStringList > QgsCptCityColorRampV2::mSchemeVariants;
QMap< QString, QString > QgsCptCityColorRampV2::mCollectionNames;
QMap< QString, QStringList > QgsCptCityColorRampV2::mCollectionSelections;

QgsCptCityColorRampV2::QgsCptCityColorRampV2( QString schemeName, QString variantName )
    : mSchemeName( schemeName ), mVariantName( variantName ), mGradientType( Continuous )
{
  // TODO replace this with hard-coded data in the default case
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


#if 0
QColor QgsCptCityColorRampV2::color( double value ) const
{
  if ( mPalette.isEmpty() || value < 0 || value > 1 )
    return QColor( 255, 0, 0 ); // red color as a warning :)

  if ( ! mContinuous )
  {
    int paletteEntry = ( int )( value * mPalette.count() );
    if ( paletteEntry >= mPalette.count() )
      paletteEntry = mPalette.count() - 1;

    return mPalette.at( paletteEntry );
  }
  else
  {
    int numStops = mPalette.count();
    if ( numStops < 2 )
      return QColor( 255, 0, 0 ); // red color as a warning :)

    StopsMap mStops; // TODO integrate in main class
    for ( int i = 0; i < numStops; i++ )
      mStops[ mPaletteStops[i] ] = mPalette[i];

    double lower = 0, upper;
    QColor c1 = mPalette[1], c2;
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
    c2 = mPalette[ numStops - 1 ];
    return upper == lower ? c1 : _interpolate( c1, c2, ( value - lower ) / ( upper - lower ) );
  }
}
#endif

QColor QgsCptCityColorRampV2::color( double value ) const
{
  if ( mPalette.isEmpty() || value < 0 || value > 1 )
    return QColor( 255, 0, 0 ); // red color as a warning :)

  int numStops = mPalette.count();
  if ( numStops < 2 )
    return QColor( 255, 0, 0 ); // red color as a warning :)

  double lower = 0, upper = 0;
  QColor c1, c2;
  c1 = mPalette[0].second;
  for ( int i = 0; i < numStops; i++ )
  {
    if ( mPalette[i].first >= value )
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

QStringList QgsCptCityColorRampV2::listSchemeCollections( QString collectionName, bool recursive )
{
  QDir dir = QDir( QgsCptCityColorRampV2::getBaseDir() + "/" + collectionName );
  if ( ! dir.exists() )
    return QStringList();

  QStringList entries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  if ( collectionName != "" )
  {
    for ( int i = 0; i < entries.count(); i++ )
    {
      entries[i] = collectionName + "/" + entries[i];
    }
  }

  // recurse
  if ( recursive )
  {
    QStringList entries2 = entries;
    foreach ( QString entry, entries2 )
    {
      entries.append( listSchemeCollections( entry, true ) );
    }
  }

  return entries;
}

QStringList QgsCptCityColorRampV2::listSchemeNames( QString collectionName )
{
  QDir dir = QDir( QgsCptCityColorRampV2::getBaseDir() + "/" + collectionName );
  if ( ! dir.exists() )
  {
    QgsDebugMsg( "dir " + dir.dirName() + " does not exist" );
    return QStringList();
  }

  QStringList entries = dir.entryList( QStringList( "*.svg" ), QDir::Files, QDir::Name );
  for ( int i = 0; i < entries.count(); i++ )
    entries[i] = entries[i].left( entries[i].length() - 4 );
  return entries;
}

QString QgsCptCityColorRampV2::getBaseDir()
{
  // currently hard-coded, but could be also in QGis install path and/or configurable
  if ( mBaseDir.isNull() )
    return QgsApplication::qgisSettingsDirPath() + "/" + "cpt-city";
  else
    return mBaseDir;
}

QString QgsCptCityColorRampV2::getFilename() const
{
  if ( mSchemeName == "" )
    return QString();
  else
    return QgsCptCityColorRampV2::getBaseDir() + "/" + mSchemeName +  mVariantName + ".svg";
}

bool QgsCptCityColorRampV2::loadFile( QString filename )
{
  if ( filename == "" )
  {
    filename = getFilename();
    if ( filename.isNull() )
      return false;
  }

  // QgsDebugMsg("filename= "+filename);

  mPalette.clear();

  QString mErrorString = QString();

  // import xml file
  QFile f( filename );
  if ( !f.open( QFile::ReadOnly ) )
  {
    mErrorString = "Couldn't open the SVG file: " + filename;
    QgsDebugMsg( mErrorString );
    return false;
  }

  // parse the document
  QDomDocument doc( "gradient" );
  if ( !doc.setContent( &f ) )
  {
    mErrorString = "Couldn't parse the SVG file: " + filename;
    f.close();
    QgsDebugMsg( mErrorString );
    return false;
  }
  f.close();

  QDomElement docElem = doc.documentElement();

  if ( docElem.tagName() != "svg" )
  {
    mErrorString = "Incorrect root tag: " + docElem.tagName();
    QgsDebugMsg( mErrorString );
    return false;
  }

  // load color ramp
  QDomElement rampsElement = docElem.firstChildElement( "linearGradient" );
  if ( rampsElement.isNull() )
  {
    mErrorString = "linearGradient tag missing";
    QgsDebugMsg( mErrorString );
    return false;
  }

  // loop for all stop tags
  QDomElement e = rampsElement.firstChildElement();
  QMap< double, QPair<QColor, QColor> > map;

  QColor prevColor;
  while ( !e.isNull() )
  {
    if ( e.tagName() == "stop" )
    {
      //todo integrate this into symbollayerutils, keep here for now...
      double offset;
      QString offsetStr = e.attribute( "offset" ); // offset="50.00%" | offset="0.5"
      QString colorStr = e.attribute( "stop-color", "" ); // stop-color="rgb(222,235,247)"
      QString opacityStr = e.attribute( "stop-opacity", "1.0" ); // stop-opacity="1.0000"
      if ( offsetStr.endsWith( "%" ) )
        offset = offsetStr.remove( offsetStr.size() - 1, 1 ).toDouble() / 100.0;
      else
        offset = offsetStr.toDouble();

      // QColor color( 255, 0, 0 ); // red color as a warning :)
      QColor color = QgsSymbolLayerV2Utils::parseColor( colorStr );
      if ( color != QColor() )
      {
        int alpha = opacityStr.toDouble() * 255; // test
        color.setAlpha( alpha );
        if ( map.contains( offset ) )
          map[offset].second = color;
        else
          map[offset] = qMakePair( color, color );
      }
      else
        QgsDebugMsg( QString( "at offset=%1 invalid color" ).arg( offset ) );
    }
    else
    {
      QgsDebugMsg( "unknown tag: " + e.tagName() );
    }

    e = e.nextSiblingElement();
  }

  // add colors to palette
  mPalette.clear();
  QMap<double, QPair<QColor, QColor> >::const_iterator it, prev;
  // first detect if file is gradient is continuous or dicrete
  // discrete: stop contains 2 colors and first color is identical to previous second
  // multi: stop contains 2 colors and no relation with previous stop
  mGradientType = Continuous;
  it = prev = map.constBegin();
  while ( it != map.constEnd() )
  {
    // look for stops that contain multiple values
    if ( it != map.constBegin() && ( it.value().first != it.value().second ) )
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

  it = prev = map.constBegin();
  while ( it != map.constEnd() )
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

  return true;
}

bool QgsCptCityColorRampV2::hasBasicSchemes()
{
  // Currently returns hasAllSchemes, because we don't have a minimal set yet
  return hasAllSchemes();
}

bool QgsCptCityColorRampV2::hasAllSchemes()
{
  // if we have no collections loaded yet, just make sure we have at least one collection
  // ideally we should test for a few schemes we know should be present
  if ( mCollections.isEmpty() )
  {
    return ( !  QgsCptCityColorRampV2::listSchemeCollections( "", false ).isEmpty() );
  }
  return true;
}

// currently this methos takes some time, so it must be explicitly requested
bool QgsCptCityColorRampV2::loadSchemes( QString rootDir, bool reset )
{
  // TODO should keep the name of the previously loaded, or see if the first element is inside rootDir
  if ( ! reset && ! mCollections.isEmpty() )
  {
    QgsDebugMsg( QString( "not loading schemes, rootDir=%1 reset=%2 empty=%3" ).arg( rootDir ).arg( reset ).arg( mCollections.isEmpty() ) );
    return true;
  }

  QString curName, prevName, prevPath, curVariant, curSep, schemeName;
  QStringList listVariant;
  QStringList schemeNamesAll, schemeNames;
  int num;
  bool ok, prevAdd, curAdd;

  if ( reset )
  {
    mCollections.clear();
    mSchemeMap.clear();
    // mSchemeNumColors.clear();
    mSchemeVariants.clear();
    mCollectionNames.clear();
    mCollectionSelections.clear();
  }

  mCollections = listSchemeCollections( rootDir, true );
  qSort( mCollections.begin(), mCollections.end() );

  foreach ( QString path, mCollections )
  {
    // QgsDebugMsg("================================");
    // QgsDebugMsg("collection = "+path);
    schemeNamesAll = QgsCptCityColorRampV2::listSchemeNames( path );
    // TODO detect if there are duplicate names with different variant counts, combine in 1
    for ( int i = 0; i < schemeNamesAll.count(); i++ )
    {
      // schemeName = QFileInfo( schemeNamesAll[i] ).baseName();
      schemeName = schemeNamesAll[i];
      // QgsDebugMsg("=============");
      // QgsDebugMsg("scheme = "+schemeName);
      curName = schemeName;
      curVariant = "";

      // stupid code to find if name ends with 1-3 digit number - should use regexp
      // TODO need to detect if ends with b/c also
      if ( schemeName.length() > 1 && schemeName.endsWith( "a" ) && ! listVariant.isEmpty() &&
           (( prevName + listVariant.last()  + "a" ) == curName ) )
      {
        curName = prevName;
        curVariant = listVariant.last() + "a";
      }
      else
      {
        num = schemeName.right( 3 ).toInt( &ok );
        Q_UNUSED( num );
        if ( ok )
        {
          curName = schemeName.left( schemeName.size() - 3 );
          curVariant = schemeName.right( 3 );
        }
        else
        {
          num = schemeName.right( 2 ).toInt( &ok );
          if ( ok )
          {
            curName = schemeName.left( schemeName.size() - 2 );
            curVariant = schemeName.right( 2 );
          }
          else
          {
            num = schemeName.right( 1 ).toInt( &ok );
            if ( ok )
            {
              curName = schemeName.left( schemeName.size() - 1 );
              curVariant = schemeName.right( 1 );
            }
          }
        }
      }
      curSep = curName.right( 1 );
      if ( curSep == "-" || curSep == "_" )
      {
        curName.chop( 1 );
        curVariant = curSep + curVariant;
      }

      if ( prevName == "" )
        prevName = curName;

      // add element, unless it is empty, or a variant of last element
      prevAdd = false;
      curAdd = false;
      if ( curName == "" )
        curName = "__empty__";
      // if current is a variant of last, don't add previous and append current variant
      if ( curName == prevName )
      {
        // add current element if it is the last one in the collection
        if ( i == schemeNamesAll.count() - 1 )
          prevAdd = true;
        listVariant << curVariant;
      }
      else
      {
        if ( prevName != "" )
        {
          prevAdd = true;
        }
        // add current element if it is the last one in the collection
        if ( i == schemeNamesAll.count() - 1 )
          curAdd = true;
      }

      // QgsDebugMsg(QString("prevAdd=%1 curAdd=%2 prevName=%3 curName=%4 count=%5").arg(prevAdd).arg(curAdd).arg(prevName).arg(curName).arg(listVariant.count()));

      if ( prevAdd )
      {
        // depending on number of variants, make one or more items
        if ( listVariant.count() == 0 )
        {
          // set num colors=-1 to parse file on request only
          // mSchemeNumColors[ prevName ] = -1;
          schemeNames << prevName;
        }
        else if ( listVariant.count() <= 2 )
        {
          // for 1-2 items, create independent items
          for ( int j = 0; j < listVariant.count(); j++ )
          {
            // mSchemeNumColors[ prevName + listVariant[j] ] = -1;
            schemeNames << prevName + listVariant[j];
          }
        }
        else
        {
          mSchemeVariants[ path + "/" + prevName ] = listVariant;
          schemeNames << prevName;
        }
        listVariant.clear();
      }
      if ( curAdd )
      {
        if ( curVariant != "" )
          curName += curVariant;
        schemeNames << curName;
      }
      // save current to compare next
      if ( prevAdd || curAdd )
      {
        prevName = curName;
        if ( curVariant != "" )
          listVariant << curVariant;
      }

    }
    // add schemes to collection
    mSchemeMap[ path ] = schemeNames;
    schemeNames.clear();
    listVariant.clear();
    prevName = "";
  }

  // populate mCollectionNames
  foreach ( QString path, mCollections )
  {
    QString filename = QgsCptCityColorRampV2::getBaseDir() + "/" + path + "/" + "DESC.xml";
    QFile f( filename );
    if ( ! f.open( QFile::ReadOnly ) )
    {
      QgsDebugMsg( "description file for path " + path + " [ " + filename + " ] does not exist" );
      continue;
    }

    // parse the document
    QString errMsg;
    QDomDocument doc( "description" );
    if ( !doc.setContent( &f, &errMsg ) )
    {
      f.close();
      QgsDebugMsg( "Couldn't parse file " + filename + " : " + errMsg );
      continue;
    }
    f.close();

    // read description
    QDomElement docElem = doc.documentElement();
    if ( docElem.tagName() != "description" )
    {
      QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
      continue;
    }
    // should we make sure the <dir> tag is ok?
    QDomElement nameElement = docElem.firstChildElement( "name" );
    if ( nameElement.isNull() )
    {
      QgsDebugMsg( "name tag missing" );
      continue;
    }

    // add info to mapping
    mCollectionNames[ path ] = nameElement.text();
  }
  // add any elements that are missing from DESC.xml (views)
  for ( int i = 0; cptCityNames[i] != NULL; i = i + 2 )
  {
    mCollectionNames[ cptCityNames[i] ] = cptCityNames[i+1];
  }

  // populate mCollectionSelections
  QString viewName;
  for ( int i = 0; cptCitySelections[i] != NULL; i++ )
  {
    curName = QString( cptCitySelections[i] );
    if ( curName == "" )
    {
      viewName = QString( cptCitySelections[i+1] );
      curName = QString( cptCitySelections[i+2] );
      i = i + 2;
    }
    mCollectionSelections[ viewName ] << curName;
  }

  return ( ! mCollections.isEmpty() );
}

