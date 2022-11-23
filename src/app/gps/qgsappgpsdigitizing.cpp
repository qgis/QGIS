/***************************************************************************
    qgsappgpsdigitizing.cpp
    -------------------
    begin                : October 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappgpsdigitizing.h"
#include "qgsrubberband.h"
#include "qgslinesymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsgui.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"
#include "qgsfeatureaction.h"
#include "qgsgpsconnection.h"
#include "qgsappgpsconnection.h"
#include "qgsprojectgpssettings.h"
#include "qgsapplication.h"
#include "qgsattributeform.h"
#include "qgsattributedialog.h"
#include "qgshighlight.h"

#include <QTimeZone>

QgsUpdateGpsDetailsAction::QgsUpdateGpsDetailsAction( QgsAppGpsConnection *connection, QgsAppGpsDigitizing *digitizing, QObject *parent )
  : QgsMapLayerAction( tr( "Update GPS Information" ), parent, Qgis::MapLayerActionTarget::SingleFeature, QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mActionRecenter.svg" ) ) )
  , mConnection( connection )
  , mDigitizing( digitizing )
{

}

bool QgsUpdateGpsDetailsAction::canRunUsingLayer( QgsMapLayer * ) const
{
  return false;
}

bool QgsUpdateGpsDetailsAction::canRunUsingLayer( QgsMapLayer *layer, const QgsMapLayerActionContext &context ) const
{
  return mConnection && mConnection->isConnected() && context.attributeDialog() && context.attributeDialog()->attributeForm()->mode() == QgsAttributeEditorContext::Mode::AddFeatureMode
         && layer == QgsProject::instance()->gpsSettings()->destinationLayer();
}

void QgsUpdateGpsDetailsAction::triggerForFeature( QgsMapLayer *layer, const QgsFeature &, const QgsMapLayerActionContext &context )
{
  QgsVectorLayer *vlayer = QgsProject::instance()->gpsSettings()->destinationLayer();
  if ( !vlayer || ! mConnection || !mConnection->isConnected()
       || layer != vlayer )
    return;

  QgsAttributeDialog *dialog = context.attributeDialog();
  if ( !dialog )
    return;

  QgsAttributeForm *form = dialog->attributeForm();
  if ( !form )
    return;

  QString error;
  const QgsGeometry resultWgs84 = mDigitizing->currentGeometry( vlayer->wkbType(), error );
  if ( !error.isEmpty() )
  {
    if ( QgsMessageBar *messageBar = context.messageBar() )
      messageBar->pushWarning( QString(), error );
    return;
  }

  const QgsCoordinateTransform wgs84ToLayerTransform( QgsCoordinateReferenceSystem( "EPSG:4326" ), vlayer->crs(), QgsProject::instance() );
  QgsGeometry geometryLayerCrs;
  try
  {
    geometryLayerCrs = resultWgs84;
    geometryLayerCrs.transform( wgs84ToLayerTransform );
  }
  catch ( QgsCsException & )
  {
    if ( QgsMessageBar *messageBar = context.messageBar() )
      messageBar->pushCritical( QString(),
                                tr( "Error reprojecting GPS location to layer CRS." ) );
    return;
  }

  if ( !geometryLayerCrs.isNull() )
  {
    form->changeGeometry( geometryLayerCrs );
    QgsHighlight *highlight = new QgsHighlight( mDigitizing->canvas(), geometryLayerCrs, vlayer );
    highlight->applyDefaultStyle();
    highlight->mPointSizeRadiusMM = 1;
    highlight->mPointSymbol = QgsHighlight::PointSymbol::Circle;
    dialog->setHighlight( highlight );

    const QgsAttributeMap updatedDerivedAttributes = mDigitizing->derivedAttributes();
    for ( auto it = updatedDerivedAttributes.constBegin(); it != updatedDerivedAttributes.constEnd(); ++it )
    {
      const int fieldIndex = it.key();
      const QString fieldName = vlayer->fields().at( fieldIndex ).name();
      form->changeAttribute( fieldName, it.value() );
    }

    if ( QgsMessageBar *messageBar = context.messageBar() )
      messageBar->pushSuccess( QString(), tr( "Updated feature location from GPS." ) );
  }
}


QgsAppGpsDigitizing::QgsAppGpsDigitizing( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent )
  : QgsGpsLogger( nullptr, parent )
  , mConnection( connection )
  , mCanvas( canvas )
{
  mUpdateGpsDetailsAction = new QgsUpdateGpsDetailsAction( mConnection, this, this );
  QgsGui::mapLayerActionRegistry()->addMapLayerAction( mUpdateGpsDetailsAction );

  mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, [ = ]
  {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [ = ]
  {
    setTransformContext( QgsProject::instance()->transformContext() );
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, transformContext() );
  } );
  setTransformContext( QgsProject::instance()->transformContext() );

  setEllipsoid( QgsProject::instance()->ellipsoid() );

  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [ = ]
  {
    setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  connect( mConnection, &QgsAppGpsConnection::connected, this, &QgsAppGpsDigitizing::gpsConnected );
  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsAppGpsDigitizing::gpsDisconnected );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsAppGpsDigitizing::gpsSettingsChanged );
  gpsSettingsChanged();

  connect( QgisApp::instance(), &QgisApp::activeLayerChanged, this, [ = ]( QgsMapLayer * layer )
  {
    if ( QgsProject::instance()->gpsSettings()->destinationFollowsActiveLayer() )
    {
      QgsProject::instance()->gpsSettings()->setDestinationLayer( qobject_cast< QgsVectorLayer *> ( layer ) );
    }
  } );
  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::destinationFollowsActiveLayerChanged, this, [ = ]( bool enabled )
  {
    if ( enabled )
    {
      QgsProject::instance()->gpsSettings()->setDestinationLayer( qobject_cast< QgsVectorLayer *> ( QgisApp::instance()->activeLayer() ) );
    }
  } );
  if ( QgsProject::instance()->gpsSettings()->destinationFollowsActiveLayer() )
  {
    QgsProject::instance()->gpsSettings()->setDestinationLayer( qobject_cast< QgsVectorLayer *> ( QgisApp::instance()->activeLayer() ) );
  }

  setAutomaticallyAddTrackVertices( QgsProject::instance()->gpsSettings()->automaticallyAddTrackVertices() );
  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::automaticallyAddTrackVerticesChanged, this, [ = ]( bool enabled )
  {
    setAutomaticallyAddTrackVertices( enabled );
  } );

  connect( this, &QgsGpsLogger::trackVertexAdded, this, &QgsAppGpsDigitizing::addVertex );
  connect( this, &QgsGpsLogger::trackReset, this, &QgsAppGpsDigitizing::onTrackReset );
}

QgsAppGpsDigitizing::~QgsAppGpsDigitizing()
{
  QgsGui::mapLayerActionRegistry()->removeMapLayerAction( mUpdateGpsDetailsAction );

  delete mRubberBand;
  mRubberBand = nullptr;
}

QgsMapCanvas *QgsAppGpsDigitizing::canvas()
{
  return mCanvas;
}

QgsAttributeMap QgsAppGpsDigitizing::derivedAttributes() const
{
  QgsVectorLayer *vlayer = QgsProject::instance()->gpsSettings()->destinationLayer();
  if ( !vlayer )
    return QgsAttributeMap();

  QgsAttributeMap attrMap;
  const int idx = vlayer->fields().indexOf( QgsProject::instance()->gpsSettings()->destinationTimeStampField() );
  if ( idx != -1 )
  {
    const QVariant ts = timestamp( vlayer, idx );
    if ( ts.isValid() )
    {
      attrMap[ idx ] = ts;
    }
  }
  return attrMap;
}

void QgsAppGpsDigitizing::addVertex( const QgsPoint &wgs84Point )
{
  if ( !mRubberBand )
  {
    createRubberBand();
  }

  // we store the rubber band points in map canvas CRS so transform to map crs
  // potential problem with transform errors and wrong coordinates if map CRS is changed after points are stored - SLM
  // should catch map CRS change and transform the points
  QgsPointXY mapPoint;
  if ( mCanvas )
  {
    try
    {
      mapPoint = mCanvasToWgs84Transform.transform( wgs84Point, Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform GPS location (%1, %2) to map CRS" ).arg( wgs84Point.x() ).arg( wgs84Point.y() ) );
      return;
    }
  }
  else
  {
    mapPoint = wgs84Point;
  }

  mRubberBand->addPoint( mapPoint );
}

void QgsAppGpsDigitizing::onTrackReset()
{
  createRubberBand(); //deletes existing rubberband
}

void QgsAppGpsDigitizing::createFeature()
{
  QgsVectorLayer *vlayer = QgsProject::instance()->gpsSettings()->destinationLayer();
  if ( !vlayer )
    return;

  QString error;
  const QgsGeometry resultWgs84 = QgsGpsLogger::currentGeometry( vlayer->wkbType(), error );
  if ( !error.isEmpty() )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), error );
    return;
  }

  const QgsCoordinateTransform wgs84ToLayerTransform( mWgs84CRS, vlayer->crs(), QgsProject::instance() );
  QgsGeometry geometryLayerCrs;
  try
  {
    geometryLayerCrs = resultWgs84;
    geometryLayerCrs.transform( wgs84ToLayerTransform );
  }
  catch ( QgsCsException & )
  {
    QgisApp::instance()->messageBar()->pushCritical( tr( "Add Feature" ),
        tr( "Error reprojecting feature to layer CRS." ) );
    return;
  }

  if ( geometryLayerCrs.type() == QgsWkbTypes::PolygonGeometry )
  {
    const int avoidIntersectionsReturn = geometryLayerCrs.avoidIntersections( QgsProject::instance()->avoidIntersectionsLayers() );
    if ( avoidIntersectionsReturn == 1 )
    {
      //not a polygon type. Impossible to get there
    }
    else if ( avoidIntersectionsReturn == 2 )
    {
      //bail out...
      QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "The feature could not be added because removing the polygon intersections would change the geometry type." ) );
      return;
    }
    else if ( avoidIntersectionsReturn == 3 )
    {
      QgisApp::instance()->messageBar()->pushCritical( tr( "Add Feature" ), tr( "The feature has been added, but at least one geometry intersected is invalid. These geometries must be manually repaired." ) );
      return;
    }
  }

  if ( geometryLayerCrs.isNull() )
    return;

  if ( !vlayer->isEditable() )
  {
    if ( vlayer->startEditing() )
    {
      if ( !QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() )
      {
        QgisApp::instance()->messageBar()->pushInfo( tr( "Add Feature" ), tr( "Layer “%1” was made editable" ).arg( vlayer->name() ) );
      }
    }
    else
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "Cannot create feature — the layer “%2” could not be made editable" ).arg( vlayer->name() ) );
      return;
    }
  }

  // Populate timestamp and other autogenerated attributes
  const QgsAttributeMap attrMap = derivedAttributes();

  QgsFeature f;
  f.setGeometry( geometryLayerCrs );

  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QgsFeatureAction action( tr( "Feature Added" ), f, vlayer, QUuid(), -1, this );
      const QgsFeatureAction::AddFeatureResult result = action.addFeature( attrMap );
      switch ( result )
      {
        case QgsFeatureAction::AddFeatureResult::Success:
        {
          if ( QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() )
          {
            // should canvas->isDrawing() be checked?
            if ( !vlayer->commitChanges() ) //assumed to be vector layer and is editable and is in editing mode (preconditions have been tested)
            {
              QgisApp::instance()->messageBar()->pushCritical(
                tr( "Save Layer Edits" ),
                tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                .arg( vlayer->name(),
                      vlayer->commitErrors().join( QLatin1String( "\n  " ) ) ) );
            }

            vlayer->startEditing();
          }
        }
        break;

        case QgsFeatureAction::AddFeatureResult::Canceled:
        case QgsFeatureAction::AddFeatureResult::Pending:
          break;

        case QgsFeatureAction::AddFeatureResult::LayerStateError:
        case QgsFeatureAction::AddFeatureResult::FeatureError:
          QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "Could not create new feature in layer %1" ).arg( vlayer->name() ) );
          break;
      }
      break;
    }

    case QgsWkbTypes::LineGeometry:
    case QgsWkbTypes::PolygonGeometry:
    {
      mBlockGpsStateChanged++;

      QgsFeatureAction action( tr( "Feature added" ), f, vlayer, QUuid(), -1, this );

      const QgsFeatureAction::AddFeatureResult result = action.addFeature( attrMap );
      switch ( result )
      {
        case QgsFeatureAction::AddFeatureResult::Success:
        {
          if ( QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() )
          {
            if ( !vlayer->commitChanges() )
            {
              QgisApp::instance()->messageBar()->pushCritical( tr( "Save Layer Edits" ),
                  tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                  .arg( vlayer->name(),
                        vlayer->commitErrors().join( QLatin1String( "\n  " ) ) ) );
            }

            vlayer->startEditing();
          }
          delete mRubberBand;
          mRubberBand = nullptr;

          // delete the elements of mCaptureList
          resetTrack();
        }
        break;

        case QgsFeatureAction::AddFeatureResult::Canceled:
        case QgsFeatureAction::AddFeatureResult::Pending:
          break;

        case QgsFeatureAction::AddFeatureResult::LayerStateError:
        case QgsFeatureAction::AddFeatureResult::FeatureError:
          QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "Could not create new feature in layer %1" ).arg( vlayer->name() ) );
          break;

      }
      mBlockGpsStateChanged--;

      break;
    }

    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      return;
  }
  vlayer->triggerRepaint();

  QgisApp::instance()->activateWindow();
}

void QgsAppGpsDigitizing::createVertexAtCurrentLocation()
{
  addTrackVertex();
}

void QgsAppGpsDigitizing::gpsSettingsChanged()
{
  updateTrackAppearance();

  updateGpsSettings();
}

void QgsAppGpsDigitizing::updateTrackAppearance()
{
  if ( !mRubberBand )
    return;

  QDomDocument doc;
  QDomElement elem;
  const QString trackLineSymbolXml = QgsAppGpsDigitizing::settingTrackLineSymbol.value();
  if ( !trackLineSymbolXml.isEmpty() )
  {
    doc.setContent( trackLineSymbolXml );
    elem = doc.documentElement();
    std::unique_ptr< QgsLineSymbol > trackLineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, QgsReadWriteContext() ) );
    if ( trackLineSymbol )
    {
      mRubberBand->setSymbol( trackLineSymbol.release() );
    }
    mRubberBand->update();
  }
}

void QgsAppGpsDigitizing::gpsConnected()
{
  setConnection( mConnection->connection() );
}

void QgsAppGpsDigitizing::gpsDisconnected()
{
  setConnection( nullptr );
}

void QgsAppGpsDigitizing::createRubberBand()
{
  delete mRubberBand;

  mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry );
  updateTrackAppearance();
  mRubberBand->show();
}

QVariant QgsAppGpsDigitizing::timestamp( QgsVectorLayer *vlayer, int idx ) const
{
  const QDateTime timestamp = lastTimestamp();
  QVariant value;
  if ( idx != -1 && timestamp.isValid() )
  {
    // Only string and datetime fields are supported
    switch ( vlayer->fields().at( idx ).type() )
    {
      case QVariant::String:
        value = timestamp.toString( Qt::DateFormat::ISODate );
        break;
      case QVariant::DateTime:
        value = timestamp;
        break;
      default:
        break;
    }
  }
  return value;
}

