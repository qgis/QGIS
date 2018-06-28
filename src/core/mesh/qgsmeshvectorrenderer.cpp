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
#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"
#include "qgsunittypes.h"

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <QPen>
#include <QPainter>
#include <cmath>

///@cond PRIVATE

inline double mag( double input )
{
  if ( input < 0.0 )
  {
    return -1.0;
  }
  return 1.0;
}

QgsMeshVectorRenderer::QgsMeshVectorRenderer( const QgsTriangularMesh &m,
    const QVector<double> &datasetValuesX,
    const QVector<double> &datasetValuesY,
    const QVector<double> &datasetValuesMag,
    bool dataIsOnVertices,
    const QgsMeshRendererVectorSettings &settings,
    QgsRenderContext &context, const QSize &size )
  : mTriangularMesh( m )
  , mDatasetValuesX( datasetValuesX )
  , mDatasetValuesY( datasetValuesY )
  , mDatasetValuesMag( datasetValuesMag )
  , mContext( context )
  , mCfg( settings )
  , mDataOnVertices( dataIsOnVertices )
  , mOutputSize( size )
{
  auto bounds = std::minmax_element( mDatasetValuesMag.constBegin(), mDatasetValuesMag.constEnd() );
  mMinMag = *bounds.first;
  mMaxMag = *bounds.second;
}

QgsMeshVectorRenderer::~QgsMeshVectorRenderer() = default;

void QgsMeshVectorRenderer::draw()
{
  // Set up the render configuration options
  QPainter *painter = mContext.painter();
  painter->save();
  if ( mContext.flags() & QgsRenderContext::Antialiasing )
    painter->setRenderHint( QPainter::Antialiasing, true );

  painter->setRenderHint( QPainter::Antialiasing );
  QPen pen = painter->pen();
  pen.setCapStyle( Qt::FlatCap );
  pen.setJoinStyle( Qt::MiterJoin );

  double penWidth = mContext.convertToPainterUnits( mCfg.lineWidth(),
                    QgsUnitTypes::RenderUnit::RenderMillimeters );
  pen.setWidthF( penWidth );
  pen.setColor( mCfg.color() );
  painter->setPen( pen );

  if ( mDataOnVertices )
    drawVectorDataOnVertices();
  else
    drawVectorDataOnFaces();

  painter->restore();
}

bool QgsMeshVectorRenderer::calcVectorLineEnd(
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
  double vectorAngle = -1.0 * atan( ( -1.0 * yVal ) / xVal );
  cosAlpha = cos( vectorAngle ) * mag( xVal );
  sinAlpha = sin( vectorAngle ) * mag( xVal );

  // Now determine the X and Y distances of the end of the line from the start
  double xDist = 0.0;
  double yDist = 0.0;
  switch ( mCfg.shaftLengthMethod() )
  {
    case QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax:
    {
      double minShaftLength = mContext.convertToPainterUnits( mCfg.minShaftLength(),
                              QgsUnitTypes::RenderUnit::RenderMillimeters );
      double maxShaftLength = mContext.convertToPainterUnits( mCfg.maxShaftLength(),
                              QgsUnitTypes::RenderUnit::RenderMillimeters );
      double minVal = mMinMag;
      double maxVal = mMaxMag;
      double k = ( magnitude - minVal ) / ( maxVal - minVal );
      double L = minShaftLength + k * ( maxShaftLength - minShaftLength );
      xDist = cosAlpha * L;
      yDist = sinAlpha * L;
      break;
    }
    case QgsMeshRendererVectorSettings::ArrowScalingMethod::Scaled:
    {
      double scaleFactor = mCfg.scaleFactor();
      xDist = scaleFactor * xVal;
      yDist = scaleFactor * yVal;
      break;
    }
    case QgsMeshRendererVectorSettings::ArrowScalingMethod::Fixed:
    {
      // We must be using a fixed length
      double fixedShaftLength = mContext.convertToPainterUnits( mCfg.fixedShaftLength(),
                                QgsUnitTypes::RenderUnit::RenderMillimeters );
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

  // Check to see if any of the coords are outside the QImage area, if so, skip the whole vector
  if ( lineStart.x() < 0 || lineStart.x() > mOutputSize.width() ||
       lineStart.y() < 0 || lineStart.y() > mOutputSize.height() ||
       lineEnd.x() < 0   || lineEnd.x() > mOutputSize.width() ||
       lineEnd.y() < 0   || lineEnd.y() > mOutputSize.height() )
    return true;

  return false; //success
}


void QgsMeshVectorRenderer::drawVectorDataOnVertices()
{
  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices();

  // currently expecting that triangulation does not add any new extra vertices on the way
  Q_ASSERT( mDatasetValuesMag.count() == vertices.count() );

  for ( int i = 0; i < vertices.size(); ++i )
  {
    const QgsMeshVertex &vertex = vertices.at( i );
    //if (!nodeInsideView(nodeIndex))
    //    continue;

    double xVal = mDatasetValuesX[i];
    double yVal = mDatasetValuesY[i];
    double V = mDatasetValuesMag[i];  // pre-calculated magnitude
    QgsPointXY lineStart = mContext.mapToPixel().transform( vertex.x(), vertex.y() );

    drawVectorArrow( lineStart, xVal, yVal, V );
  }
}

void QgsMeshVectorRenderer::drawVectorDataOnFaces()
{
  const QVector<QgsMeshVertex> &centroids = mTriangularMesh.centroids();

  for ( int i = 0; i < centroids.count(); i++ )
  {
    //if (elemOutsideView(elemIndex))
    //    continue;

    QgsPointXY center = centroids.at( i );
    double xVal = mDatasetValuesX[i];
    double yVal = mDatasetValuesY[i];
    double V = mDatasetValuesMag[i];  // pre-calculated magnitude
    QgsPointXY lineStart = mContext.mapToPixel().transform( center.x(), center.y() );

    drawVectorArrow( lineStart, xVal, yVal, V );
  }
}


void QgsMeshVectorRenderer::drawVectorArrow( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude )
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

  double vectorHeadWidthRatio  = mCfg.arrowHeadWidthRatio();
  double vectorHeadLengthRatio = mCfg.arrowHeadLengthRatio();

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
  mContext.painter()->drawLine( lineStart.toQPointF(), lineEnd.toQPointF() );
  mContext.painter()->drawPolygon( finalVectorHeadPoints );
}

///@endcond
