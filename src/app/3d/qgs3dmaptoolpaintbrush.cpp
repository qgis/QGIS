/***************************************************************************
  qgs3dmaptoolpaintbrush.cpp
  --------------------------------------
  Date                 : January 2025
  Copyright            : (C) 2025 by Matej Bagar
  Email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmaptoolpaintbrush.h"
#include "moc_qgs3dmaptoolpaintbrush.cpp"
#include "qgsrubberband3d.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgschunknode.h"
#include "qgswindow3dengine.h"
#include "qgsframegraph.h"
#include "qgsguiutils.h"
#include "qgsmultipoint.h"
#include "qgspointcloudlayer.h"
#include "qgs3dmaptoolpointcloudchangeattribute.h"
#include "qgisapp.h"

#include <QCursor>
#include <QMouseEvent>

class QgsPointCloudAttribute;
Qgs3DMapToolPaintBrush::Qgs3DMapToolPaintBrush( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
}

Qgs3DMapToolPaintBrush::~Qgs3DMapToolPaintBrush() = default;

void Qgs3DMapToolPaintBrush::processSelection() const
{
  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  const QgsGeometry searchSkeleton = QgsGeometry( new QgsLineString( mDragPositions ) );
  const QgsGeometry searchPolygon = searchSkeleton.buffer( mSelectionRubberBand->width() / 2, 37 );

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

SelectedPoints Qgs3DMapToolPaintBrush::searchPoints( QgsPointCloudLayer *layer, const QgsGeometry &searchPolygon ) const
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

QgsGeometry Qgs3DMapToolPaintBrush::box3DToPolygonInScreenSpace( const QgsBox3D &box, const MapToPixel3D &mapToPixel3D )
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

QVector<int> Qgs3DMapToolPaintBrush::selectedPointsInNode( const QgsGeometry &searchPolygon, const QgsChunkNode *ch, const MapToPixel3D &mapToPixel3D, QgsPointCloudIndex &pcIndex )
{
  QVector<int> selected;

  const QgsPointCloudNodeId n( ch->tileId().d, ch->tileId().x, ch->tileId().y, ch->tileId().z );
  QgsPointCloudRequest request;
  // TODO: apply filtering (if any)
  request.setAttributes( pcIndex.attributes() );

  // TODO: reuse cached block(s) if possible

  const std::unique_ptr block( pcIndex.nodeData( n, request ) );
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

void Qgs3DMapToolPaintBrush::activate()
{
  mCanvas->cameraController()->setInputHandlersEnabled( false );
  mSelectionRubberBand.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point, true ) );
  mSelectionRubberBand->setWidth( 32 );
  mSelectionRubberBand->setOutlineColor( mSelectionRubberBand->color() );
  mSelectionRubberBand->setColor( QColorConstants::Transparent );
  mSelectionRubberBand->addPoint( Qgs3DUtils::screenPointToMapCoordinates( QCursor::pos(), *mCanvas ) );
  mIsActive = true;
  mHighlighterRubberBand.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon ) );
  mHighlighterRubberBand->setMarkerType( QgsRubberBand3D::None );
}

void Qgs3DMapToolPaintBrush::deactivate()
{
  reset();
  mSelectionRubberBand.reset();
  mIsActive = false;
  mCanvas->cameraController()->setInputHandlersEnabled( true );
}

QCursor Qgs3DMapToolPaintBrush::cursor() const
{
  return Qt::CrossCursor;
}

void Qgs3DMapToolPaintBrush::reset()
{
  mDragPositions.clear();
  mHighlighterRubberBand->reset();
  mIsClicked = false;
}

void Qgs3DMapToolPaintBrush::setAttribute( const QString &attribute )
{
  mAttributeName = attribute;
}

void Qgs3DMapToolPaintBrush::setNewValue( double value )
{
  mNewValue = value;
}

void Qgs3DMapToolPaintBrush::generateHighlightArea()
{
  const QgsGeometry searchSkeleton = QgsGeometry( new QgsLineString( mDragPositions ) );
  const QgsGeometry searchGeometry = searchSkeleton.buffer( mSelectionRubberBand->width() / 2, 37 );
  QgsPolygon *searchPolygon = qgsgeometry_cast<QgsPolygon *>( searchGeometry.constGet() );
  Q_ASSERT( searchPolygon );
  auto transform = [this]( const QgsPoint &point ) -> QgsPoint {
    return Qgs3DUtils::screenPointToMapCoordinates( QPoint( point.x(), point.y() ), *mCanvas );
  };
  searchPolygon->addZValue( 0 );
  searchPolygon->transformVertices( transform );
  mHighlighterRubberBand->setPolygon( *searchPolygon );
}

void Qgs3DMapToolPaintBrush::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && !mIsMoving )
  {
    mIsClicked = true;
    mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
  }
}

void Qgs3DMapToolPaintBrush::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mIsClicked && event->button() == Qt::LeftButton && !mIsMoving )
  {
    mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
    mHighlighterRubberBand->reset();
    processSelection();
    mDragPositions.clear();
  }
  mIsClicked = false;
}

void Qgs3DMapToolPaintBrush::mouseMoveEvent( QMouseEvent *event )
{
  if ( mIsActive )
  {
    const QgsPoint newPos = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );
    mSelectionRubberBand->moveLastPoint( newPos );

    if ( mIsClicked && !mIsMoving )
    {
      mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
      generateHighlightArea();
    }
  }
}

void Qgs3DMapToolPaintBrush::mouseWheelEvent( QWheelEvent *event )
{
  // Moving horizontally or being in movement mode discards the event
  if ( event->angleDelta().y() == 0 || mIsMoving )
  {
    event->accept();
    return;
  }

  // Change the selection circle size. Moving the wheel forward (away) from the user makes
  // the circle smaller
  const QgsSettings settings;
  const bool reverseZoom = settings.value( QStringLiteral( "qgis/reverse_wheel_zoom" ), false ).toBool();
  const bool shrink = reverseZoom ? event->angleDelta().y() < 0 : event->angleDelta().y() > 0;
  double zoomFactor = shrink ? 0.75 : 1.5;
  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );
  mSelectionRubberBand->setWidth( mSelectionRubberBand->width() * zoomFactor );
}

void Qgs3DMapToolPaintBrush::keyPressEvent( QKeyEvent *event )
{
  if ( mIsClicked && event->key() == Qt::Key_Escape )
  {
    reset();
  }

  if ( !mIsClicked && event->key() == Qt::Key_Space )
  {
    const bool newState = !mCanvas->cameraController()->inputHandlersEnabled();
    mCanvas->cameraController()->setInputHandlersEnabled( newState );
    mIsMoving = newState;
  }
}
