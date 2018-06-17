/***************************************************************************
  qgsrasteranalysisutils.cpp
  ---------------------
  Date                 : June 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasteranalysisutils.h"

#include "qgsfeedback.h"
#include "qgsrasterblock.h"
#include "qgsrasteriterator.h"
#include "qgsgeos.h"
#include "qgsprocessingparameters.h"
#include <map>
///@cond PRIVATE

void QgsRasterAnalysisUtils::cellInfoForBBox( const QgsRectangle &rasterBBox, const QgsRectangle &featureBBox, double cellSizeX, double cellSizeY,
    int &nCellsX, int &nCellsY, int rasterWidth, int rasterHeight, QgsRectangle &rasterBlockExtent )
{
  //get intersecting bbox
  QgsRectangle intersectBox = rasterBBox.intersect( featureBBox );
  if ( intersectBox.isEmpty() )
  {
    nCellsX = 0;
    nCellsY = 0;
    rasterBlockExtent = QgsRectangle();
    return;
  }

  //get offset in pixels in x- and y- direction
  int offsetX = static_cast< int >( std::floor( ( intersectBox.xMinimum() - rasterBBox.xMinimum() ) / cellSizeX ) );
  int offsetY = static_cast< int >( std::floor( ( rasterBBox.yMaximum() - intersectBox.yMaximum() ) / cellSizeY ) );

  int maxColumn = static_cast< int >( std::floor( ( intersectBox.xMaximum() - rasterBBox.xMinimum() ) / cellSizeX ) ) + 1;
  int maxRow = static_cast< int >( std::floor( ( rasterBBox.yMaximum() - intersectBox.yMinimum() ) / cellSizeY ) ) + 1;

  nCellsX = maxColumn - offsetX;
  nCellsY = maxRow - offsetY;

  //avoid access to cells outside of the raster (may occur because of rounding)
  nCellsX = std::min( offsetX + nCellsX, rasterWidth ) - offsetX;
  nCellsY = std::min( offsetY + nCellsY, rasterHeight ) - offsetY;

  rasterBlockExtent = QgsRectangle( rasterBBox.xMinimum() + offsetX * cellSizeX,
                                    rasterBBox.yMaximum() - offsetY * cellSizeY,
                                    rasterBBox.xMinimum() + ( nCellsX + offsetX ) * cellSizeX,
                                    rasterBBox.yMaximum() - ( nCellsY + offsetY ) * cellSizeY );
}

void QgsRasterAnalysisUtils::statisticsFromMiddlePointTest( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox,  const std::function<void( double )> &addValue, bool skipNodata )
{
  std::unique_ptr< QgsGeometryEngine > polyEngine( QgsGeometry::createGeometryEngine( poly.constGet( ) ) );
  if ( !polyEngine )
  {
    return;
  }
  polyEngine->prepareGeometry();

  QgsRasterIterator iter( rasterInterface );
  iter.startRasterRead( rasterBand, nCellsX, nCellsY, rasterBBox );

  std::unique_ptr< QgsRasterBlock > block;
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;
  while ( iter.readNextRasterPart( rasterBand, iterCols, iterRows, block, iterLeft, iterTop, &blockExtent ) )
  {
    double cellCenterY = blockExtent.yMaximum() - 0.5 * cellSizeY;

    for ( int row = 0; row < iterRows; ++row )
    {
      double cellCenterX = blockExtent.xMinimum() + 0.5 * cellSizeX;
      for ( int col = 0; col < iterCols; ++col )
      {
        double pixelValue = block->value( row, col );
        if ( validPixel( pixelValue ) && ( !skipNodata || !block->isNoData( row, col ) ) )
        {
          QgsPoint cellCenter( cellCenterX, cellCenterY );
          if ( polyEngine->contains( &cellCenter ) )
          {
            addValue( pixelValue );
          }
        }
        cellCenterX += cellSizeX;
      }
      cellCenterY -= cellSizeY;
    }
  }
}

void QgsRasterAnalysisUtils::statisticsFromPreciseIntersection( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox,  const std::function<void( double, double )> &addValue, bool skipNodata )
{
  QgsGeometry pixelRectGeometry;

  double hCellSizeX = cellSizeX / 2.0;
  double hCellSizeY = cellSizeY / 2.0;
  double pixelArea = cellSizeX * cellSizeY;
  double weight = 0;

  std::unique_ptr< QgsGeometryEngine > polyEngine( QgsGeometry::createGeometryEngine( poly.constGet( ) ) );
  if ( !polyEngine )
  {
    return;
  }
  polyEngine->prepareGeometry();

  QgsRasterIterator iter( rasterInterface );
  iter.startRasterRead( rasterBand, nCellsX, nCellsY, rasterBBox );

  std::unique_ptr< QgsRasterBlock > block;
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;
  while ( iter.readNextRasterPart( rasterBand, iterCols, iterRows, block, iterLeft, iterTop, &blockExtent ) )
  {
    double currentY = blockExtent.yMaximum() - 0.5 * cellSizeY;
    for ( int row = 0; row < iterRows; ++row )
    {
      double currentX = blockExtent.xMinimum() + 0.5 * cellSizeX;
      for ( int col = 0; col < iterCols; ++col )
      {
        double pixelValue = block->value( row, col );
        if ( validPixel( pixelValue ) && ( !skipNodata || !block->isNoData( row, col ) ) )
        {
          pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - hCellSizeX, currentY - hCellSizeY, currentX + hCellSizeX, currentY + hCellSizeY ) );
          // GEOS intersects tests on prepared geometry is MAGNITUDES faster than calculating the intersection itself,
          // so we first test to see if there IS an intersection before doing the actual calculation
          if ( !pixelRectGeometry.isNull() && polyEngine->intersects( pixelRectGeometry.constGet() ) )
          {
            //intersection
            QgsGeometry intersectGeometry = pixelRectGeometry.intersection( poly );
            if ( !intersectGeometry.isEmpty() )
            {
              double intersectionArea = intersectGeometry.area();
              if ( intersectionArea > 0.0 )
              {
                weight = intersectionArea / pixelArea;
                addValue( pixelValue, weight );
              }
            }
          }
        }
        currentX += cellSizeX;
      }
      currentY -= cellSizeY;
    }
  }
}

bool QgsRasterAnalysisUtils::validPixel( double value )
{
  return !std::isnan( value );
}

static QVector< QPair< QString, Qgis::DataType > > sDataTypes;

void populateDataTypes()
{
  if ( sDataTypes.empty() )
  {
    sDataTypes.append( qMakePair( QStringLiteral( "Byte" ), Qgis::Byte ) );
    sDataTypes.append( qMakePair( QStringLiteral( "Int16" ), Qgis::Int16 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "UInt16" ), Qgis::UInt16 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "Int32" ), Qgis::Int32 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "UInt32" ), Qgis::UInt32 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "Float32" ), Qgis::Float32 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "Float64" ), Qgis::Float64 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "CInt16" ), Qgis::CInt16 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "CInt32" ), Qgis::CInt32 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "CFloat32" ), Qgis::CFloat32 ) );
    sDataTypes.append( qMakePair( QStringLiteral( "CFloat64" ), Qgis::CFloat64 ) );
  }
}

std::unique_ptr<QgsProcessingParameterDefinition> QgsRasterAnalysisUtils::createRasterTypeParameter( const QString &name, const QString &description, Qgis::DataType defaultType )
{
  populateDataTypes();

  QStringList names;
  int defaultChoice = 0;
  int i = 0;
  for ( auto it = sDataTypes.constBegin(); it != sDataTypes.constEnd(); ++it )
  {
    names.append( it->first );
    if ( it->second == defaultType )
      defaultChoice = i;
    i++;
  }

  return qgis::make_unique< QgsProcessingParameterEnum >( name, description, names, false, defaultChoice );
}

Qgis::DataType QgsRasterAnalysisUtils::rasterTypeChoiceToDataType( int choice )
{
  if ( choice < 0 || choice >= sDataTypes.count() )
    return Qgis::Float32;

  return sDataTypes.value( choice ).second;
}

///@endcond PRIVATE

