/***************************************************************************
    qgsmaptooladdfeature.cpp
    ------------------------
    begin                : April 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooladdfeature.h"
#include "moc_qgsmaptooladdfeature.cpp"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsfeatureaction.h"
#include "qgisapp.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrubberband.h"
#include "qgsvectorlayereditutils.h"

#include <QSettings>

QgsMapToolAddFeature::QgsMapToolAddFeature( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
  : QgsMapToolDigitizeFeature( canvas, cadDockWidget, mode )
{
  setLayer( canvas->currentLayer() );

  mToolName = tr( "Add feature" );
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddFeature::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddFeature::stopCapturing );
}

QgsMapToolAddFeature::QgsMapToolAddFeature( QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddFeature( canvas, QgisApp::instance()->cadDockWidget(), mode )
{
}

std::unique_ptr<QgsHighlight> QgsMapToolAddFeature::createHighlight( QgsVectorLayer *layer, const QgsFeature &f )
{
  std::unique_ptr<QgsHighlight> highlight = std::make_unique<QgsHighlight>( mCanvas, f.geometry(), layer );
  highlight->applyDefaultStyle();

  switch ( f.geometry().type() )
  {
    case Qgis::GeometryType::Point:
    {
      highlight->mPointSizeRadiusMM = 1.0;
      highlight->mPointSymbol = QgsHighlight::PointSymbol::Circle;

      break;
    }

    case Qgis::GeometryType::Polygon:
    case Qgis::GeometryType::Line:
    {
      const double rubberbandWidth = QgsSettingsRegistryCore::settingsDigitizingLineWidth->value();

      highlight->setWidth( static_cast<int>( rubberbandWidth ) + 1 );

      break;
    }
    default:
      break;
  }
  return highlight;
};

bool QgsMapToolAddFeature::addFeature( QgsVectorLayer *vlayer, const QgsFeature &f, bool showModal )
{
  QgsFeature feat( f );
  std::unique_ptr<QgsExpressionContextScope> scope( QgsExpressionContextUtils::mapToolCaptureScope( snappingMatches() ) );
  QgsFeatureAction *action = new QgsFeatureAction( tr( "add feature" ), feat, vlayer, QUuid(), -1, this );

  std::unique_ptr<QgsHighlight> highlight;
  if ( QgsRubberBand *rb = takeRubberBand() )
  {
    connect( action, &QgsFeatureAction::addFeatureFinished, rb, &QgsRubberBand::deleteLater );
  }

  // create a highlight for all spatial layers to enable distinguishing features in case
  // the user digitizes multiple features and has many pending attribute dialogs. This also
  // ensures that tools which don't create a rubber band (ie those which digitize single points)
  // still have a visible way of representing the captured geometry

  if ( vlayer->isSpatial() )
    highlight = createHighlight( vlayer, f );

  const QgsFeatureAction::AddFeatureResult res = action->addFeature( QgsAttributeMap(), showModal, std::move( scope ), false, std::move( highlight ) );
  if ( showModal )
    delete action;

  switch ( res )
  {
    case QgsFeatureAction::AddFeatureResult::Success:
    case QgsFeatureAction::AddFeatureResult::Pending:
      return true;
    case QgsFeatureAction::AddFeatureResult::LayerStateError:
    case QgsFeatureAction::AddFeatureResult::Canceled:
    case QgsFeatureAction::AddFeatureResult::FeatureError:
      return false;
  }
  BUILTIN_UNREACHABLE
}

void QgsMapToolAddFeature::featureDigitized( const QgsFeature &feature )
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  const bool res = addFeature( vlayer, feature, false );

  if ( res )
  {
    //add points to other features to keep topology up-to-date
    const bool topologicalEditing = QgsProject::instance()->topologicalEditing();
    const Qgis::AvoidIntersectionsMode avoidIntersectionsMode = QgsProject::instance()->avoidIntersectionsMode();
    if ( topologicalEditing && avoidIntersectionsMode == Qgis::AvoidIntersectionsMode::AvoidIntersectionsLayers && ( mode() == CaptureLine || mode() == CapturePolygon ) )
    {
      //use always topological editing for avoidIntersection.
      //Otherwise, no way to guarantee the geometries don't have a small gap in between.
      const QList<QgsVectorLayer *> intersectionLayers = QgsProject::instance()->avoidIntersectionsLayers();

      if ( !intersectionLayers.isEmpty() ) //try to add topological points also to background layers
      {
        for ( QgsVectorLayer *vl : intersectionLayers )
        {
          //can only add topological points if background layer is editable...
          if ( vl->geometryType() == Qgis::GeometryType::Polygon && vl->isEditable() )
          {
            vl->addTopologicalPoints( feature.geometry() );
          }
        }
      }
    }
    if ( topologicalEditing )
    {
      QgsFeatureRequest request = QgsFeatureRequest().setNoAttributes().setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setLimit( 1 );
      const QgsRectangle bbox = feature.geometry().boundingBox();
      const QList<QgsMapLayer *> layers = canvas()->layers( true );

      for ( QgsMapLayer *layer : layers )
      {
        QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
        QgsRectangle searchRect;
        QgsFeature f;
        QgsCoordinateTransform transform;

        if ( !vectorLayer || !vectorLayer->isEditable() )
          continue;

        if ( !( vectorLayer->geometryType() == Qgis::GeometryType::Polygon || vectorLayer->geometryType() == Qgis::GeometryType::Line ) )
          continue;

        if ( vectorLayer->crs() == vlayer->crs() )
        {
          searchRect = QgsRectangle( bbox );
        }
        else
        {
          transform = QgsCoordinateTransform( vlayer->crs(), vectorLayer->crs(), vectorLayer->transformContext() );
          searchRect = transform.transformBoundingBox( bbox );
        }

        searchRect.grow( QgsVectorLayerEditUtils::getTopologicalSearchRadius( vectorLayer ) );
        request.setFilterRect( searchRect );

        // We check that there is actually at least one feature intersecting our geometry in the layer to avoid creating an empty edit command and calling costly addTopologicalPoint
        if ( !vectorLayer->getFeatures( request ).nextFeature( f ) )
          continue;

        vectorLayer->beginEditCommand( tr( "Topological points added by 'Add Feature'" ) );

        int res = 2;
        if ( vectorLayer->crs() != vlayer->crs() )
        {
          QgsGeometry transformedGeom = feature.geometry();
          try
          {
            // transform digitized geometry from vlayer crs to vectorLayer crs and add topological points
            transformedGeom.transform( transform );
            res = vectorLayer->addTopologicalPoints( transformedGeom );
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse )
            QgsDebugError( QStringLiteral( "transformation to vectorLayer coordinate failed" ) );
          }
        }
        else
        {
          res = vectorLayer->addTopologicalPoints( feature.geometry() );
        }

        if ( res == 0 ) // i.e. if any points were added
          vectorLayer->endEditCommand();
        else
          vectorLayer->destroyEditCommand();
      }
    }
  }
}
