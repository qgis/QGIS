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

#include <functional>
#include <memory>
#include <vector>

#include "qgis.h"
#include "qgis_analysis.h"
#include "qgspointxy.h"

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

namespace ANALYSIS_EXPORT QgsRasterAnalysisUtils
{

  /**
   * Analyzes which cells need to be considered to completely cover the bounding box of a feature.
  */
  void cellInfoForBBox(
    const QgsRectangle &rasterBBox, const QgsRectangle &featureBBox, double cellSizeX, double cellSizeY, int &nCellsX, int &nCellsY, int rasterWidth, int rasterHeight, QgsRectangle &rasterBlockExtent
  );

  //! Returns statistics by considering the pixels where the center point is within the polygon (fast)
  void statisticsFromMiddlePointTest(
    QgsRasterInterface *rasterInterface,
    int rasterBand,
    const QgsGeometry &poly,
    int nCellsX,
    int nCellsY,
    double cellSizeX,
    double cellSizeY,
    const QgsRectangle &rasterBBox,
    const std::function<void( double, const QgsPointXY & )> &addValue,
    bool skipNodata = true
  );

  //! Returns statistics with precise pixel - polygon intersection test (slow)
  void statisticsFromPreciseIntersection(
    QgsRasterInterface *rasterInterface,
    int rasterBand,
    const QgsGeometry &poly,
    int nCellsX,
    int nCellsY,
    double cellSizeX,
    double cellSizeY,
    const QgsRectangle &rasterBBox,
    const std::function<void( double, double, const QgsPointXY & )> &addValue,
    bool skipNodata = true
  );

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

  ANALYSIS_EXPORT void applyRasterLogicOperator(
    const std::vector<QgsRasterAnalysisUtils::RasterLogicInput> &inputs,
    std::unique_ptr<QgsRasterDataProvider> destinationRaster,
    double outputNoDataValue,
    const bool treatNoDataAsFalse,
    int width,
    int height,
    const QgsRectangle &extent,
    QgsFeedback *feedback,
    std::function<void( const std::vector<std::unique_ptr<QgsRasterBlock>> &, bool &, bool &, int, int, bool )> &applyLogicFunc,
    qgssize &noDataCount,
    qgssize &trueCount,
    qgssize &falseCount
  );

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

  /**
   * Calculates the Euclidean distance from a central grid cell to an adjacent neighbor
   * cell in a specified direction, accounting for rectangular cell dimensions.
   *
   * Neighbor directions follow SAGA's 8-neighbor clockwise convention starting at North:
   *
   * - 0 = North
   * - 1 = North-East
   * - 2 = East
   * - 3 = South-East
   * - 4 = South
   * - 5 = South-West
   * - 6 = West
   * - 7 = North-West
   *
   * \param direction Neighbor direction index (0 to 7).
   * \param cellSizeX Cell width in map units.
   * \param cellSizeY Cell height in map units.
   *
   * \returns The distance to the neighbor cell in map units.
   *
   * \since QGIS 4.4
   */
  double neighborCellDistance( int direction, double cellSizeX, double cellSizeY );

  /**
   * Computes the column and row indices for an adjacent neighbor cell in a specified direction,
   * and determines whether the resulting cell falls within valid grid extent boundaries.
   *
   * Neighbor directions follow SAGA's 8-neighbor clockwise convention starting at North:
   *
   * - 0 = North
   * - 1 = North-East
   * - 2 = East
   * - 3 = South-East
   * - 4 = South
   * - 5 = South-West
   * - 6 = West
   * - 7 = North-West
   *
   * \param direction Direction to the neighbor cell (0 to 7).
   * \param column Origin cell column index.
   * \param row Origin cell row index.
   * \param neighborColumn will be set to the column index of the neighbor cell.
   * \param neighborRow will be set to the row index of the neighbor cell.
   * \param columns Total number of columns in the grid.
   * \param rows Total number of rows in the grid.
   *
   * \returns TRUE if the neighbor cell lies strictly within grid boundaries
   *
   * \since QGIS 4.4
   */
  bool neighborCellCoordinates( int direction, int row, int column, int &neighborRow, int &neighborColumn, int rows, int columns );

  /**
   * Identifies the neighbor direction corresponding to the maximum surface gradient
   * from a central cell using the Deterministic 8 (D8) flow routing model.
   *
   * Evaluates all 8 adjacent neighbor cells in the raster block to find the direction of steepest
   * downslope drop (or steepest upslope rise if \a downhill is FALSE).
   *
   * - 0 = North
   * - 1 = North-East
   * - 2 = East
   * - 3 = South-East
   * - 4 = South
   * - 5 = South-West
   * - 6 = West
   * - 7 = North-West
   *
   * \param demBlock Raster block containing elevation surface values.
   * \param rows total number of rows in raster block.
   * \param row Central cell row index.
   * \param column Central cell column index.
   * \param cellSizeX Cell width in map units.
   * \param cellSizeY Cell height in map units.
   * \param downhill If TRUE, finds the steepest downslope direction. If FALSE, finds the steepest upslope direction.
   * \param noEdges If TRUE returns -1 if the central cell or any adjacent neighbor lies on the outer boundary of the raster block or borders a NoData cell.
   *
   * \returns the Direction index (0 to 7) corresponding to the steepest gradient, or -1 if no valid gradient
   *         exists (e.g. flat terrain, sink/pit cell, NoData cell, or edge cell when \a noEdges is TRUE).
   */
  int steepestGradientDirection( const QgsRasterBlock *demBlock, int row, int column, double cellSizeX, double cellSizeY, bool downhill = true, bool noEdges = true );


} //namespace ANALYSIS_EXPORT QgsRasterAnalysisUtils


///@endcond PRIVATE

#endif // QGSRASTERANALYSISUTILS_H
