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
    // reset the map canvas extent since the extent may now be smaller
    // We can't use a constructor since QgsRectangle normalizes the rectangle upon construction
    QgsRectangle fullExtent;
    fullExtent.setMinimal();

    // iterate through the map layers and test each layers extent
    // against the current min and max values
    const QMap<QString, QgsMapLayer *> layers = mProject->mapLayers( true );
    QgsDebugMsgLevel( QStringLiteral( "Layer count: %1" ).arg( layers.count() ), 5 );
    for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
    {
      QgsDebugMsgLevel( "Updating extent using " + it.value()->name(), 5 );
      QgsDebugMsgLevel( "Input extent: " + it.value()->extent().toString(), 5 );

      if ( it.value()->extent().isNull() )
        continue;

      // Layer extents are stored in the coordinate system (CS) of the
      // layer. The extent must be projected to the canvas CS
      QgsCoordinateTransform ct( it.value()->crs(), mProject->crs(), mProject->transformContext() );
      ct.setBallparkTransformsAreAppropriate( true );
      try
      {
        const QgsRectangle extent = ct.transformBoundingBox( it.value()->extent() );

        QgsDebugMsgLevel( "Output extent: " + extent.toString(), 5 );
        fullExtent.combineExtentWith( extent );
      }
      catch ( QgsCsException & )
      {
        QgsDebugMsg( QStringLiteral( "Could not reproject layer extent" ) );
      }
    }

    if ( fullExtent.width() == 0.0 || fullExtent.height() == 0.0 )
    {
      // If all of the features are at the one point, buffer the
      // rectangle a bit. If they are all at zero, do something a bit
      // more crude.

      if ( fullExtent.xMinimum() == 0.0 && fullExtent.xMaximum() == 0.0 &&
           fullExtent.yMinimum() == 0.0 && fullExtent.yMaximum() == 0.0 )
      {
        fullExtent.set( -1.0, -1.0, 1.0, 1.0 );
      }
      else
      {
        const double padFactor = 1e-8;
        double widthPad = fullExtent.xMinimum() * padFactor;
        double heightPad = fullExtent.yMinimum() * padFactor;
        double xmin = fullExtent.xMinimum() - widthPad;
        double xmax = fullExtent.xMaximum() + widthPad;
        double ymin = fullExtent.yMinimum() - heightPad;
        double ymax = fullExtent.yMaximum() + heightPad;
        fullExtent.set( xmin, ymin, xmax, ymax );
      }
    }

    QgsDebugMsgLevel( "Full extent: " + fullExtent.toString(), 5 );
    return QgsReferencedRectangle( fullExtent, mProject->crs() );
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
  bool useProjectScale = element.attribute( QStringLiteral( "UseProjectScales" ), QStringLiteral( "0" ) ).toInt();

  QDomNodeList scalesNodes = element.elementsByTagName( QStringLiteral( "Scales" ) );
  QVector< double > newScales;
  if ( !scalesNodes.isEmpty() )
  {
    QDomElement scalesElement = scalesNodes.at( 0 ).toElement();
    scalesNodes = scalesElement.elementsByTagName( QStringLiteral( "Scale" ) );
    for ( int i = 0; i < scalesNodes.count(); i++ )
    {
      QDomElement scaleElement = scalesNodes.at( i ).toElement();
      newScales.append( scaleElement.attribute( QStringLiteral( "Value" ) ).toDouble() );
    }
  }
  if ( useProjectScale != mUseProjectScales || newScales != mMapScales )
  {
    mMapScales = newScales;
    mUseProjectScales = useProjectScale;
    emit mapScalesChanged();
  }

  QDomElement defaultViewElement = element.firstChildElement( QStringLiteral( "DefaultViewExtent" ) );
  if ( !defaultViewElement.isNull() )
  {
    double xMin = defaultViewElement.attribute( QStringLiteral( "xmin" ) ).toDouble();
    double yMin = defaultViewElement.attribute( QStringLiteral( "ymin" ) ).toDouble();
    double xMax = defaultViewElement.attribute( QStringLiteral( "xmax" ) ).toDouble();
    double yMax = defaultViewElement.attribute( QStringLiteral( "ymax" ) ).toDouble();
    QgsCoordinateReferenceSystem crs;
    crs.readXml( defaultViewElement );
    mDefaultViewExtent = QgsReferencedRectangle( QgsRectangle( xMin, yMin, xMax, yMax ), crs );
  }
  else
  {
    mDefaultViewExtent = QgsReferencedRectangle();
  }

  QDomElement presetViewElement = element.firstChildElement( QStringLiteral( "PresetFullExtent" ) );
  if ( !presetViewElement.isNull() )
  {
    double xMin = presetViewElement.attribute( QStringLiteral( "xmin" ) ).toDouble();
    double yMin = presetViewElement.attribute( QStringLiteral( "ymin" ) ).toDouble();
    double xMax = presetViewElement.attribute( QStringLiteral( "xmax" ) ).toDouble();
    double yMax = presetViewElement.attribute( QStringLiteral( "ymax" ) ).toDouble();
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
  for ( double scale : mMapScales )
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
