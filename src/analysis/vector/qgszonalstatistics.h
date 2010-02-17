/***************************************************************************
                          qgszonalstatistics.h  -  description
                          ----------------------------
    begin                : August 29th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSZONALSTATISTICS_H
#define QGSZONALSTATISTICS_H

#include "qgsrectangle.h"
#include <QString>

class QgsGeometry;
class QgsVectorLayer;
class QProgressDialog;

/**A class that calculates raster statistics (count, sum, mean) for a polygon or multipolygon layer and appends the results as attributes*/
class ANALYSIS_EXPORT QgsZonalStatistics
{
  public:
    QgsZonalStatistics( QgsVectorLayer* polygonLayer, const QString& rasterFile, const QString& attributePrefix = "", int rasterBand = 1 );
    ~QgsZonalStatistics();

    /**Starts the calculation
      @return 0 in case of success*/
    int calculateStatistics( QProgressDialog* p );

  private:
    QgsZonalStatistics();
    /**Analysis what cells need to be considered to cover the bounding box of a feature
      @return 0 in case of success*/
    int cellInfoForBBox( const QgsRectangle& rasterBBox, const QgsRectangle& featureBBox, double cellSizeX, double cellSizeY,
                         int& offsetX, int& offsetY, int& nCellsX, int& nCellsY ) const;

    /**Returns statistics by considering the pixels where the center point is within the polygon (fast)*/
    void statisticsFromMiddlePointTest( void* band, QgsGeometry* poly, int pixelOffsetX, int pixelOffsetY, int nCellsX, int nCellsY, \
                                        double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, double& sum, double& count );

    /**Returns statistics with precise pixel - polygon intersection test (slow) */
    void statisticsFromPreciseIntersection( void* band, QgsGeometry* poly, int pixelOffsetX, int pixelOffsetY, int nCellsX, int nCellsY, \
                                            double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, double& sum, double& count );


    QString mRasterFilePath;
    /**Raster band to calculate statistics from (defaults to 1)*/
    int mRasterBand;
    QgsVectorLayer* mPolygonLayer;
    QString mAttributePrefix;
    /**The nodata value of the input layer*/
    float mInputNodataValue;
};

#endif // QGSZONALSTATISTICS_H
