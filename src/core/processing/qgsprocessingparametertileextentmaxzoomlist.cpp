/***************************************************************************
  qgsprocessingparametertileextentmaxzoomlist.cpp
  ---------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparametertileextentmaxzoomlist.h"

#include "qgsprocessingcontext.h"

#include <QString>

using namespace Qt::StringLiterals;

//
// QgsProcessingParameterTileExtentMaxZoomList
//
QgsProcessingParameterTileExtentMaxZoomList::QgsProcessingParameterTileExtentMaxZoomList( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{}

QgsProcessingParameterTileExtentMaxZoomList *QgsProcessingParameterTileExtentMaxZoomList::clone() const
{
  return new QgsProcessingParameterTileExtentMaxZoomList( *this );
}

QString QgsProcessingParameterTileExtentMaxZoomList::type() const
{
  return typeName();
}

bool QgsProcessingParameterTileExtentMaxZoomList::checkValueIsAcceptable( const QVariant &value, QgsProcessingContext * ) const
{
  QVariant input = value;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.typeId() == QMetaType::Type::QVariantList )
  {
    const QVariantList list = input.toList();
    for ( const QVariant &item : list )
    {
      if ( item.typeId() != QMetaType::Type::QVariantMap )
      {
        return false;
      }

      const QVariantMap map = item.toMap();
      if ( !map.contains( u"extent"_s ) || !map.contains( u"max_zoom"_s ) )
      {
        return false;
      }
    }
    return true;
  }
  else if ( input.typeId() == QMetaType::Type::QString )
  {
    const QStringList regionStrings = input.toString().split( "::|::"_L1 );
    QList<QgsTileExtentMaxZoomRegion> regions;
    regions.reserve( regionStrings.size() );
    for ( const QString &regionString : regionStrings )
    {
      const int sepPos = regionString.indexOf( ':' );
      if ( sepPos < 0 )
        return false;

      const QString zoomPart = regionString.left( sepPos );
      bool ok = false;
      ( void ) zoomPart.toInt( &ok );
      if ( !ok )
        return false;

      const QString extentPart = regionString.mid( sepPos + 1 );
      if ( extentPart.isEmpty() )
        return false;
    }
    return true;
  }

  return false;
}

QString QgsProcessingParameterTileExtentMaxZoomList::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return u"None"_s;

  const QList<QgsTileExtentMaxZoomRegion> regions = parameterAsRegionList( value, context );
  QStringList listItems;
  listItems.reserve( regions.size() );
  for ( const QgsTileExtentMaxZoomRegion &region : regions )
  {
    const QString regionExtentString = u"'%1, %3, %2, %4 [%5]'"_s.arg(
      qgsDoubleToString( region.extent.xMinimum() ),
      qgsDoubleToString( region.extent.yMinimum() ),
      qgsDoubleToString( region.extent.xMaximum() ),
      qgsDoubleToString( region.extent.yMaximum() ),
      region.extent.crs().authid()
    );
    listItems.append( u"{'extent': %1, 'max_zoom': %2}"_s.arg( regionExtentString ).arg( region.maxZoom ) );
  }

  return u"[%1]"_s.arg( listItems.join( ", "_L1 ) );
}

QString QgsProcessingParameterTileExtentMaxZoomList::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = true;
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  const QList<QgsTileExtentMaxZoomRegion> regions = parameterAsRegionList( value, context );
  QStringList stringListItems;
  stringListItems.reserve( regions.size() );
  for ( const QgsTileExtentMaxZoomRegion &region : regions )
  {
    const QString regionExtentString = u"%1,%3,%2,%4 [%5]"_s.arg(
      qgsDoubleToString( region.extent.xMinimum() ),
      qgsDoubleToString( region.extent.yMinimum() ),
      qgsDoubleToString( region.extent.xMaximum() ),
      qgsDoubleToString( region.extent.yMaximum() ),
      region.extent.crs().authid()
    );
    stringListItems.append( u"%1:%2"_s.arg( region.maxZoom ).arg( regionExtentString ) );
  }

  return stringListItems.join( "::|::"_L1 );
}

QVariant QgsProcessingParameterTileExtentMaxZoomList::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QVariant();

  const QList<QgsTileExtentMaxZoomRegion> regions = parameterAsRegionList( value, context );
  QVariantList jsonListItems;
  jsonListItems.reserve( regions.size() );
  for ( const QgsTileExtentMaxZoomRegion &region : regions )
  {
    const QString regionExtentString = u"'%1,%3,%2,%4 [%5]'"_s.arg(
      qgsDoubleToString( region.extent.xMinimum() ),
      qgsDoubleToString( region.extent.yMinimum() ),
      qgsDoubleToString( region.extent.xMaximum() ),
      qgsDoubleToString( region.extent.yMaximum() ),
      region.extent.crs().authid()
    );
    QVariantMap regionMap;
    regionMap[u"extent"_s] = regionExtentString;
    regionMap[u"max_zoom"_s] = region.maxZoom;
    jsonListItems.append( regionMap );
  }

  return jsonListItems;
}

QList<QgsTileExtentMaxZoomRegion> QgsProcessingParameterTileExtentMaxZoomList::parameterAsRegionList( const QVariant &variant, QgsProcessingContext &context ) const
{
  if ( QgsVariantUtils::isNull( variant ) )
    return {};

  if ( variant.typeId() == QMetaType::Type::QVariantList )
  {
    QList<QgsTileExtentMaxZoomRegion> regions;
    const QVariantList list = variant.toList();
    regions.reserve( list.size() );

    for ( const QVariant &item : list )
    {
      if ( item.typeId() == QMetaType::Type::QVariantMap )
      {
        regions.append( regionFromVariantMap( item.toMap(), context ) );
      }
    }
    return regions;
  }
  else if ( variant.typeId() == QMetaType::Type::QString )
  {
    const QStringList regionStrings = variant.toString().split( "::|::"_L1 );
    QList<QgsTileExtentMaxZoomRegion> regions;
    regions.reserve( regionStrings.size() );
    for ( const QString &regionString : regionStrings )
    {
      const QgsTileExtentMaxZoomRegion region = regionFromVariant( regionString, context );
      if ( region.extent.isNull() )
        continue;

      regions.append( region );
    }
    return regions;
  }
  return {};
}

QVariant QgsProcessingParameterTileExtentMaxZoomList::toVariant( const QList<QgsTileExtentMaxZoomRegion> &regions )
{
  QVariantList res;
  res.reserve( regions.size() );
  for ( const QgsTileExtentMaxZoomRegion &region : regions )
  {
    res.append( regionToVariantMap( region ) );
  }
  return res;
}

QgsTileExtentMaxZoomRegion QgsProcessingParameterTileExtentMaxZoomList::regionFromVariant( const QVariant &variant, QgsProcessingContext &context ) const
{
  if ( variant.userType() == QVariant::Map )
  {
    return regionFromVariantMap( variant.toMap(), context );
  }
  else if ( variant.userType() == QVariant::String )
  {
    const QString regionString = variant.toString();
    const int sepPos = regionString.indexOf( ':' );
    if ( sepPos < 0 )
      return QgsTileExtentMaxZoomRegion();

    const QString zoomPart = regionString.left( sepPos );
    const QString extentPart = regionString.mid( sepPos + 1 );
    QgsTileExtentMaxZoomRegion region;
    region.maxZoom = zoomPart.toInt();
    const QgsCoordinateReferenceSystem crs = QgsProcessingParameters::parameterAsExtentCrs( this, extentPart, context );
    const QgsRectangle rect = QgsProcessingParameters::parameterAsExtent( this, extentPart, context );
    region.extent = QgsReferencedRectangle( rect, crs );
    return region;
  }
  return QgsTileExtentMaxZoomRegion();
}

int QgsProcessingParameterTileExtentMaxZoomList::maxZoomForTile( const QgsRectangle &tileExtent, const QList<QgsTileExtentMaxZoomRegion> &regions, int defaultMaxZoom )
{
  int effectiveMaxZoom = -1;

  for ( const QgsTileExtentMaxZoomRegion &region : regions )
  {
    if ( region.extent.intersects( tileExtent ) )
    {
      effectiveMaxZoom = std::max( effectiveMaxZoom, region.maxZoom );
    }
  }

  return ( effectiveMaxZoom >= 0 ) ? effectiveMaxZoom : defaultMaxZoom;
}

QVariantMap QgsProcessingParameterTileExtentMaxZoomList::regionToVariantMap( const QgsTileExtentMaxZoomRegion &region )
{
  QVariantMap map;
  const QString regionExtentString = u"%1,%3,%2,%4 [%5]"_s.arg(
    qgsDoubleToString( region.extent.xMinimum() ),
    qgsDoubleToString( region.extent.yMinimum() ),
    qgsDoubleToString( region.extent.xMaximum() ),
    qgsDoubleToString( region.extent.yMaximum() ),
    region.extent.crs().authid()
  );

  map.insert( u"extent"_s, regionExtentString );
  map.insert( u"max_zoom"_s, region.maxZoom );
  return map;
}

QgsTileExtentMaxZoomRegion QgsProcessingParameterTileExtentMaxZoomList::regionFromVariantMap( const QVariantMap &map, QgsProcessingContext &context ) const
{
  QgsTileExtentMaxZoomRegion region;
  region.maxZoom = map.value( u"max_zoom"_s, 0 ).toInt();

  const QgsCoordinateReferenceSystem crs = QgsProcessingParameters::parameterAsExtentCrs( this, map.value( "extent" ), context );
  const QgsRectangle rect = QgsProcessingParameters::parameterAsExtent( this, map.value( "extent" ), context );
  region.extent = QgsReferencedRectangle( rect, crs );

  return region;
}
