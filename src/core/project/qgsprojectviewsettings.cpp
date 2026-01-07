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
#include "qgscoordinatetransform.h"
#include "qgsmaplayerutils.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"

#include <QDomElement>

#include "moc_qgsprojectviewsettings.cpp"

QgsProjectViewSettings::QgsProjectViewSettings( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

void QgsProjectViewSettings::reset()
{
  mDefaultViewExtent = QgsReferencedRectangle();

  mDefaultRotation = 0;

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

void QgsProjectViewSettings::setRestoreProjectExtentOnProjectLoad( bool projectExtentCheckboxState )
{
  mRestoreProjectExtentOnProjectLoad = projectExtentCheckboxState;
}

bool QgsProjectViewSettings::restoreProjectExtentOnProjectLoad( )
{
  return mRestoreProjectExtentOnProjectLoad;
}


QgsReferencedRectangle QgsProjectViewSettings::fullExtent() const
{
  if ( !mProject )
    return mPresetFullExtent;

  if ( !mPresetFullExtent.isNull() )
  {
    QgsCoordinateTransform ct( mPresetFullExtent.crs(), mProject->crs(), mProject->transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    try
    {
      return QgsReferencedRectangle( ct.transformBoundingBox( mPresetFullExtent ), mProject->crs() );
    }
    catch ( QgsCsException &e )
    {
      QgsDebugError( u"Transform error encountered while determining project extent: %1"_s.arg( e.what() ) );
      return QgsReferencedRectangle();
    }
  }
  else
  {
    const QList< QgsMapLayer * > layers = mProject->mapLayers( true ).values();

    QList< QgsMapLayer * > nonBaseMapLayers;
    std::copy_if( layers.begin(), layers.end(),
                  std::back_inserter( nonBaseMapLayers ),
    []( const QgsMapLayer * layer ) { return !( layer->properties() & Qgis::MapLayerProperty::IsBasemapLayer ) && !( layer->properties() & Qgis::MapLayerProperty::Is3DBasemapLayer ); } );

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

double QgsProjectViewSettings::defaultRotation() const
{
  return mDefaultRotation;
}

void QgsProjectViewSettings::setDefaultRotation( double rotation )
{
  mDefaultRotation = rotation;
}

bool QgsProjectViewSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const bool useProjectScale = element.attribute( u"UseProjectScales"_s, u"0"_s ).toInt();

  QDomNodeList scalesNodes = element.elementsByTagName( u"Scales"_s );
  QVector< double > newScales;
  if ( !scalesNodes.isEmpty() )
  {
    const QDomElement scalesElement = scalesNodes.at( 0 ).toElement();
    scalesNodes = scalesElement.elementsByTagName( u"Scale"_s );
    for ( int i = 0; i < scalesNodes.count(); i++ )
    {
      const QDomElement scaleElement = scalesNodes.at( i ).toElement();
      newScales.append( scaleElement.attribute( u"Value"_s ).toDouble() );
    }
  }
  if ( useProjectScale != mUseProjectScales || newScales != mMapScales )
  {
    mMapScales = newScales;
    mUseProjectScales = useProjectScale;
    emit mapScalesChanged();
  }

  const QDomElement defaultViewElement = element.firstChildElement( u"DefaultViewExtent"_s );
  if ( !defaultViewElement.isNull() )
  {
    const double xMin = defaultViewElement.attribute( u"xmin"_s ).toDouble();
    const double yMin = defaultViewElement.attribute( u"ymin"_s ).toDouble();
    const double xMax = defaultViewElement.attribute( u"xmax"_s ).toDouble();
    const double yMax = defaultViewElement.attribute( u"ymax"_s ).toDouble();
    QgsCoordinateReferenceSystem crs;
    crs.readXml( defaultViewElement );
    mDefaultViewExtent = QgsReferencedRectangle( QgsRectangle( xMin, yMin, xMax, yMax ), crs );
  }
  else
  {
    mDefaultViewExtent = QgsReferencedRectangle();
  }

  const QDomElement presetViewElement = element.firstChildElement( u"PresetFullExtent"_s );
  if ( !presetViewElement.isNull() )
  {
    const double xMin = presetViewElement.attribute( u"xmin"_s ).toDouble();
    const double yMin = presetViewElement.attribute( u"ymin"_s ).toDouble();
    const double xMax = presetViewElement.attribute( u"xmax"_s ).toDouble();
    const double yMax = presetViewElement.attribute( u"ymax"_s ).toDouble();
    QgsCoordinateReferenceSystem crs;
    crs.readXml( presetViewElement );
    setPresetFullExtent( QgsReferencedRectangle( QgsRectangle( xMin, yMin, xMax, yMax ), crs ) );
  }
  else
  {
    setPresetFullExtent( QgsReferencedRectangle() );
  }

  mDefaultRotation = element.attribute( u"rotation"_s, u"0"_s ).toDouble();
  mRestoreProjectExtentOnProjectLoad = element.attribute( u"LoadProjectExtent"_s, u"0"_s ).toInt();

  return true;
}

QDomElement QgsProjectViewSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( u"ProjectViewSettings"_s );
  element.setAttribute( u"UseProjectScales"_s, mUseProjectScales ? u"1"_s : u"0"_s );

  if ( mRestoreProjectExtentOnProjectLoad )
  {
    element.setAttribute( u"LoadProjectExtent"_s, u"1"_s );
  }

  element.setAttribute( u"rotation"_s, qgsDoubleToString( mDefaultRotation ) );

  QDomElement scales = doc.createElement( u"Scales"_s );
  for ( const double scale : mMapScales )
  {
    QDomElement scaleElement = doc.createElement( u"Scale"_s );
    scaleElement.setAttribute( u"Value"_s, qgsDoubleToString( scale ) );
    scales.appendChild( scaleElement );
  }
  element.appendChild( scales );

  if ( !mDefaultViewExtent.isNull() )
  {
    QDomElement defaultViewElement = doc.createElement( u"DefaultViewExtent"_s );
    defaultViewElement.setAttribute( u"xmin"_s, qgsDoubleToString( mDefaultViewExtent.xMinimum() ) );
    defaultViewElement.setAttribute( u"ymin"_s, qgsDoubleToString( mDefaultViewExtent.yMinimum() ) );
    defaultViewElement.setAttribute( u"xmax"_s, qgsDoubleToString( mDefaultViewExtent.xMaximum() ) );
    defaultViewElement.setAttribute( u"ymax"_s, qgsDoubleToString( mDefaultViewExtent.yMaximum() ) );
    mDefaultViewExtent.crs().writeXml( defaultViewElement, doc );
    element.appendChild( defaultViewElement );
  }

  if ( !mPresetFullExtent.isNull() )
  {
    QDomElement presetViewElement = doc.createElement( u"PresetFullExtent"_s );
    presetViewElement.setAttribute( u"xmin"_s, qgsDoubleToString( mPresetFullExtent.xMinimum() ) );
    presetViewElement.setAttribute( u"ymin"_s, qgsDoubleToString( mPresetFullExtent.yMinimum() ) );
    presetViewElement.setAttribute( u"xmax"_s, qgsDoubleToString( mPresetFullExtent.xMaximum() ) );
    presetViewElement.setAttribute( u"ymax"_s, qgsDoubleToString( mPresetFullExtent.yMaximum() ) );
    mPresetFullExtent.crs().writeXml( presetViewElement, doc );
    element.appendChild( presetViewElement );
  }

  return element;
}
