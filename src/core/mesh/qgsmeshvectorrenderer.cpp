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
#include "qgsrendercontext.h"
#include "qgsmaptopixel.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshtracerenderer.h"

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <QPen>
#include <QPainter>
#include <cmath>

///@cond PRIVATE

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif

inline double mag( double input )
{
  if ( input < 0.0 )
  {
    return -1.0;
  }
  return 1.0;
}

inline bool nodataValue( double x, double y )
{
  return ( std::isnan( x ) || std::isnan( y ) );
}

QgsMeshVectorArrowRenderer::QgsMeshVectorArrowRenderer(
  const QgsTriangularMesh &m,
  const QgsMeshDataBlock &datasetValues,
  const QVector<double> &datasetValuesMag,
  double datasetMagMaximumValue, double datasetMagMinimumValue,
  QgsMeshDatasetGroupMetadata::DataType dataType,
  const QgsMeshRendererVectorSettings &settings,
  QgsRenderContext &context,
  QSize size )  :
  mTriangularMesh( m )
  , mDatasetValues( datasetValues )
  , mDatasetValuesMag( datasetValuesMag )
  , mMinMag( datasetMagMinimumValue )
  , mMaxMag( datasetMagMaximumValue )
  , mContext( context )
  , mCfg( settings )
  , mDataType( dataType )
  , mOutputSize( size )
  , mBufferedExtent( context.mapExtent() )
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

  mVectorColoring = settings.vectorStrokeColoring();
}

QgsMeshVectorArrowRenderer::~QgsMeshVectorArrowRenderer() = default;

void QgsMeshVectorArrowRenderer::draw()
{
  // Set up the render configuration options
  QPainter *painter = mContext.painter();

  const QgsScopedQPainterState painterState( painter );
  mContext.setPainterFlagsUsingContext( painter );

  QPen pen = painter->pen();
  pen.setCapStyle( Qt::FlatCap );
  pen.setJoinStyle( Qt::MiterJoin );

  const double penWidth = mContext.convertToPainterUnits( mCfg.lineWidth(),
                          Qgis::RenderUnit::Millimeters );
  pen.setWidthF( penWidth );
  painter->setPen( pen );

  if ( mCfg.isOnUserDefinedGrid() )
  {
    drawVectorDataOnGrid( );
  }
  else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
  {
    drawVectorDataOnVertices( );
  }
  else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnFaces )
  {
    drawVectorDataOnFaces( );
  }
  else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnEdges )
  {
    drawVectorDataOnEdges( );
  }
}

bool QgsMeshVectorArrowRenderer::calcVectorLineEnd(
  QgsPointXY &lineEnd,
  double &vectorLength,
  double &cosAlpha,
  double &sinAlpha, //out
  const QgsPointXY &lineStart,
  double xVal,
  double yVal,
  double magnitude //in
)
{
  // return true on error

  if ( xVal == 0.0 && yVal == 0.0 )
    return true;

  // do not render if magnitude is outside of the filtered range (if filtering is enabled)
  if ( mCfg.filterMin() >= 0 && magnitude < mCfg.filterMin() )
    return true;
  if ( mCfg.filterMax() >= 0 && magnitude > mCfg.filterMax() )
    return true;

  // Determine the angle of the vector, counter-clockwise, from east
  // (and associated trigs)
  const double vectorAngle = -1.0 * atan( ( -1.0 * yVal ) / xVal ) - mContext.mapToPixel().mapRotation() * M_DEG2RAD;

  cosAlpha = cos( vectorAngle ) * mag( xVal );
  sinAlpha = sin( vectorAngle ) * mag( xVal );

  // Now determine the X and Y distances of the end of the line from the start
  double xDist = 0.0;
  double yDist = 0.0;
  switch ( mCfg.arrowSettings().shaftLengthMethod() )
  {
    case QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::MinMax:
    {
      const double minShaftLength = mContext.convertToPainterUnits( mCfg.arrowSettings().minShaftLength(),
                                    Qgis::RenderUnit::Millimeters );
      const double maxShaftLength = mContext.convertToPainterUnits( mCfg.arrowSettings().maxShaftLength(),
                                    Qgis::RenderUnit::Millimeters );
      const double minVal = mMinMag;
      const double maxVal = mMaxMag;
      const double k = ( magnitude - minVal ) / ( maxVal - minVal );
      const double L = minShaftLength + k * ( maxShaftLength - minShaftLength );
      xDist = cosAlpha * L;
      yDist = sinAlpha * L;
      break;
    }
    case QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::Scaled:
    {
      const double scaleFactor = mCfg.arrowSettings().scaleFactor();
      xDist = scaleFactor * xVal;
      yDist = scaleFactor * yVal;
      break;
    }
    case QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::Fixed:
    {
      // We must be using a fixed length
      const double fixedShaftLength = mContext.convertToPainterUnits( mCfg.arrowSettings().fixedShaftLength(),
                                      Qgis::RenderUnit::Millimeters );
      xDist = cosAlpha * fixedShaftLength;
      yDist = sinAlpha * fixedShaftLength;
      break;
    }
  }

  // Flip the Y axis (pixel vs real-world axis)
  yDist *= -1.0;

  if ( std::abs( xDist ) < 1 && std::abs( yDist ) < 1 )
    return true;

  // Determine the line coords
  lineEnd = QgsPointXY( lineStart.x() + xDist,
                        lineStart.y() + yDist );

  vectorLength = sqrt( xDist * xDist + yDist * yDist );

  // Check to see if both of the coords are outside the QImage area, if so, skip the whole vector
  if ( ( lineStart.x() < 0 || lineStart.x() > mOutputSize.width() ||
         lineStart.y() < 0 || lineStart.y() > mOutputSize.height() ) &&
       ( lineEnd.x() < 0   || lineEnd.x() > mOutputSize.width() ||
         lineEnd.y() < 0   || lineEnd.y() > mOutputSize.height() ) )
    return true;

  return false; //success
}

