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

#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgscoordinatetransform.h"
#include "qgseventtracing.h"
#include "qgsgeos.h"
#include "qgsmultipoint.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointcloudlayerelevationproperties.h"

#include <QCamera>
#include <QQueue>
#include <QtConcurrentMap>

#include "moc_qgs3dmaptoolpointcloudchangeattribute.cpp"

class QgsPointCloudAttribute;
class QgsMapLayer;

Qgs3DMapToolPointCloudChangeAttribute::Qgs3DMapToolPointCloudChangeAttribute( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
}

Qgs3DMapToolPointCloudChangeAttribute::~Qgs3DMapToolPointCloudChangeAttribute() = default;

void Qgs3DMapToolPointCloudChangeAttribute::setAttribute( const QString &attribute )
{
  mAttributeName = attribute;
}

void Qgs3DMapToolPointCloudChangeAttribute::setNewValue( const double value )
{
  mNewValue = value;
}

void Qgs3DMapToolPointCloudChangeAttribute::setPointFilter( const QString &filter )
{
  mPointFilter = filter;
}

void Qgs3DMapToolPointCloudChangeAttribute::run()
{
}

void Qgs3DMapToolPointCloudChangeAttribute::restart()
{
}

void Qgs3DMapToolPointCloudChangeAttribute::changeAttributeValue( const QgsGeometry &geometry, const QString &attributeName, const double newValue, Qgs3DMapCanvas &canvas, QgsMapLayer *mapLayer )
{
  QgsEventTracing::ScopedEvent _trace( u"PointCloud"_s, u"Qgs3DMapToolPointCloudChangeAttribute::changeAttributeValue"_s );
  QgsGeos preparedPolygon = QgsGeos( geometry.constGet() );
  preparedPolygon.prepareGeometry();

  Q_ASSERT( mapLayer->type() == Qgis::LayerType::PointCloud );
  QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( mapLayer );
  const SelectedPoints sel = searchPoints( pcLayer, preparedPolygon, canvas );

  int offset;
  const QgsPointCloudAttribute *attribute = pcLayer->attributes().find( attributeName, offset );

  pcLayer->undoStack()->beginMacro( QObject::tr( "Change attribute values" ) );
  pcLayer->changeAttributeValue( sel, *attribute, newValue );
  pcLayer->undoStack()->endMacro();
}

SelectedPoints Qgs3DMapToolPointCloudChangeAttribute::searchPoints( QgsPointCloudLayer *layer, const QgsGeos &searchPolygon, Qgs3DMapCanvas &canvas )
{
  MapToPixel3D mapToPixel3D;
  mapToPixel3D.VP = canvas.camera()->projectionMatrix() * canvas.camera()->viewMatrix();
  mapToPixel3D.origin = canvas.mapSettings()->origin();
  mapToPixel3D.canvasSize = canvas.size();

  QgsCoordinateTransform ct( layer->crs(), canvas.mapSettings()->crs(), canvas.mapSettings()->transformContext() );
  ct.setBallparkTransformsAreAppropriate( true );
  const double zValueScale = layer->elevationProperties()->zScale();
  const double zValueOffset = layer->elevationProperties()->zOffset();

  QVector<QgsPointCloudNodeId> nodes;
  {
    QgsEventTracing::ScopedEvent _trace( u"PointCloud"_s, u"Qgs3DMapToolPointCloudChangeAttribute::searchPoints, looking for affected nodes"_s );
    QgsPointCloudIndex index = layer->index();
    const QList<QVector4D> clipPlanes = mCanvas->scene()->clipPlaneEquations();
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
        QgsDebugError( u"Error transforming node bounds coordinate"_s );
        continue;
      }

      const QgsBox3D box( extentMin3D.x(), extentMin3D.y(), extentMin3D.z(), extentMax3D.x(), extentMax3D.y(), extentMax3D.z() );
      // check whether the hull intersects the search polygon
      const QgsGeometry hull = box3DToPolygonInScreenSpace( box, mapToPixel3D );
      if ( !searchPolygon.intersects( hull.constGet() ) )
        continue;

      // check whether no one clip plane excludes the whole node
      bool allCornersClipped = false;
      for ( const QVector4D &plane : clipPlanes )
      {
        bool isClipped = true;
        for ( double x : { bounds.xMinimum(), bounds.xMaximum() } )
          for ( double y : { bounds.yMinimum(), bounds.yMaximum() } )
            for ( double z : { bounds.zMinimum(), bounds.zMaximum() } )
              isClipped = isClipped && pointIsClipped( mapToPixel3D.origin, { plane }, x, y, z );
        // This half-space excludes all of the node's corners, so it excludes
        // the whole node.
        if ( isClipped )
        {
          allCornersClipped = true;
          break;
        }
      }
      if ( allCornersClipped )
        continue;

      nodes.append( node.id() );
      for ( const QgsPointCloudNodeId &child : node.children() )
      {
        queue.append( child );
      }
    }
  }

  QgsEventTracing::ScopedEvent _trace2( u"PointCloud"_s, u"Qgs3DMapToolPointCloudChangeAttribute::searchPoints, selecting points"_s );

  // Get the map's clipping extent in layer crs and skip if empty. We only need points within this extent.
  const Qgs3DMapSettings *map = canvas.mapSettings();
  const QgsRectangle mapExtent = Qgs3DUtils::tryReprojectExtent2D( map->extent(), map->crs(), layer->crs(), map->transformContext() );
  if ( mapExtent.isEmpty() )
    return {};

  // The bulk of the time of this method is spent here, parallelise it.
  QgsPointCloudIndex index = layer->index();
  QgsPointCloudLayerElevationProperties elevationProperties = layer->elevationProperties();
  QgsAbstract3DRenderer *renderer3D = layer->renderer3D();

  // QtConcurrent requires std::function, bare lambdas lead to compile errors.
  std::function mapFn =
    [this, &searchPolygon, &mapToPixel3D, index = std::move( index ), &elevationProperties, renderer3D, mapExtent](
      const QgsPointCloudNodeId &n
    ) {
      const QVector<int> pts = selectedPointsInNode( searchPolygon, n, mapToPixel3D, index, mapExtent, elevationProperties, renderer3D );
      if ( pts.isEmpty() )
        return SelectedPoints {};
      else
        return SelectedPoints { { n, pts } };
    };

  std::function reduceFn = []( SelectedPoints &result, const SelectedPoints &pts ) {
    result.insert( pts );
  };

  SelectedPoints result = QtConcurrent::blockingMappedReduced<SelectedPoints>( nodes, std::move( mapFn ), std::move( reduceFn ) );
  return result;
}

