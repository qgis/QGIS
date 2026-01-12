/***************************************************************************
    qgsmaptooladvanceddigitizing.cpp  - map tool with event in map coordinates
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooladvanceddigitizing.h"

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsgeometryoptions.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsregistrycore.h"
#include "qgssnaptogridcanvasitem.h"
#include "qgsstatusbar.h"
#include "qgsunittypes.h"
#include "qgsvectorlayer.h"

#include "moc_qgsmaptooladvanceddigitizing.cpp"

QgsMapToolAdvancedDigitizing::QgsMapToolAdvancedDigitizing( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolEdit( canvas )
  , mCadDockWidget( cadDockWidget )
{
  Q_ASSERT( cadDockWidget );
  connect( canvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMapToolAdvancedDigitizing::onCurrentLayerChanged );
}

QgsMapToolAdvancedDigitizing::~QgsMapToolAdvancedDigitizing() = default;

void QgsMapToolAdvancedDigitizing::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( isAdvancedDigitizingAllowed() && mCadDockWidget->cadEnabled() )
  {
    mCadDockWidget->applyConstraints( e ); // updates event's map point
    mCadDockWidget->processCanvasPressEvent( e );
    if ( !e->isAccepted() )
    {
      return; // The dock widget has taken the event
    }
  }
  else if ( isAutoSnapEnabled() )
  {
    e->snapPoint();
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( mSnapToLayerGridEnabled && layer )
  {
    e->snapToGrid( layer->geometryOptions()->geometryPrecision(), layer->crs() );
  }

  cadCanvasPressEvent( e );
}

void QgsMapToolAdvancedDigitizing::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( isAdvancedDigitizingAllowed() && mCadDockWidget->cadEnabled() )
  {
    mCadDockWidget->processCanvasReleaseEvent( e );
    if ( !e->isAccepted() )
    {
      return; // The dock widget has taken the event
    }
  }
  else if ( isAutoSnapEnabled() )
  {
    e->snapPoint();
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( mSnapToGridCanvasItem && mSnapToLayerGridEnabled && layer )
  {
    e->snapToGrid( layer->geometryOptions()->geometryPrecision(), layer->crs() );
  }

  cadCanvasReleaseEvent( e );
}

void QgsMapToolAdvancedDigitizing::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( isAdvancedDigitizingAllowed() && mCadDockWidget->cadEnabled() )
  {
    mCadDockWidget->applyConstraints( e ); // updates event's map point
    mCadDockWidget->processCanvasMoveEvent( e );
    if ( !e->isAccepted() )
    {
      return; // The dock widget has taken the event
    }
  }
  else if ( isAutoSnapEnabled() )
  {
    e->snapPoint();
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( mSnapToGridCanvasItem && mSnapToLayerGridEnabled && layer )
  {
    e->snapToGrid( layer->geometryOptions()->geometryPrecision(), layer->crs() );
    mSnapToGridCanvasItem->setPoint( e->mapPoint() );
  }

  if ( mSnapIndicator )
  {
    mSnapIndicator->setMatch( e->mapPointMatch() );
  }

  cadCanvasMoveEvent( e );
}

void QgsMapToolAdvancedDigitizing::activate()
{
  QgsMapToolEdit::activate();
  connect( mCadDockWidget, &QgsAdvancedDigitizingDockWidget::pointChangedV2, this, &QgsMapToolAdvancedDigitizing::cadPointChanged );
  connect( this, &QgsMapToolAdvancedDigitizing::transientGeometryChanged, this, &QgsMapToolAdvancedDigitizing::onTransientGeometryChanged );
  mCadDockWidget->enable();
  mSnapToGridCanvasItem = new QgsSnapToGridCanvasItem( mCanvas );
  QgsVectorLayer *layer = currentVectorLayer();
  if ( layer )
  {
    mSnapToGridCanvasItem->setCrs( currentVectorLayer()->crs() );
    mSnapToGridCanvasItem->setPrecision( currentVectorLayer()->geometryOptions()->geometryPrecision() );
  }
  mSnapToGridCanvasItem->setEnabled( mSnapToLayerGridEnabled );
}

void QgsMapToolAdvancedDigitizing::deactivate()
{
  QgsMapToolEdit::deactivate();
  disconnect( mCadDockWidget, &QgsAdvancedDigitizingDockWidget::pointChangedV2, this, &QgsMapToolAdvancedDigitizing::cadPointChanged );
  disconnect( this, &QgsMapToolAdvancedDigitizing::transientGeometryChanged, this, &QgsMapToolAdvancedDigitizing::onTransientGeometryChanged );
  mCadDockWidget->disable();
  delete mSnapToGridCanvasItem;
  mSnapToGridCanvasItem = nullptr;

  if ( mSnapIndicator )
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
}

QgsMapLayer *QgsMapToolAdvancedDigitizing::layer() const
{
  return canvas()->currentLayer();
}

bool QgsMapToolAdvancedDigitizing::useSnappingIndicator() const
{
  return static_cast<bool>( mSnapIndicator.get() );
}

void QgsMapToolAdvancedDigitizing::calculateGeometryMeasures( const QgsReferencedGeometry &geometry, const QgsCoordinateReferenceSystem &destinationCrs, Qgis::CadMeasurementDisplayType areaType, Qgis::CadMeasurementDisplayType totalLengthType, QString &areaString, QString &totalLengthString )
{
  areaString = QString();
  totalLengthString = QString();

  // transform to map crs
  QgsGeometry g = geometry;
  const QgsCoordinateTransform ct( geometry.crs(), destinationCrs, QgsProject::instance()->transformContext() );
  try
  {
    g.transform( ct );
  }
  catch ( QgsCsException &e )
  {
    QgsDebugError( u"Error transforming transient geometry: %1"_s.arg( e.what() ) );
    return;
  }

  std::unique_ptr< QgsDistanceArea > distanceArea;
  auto createDistanceArea = [destinationCrs, &distanceArea] {
    // reuse existing if we've already created one
    if ( distanceArea )
      return;

    distanceArea = std::make_unique< QgsDistanceArea >();
    distanceArea->setSourceCrs( destinationCrs, QgsProject::instance()->transformContext() );
    distanceArea->setEllipsoid( QgsProject::instance()->ellipsoid() );
  };

  if ( g.type() == Qgis::GeometryType::Polygon && areaType != Qgis::CadMeasurementDisplayType::Hidden )
  {
    switch ( areaType )
    {
      case Qgis::CadMeasurementDisplayType::Hidden:
        break;

      case Qgis::CadMeasurementDisplayType::Cartesian:
      {
        if ( destinationCrs.mapUnits() != Qgis::DistanceUnit::Unknown )
        {
          areaString = tr( "%1 %2" ).arg(
            QString::number( g.area(), 'f', destinationCrs.mapUnits() == Qgis::DistanceUnit::Degrees ? 6 : 4 ),
            QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::distanceToAreaUnit( destinationCrs.mapUnits() ) )
          );
        }
        else
        {
          areaString = QString::number( g.area(), 'f', 4 );
        }
        break;
      }

      case Qgis::CadMeasurementDisplayType::Ellipsoidal:
      {
        createDistanceArea();
        const double area = distanceArea->measureArea( g );
        areaString = distanceArea->formatArea( area, destinationCrs.mapUnits() == Qgis::DistanceUnit::Degrees ? 6 : 4, QgsProject::instance()->areaUnits() );
        break;
      }
    }
  }

  if ( totalLengthType != Qgis::CadMeasurementDisplayType::Hidden )
  {
    switch ( totalLengthType )
    {
      case Qgis::CadMeasurementDisplayType::Hidden:
        break;

      case Qgis::CadMeasurementDisplayType::Cartesian:
      {
        if ( destinationCrs.mapUnits() != Qgis::DistanceUnit::Unknown )
        {
          totalLengthString = tr( "%1 %2" ).arg(
            QString::number( g.length(), 'f', destinationCrs.mapUnits() == Qgis::DistanceUnit::Degrees ? 6 : 4 ),
            QgsUnitTypes::toAbbreviatedString( destinationCrs.mapUnits() )
          );
        }
        else
        {
          totalLengthString = QString::number( g.length(), 'f', 4 );
        }
        break;
      }
      case Qgis::CadMeasurementDisplayType::Ellipsoidal:
      {
        createDistanceArea();
        const double length = g.type() == Qgis::GeometryType::Polygon ? distanceArea->measurePerimeter( g ) : distanceArea->measureLength( g );
        totalLengthString = distanceArea->formatDistance( length, destinationCrs.mapUnits() == Qgis::DistanceUnit::Degrees ? 6 : 4, QgsProject::instance()->distanceUnits() );
        break;
      }
    }
  }
}

void QgsMapToolAdvancedDigitizing::setUseSnappingIndicator( bool enabled )
{
  if ( enabled && !mSnapIndicator )
  {
    mSnapIndicator = std::make_unique<QgsSnapIndicator>( mCanvas );
  }
  else if ( !enabled && mSnapIndicator )
  {
    mSnapIndicator.reset();
  }
}

void QgsMapToolAdvancedDigitizing::cadPointChanged( const QgsPointXY &point )
{
  Q_UNUSED( point )
  QMouseEvent *ev = new QMouseEvent( QEvent::MouseMove, mCanvas->mouseLastXY(), Qt::NoButton, Qt::NoButton, Qt::NoModifier );
  qApp->postEvent( mCanvas->viewport(), ev ); // event queue will delete the event when processed
}

void QgsMapToolAdvancedDigitizing::onCurrentLayerChanged()
{
  if ( mSnapToGridCanvasItem )
  {
    QgsVectorLayer *layer = currentVectorLayer();
    if ( layer && mSnapToLayerGridEnabled )
    {
      mSnapToGridCanvasItem->setPrecision( layer->geometryOptions()->geometryPrecision() );
      mSnapToGridCanvasItem->setCrs( layer->crs() );
    }
    if ( !layer || !layer->isSpatial() )
    {
      mCadDockWidget->clear();
      mCadDockWidget->disable();
      mSnapToGridCanvasItem->setEnabled( false );
    }
    else
    {
      mCadDockWidget->enable();
      mSnapToGridCanvasItem->setEnabled( mSnapToLayerGridEnabled );
    }
  }
}

void QgsMapToolAdvancedDigitizing::onTransientGeometryChanged( const QgsReferencedGeometry &geometry )
{
  if ( mCadDockWidget )
    mCadDockWidget->updateTransientGeometryProperties( geometry );

  QgsStatusBar *statusBar = mCanvas ? mCanvas->statusBar() : nullptr;
  if ( !statusBar )
    return;

  const Qgis::CadMeasurementDisplayType areaDisplayType = QgsSettingsRegistryCore::settingsDigitizingStatusBarAreaDisplay->value();
  const Qgis::CadMeasurementDisplayType totalLengthDisplayType = QgsSettingsRegistryCore::settingsDigitizingStatusBarTotalLengthDisplay->value();
  if ( areaDisplayType == Qgis::CadMeasurementDisplayType::Hidden && totalLengthDisplayType == Qgis::CadMeasurementDisplayType::Hidden )
    return;

  QString areaString;
  QString totalLengthString;
  QgsMapToolAdvancedDigitizing::calculateGeometryMeasures( geometry, mCanvas->mapSettings().destinationCrs(), areaDisplayType, totalLengthDisplayType, areaString, totalLengthString );

  QStringList messageParts;
  if ( !areaString.isEmpty() )
    messageParts.append( tr( "Total area: %1" ).arg( areaString ) );
  if ( !totalLengthString.isEmpty() )
  {
    if ( geometry.type() == Qgis::GeometryType::Polygon )
      messageParts.append( tr( "Perimeter: %1" ).arg( totalLengthString ) );
    else
      messageParts.append( tr( "Total length: %1" ).arg( totalLengthString ) );
  }

  statusBar->showMessage( messageParts.join( ' ' ) );
}

bool QgsMapToolAdvancedDigitizing::snapToLayerGridEnabled() const
{
  return mSnapToLayerGridEnabled;
}

void QgsMapToolAdvancedDigitizing::setSnapToLayerGridEnabled( bool snapToGridEnabled )
{
  mSnapToLayerGridEnabled = snapToGridEnabled;

  if ( mSnapToGridCanvasItem )
  {
    mSnapToGridCanvasItem->setEnabled( snapToGridEnabled );
  }
}
