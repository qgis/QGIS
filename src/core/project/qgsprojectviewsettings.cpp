/***************************************************************************
    qgsprojectviewsettings.cpp
    -----------------------------
    begin                : October 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectviewsettings.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsmaplayerutils.h"
#include "qgscoordinatetransform.h"
#include <QDomElement>

QgsProjectViewSettings::QgsProjectViewSettings( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

void QgsProjectViewSettings::reset()
{
  mDefaultViewExtent = QgsReferencedRectangle();

  const bool fullExtentChanged = !mPresetFullExtent.isNull();
  mPresetFullExtent = QgsReferencedRectangle();
  if ( fullExtentChanged )
    emit presetFullExtentChanged();

  if ( mUseProjectScales || !mMapScales.empty() )
  {
    mUseProjectScales = false;
    mMapScales.clear();
    emit mapScalesChanged();
  }
}

QgsReferencedRectangle QgsProjectViewSettings::defaultViewExtent() const
{
  return mDefaultViewExtent;
}

void QgsProjectViewSettings::setDefaultViewExtent( const QgsReferencedRectangle &extent )
{
  mDefaultViewExtent = extent;
}

QgsReferencedRectangle QgsProjectViewSettings::presetFullExtent() const
{
  return mPresetFullExtent;
}

void QgsProjectViewSettings::setPresetFullExtent( const QgsReferencedRectangle &extent )
{
  if ( extent == mPresetFullExtent )
    return;

  mPresetFullExtent = extent;
  emit presetFullExtentChanged();
}

QgsReferencedRectangle QgsProjectViewSettings::fullExtent() const
{
  if ( !mProject )
    return mPresetFullExtent;

  if ( !mPresetFullExtent.isNull() )
  {
    QgsCoordinateTransform ct( mPresetFullExtent.crs(), mProject->crs(), mProject->transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    return QgsReferencedRectangle( ct.transformBoundingBox( mPresetFullExtent ), mProject->crs() );
  }
  else
  {
    const QList< QgsMapLayer * > layers = mProject->mapLayers( true ).values();

    QList< QgsMapLayer * > nonBaseMapLayers;
    std::copy_if( layers.begin(), layers.end(),
                  std::back_inserter( nonBaseMapLayers ),
    []( const QgsMapLayer * layer ) { return !( layer->properties() & Qgis::MapLayerProperty::IsBasemapLayer ); } );

    // unless ALL layers from the project are basemap layers, we exclude these by default as their extent won't be useful for the project.
    if ( !nonBaseMapLayers.empty( ) )
      return QgsReferencedRectangle( QgsMapLayerUtils::combinedExtent( nonBaseMapLayers, mProject->crs(), mProject->transformContext() ), mProject->crs() );
    else
      return QgsReferencedRectangle( QgsMapLayerUtils::combinedExtent( layers, mProject->crs(), mProject->transformContext() ), mProject->crs() );
  }
}

void QgsProjectViewSettings::setMapScales( const QVector<double> &scales )
{
  // sort scales in descending order
  QVector< double > sorted = scales;
  std::sort( sorted.begin(), sorted.end(), std::greater<double>() );

  if ( sorted == mapScales() )
    return;

  mMapScales = sorted;

  emit mapScalesChanged();
}

QVector<double> QgsProjectViewSettings::mapScales() const
{
  return mMapScales;
}

void QgsProjectViewSettings::setUseProjectScales( bool enabled )
{
  if ( enabled == useProjectScales() )
    return;

  mUseProjectScales = enabled;
  emit mapScalesChanged();
}

bool QgsProjectViewSettings::useProjectScales() const
{
  return mUseProjectScales;
}

bool QgsProjectViewSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const bool useProjectScale = element.attribute( QStringLiteral( "UseProjectScales" ), QStringLiteral( "0" ) ).toInt();

  QDomNodeList scalesNodes = element.elementsByTagName( QStringLiteral( "Scales" ) );
  QVector< double > newScales;
  if ( !scalesNodes.isEmpty() )
  {
    const QDomElement scalesElement = scalesNodes.at( 0 ).toElement();
    scalesNodes = scalesElement.elementsByTagName( QStringLiteral( "Scale" ) );
    for ( int i = 0; i < scalesNodes.count(); i++ )
    {
      const QDomElement scaleElement = scalesNodes.at( i ).toElement();
      newScales.append( scaleElement.attribute( QStringLiteral( "Value" ) ).toDouble() );
    }
  }
  if ( useProjectScale != mUseProjectScales || newScales != mMapScales )
  {
    mMapScales = newScales;
    mUseProjectScales = useProjectScale;
    emit mapScalesChanged();
  }

  const QDomElement defaultViewElement = element.firstChildElement( QStringLiteral( "DefaultViewExtent" ) );
  if ( !defaultViewElement.isNull() )
  {
    const double xMin = defaultViewElement.attribute( QStringLiteral( "xmin" ) ).toDouble();
    const double yMin = defaultViewElement.attribute( QStringLiteral( "ymin" ) ).toDouble();
    const double xMax = defaultViewElement.attribute( QStringLiteral( "xmax" ) ).toDouble();
    const double yMax = defaultViewElement.attribute( QStringLiteral( "ymax" ) ).toDouble();
    QgsCoordinateReferenceSystem crs;
    crs.readXml( defaultViewElement );
    mDefaultViewExtent = QgsReferencedRectangle( QgsRectangle( xMin, yMin, xMax, yMax ), crs );
  }
  else
  {
    mDefaultViewExtent = QgsReferencedRectangle();
  }

  const QDomElement presetViewElement = element.firstChildElement( QStringLiteral( "PresetFullExtent" ) );
  if ( !presetViewElement.isNull() )
  {
    const double xMin = presetViewElement.attribute( QStringLiteral( "xmin" ) ).toDouble();
    const double yMin = presetViewElement.attribute( QStringLiteral( "ymin" ) ).toDouble();
    const double xMax = presetViewElement.attribute( QStringLiteral( "xmax" ) ).toDouble();
    const double yMax = presetViewElement.attribute( QStringLiteral( "ymax" ) ).toDouble();
    QgsCoordinateReferenceSystem crs;
    crs.readXml( presetViewElement );
    setPresetFullExtent( QgsReferencedRectangle( QgsRectangle( xMin, yMin, xMax, yMax ), crs ) );
  }
  else
  {
    setPresetFullExtent( QgsReferencedRectangle() );
  }

  return true;
}

QDomElement QgsProjectViewSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectViewSettings" ) );
  element.setAttribute( QStringLiteral( "UseProjectScales" ), mUseProjectScales ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  QDomElement scales = doc.createElement( QStringLiteral( "Scales" ) );
  for ( const double scale : mMapScales )
  {
    QDomElement scaleElement = doc.createElement( QStringLiteral( "Scale" ) );
    scaleElement.setAttribute( QStringLiteral( "Value" ), qgsDoubleToString( scale ) );
    scales.appendChild( scaleElement );
  }
  element.appendChild( scales );

  if ( !mDefaultViewExtent.isNull() )
  {
    QDomElement defaultViewElement = doc.createElement( QStringLiteral( "DefaultViewExtent" ) );
    defaultViewElement.setAttribute( QStringLiteral( "xmin" ), qgsDoubleToString( mDefaultViewExtent.xMinimum() ) );
    defaultViewElement.setAttribute( QStringLiteral( "ymin" ), qgsDoubleToString( mDefaultViewExtent.yMinimum() ) );
    defaultViewElement.setAttribute( QStringLiteral( "xmax" ), qgsDoubleToString( mDefaultViewExtent.xMaximum() ) );
    defaultViewElement.setAttribute( QStringLiteral( "ymax" ), qgsDoubleToString( mDefaultViewExtent.yMaximum() ) );
    mDefaultViewExtent.crs().writeXml( defaultViewElement, doc );
    element.appendChild( defaultViewElement );
  }

  if ( !mPresetFullExtent.isNull() )
  {
    QDomElement presetViewElement = doc.createElement( QStringLiteral( "PresetFullExtent" ) );
    presetViewElement.setAttribute( QStringLiteral( "xmin" ), qgsDoubleToString( mPresetFullExtent.xMinimum() ) );
    presetViewElement.setAttribute( QStringLiteral( "ymin" ), qgsDoubleToString( mPresetFullExtent.yMinimum() ) );
    presetViewElement.setAttribute( QStringLiteral( "xmax" ), qgsDoubleToString( mPresetFullExtent.xMaximum() ) );
    presetViewElement.setAttribute( QStringLiteral( "ymax" ), qgsDoubleToString( mPresetFullExtent.yMaximum() ) );
    mPresetFullExtent.crs().writeXml( presetViewElement, doc );
    element.appendChild( presetViewElement );
  }

  return element;
}
