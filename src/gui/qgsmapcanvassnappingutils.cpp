/***************************************************************************
    qgsmapcanvassnappingutils.cpp
    ---------------------
    begin                : December 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmapcanvassnappingutils.h"

#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"

#include <QApplication>
#include <QProgressDialog>

QgsMapCanvasSnappingUtils::QgsMapCanvasSnappingUtils( QgsMapCanvas *canvas, QObject *parent )
  : QgsSnappingUtils( parent, QgsSettings().value( QStringLiteral( "/qgis/digitizing/snap_invisible_feature" ), false ).toBool() )
  , mCanvas( canvas )

{
  connect( canvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasSnappingUtils::canvasMapSettingsChanged );
  connect( canvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapCanvasSnappingUtils::canvasMapSettingsChanged );
  connect( canvas, &QgsMapCanvas::layersChanged, this, &QgsMapCanvasSnappingUtils::canvasMapSettingsChanged );
  connect( canvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMapCanvasSnappingUtils::canvasCurrentLayerChanged );
  connect( canvas, &QgsMapCanvas::transformContextChanged, this, &QgsMapCanvasSnappingUtils::canvasTransformContextChanged );
  connect( canvas, &QgsMapCanvas::mapToolSet, this, &QgsMapCanvasSnappingUtils::canvasMapToolChanged );
  canvasMapSettingsChanged();
  canvasCurrentLayerChanged();
}

void QgsMapCanvasSnappingUtils::canvasMapSettingsChanged()
{
  setMapSettings( mCanvas->mapSettings() );
  setEnableSnappingForInvisibleFeature( QgsSettings().value( QStringLiteral( "/qgis/digitizing/snap_invisible_feature" ), false ).toBool() );
}

void QgsMapCanvasSnappingUtils::canvasTransformContextChanged()
{
  // can't trust any of our previous locators, as we don't know exactly how datum transform changes would affect these
  clearAllLocators();
  setMapSettings( mCanvas->mapSettings() );
}

void QgsMapCanvasSnappingUtils::canvasCurrentLayerChanged()
{
  setCurrentLayer( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ) );
}

void QgsMapCanvasSnappingUtils::canvasMapToolChanged()
{
  setEnableSnappingForInvisibleFeature( QgsSettings().value( QStringLiteral( "/qgis/digitizing/snap_invisible_feature" ), false ).toBool() );
}

void QgsMapCanvasSnappingUtils::prepareIndexStarting( int count )
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  mProgress = new QProgressDialog( tr( "Indexing data…" ), QString(), 0, count, mCanvas->topLevelWidget() );
  mProgress->setWindowModality( Qt::WindowModal );
}

void QgsMapCanvasSnappingUtils::prepareIndexProgress( int index )
{
  if ( !mProgress )
    return;

  mProgress->setValue( index );
  if ( index == mProgress->maximum() )
  {
    delete mProgress;
    mProgress = nullptr;
    QApplication::restoreOverrideCursor();
  }
}
