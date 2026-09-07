/***************************************************************************
                         qgsmeshvectorrenderer.cpp
                         -------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshvectorrenderer.h"

#include <cmath>

#include "qgsmaptopixel.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshtracerenderer.h"
#include "qgsrendercontext.h"
#include "qgstriangularmesh.h"

#include <QPainter>
#include <QPen>

///@cond PRIVATE

inline bool nodataValue( double x, double y )
{
  return ( std::isnan( x ) || std::isnan( y ) );
}

QgsMeshVectorRenderer::~QgsMeshVectorRenderer() = default;

QgsMeshVectorRenderer *QgsMeshVectorRenderer::makeVectorRenderer(
  const QgsTriangularMesh &m,
  const QgsMeshDataBlock &datasetVectorValues,
  const QgsMeshDataBlock &scalarActiveFaceFlagValues,
  const QVector<double> &datasetValuesMag,
  double datasetMagMaximumValue,
  double datasetMagMinimumValue,
  QgsMeshDatasetGroupMetadata::DataType dataType,
  const QgsVectorFieldSettings &settings,
  QgsRenderContext &context,
  const QgsRectangle &layerExtent,
  QgsMeshLayerRendererFeedback *feedBack,
  const QSize &size
)
{
  QgsMeshVectorRenderer *renderer = nullptr;

  switch ( settings.symbology() )
  {
    case QgsVectorFieldSettings::Symbology::Arrows:
    case QgsVectorFieldSettings::Symbology::WindBarbs:
      renderer = new QgsMeshVectorGlyphRenderer( m, datasetVectorValues, datasetValuesMag, datasetMagMaximumValue, datasetMagMinimumValue, dataType, settings, context, size );
      break;
    case QgsVectorFieldSettings::Symbology::Streamlines:
      renderer
        = new QgsMeshVectorStreamlineRenderer( m, datasetVectorValues, scalarActiveFaceFlagValues, datasetValuesMag, dataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices, settings, context, layerExtent, feedBack, datasetMagMaximumValue );
      break;
    case QgsVectorFieldSettings::Symbology::Traces:
      renderer
        = new QgsMeshVectorTraceRenderer( m, datasetVectorValues, scalarActiveFaceFlagValues, dataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices, settings, context, layerExtent, datasetMagMaximumValue );
      break;
  }

  return renderer;
}

QgsMeshVectorGlyphRenderer::QgsMeshVectorGlyphRenderer(
  const QgsTriangularMesh &m,
  const QgsMeshDataBlock &datasetValues,
  const QVector<double> &datasetValuesMag,
  double datasetMagMaximumValue,
  double datasetMagMinimumValue,
  QgsMeshDatasetGroupMetadata::DataType dataType,
  const QgsVectorFieldSettings &settings,
  QgsRenderContext &context,
  QSize size
)
  : mTriangularMesh( m )
  , mDatasetValues( datasetValues )
  , mDatasetValuesMag( datasetValuesMag )
  , mMinMag( datasetMagMinimumValue )
  , mMaxMag( datasetMagMaximumValue )
  , mDataType( dataType )
  , mBufferedExtent( context.mapExtent() )
  , mContext( context )
  , mCfg( settings )
  , mOutputSize( size )
  , mEngine( datasetMagMaximumValue, datasetMagMinimumValue, settings, context, size )
{
  // should be checked in caller
  Q_ASSERT( !mDatasetValuesMag.empty() );
  Q_ASSERT( !std::isnan( mMinMag ) );
  Q_ASSERT( !std::isnan( mMaxMag ) );
  Q_ASSERT( mDatasetValues.isValid() );
  Q_ASSERT( QgsMeshDataBlock::Vector2DDouble == mDatasetValues.type() );

  // we need to expand out the extent so that it includes
  // arrows which start or end up outside of the
  // actual visible extent
  const double extension = context.convertToMapUnits( calcExtentBufferSize(), Qgis::RenderUnit::Pixels );
  mBufferedExtent.setXMinimum( mBufferedExtent.xMinimum() - extension );
  mBufferedExtent.setXMaximum( mBufferedExtent.xMaximum() + extension );
  mBufferedExtent.setYMinimum( mBufferedExtent.yMinimum() - extension );
  mBufferedExtent.setYMaximum( mBufferedExtent.yMaximum() + extension );
}

QgsMeshVectorGlyphRenderer::~QgsMeshVectorGlyphRenderer() = default;

void QgsMeshVectorGlyphRenderer::draw()
{
  if ( mCfg.isOnUserDefinedGrid() )
  {
    drawVectorDataOnGrid();
  }
  else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
  {
    drawVectorDataOnVertices();
  }
  else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnFaces )
  {
    drawVectorDataOnFaces();
  }
  else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnEdges )
  {
    drawVectorDataOnEdges();
  }
}

double QgsMeshVectorGlyphRenderer::calcExtentBufferSize() const
{
  double buffer = 0;
  switch ( mCfg.arrowSettings().shaftLengthMethod() )
  {
    case QgsVectorFieldArrowSettings::ArrowScalingMethod::MinMax:
    {
      buffer = mContext.convertToPainterUnits( mCfg.arrowSettings().maxShaftLength(), Qgis::RenderUnit::Millimeters );
      break;
    }
    case QgsVectorFieldArrowSettings::ArrowScalingMethod::Scaled:
    {
      buffer = mCfg.arrowSettings().scaleFactor() * mMaxMag;
      break;
    }
    case QgsVectorFieldArrowSettings::ArrowScalingMethod::Fixed:
    {
      buffer = mContext.convertToPainterUnits( mCfg.arrowSettings().fixedShaftLength(), Qgis::RenderUnit::Millimeters );
      break;
    }
  }

  if ( mCfg.filterMax() >= 0 && buffer > mCfg.filterMax() )
    buffer = mCfg.filterMax();

  if ( buffer < 0.0 )
    buffer = 0.0;

  return buffer;
}


void QgsMeshVectorGlyphRenderer::drawVectorDataOnVertices()
{
  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices();
  QSet<int> verticesToDraw;

  // currently expecting that triangulation does not add any new extra vertices on the way
  Q_ASSERT( mDatasetValuesMag.count() == vertices.count() );

  // find all vertices from faces to render
  {
    const QList<int> trianglesInExtent = mTriangularMesh.faceIndexesForRectangle( mBufferedExtent );
    const QVector<QgsMeshFace> &triangles = mTriangularMesh.triangles();
    verticesToDraw.unite( QgsMeshUtils::nativeVerticesFromTriangles( trianglesInExtent, triangles ) );
  }

  // find all vertices from edges to render
  {
    const QList<int> edgesInExtent = mTriangularMesh.edgeIndexesForRectangle( mBufferedExtent );
    const QVector<QgsMeshEdge> &edges = mTriangularMesh.edges();
    verticesToDraw.unite( QgsMeshUtils::nativeVerticesFromEdges( edgesInExtent, edges ) );
  }

  // render
  drawVectorDataOnPoints( verticesToDraw, vertices );
}

void QgsMeshVectorGlyphRenderer::drawVectorDataOnPoints( const QSet<int> indexesToRender, const QVector<QgsMeshVertex> &points )
{
  for ( const int i : indexesToRender )
  {
    if ( mContext.renderingStopped() )
      break;

    const QgsPointXY center = points.at( i );
    if ( !mBufferedExtent.contains( center ) )
      continue;

    const QgsMeshDatasetValue val = mDatasetValues.value( i );
    const double xVal = val.x();
    const double yVal = val.y();
    if ( nodataValue( xVal, yVal ) )
      continue;

    const double V = mDatasetValuesMag[i]; // pre-calculated magnitude
    const QgsPointXY lineStart = mContext.mapToPixel().transform( center.x(), center.y() );

    mEngine.drawGlyph( lineStart, xVal, yVal, V );
  }
}

void QgsMeshVectorGlyphRenderer::drawVectorDataOnFaces()
{
  const QList<int> trianglesInExtent = mTriangularMesh.faceIndexesForRectangle( mBufferedExtent );
  const QVector<QgsMeshVertex> &centroids = mTriangularMesh.faceCentroids();
  const QSet<int> nativeFacesInExtent = QgsMeshUtils::nativeFacesFromTriangles( trianglesInExtent, mTriangularMesh.trianglesToNativeFaces() );
  drawVectorDataOnPoints( nativeFacesInExtent, centroids );
}

void QgsMeshVectorGlyphRenderer::drawVectorDataOnEdges()
{
  const QList<int> edgesInExtent = mTriangularMesh.edgeIndexesForRectangle( mBufferedExtent );
  const QVector<QgsMeshVertex> &centroids = mTriangularMesh.edgeCentroids();
  const QSet<int> nativeEdgesInExtent = QgsMeshUtils::nativeEdgesFromEdges( edgesInExtent, mTriangularMesh.edgesToNativeEdges() );
  drawVectorDataOnPoints( nativeEdgesInExtent, centroids );
}

void QgsMeshVectorGlyphRenderer::drawVectorDataOnGrid()
{
  if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnEdges || mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVolumes )
    return;

  const QList<int> trianglesInExtent = mTriangularMesh.faceIndexesForRectangle( mBufferedExtent );
  const int cellx = mCfg.userGridCellWidth();
  const int celly = mCfg.userGridCellHeight();

  const QVector<QgsMeshFace> &triangles = mTriangularMesh.triangles();
  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices();

  for ( const int i : trianglesInExtent )
  {
    if ( mContext.renderingStopped() )
      break;

    const QgsMeshFace &face = triangles[i];

    const int v1 = face[0], v2 = face[1], v3 = face[2];
    const QgsPoint p1 = vertices[v1], p2 = vertices[v2], p3 = vertices[v3];

    const int nativeFaceIndex = mTriangularMesh.trianglesToNativeFaces()[i];

    // Get the BBox of the element in pixels
    const QgsRectangle bbox = QgsMeshLayerUtils::triangleBoundingBox( p1, p2, p3 );
    int left, right, top, bottom;
    QgsMeshLayerUtils::boundingBoxToScreenRectangle( mContext.mapToPixel(), mOutputSize, bbox, left, right, top, bottom );

    // Align rect to the grid (e.g. interval <13, 36> with grid cell 10 will be trimmed to <20,30>
    if ( left % cellx != 0 )
      left += cellx - ( left % cellx );
    if ( right % cellx != 0 )
      right -= ( right % cellx );
    if ( top % celly != 0 )
      top += celly - ( top % celly );
    if ( bottom % celly != 0 )
      bottom -= ( bottom % celly );

    for ( int y = top; y <= bottom; y += celly )
    {
      for ( int x = left; x <= right; x += cellx )
      {
        QgsMeshDatasetValue val;
        const QgsPointXY p = mContext.mapToPixel().toMapCoordinates( x, y );

        if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
        {
          const auto val1 = mDatasetValues.value( v1 );
          const auto val2 = mDatasetValues.value( v2 );
          const auto val3 = mDatasetValues.value( v3 );
          val.setX( QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.x(), val2.x(), val3.x(), p ) );
          val.setY( QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.y(), val2.y(), val3.y(), p ) );
        }
        else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnFaces )
        {
          const auto val1 = mDatasetValues.value( nativeFaceIndex );
          val.setX( QgsMeshLayerUtils::interpolateFromFacesData( p1, p2, p3, val1.x(), p ) );
          val.setY( QgsMeshLayerUtils::interpolateFromFacesData( p1, p2, p3, val1.y(), p ) );
        }
        if ( nodataValue( val.x(), val.y() ) )
          continue;

        const QgsPointXY lineStart( x, y );
        mEngine.drawGlyph( lineStart, val.x(), val.y(), val.scalar() );
      }
    }
  }
}

///@endcond
