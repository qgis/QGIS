/***************************************************************************
qgsmaptoolselectutils.cpp  -  Utility methods to help with select map tools
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <limits>

#include "qgsmaptoolselectutils.h"
#include "qgsfeatureiterator.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilelayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgshighlight.h"
#include "qgsrenderer.h"
#include "qgsrubberband.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgsmapcanvasutils.h"
#include "qgsselectioncontext.h"

#include <QMouseEvent>
#include <QApplication>
#include <QAction>
#include <QtConcurrent>

QgsMapLayer *QgsMapToolSelectUtils::getCurrentTargetLayer( QgsMapCanvas *canvas )
{
  QgsMapLayer *layer = canvas->currentLayer();
  if ( layer )
  {
    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      case QgsMapLayerType::VectorTileLayer:
        // supported
        break;
      case QgsMapLayerType::RasterLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        layer = nullptr; //not supported
        break;
    }
  }

  if ( !layer )
  {
    QgisApp::instance()->messageBar()->pushMessage(
      QObject::tr( "No active vector layer" ),
      QObject::tr( "To select features, choose a vector layer in the layers panel" ),
      Qgis::MessageLevel::Info );
  }
  return layer;
}

void QgsMapToolSelectUtils::setRubberBand( QgsMapCanvas *canvas, QRect &selectRect, QgsRubberBand *rubberBand )
{
  const QgsMapToPixel *transform = canvas->getCoordinateTransform();
  QgsPointXY ll = transform->toMapCoordinates( selectRect.left(), selectRect.bottom() );
  QgsPointXY lr = transform->toMapCoordinates( selectRect.right(), selectRect.bottom() );
  QgsPointXY ul = transform->toMapCoordinates( selectRect.left(), selectRect.top() );
  QgsPointXY ur = transform->toMapCoordinates( selectRect.right(), selectRect.top() );

  if ( rubberBand )
  {
    rubberBand->reset( QgsWkbTypes::PolygonGeometry );
    rubberBand->addPoint( ll, false );
    rubberBand->addPoint( lr, false );
    rubberBand->addPoint( ur, false );
    rubberBand->addPoint( ul, true );
  }
}

QgsRectangle QgsMapToolSelectUtils::expandSelectRectangle( QgsPointXY mapPoint, QgsMapCanvas *canvas, QgsMapLayer *layer )
{
  int boxSize = 0;
  if ( !layer )
  {
    boxSize = 5;
  }
  else
  {
    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vLayer = qobject_cast< QgsVectorLayer * >( layer );
        if ( vLayer->geometryType() != QgsWkbTypes::PolygonGeometry )
        {
          //if point or line use an artificial bounding box of 10x10 pixels
          //to aid the user to click on a feature accurately
          boxSize = 5;
        }
        else
        {
          //otherwise just use the click point for polys
          boxSize = 1;
        }
        break;
      }
      case QgsMapLayerType::VectorTileLayer:
        // mixed layer type, so aim for somewhere between the vector layer polygon/point sizes
        boxSize = 2;
        break;

      case QgsMapLayerType::RasterLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }

  const QgsMapToPixel *transform = canvas->getCoordinateTransform();
  QgsPointXY point = transform->transform( mapPoint );
  QgsPointXY ll = transform->toMapCoordinates( static_cast<int>( point.x() - boxSize ), static_cast<int>( point.y() + boxSize ) );
  QgsPointXY ur = transform->toMapCoordinates( static_cast<int>( point.x() + boxSize ), static_cast<int>( point.y() - boxSize ) );
  return QgsRectangle( ll, ur );
}

void QgsMapToolSelectUtils::selectMultipleFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers )
{
  Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection;
  if ( modifiers & Qt::ShiftModifier && modifiers & Qt::ControlModifier )
    behavior = Qgis::SelectBehavior::IntersectSelection;
  else if ( modifiers & Qt::ShiftModifier )
    behavior = Qgis::SelectBehavior::AddToSelection;
  else if ( modifiers & Qt::ControlModifier )
    behavior = Qgis::SelectBehavior::RemoveFromSelection;

  bool doContains = modifiers & Qt::AltModifier;
  setSelectedFeatures( canvas, selectGeometry, behavior, doContains );
}

bool transformSelectGeometry( const QgsGeometry &selectGeometry, QgsGeometry &selectGeomTrans, const QgsCoordinateTransform &ct )
{
  selectGeomTrans = selectGeometry;
  try
  {
    if ( !ct.isShortCircuited() && selectGeomTrans.type() == QgsWkbTypes::PolygonGeometry )
    {
      // convert add more points to the edges of the rectangle
      // improve transformation result
      QgsPolygonXY poly( selectGeomTrans.asPolygon() );
      if ( poly.size() == 1 && poly.at( 0 ).size() == 5 )
      {
        const QgsPolylineXY &ringIn = poly.at( 0 );

        QgsPolygonXY newpoly( 1 );
        newpoly[0].resize( 41 );
        QgsPolylineXY &ringOut = newpoly[0];

        ringOut[ 0 ] = ringIn.at( 0 );

        int i = 1;
        for ( int j = 1; j < 5; j++ )
        {
          QgsVector v( ( ringIn.at( j ) - ringIn.at( j - 1 ) ) / 10.0 );
          for ( int k = 0; k < 9; k++ )
          {
            ringOut[ i ] = ringOut[ i - 1 ] + v;
            i++;
          }
          ringOut[ i++ ] = ringIn.at( j );
        }
        selectGeomTrans = QgsGeometry::fromPolygonXY( newpoly );
      }
    }

    selectGeomTrans.transform( ct );
    return true;
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    // catch exception for 'invalid' point and leave existing selection unchanged
    QgsDebugMsg( QStringLiteral( "Caught CRS exception " ) );
    QgisApp::instance()->messageBar()->pushMessage(
      QObject::tr( "CRS Exception" ),
      QObject::tr( "Selection extends beyond layer's coordinate system" ),
      Qgis::MessageLevel::Warning );
    return false;
  }
}

void QgsMapToolSelectUtils::selectSingleFeature( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers )
{
  QgsMapLayer *layer = QgsMapToolSelectUtils::getCurrentTargetLayer( canvas );
  if ( !layer )
    return;

  Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection;
  QgsSelectionContext context;
  context.setScale( canvas->scale() );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer *>( layer );
      QgsFeatureIds selectedFeatures = getMatchingFeatures( canvas, selectGeometry, false, true );
      if ( selectedFeatures.isEmpty() )
      {
        if ( !( modifiers & Qt::ShiftModifier || modifiers & Qt::ControlModifier ) )
        {
          // if no modifiers then clicking outside features clears the selection
          // but if there's a shift or ctrl modifier, then it's likely the user was trying
          // to modify an existing selection by adding or subtracting features and just
          // missed the feature
          vlayer->removeSelection();
        }
        QApplication::restoreOverrideCursor();
        return;
      }

      //either shift or control modifier switches to "toggle" selection mode
      if ( modifiers & Qt::ShiftModifier || modifiers & Qt::ControlModifier )
      {
        QgsFeatureId selectId = *selectedFeatures.constBegin();
        QgsFeatureIds layerSelectedFeatures = vlayer->selectedFeatureIds();
        if ( layerSelectedFeatures.contains( selectId ) )
          behavior = Qgis::SelectBehavior::RemoveFromSelection;
        else
          behavior = Qgis::SelectBehavior::AddToSelection;
      }

      vlayer->selectByIds( selectedFeatures, behavior );
      break;
    }

    case QgsMapLayerType::VectorTileLayer:
    {
      QgsVectorTileLayer *vtLayer = qobject_cast< QgsVectorTileLayer *>( layer );

      QgsCoordinateTransform ct( canvas->mapSettings().destinationCrs(), layer->crs(), QgsProject::instance() );
      QgsGeometry selectGeomTrans;
      if ( !transformSelectGeometry( selectGeometry, selectGeomTrans, ct ) )
      {
        break;
      }

      Qgis::SelectionFlags flags = Qgis::SelectionFlag::SingleFeatureSelection;
      if ( modifiers & Qt::ShiftModifier || modifiers & Qt::ControlModifier )
        flags |= Qgis::SelectionFlag::ToggleSelection;

      QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( canvas->mapSettings() );
      QgsExpressionContext expressionContext = canvas->createExpressionContext();
      expressionContext << QgsExpressionContextUtils::layerScope( vtLayer );
      renderContext.setExpressionContext( expressionContext );

      vtLayer->selectByGeometry( selectGeomTrans, context, behavior, Qgis::SelectGeometryRelationship::Intersect, flags, &renderContext );
      break;
    }

    case QgsMapLayerType::RasterLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }

  QApplication::restoreOverrideCursor();
}

void QgsMapToolSelectUtils::setSelectedFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry,
    Qgis::SelectBehavior selectBehavior, bool doContains, bool singleSelect )
{
  QgsMapLayer *layer = QgsMapToolSelectUtils::getCurrentTargetLayer( canvas );
  if ( !layer )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsSelectionContext context;
  context.setScale( canvas->scale() );

  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer *vLayer = qobject_cast< QgsVectorLayer * >( layer );
      QgsFeatureIds selectedFeatures = getMatchingFeatures( canvas, selectGeometry, doContains, singleSelect );
      vLayer->selectByIds( selectedFeatures, selectBehavior );
      break;
    }

    case QgsMapLayerType::VectorTileLayer:
    {
      QgsVectorTileLayer *vtLayer = qobject_cast< QgsVectorTileLayer * >( layer );
      QgsCoordinateTransform ct( canvas->mapSettings().destinationCrs(), layer->crs(), QgsProject::instance() );
      QgsGeometry selectGeomTrans;
      if ( !transformSelectGeometry( selectGeometry, selectGeomTrans, ct ) )
      {
        break;
      }

      QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( canvas->mapSettings() );
      QgsExpressionContext expressionContext = canvas->createExpressionContext();
      expressionContext << QgsExpressionContextUtils::layerScope( vtLayer );
      renderContext.setExpressionContext( expressionContext );

      vtLayer->selectByGeometry( selectGeomTrans, context, selectBehavior, doContains ? Qgis::SelectGeometryRelationship::Within : Qgis::SelectGeometryRelationship::Intersect, Qgis::SelectionFlags(), &renderContext );
      break;
    }

    case QgsMapLayerType::RasterLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }

  QApplication::restoreOverrideCursor();
}

QgsFeatureIds QgsMapToolSelectUtils::getMatchingFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, bool doContains, bool singleSelect )
{
  QgsFeatureIds newSelectedFeatures;

  if ( selectGeometry.type() != QgsWkbTypes::PolygonGeometry )
    return newSelectedFeatures;

  QgsMapLayer *targetLayer = QgsMapToolSelectUtils::getCurrentTargetLayer( canvas );
  QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( targetLayer );
  if ( !vlayer )
    return newSelectedFeatures;

  // toLayerCoordinates will throw an exception for any 'invalid' points in
  // the rubber band.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  QgsGeometry selectGeomTrans;
  QgsCoordinateTransform ct( canvas->mapSettings().destinationCrs(), vlayer->crs(), QgsProject::instance() );
  if ( !transformSelectGeometry( selectGeometry, selectGeomTrans, ct ) )
    return newSelectedFeatures;

  QgsDebugMsgLevel( "Selection layer: " + vlayer->name(), 3 );
  QgsDebugMsgLevel( "Selection polygon: " + selectGeomTrans.asWkt(), 3 );
  QgsDebugMsgLevel( "doContains: " + QString( doContains ? QStringLiteral( "T" ) : QStringLiteral( "F" ) ), 3 );

  // make sure the selection geometry is valid, or intersection tests won't work correctly...
  if ( !selectGeomTrans.isGeosValid( ) )
  {
    // a zero width buffer is safer than calling make valid here!
    selectGeomTrans = selectGeomTrans.buffer( 0, 1 );
    if ( selectGeomTrans.isEmpty() )
      return newSelectedFeatures;
  }

  std::unique_ptr< QgsGeometryEngine > selectionGeometryEngine( QgsGeometry::createGeometryEngine( selectGeomTrans.constGet() ) );
  selectionGeometryEngine->setLogErrors( false );
  selectionGeometryEngine->prepareGeometry();

  QgsRenderContext context = QgsRenderContext::fromMapSettings( canvas->mapSettings() );

  QgsExpressionContext expressionContext = canvas->createExpressionContext();
  expressionContext << QgsExpressionContextUtils::layerScope( vlayer );
  context.setExpressionContext( expressionContext );

  std::unique_ptr< QgsFeatureRenderer > r;
  if ( vlayer->renderer() )
  {
    r.reset( vlayer->renderer()->clone() );
    r->startRender( context, vlayer->fields() );
  }

  const QString canvasFilter = QgsMapCanvasUtils::filterForLayer( canvas, vlayer );
  if ( canvasFilter == QLatin1String( "FALSE" ) )
    return newSelectedFeatures;

  QgsFeatureRequest request;
  request.setFilterRect( selectGeomTrans.boundingBox() );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  if ( r )
    request.setSubsetOfAttributes( r->usedAttributes( context ), vlayer->fields() );
  else
    request.setNoAttributes();

  if ( !canvasFilter.isEmpty() )
    request.setFilterExpression( canvasFilter );
  if ( r )
  {
    const QString filterExpression = r->filter( vlayer->fields() );
    if ( !filterExpression.isEmpty() )
    {
      request.combineFilterExpression( filterExpression );
    }
  }

  request.setExpressionContext( context.expressionContext() );
  QgsFeatureIterator fit = vlayer->getFeatures( request );

  QgsFeature f;
  QgsFeatureId closestFeatureId = 0;
  bool foundSingleFeature = false;
  double closestFeatureDist = std::numeric_limits<double>::max();
  while ( fit.nextFeature( f ) )
  {
    context.expressionContext().setFeature( f );
    // make sure to only use features that are visible
    if ( r && !r->willRenderFeature( f, context ) )
      continue;

    QgsGeometry g = f.geometry();
    QString errorMessage;
    if ( doContains )
    {
      // if we get an error from the contains check then it indicates that the geometry is invalid and GEOS choked on it.
      // in this case we consider the bounding box intersection check which has already been performed by the iterator as sufficient and
      // allow the feature to be selected
      const bool notContained = !selectionGeometryEngine->contains( g.constGet(), &errorMessage ) &&
                                ( errorMessage.isEmpty() || /* message will be non empty if geometry g is invalid */
                                  !selectionGeometryEngine->contains( g.makeValid().constGet(), &errorMessage ) ); /* second chance for invalid geometries, repair and re-test */

      if ( !errorMessage.isEmpty() )
      {
        // contains relation test still failed, even after trying to make valid!
        QgsMessageLog::logMessage( QObject::tr( "Error determining selection: %1" ).arg( errorMessage ), QString(), Qgis::MessageLevel::Warning );
      }

      if ( notContained )
        continue;
    }
    else
    {
      // if we get an error from the intersects check then it indicates that the geometry is invalid and GEOS choked on it.
      // in this case we consider the bounding box intersection check which has already been performed by the iterator as sufficient and
      // allow the feature to be selected
      const bool notIntersects = !selectionGeometryEngine->intersects( g.constGet(), &errorMessage ) &&
                                 ( errorMessage.isEmpty() || /* message will be non empty if geometry g is invalid */
                                   !selectionGeometryEngine->intersects( g.makeValid().constGet(), &errorMessage ) ); /* second chance for invalid geometries, repair and re-test */

      if ( !errorMessage.isEmpty() )
      {
        // intersects relation test still failed, even after trying to make valid!
        QgsMessageLog::logMessage( QObject::tr( "Error determining selection: %1" ).arg( errorMessage ), QString(), Qgis::MessageLevel::Warning );
      }

      if ( notIntersects )
        continue;
    }
    if ( singleSelect )
    {
      foundSingleFeature = true;
      double distance = g.distance( selectGeomTrans );
      if ( distance <= closestFeatureDist )
      {
        closestFeatureDist = distance;
        closestFeatureId = f.id();
      }
    }
    else
    {
      newSelectedFeatures.insert( f.id() );
    }
  }
  if ( singleSelect && foundSingleFeature )
  {
    newSelectedFeatures.insert( closestFeatureId );
  }

  if ( r )
    r->stopRender( context );

  QgsDebugMsgLevel( "Number of new selected features: " + QString::number( newSelectedFeatures.size() ), 2 );

  return newSelectedFeatures;
}


QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::QgsMapToolSelectMenuActions( QgsMapCanvas *canvas,
    QgsVectorLayer *vectorLayer,
    Qgis::SelectBehavior behavior, const QgsGeometry &selectionGeometry,
    QObject *parent )
  : QObject( parent )
  , mCanvas( canvas )
  , mVectorLayer( vectorLayer )
  , mBehavior( behavior )
  , mSelectGeometry( selectionGeometry )
{
  connect( mVectorLayer, &QgsMapLayer::destroyed, this, &QgsMapToolSelectMenuActions::onLayerDestroyed );

  mFutureWatcher = new QFutureWatcher<QgsFeatureIds>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapToolSelectMenuActions::onSearchFinished );
}

QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::~QgsMapToolSelectMenuActions()
{
  removeHighlight();
  if ( mJobData )
    mJobData->isCanceled = true;
  if ( mFutureWatcher )
    mFutureWatcher->waitForFinished();
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::populateMenu( QMenu *menu )
{
  mActionChooseAll = new QAction( textForChooseAll(), this );
  menu->addAction( mActionChooseAll );
  connect( mActionChooseAll, &QAction::triggered, this, &QgsMapToolSelectMenuActions::chooseAllCandidateFeature );
  mMenuChooseOne = new QMenu( textForChooseOneMenu() );
  menu->addMenu( mMenuChooseOne );
  mMenuChooseOne->setEnabled( false );

  startFeatureSearch();
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::startFeatureSearch()
{
  const QString canvasFilter = QgsMapCanvasUtils::filterForLayer( mCanvas, mVectorLayer );
  if ( canvasFilter == QLatin1String( "FALSE" ) )
    return;

  mJobData = std::make_shared<DataForSearchingJob>();
  mJobData->isCanceled = false;
  mJobData->source.reset( new QgsVectorLayerFeatureSource( mVectorLayer ) );
  mJobData->selectGeometry = mSelectGeometry;
  mJobData->context = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
  mJobData->filterString = canvasFilter;
  mJobData->ct = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mVectorLayer->crs(), mJobData->context.transformContext() );
  mJobData->featureRenderer.reset( mVectorLayer->renderer()->clone() );

  mJobData->context.setExpressionContext( mCanvas->createExpressionContext() );
  mJobData->context.expressionContext() << QgsExpressionContextUtils::layerScope( mVectorLayer );
  mJobData->selectBehavior = mBehavior;
  if ( mBehavior != Qgis::SelectBehavior::SetSelection )
    mJobData->existingSelection = mVectorLayer->selectedFeatureIds();
  QFuture<QgsFeatureIds> future = QtConcurrent::run( search, mJobData );
  mFutureWatcher->setFuture( future );
}

QgsFeatureIds QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::search( std::shared_ptr<DataForSearchingJob> data )
{
  QgsFeatureIds newSelectedFeatures;

  if ( data->selectGeometry.type() != QgsWkbTypes::PolygonGeometry )
    return newSelectedFeatures;

  QgsGeometry selectGeomTrans = data->selectGeometry;

  if ( ! transformSelectGeometry( data->selectGeometry, selectGeomTrans, data->ct ) )
    return newSelectedFeatures;

  // make sure the selection geometry is valid, or intersection tests won't work correctly...
  if ( !selectGeomTrans.isGeosValid( ) )
  {
    // a zero width buffer is safer than calling make valid here!
    selectGeomTrans = selectGeomTrans.buffer( 0, 1 );
  }

  std::unique_ptr< QgsGeometryEngine > selectionGeometryEngine( QgsGeometry::createGeometryEngine( selectGeomTrans.constGet() ) );
  selectionGeometryEngine->setLogErrors( false );
  selectionGeometryEngine->prepareGeometry();

  std::unique_ptr<QgsFeatureRenderer> r;
  if ( data->featureRenderer )
  {
    r.reset( data->featureRenderer->clone() );
    r->startRender( data->context, data->source->fields() );
  }

  QgsFeatureRequest request;
  request.setFilterRect( selectGeomTrans.boundingBox() );
  request.setFlags( QgsFeatureRequest::ExactIntersect );

  if ( !data->filterString.isEmpty() )
    request.setFilterExpression( data->filterString );

  if ( r )
  {
    request.setSubsetOfAttributes( r->usedAttributes( data->context ), data->source->fields() );
    const QString filterExpression = r->filter( data->source->fields() );
    if ( !filterExpression.isEmpty() )
    {
      request.combineFilterExpression( filterExpression );
    }
  }
  request.setExpressionContext( data->context.expressionContext() );

  QgsFeatureIterator fit = data->source->getFeatures( request );

  QgsFeature f;

  while ( fit.nextFeature( f ) && !data->isCanceled )
  {
    data->context.expressionContext().setFeature( f );
    // make sure to only use features that are visible
    if ( r && !r->willRenderFeature( f, data->context ) )
      continue;

    QgsGeometry g = f.geometry();
    QString errorMessage;

    // if we get an error from the intersects check then it indicates that the geometry is invalid and GEOS choked on it.
    // in this case we consider the bounding box intersection check which has already been performed by the iterator as sufficient and
    // allow the feature to be selected
    const bool notIntersects = !selectionGeometryEngine->intersects( g.constGet(), &errorMessage ) &&
                               ( errorMessage.isEmpty() || /* message will be non empty if geometry g is invalid */
                                 !selectionGeometryEngine->intersects( g.makeValid().constGet(), &errorMessage ) ); /* second chance for invalid geometries, repair and re-test */

    if ( !errorMessage.isEmpty() )
    {
      // intersects relation test still failed, even after trying to make valid!
      QgsMessageLog::logMessage( QObject::tr( "Error determining selection: %1" ).arg( errorMessage ), QString(), Qgis::MessageLevel::Warning );
    }

    if ( notIntersects )
      continue;

    newSelectedFeatures.insert( f.id() );
  }

  if ( r )
    r->stopRender( data->context );
  return filterIds( newSelectedFeatures, data->existingSelection, data->selectBehavior );
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::onSearchFinished()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
    return;

  mAllFeatureIds = mFutureWatcher->result();
  mActionChooseAll->setText( textForChooseAll( mAllFeatureIds.size() ) );
  if ( !mAllFeatureIds.isEmpty() )
    connect( mActionChooseAll, &QAction::hovered, this, &QgsMapToolSelectMenuActions::highlightAllFeatures );
  else
    mActionChooseAll->setEnabled( false );
  if ( mAllFeatureIds.count() > 1 )
    populateChooseOneMenu( mAllFeatureIds );
}


QString QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::textForChooseAll( qint64 featureCount ) const
{
  if ( featureCount == 0 || featureCount == 1 )
  {
    switch ( mBehavior )
    {
      case Qgis::SelectBehavior::SetSelection:
        return tr( "Select Feature" );

      case Qgis::SelectBehavior::AddToSelection:
        return tr( "Add to Selection" );

      case Qgis::SelectBehavior::IntersectSelection:
        return tr( "Intersect with Selection" );

      case Qgis::SelectBehavior::RemoveFromSelection:
        return tr( "Remove from Selection" );
    }
  }

  QString featureCountText;
  if ( featureCount < 0 )
    featureCountText = tr( "Searching…" );
  else
    featureCountText = QLocale().toString( featureCount );

  switch ( mBehavior )
  {
    case Qgis::SelectBehavior::SetSelection:
      return tr( "Select All (%1)" ).arg( featureCountText );
    case Qgis::SelectBehavior::AddToSelection:
      return tr( "Add All to Selection (%1)" ).arg( featureCountText );
    case Qgis::SelectBehavior::IntersectSelection:
      return tr( "Intersect All with Selection (%1)" ).arg( featureCountText );
    case Qgis::SelectBehavior::RemoveFromSelection:
      return tr( "Remove All from Selection (%1)" ).arg( featureCountText );
  }

  return QString();
}

QString QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::textForChooseOneMenu() const
{
  switch ( mBehavior )
  {
    case Qgis::SelectBehavior::SetSelection:
      return tr( "Select Feature" );
    case Qgis::SelectBehavior::AddToSelection:
      return tr( "Add Feature to Selection" );
    case Qgis::SelectBehavior::IntersectSelection:
      return tr( "Intersect Feature with Selection" );
    case Qgis::SelectBehavior::RemoveFromSelection:
      return tr( "Remove Feature from Selection" );
  }

  return QString();
}


void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::populateChooseOneMenu( const QgsFeatureIds &ids )
{
  if ( !mVectorLayer )
    return;

  QgsFeatureIds displayedFeatureIds;

  QgsFeatureIds::ConstIterator it = ids.constBegin();
  while ( displayedFeatureIds.count() <= 20 && it != ids.constEnd() ) //for now hardcoded, but maybe define a settings for this
    displayedFeatureIds.insert( *( it++ ) );

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mVectorLayer ) );
  QgsExpression exp = mVectorLayer->displayExpression();
  exp.prepare( &context );

  QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( displayedFeatureIds );
  QgsFeature feat;
  QgsFeatureIterator featureIt = mVectorLayer->getFeatures( request );
  while ( featureIt.nextFeature( feat ) )
  {
    const QgsFeatureId id = feat.id();
    context.setFeature( feat );

    QString featureTitle = exp.evaluate( &context ).toString();
    if ( featureTitle.isEmpty() )
      featureTitle = tr( "Feature %1" ).arg( FID_TO_STRING( feat.id() ) );

    QAction *featureAction = new QAction( featureTitle, this ) ;
    connect( featureAction, &QAction::triggered, this, [this, id]() {chooseOneCandidateFeature( id );} );
    connect( featureAction, &QAction::hovered, this, [this, id]() {this->highlightOneFeature( id );} );
    mMenuChooseOne->addAction( featureAction );
  }

  mMenuChooseOne->setEnabled( ids.count() != 0 );
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::chooseOneCandidateFeature( QgsFeatureId id )
{
  if ( !mVectorLayer )
    return;

  QgsFeatureIds ids;
  ids << id;
  mVectorLayer->selectByIds( ids, mBehavior );
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::chooseAllCandidateFeature()
{
  if ( ! mFutureWatcher )
    return;

  if ( !mFutureWatcher->isFinished() )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    mFutureWatcher->waitForFinished();
    QApplication::restoreOverrideCursor();
    mAllFeatureIds = mFutureWatcher->result();
  }

  if ( !mAllFeatureIds.empty() )
    mVectorLayer->selectByIds( mAllFeatureIds, mBehavior );
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::highlightAllFeatures()
{
  removeHighlight();

  if ( !mVectorLayer )
    return;

  if ( !mAllFeatureIds.empty() )
  {
    int count = 0;
    for ( const QgsFeatureId &id : std::as_const( mAllFeatureIds ) )
    {
      QgsFeature feat = mVectorLayer->getFeature( id );
      QgsGeometry geom = feat.geometry();
      if ( !geom.isEmpty() )
      {
        QgsHighlight *hl = new QgsHighlight( mCanvas, geom, mVectorLayer );
        styleHighlight( hl );
        mHighlight.append( hl );
        count++;
      }
      if ( count > 1000 ) //for now hardcoded, but maybe define a settings for this
        return;
    }
  }
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::highlightOneFeature( QgsFeatureId id )
{
  removeHighlight();

  if ( !mVectorLayer )
    return;

  QgsFeature feat = mVectorLayer->getFeature( id );
  QgsGeometry geom = feat.geometry();
  if ( !geom.isEmpty() )
  {
    QgsHighlight *hl = new QgsHighlight( mCanvas, geom, mVectorLayer );
    styleHighlight( hl );
    mHighlight.append( hl );
  }
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::styleHighlight( QgsHighlight *highlight )
{
  QgsSettings settings;
  QColor color = QColor( settings.value( QStringLiteral( "Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  int alpha = settings.value( QStringLiteral( "Map/highlight/colorAlpha" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toInt();
  double buffer = settings.value( QStringLiteral( "Map/highlight/buffer" ), Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM ).toDouble();
  double minWidth = settings.value( QStringLiteral( "Map/highlight/minWidth" ), Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM ).toDouble();

  highlight->setColor( color ); // sets also fill with default alpha
  color.setAlpha( alpha );
  highlight->setFillColor( color ); // sets fill with alpha
  highlight->setBuffer( buffer );
  highlight->setMinWidth( minWidth );
}

QgsFeatureIds QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::filterIds( const QgsFeatureIds &ids,
    const QgsFeatureIds &existingSelection,
    Qgis::SelectBehavior behavior )
{
  QgsFeatureIds effectiveFeatureIds = ids;
  switch ( behavior )
  {
    case Qgis::SelectBehavior::SetSelection:
      break;
    case Qgis::SelectBehavior::AddToSelection:
    {
      for ( QgsFeatureId newSelected : ids )
      {
        if ( existingSelection.contains( newSelected ) )
          effectiveFeatureIds.remove( newSelected );
      }
    }
    break;
    case Qgis::SelectBehavior::IntersectSelection:
    case Qgis::SelectBehavior::RemoveFromSelection:
    {
      for ( QgsFeatureId newSelected : ids )
      {
        if ( !existingSelection.contains( newSelected ) )
          effectiveFeatureIds.remove( newSelected );
      }
    }
    break;
  }

  return effectiveFeatureIds;
}


void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::onLayerDestroyed()
{
  mVectorLayer = nullptr;
  mJobData->isCanceled = true;
  removeHighlight();
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::removeHighlight()
{
  qDeleteAll( mHighlight );
  mHighlight.clear();
}
