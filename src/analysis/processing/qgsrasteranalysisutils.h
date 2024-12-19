/***************************************************************************
  qgsrasteranalysisutils.h
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

#ifndef QGSRASTERANALYSISUTILS_H
#define QGSRASTERANALYSISUTILS_H

#include "qgis_analysis.h"
#include "qgis.h"
#include "qgspointxy.h"

#include <functional>
#include <memory>
#include <vector>

#define SIP_NO_FILE

///@cond PRIVATE

class QgsRasterInterface;
class QgsGeometry;
class QgsRectangle;
class QgsProcessingParameterDefinition;
class QgsRasterProjector;
class QgsRasterDataProvider;
class QgsFeedback;
class QgsRasterBlock;

namespace QgsRasterAnalysisUtils
{

  /**
   * Analyzes which cells need to be considered to completely cover the bounding box of a feature.
  */
  void cellInfoForBBox( const QgsRectangle &rasterBBox, const QgsRectangle &featureBBox, double cellSizeX, double cellSizeY, int &nCellsX, int &nCellsY, int rasterWidth, int rasterHeight, QgsRectangle &rasterBlockExtent );

  //! Returns statistics by considering the pixels where the center point is within the polygon (fast)
  void statisticsFromMiddlePointTest( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox, const std::function<void( double, const QgsPointXY & )> &addValue, bool skipNodata = true );

  //! Returns statistics with precise pixel - polygon intersection test (slow)
  void statisticsFromPreciseIntersection( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox, const std::function<void( double, double, const QgsPointXY & )> &addValue, bool skipNodata = true );

  //! Tests whether a pixel's value should be included in the result
  bool validPixel( double value );

  //! Converts real-world map coordinates to raster row/col coordinates
  void mapToPixel( const double x, const double y, const QgsRectangle bounds, const double unitsPerPixelX, const double unitsPerPixelY, int &px, int &py );

  //! Converts raster row-col coordinates to real-world map coordinates
  void pixelToMap( const int px, const int py, const QgsRectangle bounds, const double unitsPerPixelX, const double unitsPerPixelY, double &x, double &y );

  /**
   * Returns a new processing enum parameter for choice of raster data types.
   * \see rasterTypeChoiceToDataType()
   */
  std::unique_ptr<QgsProcessingParameterDefinition> createRasterTypeParameter( const QString &name, const QString &description, Qgis::DataType defaultType = Qgis::DataType::Float32 );

  /**
   * Converts the value of a raster type parameter to the corresponding data type.
   * \see createRasterTypeParameter()
   */
  Qgis::DataType rasterTypeChoiceToDataType( int choice );

  struct RasterLogicInput
  {
      std::unique_ptr<QgsRasterInterface> sourceDataProvider;
      std::unique_ptr<QgsRasterProjector> projector;
      QgsRasterInterface *interface = nullptr;
      bool hasNoDataValue = false;
      std::vector<int> bands { 1 };
  };

  ANALYSIS_EXPORT void applyRasterLogicOperator( const std::vector<QgsRasterAnalysisUtils::RasterLogicInput> &inputs, QgsRasterDataProvider *destinationRaster, double outputNoDataValue, const bool treatNoDataAsFalse, int width, int height, const QgsRectangle &extent, QgsFeedback *feedback, std::function<void( const std::vector<std::unique_ptr<QgsRasterBlock>> &, bool &, bool &, int, int, bool )> &applyLogicFunc, qgssize &noDataCount, qgssize &trueCount, qgssize &falseCount );

  /**
   * Returns a vector of double values obtained from a stack of input QgsRasterBlocks
   */
  std::vector<double> getCellValuesFromBlockStack( const std::vector<std::unique_ptr<QgsRasterBlock>> &inputBlocks, int &row, int &col, bool &noDataInStack );

  /**
   * Enum of cell value statistic methods to be used with QgsProcessingParameterEnum
   */
  enum CellValueStatisticMethods
  {
    Sum,
    Count,
    Mean,
    Median,
    StandardDeviation,
    Variance,
    Minimum,
    Maximum,
    Minority,
    Majority,
    Range,
    Variety
  };

  /**
   * Returns the arithmetic mean from a vector of cell values
   */
  double meanFromCellValues( std::vector<double> &cellValues, int stackSize );

  /**
   * Returns the median from a vector of cell values
   */
  double medianFromCellValues( std::vector<double> &cellValues, int stackSize );

  /**
   * Returns the standard deviation from a vector of cell values
   */
  double stddevFromCellValues( std::vector<double> &cellValues, int stackSize );

  /**
   * Returns the variance from a vector of cell values
   */
  double varianceFromCellValues( std::vector<double> &cellValues, int stackSize );

  /**
   * Returns the maximum value from a vector of cell values
   */
  double maximumFromCellValues( std::vector<double> &cellValues );

  /**
   * Returns the minimum value from a vector of cell values
   */
  double minimumFromCellValues( std::vector<double> &cellValues );

  /**
   * Returns the majority value from a vector of cell values
   */
  double majorityFromCellValues( std::vector<double> &cellValues, const double noDataValue, int stackSize );

  /**
   * Returns the minority value from a vector of cell values
   */
  double minorityFromCellValues( std::vector<double> &cellValues, const double noDataValue, int stackSize );

  /**
   * Returns the range from a vector of cell values
   */
  double rangeFromCellValues( std::vector<double> &cellValues );

  /**
   * Returns the variety from a vector of cell values
   */
  double varietyFromCellValues( std::vector<double> &cellValues );

  enum CellValuePercentileMethods
  {
    NearestRankPercentile,
    InterpolatedPercentileInc,
    InterpolatedPercentileExc
  };

  /**
   * Returns the nearest rank percentile from a vector of cellValues,
   * percentile parameter ranges between 0 and 1
   */
  double nearestRankPercentile( std::vector<double> &cellValues, int stackSize, double percentile );

  /**
   * Returns the linearly interpolated percentile inclusive from a vector of cellValues,
   * percentile parameter ranges between 0 and 1 inclusive
   * see LibreOffice Calc's or Microsoft Excel's PERCENTILE.INC() function
   */
  double interpolatedPercentileInc( std::vector<double> &cellValues, int stackSize, double percentile );

  /**
   * Returns the linearly interpolated percentile inclusive from a vector of cellValues,
   * percentile parameter ranges between 0 and 1 exclusive
   * see LibreOffice Calc's or Microsoft Excel's PERCENTILE.EXC() function
   */
  double interpolatedPercentileExc( std::vector<double> &cellValues, int stackSize, double percentile, double noDataValue );

  enum CellValuePercentRankMethods
  {
    InterpolatedPercentRankInc,
    InterpolatedPercentRankExc
  };

  /**
   * Returns the linearly interpolated percentrank inclusive of a value from a vector of cellValues,
   * values outside the cellValue distribution (greater or smaller) will return noData
   * see LibreOffice Calc's or Microsoft Excel's PERCENTRANK.INC() function
   */
  double interpolatedPercentRankInc( std::vector<double> &cellValues, int stackSize, double value, double noDataValue );

  /**
   * Returns the linearly interpolated percentrank exclusive of a value from a vector of cellValues,
   * values outside the cellValue distribution (greater or smaller) will return noData
   * see LibreOffice Calc's or Microsoft Excel's PERCENTRANK.EXC() function
   */
  double interpolatedPercentRankExc( std::vector<double> &cellValues, int stackSize, double value, double noDataValue );

} // namespace QgsRasterAnalysisUtils


///@endcond PRIVATE

#endif // QGSRASTERANALYSISUTILS_H
