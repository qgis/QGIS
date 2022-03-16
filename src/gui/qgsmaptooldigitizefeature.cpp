/***************************************************************************
  qgsmaptooldigitizefeature.cpp

 ---------------------
 begin                : 7.12.2017
 copyright            : (C) 2017 by David Signer
 email                : david@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooldigitizefeature.h"

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsattributedialog.h"
#include "qgscurvepolygon.h"
#include "qgsexception.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmultipoint.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QSettings>

QgsMapToolDigitizeFeature::QgsMapToolDigitizeFeature( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
  : QgsMapToolCaptureLayerGeometry( canvas, cadDockWidget, mode )
  , mCheckGeometryType( true )
{
  mToolName = tr( "Digitize feature" );
  connect( QgsProject::instance(), &QgsProject::cleared, this, &QgsMapToolDigitizeFeature::stopCapturing );
  connect( QgsProject::instance(), &QgsProject::readProject, this, &QgsMapToolDigitizeFeature::stopCapturing );
}

QgsMapToolCapture::Capabilities QgsMapToolDigitizeFeature::capabilities() const
{
  return QgsMapToolCapture::SupportsCurves | QgsMapToolCapture::ValidateGeometries;
}

bool QgsMapToolDigitizeFeature::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
      return true;
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Streaming:
    case Qgis::CaptureTechnique::Shape:
      return mode() != QgsMapToolCapture::CapturePoint;
  }
  return false;
}

void QgsMapToolDigitizeFeature::layerGeometryCaptured( const QgsGeometry &geometry )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vlayer )
    vlayer = currentVectorLayer();

  if ( !vlayer )
    return;

  const QgsWkbTypes::Type layerWKBType = vlayer->wkbType();

  QgsGeometry layerGeometry;

  if ( mCheckGeometryType )
  {
    double defaultZ = QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.value();
    double defaultM = QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.value();
    QVector<QgsGeometry> layerGeometries = geometry.coerceToType( layerWKBType, defaultZ, defaultM );
    if ( layerGeometries.count() > 0 )
      layerGeometry = layerGeometries.at( 0 );

    if ( layerGeometry.wkbType() != layerWKBType && layerGeometry.wkbType() != QgsWkbTypes::linearType( layerWKBType ) )
    {
      emit messageEmitted( tr( "The digitized geometry type (%1) does not correspond to the layer geometry type (%2)." ).arg( QgsWkbTypes::displayString( layerGeometry.wkbType() ) ).arg( QgsWkbTypes::displayString( layerWKBType ) ), Qgis::MessageLevel::Warning );
      return;
    }
  }
  else
  {
    layerGeometry = geometry;
  }
  QgsFeature f( vlayer->fields(), 0 );
  f.setGeometry( layerGeometry );
  f.setValid( true );
  emit digitizingCompleted( f );
  featureDigitized( f );
}

void QgsMapToolDigitizeFeature::activate()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vlayer )
    vlayer = currentVectorLayer();

  if ( vlayer && vlayer->geometryType() == QgsWkbTypes::NullGeometry )
  {
    layerGeometryCaptured( QgsGeometry() );
    return;
  }

  if ( mLayer )
  {
    //remember current layer
    mCurrentLayer = mCanvas->currentLayer();
    //set the layer with the given
    mCanvas->setCurrentLayer( mLayer );
  }

  QgsMapToolCaptureLayerGeometry::activate();
}

void QgsMapToolDigitizeFeature::deactivate()
{
  QgsMapToolCaptureLayerGeometry::deactivate();

  if ( mCurrentLayer )
    //set the layer back to the one remembered
    mCanvas->setCurrentLayer( mCurrentLayer );
  emit digitizingFinished();
}

bool QgsMapToolDigitizeFeature::checkGeometryType() const
{
  return mCheckGeometryType;
}

void QgsMapToolDigitizeFeature::setCheckGeometryType( bool checkGeometryType )
{
  mCheckGeometryType = checkGeometryType;
}

void QgsMapToolDigitizeFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayer );

  if ( !vlayer )
    //if no given layer take the current from canvas
    vlayer = currentVectorLayer();

  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  QgsVectorDataProvider *provider = vlayer->dataProvider();

  if ( !( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    emit messageEmitted( tr( "The data provider for this layer does not support the addition of features." ), Qgis::MessageLevel::Warning );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  //check we only use this tool for point/multipoint layers
  if ( mode() == CapturePoint && vlayer->geometryType() != QgsWkbTypes::PointGeometry && mCheckGeometryType )
  {
    emit messageEmitted( tr( "Wrong editing tool, cannot apply the 'capture point' tool on this vector layer" ), Qgis::MessageLevel::Warning );
    return;
  }

  //check we only use the line tool for line/multiline layers
  if ( mode() == CaptureLine && vlayer->geometryType() != QgsWkbTypes::LineGeometry && mCheckGeometryType )
  {
    emit messageEmitted( tr( "Wrong editing tool, cannot apply the 'capture line' tool on this vector layer" ), Qgis::MessageLevel::Warning );
    return;
  }

  //check we only use the polygon tool for polygon/multipolygon layers
  if ( mode() == CapturePolygon && vlayer->geometryType() != QgsWkbTypes::PolygonGeometry && mCheckGeometryType )
  {
    emit messageEmitted( tr( "Wrong editing tool, cannot apply the 'capture polygon' tool on this vector layer" ), Qgis::MessageLevel::Warning );
    return;
  }

  QgsMapToolCaptureLayerGeometry::cadCanvasReleaseEvent( e );
}

void QgsMapToolDigitizeFeature::setLayer( QgsMapLayer *vl )
{
  mLayer = vl;
}

