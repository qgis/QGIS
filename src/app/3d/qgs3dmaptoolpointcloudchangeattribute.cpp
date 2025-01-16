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
#include "qgsmultipoint.h"
#include "qgsguiutils.h"
#include "qgisapp.h"

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

  QgsMapLayer *mapLayer = QgisApp::instance()->activeLayer();
  Q_ASSERT( mapLayer->type() == Qgis::LayerType::PointCloud );
  QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( mapLayer );
  const SelectedPoints sel = searchPoints( pcLayer, searchPolygon );

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

QgsGeometry Qgs3DMapToolPointCloudChangeAttribute::box3DToPolygonInScreenSpace( QgsBox3D box, const MapToPixel3D &mapToPixel3D )
{
  QVector<QgsPointXY> pts;
  for ( QgsVector3D c : box.corners() )
  {
    const QPointF pt = mapToPixel3D.transform( c.x(), c.y(), c.z() );
    pts.append( QgsPointXY( pt.x(), pt.y() ) );
  }

  // TODO: maybe we should only do rectangle check rather than (more precise) convex hull?

  // combine into QgsMultiPoint + apply convex hull
  const QgsGeometry g( new QgsMultiPoint( pts ) );
  return g.convexHull();
}

SelectedPoints Qgs3DMapToolPointCloudChangeAttribute::searchPoints( QgsPointCloudLayer *layer, const QgsGeometry &searchPolygon ) const
{
  SelectedPoints result;

  MapToPixel3D mapToPixel3D;
  mapToPixel3D.VP = mCanvas->camera()->projectionMatrix() * mCanvas->camera()->viewMatrix();
  mapToPixel3D.origin = mCanvas->mapSettings()->origin();
  mapToPixel3D.canvasSize = mCanvas->size();

  QgsPointCloudIndex pcIndex = layer->index();
  const QVector<const QgsChunkNode *> chunks = mCanvas->scene()->getLayerActiveChunkNodes( layer );
  for ( const QgsChunkNode *chunk : chunks )
  {
    // check whether the hull intersects the search polygon
    const QgsGeometry hull = box3DToPolygonInScreenSpace( chunk->box3D(), mapToPixel3D );
    if ( !hull.intersects( searchPolygon ) )
      continue;

    const QVector<int> pts = selectedPointsInNode( searchPolygon, chunk, mapToPixel3D, pcIndex );
    if ( !pts.isEmpty() )
    {
      const QgsPointCloudNodeId n( chunk->tileId().d, chunk->tileId().x, chunk->tileId().y, chunk->tileId().z );
      result.insert( n, pts );
    }
  }
  return result;
}


QVector<int> Qgs3DMapToolPointCloudChangeAttribute::selectedPointsInNode( const QgsGeometry &searchPolygon, const QgsChunkNode *ch, const MapToPixel3D &mapToPixel3D, QgsPointCloudIndex &pcIndex )
{
  QVector<int> selected;

  const QgsPointCloudNodeId n( ch->tileId().d, ch->tileId().x, ch->tileId().y, ch->tileId().z );
  QgsPointCloudRequest request;
  // TODO: apply filtering (if any)
  request.setAttributes( pcIndex.attributes() );

  // TODO: reuse cached block(s) if possible

  std::unique_ptr<QgsPointCloudBlock> block( pcIndex.nodeData( n, request ) );
  if ( !block )
    return selected;

  const QgsVector3D blockScale = block->scale();
  const QgsVector3D blockOffset = block->offset();

  const char *ptr = block->data();
  const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
  const std::size_t recordSize = blockAttributes.pointRecordSize();
  int xOffset = 0, yOffset = 0, zOffset = 0;
  const QgsPointCloudAttribute::DataType xType = blockAttributes.find( QStringLiteral( "X" ), xOffset )->type();
  const QgsPointCloudAttribute::DataType yType = blockAttributes.find( QStringLiteral( "Y" ), yOffset )->type();
  const QgsPointCloudAttribute::DataType zType = blockAttributes.find( QStringLiteral( "Z" ), zOffset )->type();
  for ( int i = 0; i < block->pointCount(); ++i )
  {
    // get map coordinates
    double x, y, z;
    QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, blockScale, blockOffset, x, y, z );

    // project to screen (map coords -> world coords -> clip coords -> NDC -> screen coords)
    const QPointF ptScreen = mapToPixel3D.transform( x, y, z );

    if ( searchPolygon.intersects( QgsGeometry( new QgsPoint( ptScreen.x(), ptScreen.y() ) ) ) )
    {
      selected.append( i );
    }
  }
  return selected;
}
