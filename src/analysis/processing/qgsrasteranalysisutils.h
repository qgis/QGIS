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
  void cellInfoForBBox( const QgsRectangle &rasterBBox, const QgsRectangle &featureBBox, double cellSizeX, double cellSizeY, int &nCellsX, int &nCellsY,
                        int rasterWidth, int rasterHeight,
                        QgsRectangle &rasterBlockExtent );

  //! Returns statistics by considering the pixels where the center point is within the polygon (fast)
  void statisticsFromMiddlePointTest( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY,
                                      double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox, const std::function<void( double )> &addValue, bool skipNodata = true );

  //! Returns statistics with precise pixel - polygon intersection test (slow)
  void statisticsFromPreciseIntersection( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY,
                                          double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox, const std::function<void( double, double )> &addValue, bool skipNodata = true );

  //! Tests whether a pixel's value should be included in the result
  bool validPixel( double value );

  /**
   * Returns a new processing enum parameter for choice of raster data types.
   * \see rasterTypeChoiceToDataType()
   */
  std::unique_ptr< QgsProcessingParameterDefinition > createRasterTypeParameter( const QString &name,
      const QString &description,
      Qgis::DataType defaultType = Qgis::Float32 );

  /**
   * Converts the value of a raster type parameter to the corresponding data type.
   * \see createRasterTypeParameter()
   */
  Qgis::DataType rasterTypeChoiceToDataType( int choice );

  struct RasterLogicInput
  {
    std::unique_ptr< QgsRasterInterface > sourceDataProvider;
    std::unique_ptr< QgsRasterProjector> projector;
    QgsRasterInterface *interface = nullptr;
    bool hasNoDataValue = false;
    std::vector< int > bands { 1 };
  };

  ANALYSIS_EXPORT void applyRasterLogicOperator( const std::vector< QgsRasterAnalysisUtils::RasterLogicInput > &inputs, QgsRasterDataProvider *destinationRaster, double outputNoDataValue, const bool treatNoDataAsFalse,
      int width, int height, const QgsRectangle &extent, QgsFeedback *feedback,
      std::function<void( const std::vector< std::unique_ptr< QgsRasterBlock > > &, bool &, bool &, int, int, bool )> &applyLogicFunc,
      qgssize &noDataCount, qgssize &trueCount, qgssize &falseCount );

}


///@endcond PRIVATE

#endif // QGSRASTERANALYSISUTILS_H
