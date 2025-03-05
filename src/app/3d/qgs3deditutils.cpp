/***************************************************************************
    qgs3deditutils.cpp
    ---------------------
    begin                : February 2025
    copyright            : (C) 2025 by Matej Bagar
    email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3deditutils.h"
#include "../qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgscoordinatetransform.h"
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

class QgsPointCloudAttribute;
class QgsMapLayer;
void Qgs3DEditUtils::changeAttributeValue( const QgsGeometry &geometry, const QString &attributeName, const double newValue, Qgs3DMapCanvas &canvas )
{
  QgsGeos preparedPolygon = QgsGeos( geometry.constGet() );
  preparedPolygon.prepareGeometry();

  QgsMapLayer *mapLayer = QgisApp::instance()->activeLayer();
  Q_ASSERT( mapLayer->type() == Qgis::LayerType::PointCloud );
  QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( mapLayer );
  const SelectedPoints sel = searchPoints( pcLayer, preparedPolygon, canvas );

  int offset;
  const QgsPointCloudAttribute *attribute = pcLayer->attributes().find( attributeName, offset );

  pcLayer->undoStack()->beginMacro( QObject::tr( "Change attribute values" ) );
  for ( auto it = sel.begin(); it != sel.end(); ++it )
  {
    pcLayer->changeAttributeValue( it.key(), it.value(), *attribute, newValue );
  }
  pcLayer->undoStack()->endMacro();
}

SelectedPoints Qgs3DEditUtils::searchPoints( QgsPointCloudLayer *layer, const QgsGeos &searchPolygon, Qgs3DMapCanvas &canvas )
{
  SelectedPoints result;

  MapToPixel3D mapToPixel3D;
  mapToPixel3D.VP = canvas.camera()->projectionMatrix() * canvas.camera()->viewMatrix();
  mapToPixel3D.origin = canvas.mapSettings()->origin();
  mapToPixel3D.canvasSize = canvas.size();

  QgsCoordinateTransform ct( layer->crs(), canvas.mapSettings()->crs(), canvas.mapSettings()->transformContext() );
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
    const QVector<int> pts = selectedPointsInNode( searchPolygon, n, mapToPixel3D, layer, canvas );
    if ( !pts.isEmpty() )
    {
      result.insert( n, pts );
    }
  }
  return result;
}

QVector<int> Qgs3DEditUtils::selectedPointsInNode( const QgsGeos &searchPolygon, const QgsPointCloudNodeId &n, const MapToPixel3D &mapToPixel3D, QgsPointCloudLayer *layer, Qgs3DMapCanvas &canvas )
{
  QVector<int> selected;

  // Get the map's clipping extent in layer crs and skip if empty. We only need points within this extent.
  const Qgs3DMapSettings *map = canvas.mapSettings();
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

QgsGeometry Qgs3DEditUtils::box3DToPolygonInScreenSpace( const QgsBox3D &box, const MapToPixel3D &mapToPixel3D )
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