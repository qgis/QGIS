/***************************************************************************
    qgs3dmaptoolpointcloudchangeattribute.cpp
    ---------------------
    begin                : January 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmaptoolpointcloudchangeattribute.h"
#include "moc_qgs3dmaptoolpointcloudchangeattribute.cpp"
#include "qgs3dmapcanvaswidget.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgsrubberband3d.h"
#include "qgswindow3dengine.h"
#include "qgsframegraph.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgspolygon.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsmultipoint.h"
#include "qgsguiutils.h"
#include "qgisapp.h"
#include "qgschunknode.h"
#include "qgsgeos.h"

#include <QStringLiteral>

Qgs3DMapToolPointCloudChangeAttribute::Qgs3DMapToolPointCloudChangeAttribute( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
}

Qgs3DMapToolPointCloudChangeAttribute::~Qgs3DMapToolPointCloudChangeAttribute() = default;

void Qgs3DMapToolPointCloudChangeAttribute::mousePressEvent( QMouseEvent *event )
{
  mClickPoint = event->pos();
}

void Qgs3DMapToolPointCloudChangeAttribute::mouseMoveEvent( QMouseEvent *event )
{
  const QgsPoint movedPoint = screenPointToMap( event->pos() );
  mPolygonRubberBand->moveLastPoint( movedPoint );
}

void Qgs3DMapToolPointCloudChangeAttribute::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete )
  {
    if ( mScreenPoints.isEmpty() )
    {
      return;
    }
    else if ( mScreenPoints.size() == 1 )
    {
      //removing first point, so restart everything
      restart();
    }
    else
    {
      mScreenPoints.removeLast();
      mPolygonRubberBand->removeLastPoint();
    }
  }
  else if ( event->key() == Qt::Key_Escape )
  {
    restart();
  }
}

void Qgs3DMapToolPointCloudChangeAttribute::mouseReleaseEvent( QMouseEvent *event )
{
  if ( ( event->pos() - mClickPoint ).manhattanLength() > QApplication::startDragDistance() )
    return;

  const QgsPoint newPoint = screenPointToMap( event->pos() );

  if ( event->button() == Qt::LeftButton )
  {
    if ( mPolygonRubberBand->isEmpty() )
    {
      mPolygonRubberBand->addPoint( newPoint );
      mCanvas->cameraController()->setInputHandlersEnabled( false );
    }
    mPolygonRubberBand->addPoint( newPoint );
    mScreenPoints.append( QgsPointXY( event->x(), event->y() ) );
  }
  else if ( event->button() == Qt::RightButton )
  {
    run();
    restart();
  }
}

void Qgs3DMapToolPointCloudChangeAttribute::activate()
{
  // cannot move this to the costructor as there are no mapSettings available yet when the tool is created
  if ( !mPolygonRubberBand )
  {
    mPolygonRubberBand = new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
    mPolygonRubberBand->setHideLastMarker( true );
  }
}

void Qgs3DMapToolPointCloudChangeAttribute::deactivate()
{
  restart();
}

void Qgs3DMapToolPointCloudChangeAttribute::setAttribute( const QString &attribute )
{
  mAttributeName = attribute;
}

void Qgs3DMapToolPointCloudChangeAttribute::setNewValue( double value )
{
  mNewValue = value;
}

void Qgs3DMapToolPointCloudChangeAttribute::run()
{
  if ( mScreenPoints.size() < 3 )
    return;

  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  const QgsGeometry searchPolygon = QgsGeometry( new QgsPolygon( new QgsLineString( mScreenPoints ) ) );
  QgsGeos preparedPolygon = QgsGeos( searchPolygon.constGet() );
  preparedPolygon.prepareGeometry();

  QgsMapLayer *mapLayer = QgisApp::instance()->activeLayer();
  Q_ASSERT( mapLayer->type() == Qgis::LayerType::PointCloud );
  QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( mapLayer );
  const SelectedPoints sel = searchPoints( pcLayer, preparedPolygon );

  int offset;
  const QgsPointCloudAttribute *attribute = pcLayer->attributes().find( mAttributeName, offset );

  pcLayer->undoStack()->beginMacro( tr( "Change attribute values" ) );
  for ( auto it = sel.begin(); it != sel.end(); ++it )
  {
    pcLayer->changeAttributeValue( it.key(), it.value(), *attribute, mNewValue );
  }
  pcLayer->undoStack()->endMacro();
}

void Qgs3DMapToolPointCloudChangeAttribute::restart()
{
  mCanvas->cameraController()->setInputHandlersEnabled( true );
  mScreenPoints.clear();
  mPolygonRubberBand->reset();
}


QgsPoint Qgs3DMapToolPointCloudChangeAttribute::screenPointToMap( const QPoint &pos ) const
{
  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( pos, mCanvas->size(), mCanvas->cameraController()->camera() );

  // pick an arbitrary point mid-way between near and far plane
  const float pointDistance = ( mCanvas->cameraController()->camera()->farPlane() + mCanvas->cameraController()->camera()->nearPlane() ) / 2;
  const QVector3D pointWorld = ray.origin() + pointDistance * ray.direction().normalized();

  const QgsVector3D origin = mCanvas->mapSettings()->origin();
  const QgsPoint pointMap( pointWorld.x() + origin.x(), pointWorld.y() + origin.y(), pointWorld.z() + origin.z() );
  return pointMap;
}


QgsGeometry Qgs3DMapToolPointCloudChangeAttribute::box3DToPolygonInScreenSpace( const QgsBox3D &box, const MapToPixel3D &mapToPixel3D )
{
  QVector<QgsPointXY> pts;
  for ( const QgsVector3D &c : box.corners() )
  {
    const QPointF pt = mapToPixel3D.transform( c.x(), c.y(), c.z() );
    pts.append( QgsPointXY( pt.x(), pt.y() ) );
  }

  // TODO: maybe we should only do rectangle check rather than (more precise) convex hull?

  // combine into QgsMultiPoint + apply convex hull
  const QgsGeometry g( new QgsMultiPoint( pts ) );
  return g.convexHull();
}


SelectedPoints Qgs3DMapToolPointCloudChangeAttribute::searchPoints( QgsPointCloudLayer *layer, const QgsGeos &searchPolygon ) const
{
  SelectedPoints result;

  MapToPixel3D mapToPixel3D;
  mapToPixel3D.VP = mCanvas->camera()->projectionMatrix() * mCanvas->camera()->viewMatrix();
  mapToPixel3D.origin = mCanvas->mapSettings()->origin();
  mapToPixel3D.canvasSize = mCanvas->size();

  QgsCoordinateTransform ct( layer->crs(), mCanvas->mapSettings()->crs(), mCanvas->mapSettings()->transformContext() );
  ct.setBallparkTransformsAreAppropriate( true );
  const double zValueScale = layer->elevationProperties()->zScale();
  const double zValueOffset = layer->elevationProperties()->zOffset();

  QgsPointCloudIndex index = layer->index();
  QVector<QgsPointCloudNodeId> nodes;
  QQueue<QgsPointCloudNodeId> queue;
  queue.append( index.root() );
  while ( !queue.empty() )
  {
    const QgsPointCloudNode node = index.getNode( queue.constFirst() );
    queue.removeFirst();

    const QgsBox3D bounds = node.bounds();
    QgsVector3D extentMin3D( bounds.xMinimum(), bounds.yMinimum(), bounds.zMinimum() * zValueScale + zValueOffset );
    QgsVector3D extentMax3D( bounds.xMaximum(), bounds.yMaximum(), bounds.zMaximum() * zValueScale + zValueOffset );
    try
    {
      extentMin3D = ct.transform( extentMin3D );
      extentMax3D = ct.transform( extentMax3D );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Error transforming node bounds coordinate" ) );
      continue;
    }

    const QgsBox3D box( extentMin3D.x(), extentMin3D.y(), extentMin3D.z(), extentMax3D.x(), extentMax3D.y(), extentMax3D.z() );
    // check whether the hull intersects the search polygon
    const QgsGeometry hull = box3DToPolygonInScreenSpace( box, mapToPixel3D );
    if ( searchPolygon.intersects( hull.constGet() ) )
    {
      nodes.append( node.id() );
      for ( const QgsPointCloudNodeId &child : node.children() )
      {
        queue.append( child );
      }
    }
  }

  for ( const QgsPointCloudNodeId &n : nodes )
  {
    const QVector<int> pts = selectedPointsInNode( searchPolygon, n, mapToPixel3D, layer );
    if ( !pts.isEmpty() )
    {
      result.insert( n, pts );
    }
  }
  return result;
}


QVector<int> Qgs3DMapToolPointCloudChangeAttribute::selectedPointsInNode( const QgsGeos &searchPolygon, const QgsPointCloudNodeId &n, const MapToPixel3D &mapToPixel3D, QgsPointCloudLayer *layer ) const
{
  QVector<int> selected;

  // Get the map's clipping extent in layer crs and skip if empty. We only need points within this extent.
  const Qgs3DMapSettings *map = mCanvas->mapSettings();
  const QgsRectangle mapExtent = Qgs3DUtils::tryReprojectExtent2D( map->extent(), map->crs(), layer->crs(), map->transformContext() );
  if ( mapExtent.isEmpty() )
    return selected;

  QgsPointCloudIndex pcIndex = layer->index();
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  QString categoryAttributeName;
  QSet<int> categoryValues;
  if ( QgsPointCloudLayer3DRenderer *renderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() ) )
  {
    if ( const QgsClassificationPointCloud3DSymbol *symbol = dynamic_cast<const QgsClassificationPointCloud3DSymbol *>( renderer->symbol() ) )
    {
      const QgsPointCloudCategoryList categories = symbol->categoriesList();
      for ( const auto &category : categories )
      {
        if ( category.renderState() )
          categoryValues.insert( category.value() );
      }

      const QgsPointCloudAttributeCollection allAttributes = pcIndex.attributes();
      categoryAttributeName = symbol->attribute();
      attributes.extend( allAttributes, { categoryAttributeName } );
    }
  }

  // we also need the filter expression so we can exclude filtered out points
  QgsPointCloudExpression filterExpression = pcIndex.subsetString();
  attributes.extend( pcIndex.attributes(), filterExpression.referencedAttributes() );

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  // we want to iterate all points so we have the correct point indexes within the node
  request.setIgnoreIndexFilterEnabled( true );

  // TODO: reuse cached block(s) if possible

  std::unique_ptr<QgsPointCloudBlock> block( pcIndex.nodeData( n, request ) );

  if ( !block )
    return selected;

  const bool filterIsValid = filterExpression.isValid();
  if ( !filterExpression.prepare( block.get() ) && filterIsValid )
    return selected;

  const QgsVector3D blockScale = block->scale();
  const QgsVector3D blockOffset = block->offset();

  const char *ptr = block->data();
  const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
  const std::size_t recordSize = blockAttributes.pointRecordSize();
  int xOffset = 0, yOffset = 0, zOffset = 0, categoryAttributeOffset = 0;
  const QgsPointCloudAttribute::DataType xType = blockAttributes.find( QStringLiteral( "X" ), xOffset )->type();
  const QgsPointCloudAttribute::DataType yType = blockAttributes.find( QStringLiteral( "Y" ), yOffset )->type();
  const QgsPointCloudAttribute::DataType zType = blockAttributes.find( QStringLiteral( "Z" ), zOffset )->type();
  const QgsPointCloudAttribute *categoryAttribute = const_cast<QgsPointCloudAttribute *>( blockAttributes.find( categoryAttributeName, categoryAttributeOffset ) );

  // we should adjust the Z based on the layer's elevation properties scale and offset
  const double layerZScale = static_cast<const QgsPointCloudLayerElevationProperties *>( layer->elevationProperties() )->zScale();
  const double layerZOffset = static_cast<const QgsPointCloudLayerElevationProperties *>( layer->elevationProperties() )->zOffset();

  QgsPoint pt;

  for ( int i = 0; i < block->pointCount(); ++i )
  {
    // ignore filtered out points
    if ( filterIsValid )
    {
      double eval = filterExpression.evaluate( i );
      if ( eval == 0.0 || std::isnan( eval ) )
      {
        continue;
      }
    }

    // if using categorized renderer, point might not be in a visible category
    if ( categoryAttribute )
    {
      const double categoryAttributeValue = categoryAttribute->convertValueToDouble( ptr + i * recordSize + categoryAttributeOffset );
      if ( !categoryValues.contains( categoryAttributeValue ) )
        continue;
    }

    // get map coordinates
    double x, y, z;
    QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, blockScale, blockOffset, x, y, z );
    z = z * layerZScale + layerZOffset;

    // check if inside map extent
    if ( !mapExtent.contains( x, y ) )
      continue;

    // project to screen (map coords -> world coords -> clip coords -> NDC -> screen coords)
    bool isInFrustum;
    const QPointF ptScreen = mapToPixel3D.transform( x, y, z, &isInFrustum );

    if ( !isInFrustum )
      continue;

    pt.setX( ptScreen.x() );
    pt.setY( ptScreen.y() );

    if ( searchPolygon.intersects( &pt ) )
    {
      selected.append( i );
    }
  }
  return selected;
}