bool Qgs3DMapToolPointCloudChangeAttribute::pointIsClipped( const QgsVector3D &mapOrigin, const QList<QVector4D> &clipPlanes, double x, double y, double z )
{
  const QgsVector3D pointInWorldCoords = Qgs3DUtils::mapToWorldCoordinates( QgsVector3D( x, y, z ), mapOrigin );
  for ( const QVector4D &clipPlane : clipPlanes )
  {
    // we manually calculate the dot product here so we save resources on some transformation
    const double distance = clipPlane.x() * pointInWorldCoords.x() + clipPlane.y() * pointInWorldCoords.y() + clipPlane.z() * pointInWorldCoords.z() + clipPlane.w();
    if ( distance < 0 )
      return true;
  }
  return false;
}

QVector<int> Qgs3DMapToolPointCloudChangeAttribute::selectedPointsInNode( const QgsGeos &searchPolygon, const QgsPointCloudNodeId &n, const MapToPixel3D &mapToPixel3D, QgsPointCloudIndex pcIndex, QgsRectangle mapExtent, QgsPointCloudLayerElevationProperties &elevationProperties, QgsAbstract3DRenderer *renderer3D )
{
  QVector<int> selected;

  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( u"X"_s, QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( u"Y"_s, QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( u"Z"_s, QgsPointCloudAttribute::Int32 ) );

  QString categoryAttributeName;
  QSet<int> categoryValues;
  if ( QgsPointCloudLayer3DRenderer *renderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( renderer3D ) )
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

  // we need the layer's filter expression so we can exclude filtered out points
  // and we need to combine that with the tool's point filter expression
  const QString layerFilter = pcIndex.subsetString();

  QgsPointCloudExpression filterExpression;

  if ( !layerFilter.isEmpty() && !mPointFilter.isEmpty() )
    filterExpression.setExpression( u"(%1) AND (%2)"_s.arg( layerFilter, mPointFilter ) );
  else if ( !mPointFilter.isEmpty() )
    filterExpression.setExpression( mPointFilter );
  else
    filterExpression.setExpression( layerFilter );

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
  const QgsPointCloudAttribute::DataType xType = blockAttributes.find( u"X"_s, xOffset )->type();
  const QgsPointCloudAttribute::DataType yType = blockAttributes.find( u"Y"_s, yOffset )->type();
  const QgsPointCloudAttribute::DataType zType = blockAttributes.find( u"Z"_s, zOffset )->type();
  const QgsPointCloudAttribute *categoryAttribute = const_cast<QgsPointCloudAttribute *>( blockAttributes.find( categoryAttributeName, categoryAttributeOffset ) );

  // we should adjust the Z based on the layer's elevation properties scale and offset
  const double layerZScale = elevationProperties.zScale();
  const double layerZOffset = elevationProperties.zOffset();

  QgsPoint pt;
  const QList<QVector4D> clipPlanes = mCanvas->scene()->clipPlaneEquations();
  const bool haveClipPlanes = !clipPlanes.isEmpty();

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

    if ( haveClipPlanes && pointIsClipped( mapToPixel3D.origin, clipPlanes, x, y, z ) )
      continue;

    if ( !searchPolygon.intersects( &pt ) )
      continue;

    selected.append( i );
  }
  return selected;
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