double QgsMeshVectorArrowRenderer::calcExtentBufferSize() const
{
  double buffer = 0;
  switch ( mCfg.arrowSettings().shaftLengthMethod() )
  {
    case QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::MinMax:
    {
      buffer = mContext.convertToPainterUnits( mCfg.arrowSettings().maxShaftLength(),
               Qgis::RenderUnit::Millimeters );
      break;
    }
    case QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::Scaled:
    {
      buffer = mCfg.arrowSettings().scaleFactor() * mMaxMag;
      break;
    }
    case QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::Fixed:
    {
      buffer = mContext.convertToPainterUnits( mCfg.arrowSettings().fixedShaftLength(),
               Qgis::RenderUnit::Millimeters );
      break;
    }
  }

  if ( mCfg.filterMax() >= 0 && buffer > mCfg.filterMax() )
    buffer = mCfg.filterMax();

  if ( buffer < 0.0 )
    buffer = 0.0;

  return buffer;
}


void QgsMeshVectorArrowRenderer::drawVectorDataOnVertices()
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

void QgsMeshVectorArrowRenderer::drawVectorDataOnPoints( const QSet<int> indexesToRender, const QVector<QgsMeshVertex> &points )
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

    const double V = mDatasetValuesMag[i];  // pre-calculated magnitude
    const QgsPointXY lineStart = mContext.mapToPixel().transform( center.x(), center.y() );

    drawVectorArrow( lineStart, xVal, yVal, V );
  }
}

void QgsMeshVectorArrowRenderer::drawVectorDataOnFaces( )
{
  const QList<int> trianglesInExtent = mTriangularMesh.faceIndexesForRectangle( mBufferedExtent );
  const QVector<QgsMeshVertex> &centroids = mTriangularMesh.faceCentroids();
  const QSet<int> nativeFacesInExtent = QgsMeshUtils::nativeFacesFromTriangles( trianglesInExtent,
                                        mTriangularMesh.trianglesToNativeFaces() );
  drawVectorDataOnPoints( nativeFacesInExtent, centroids );
}

void QgsMeshVectorArrowRenderer::drawVectorDataOnEdges()
{
  const QList<int> edgesInExtent = mTriangularMesh.edgeIndexesForRectangle( mBufferedExtent );
  const QVector<QgsMeshVertex> &centroids = mTriangularMesh.edgeCentroids();
  const QSet<int> nativeEdgesInExtent = QgsMeshUtils::nativeEdgesFromEdges( edgesInExtent,
                                        mTriangularMesh.edgesToNativeEdges() );
  drawVectorDataOnPoints( nativeEdgesInExtent, centroids );
}

void QgsMeshVectorArrowRenderer::drawVectorDataOnGrid( )
{
  if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnEdges ||
       mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVolumes )
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
          val.setX(
            QgsMeshLayerUtils::interpolateFromVerticesData(
              p1, p2, p3,
              val1.x(),
              val2.x(),
              val3.x(),
              p )
          );
          val.setY(
            QgsMeshLayerUtils::interpolateFromVerticesData(
              p1, p2, p3,
              val1.y(),
              val2.y(),
              val3.y(),
              p )
          );
        }
        else if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnFaces )
        {
          const auto val1 = mDatasetValues.value( nativeFaceIndex );
          val.setX(
            QgsMeshLayerUtils::interpolateFromFacesData(
              p1, p2, p3,
              val1.x(),
              p
            )
          );
          val.setY(
            QgsMeshLayerUtils::interpolateFromFacesData(
              p1, p2, p3,
              val1.y(),
              p
            )
          );
        }
        if ( nodataValue( val.x(), val.y() ) )
          continue;

        const QgsPointXY lineStart( x, y );
        drawVectorArrow( lineStart, val.x(), val.y(), val.scalar() );
      }
    }
  }
}

void QgsMeshVectorArrowRenderer::drawVectorArrow( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude )
{
  QgsPointXY lineEnd;
  double vectorLength;
  double cosAlpha, sinAlpha;
  if ( calcVectorLineEnd( lineEnd, vectorLength, cosAlpha, sinAlpha,
                          lineStart, xVal, yVal, magnitude ) )
    return;

  // Make a set of vector head coordinates that we will place at the end of each vector,
  // scale, translate and rotate.
  QgsPointXY vectorHeadPoints[3];
  QVector<QPointF> finalVectorHeadPoints( 3 );

  const double vectorHeadWidthRatio  = mCfg.arrowSettings().arrowHeadWidthRatio();
  const double vectorHeadLengthRatio = mCfg.arrowSettings().arrowHeadLengthRatio();

  // First head point:  top of ->
  vectorHeadPoints[0].setX( -1.0 * vectorHeadLengthRatio );
  vectorHeadPoints[0].setY( vectorHeadWidthRatio * 0.5 );

  // Second head point:  right of ->
  vectorHeadPoints[1].setX( 0.0 );
  vectorHeadPoints[1].setY( 0.0 );

  // Third head point:  bottom of ->
  vectorHeadPoints[2].setX( -1.0 * vectorHeadLengthRatio );
  vectorHeadPoints[2].setY( -1.0 * vectorHeadWidthRatio * 0.5 );

  // Determine the arrow head coords
  for ( int j = 0; j < 3; j++ )
  {
    finalVectorHeadPoints[j].setX( lineEnd.x()
                                   + ( vectorHeadPoints[j].x() * cosAlpha * vectorLength )
                                   - ( vectorHeadPoints[j].y() * sinAlpha * vectorLength )
                                 );

    finalVectorHeadPoints[j].setY( lineEnd.y()
                                   - ( vectorHeadPoints[j].x() * sinAlpha * vectorLength )
                                   - ( vectorHeadPoints[j].y() * cosAlpha * vectorLength )
                                 );
  }

  // Now actually draw the vector
  QPen pen( mContext.painter()->pen() );
  pen.setColor( mVectorColoring.color( magnitude ) );
  mContext.painter()->setPen( pen );
  mContext.painter()->drawLine( lineStart.toQPointF(), lineEnd.toQPointF() );
  mContext.painter()->drawPolygon( finalVectorHeadPoints );
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
  const QgsMeshRendererVectorSettings &settings,
  QgsRenderContext &context,
  const QgsRectangle &layerExtent,
  QgsMeshLayerRendererFeedback *feedBack,
  const QSize &size )
{
  QgsMeshVectorRenderer *renderer = nullptr;

  switch ( settings.symbology() )
  {
    case QgsMeshRendererVectorSettings::Arrows:
      renderer = new QgsMeshVectorArrowRenderer(
        m,
        datasetVectorValues,
        datasetValuesMag,
        datasetMagMaximumValue,
        datasetMagMinimumValue,
        dataType,
        settings,
        context,
        size );
      break;
    case QgsMeshRendererVectorSettings::Streamlines:
      renderer = new QgsMeshVectorStreamlineRenderer(
        m,
        datasetVectorValues,
        scalarActiveFaceFlagValues,
        datasetValuesMag,
        dataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices,
        settings,
        context,
        layerExtent,
        feedBack,
        datasetMagMaximumValue );
      break;
    case QgsMeshRendererVectorSettings::Traces:
      renderer = new QgsMeshVectorTraceRenderer(
        m,
        datasetVectorValues,
        scalarActiveFaceFlagValues,
        dataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices,
        settings,
        context,
        layerExtent,
        datasetMagMaximumValue );
      break;
  }

  return renderer;
}


///@endcond
