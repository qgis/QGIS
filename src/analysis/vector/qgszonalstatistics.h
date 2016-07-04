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

/** \ingroup analysis
 *  A class that calculates raster statistics (count, sum, mean) for a polygon or multipolygon layer and appends the results as attributes*/
class ANALYSIS_EXPORT QgsZonalStatistics
{
  public:

    //! Enumeration of flags that specify statistics to be calculated
    enum Statistic
    {
      Count       = 1,  //!< Pixel count
      Sum    = 2,  //!< Sum of pixel values
      Mean = 4,  //!< Mean of pixel values
      Median = 8, //!< Median of pixel values
      StDev = 16, //!< Standard deviation of pixel values
      Min = 32,  //!< Min of pixel values
      Max = 64,  //!< Max of pixel values
      Range = 128, //!< Range of pixel values (max - min)
      Minority = 256, //!< Minority of pixel values
      Majority = 512, //!< Majority of pixel values
      Variety = 1024, //!< Variety (count of distinct) pixel values
      All = Count | Sum | Mean | Median | StDev | Max | Min | Range | Minority | Majority | Variety
    };
    Q_DECLARE_FLAGS( Statistics, Statistic )

    QgsZonalStatistics( QgsVectorLayer* polygonLayer, const QString& rasterFile, const QString& attributePrefix = "", int rasterBand = 1,
                        const Statistics& stats = Statistics( Count | Sum | Mean ) );

    /** Starts the calculation
      @return 0 in case of success*/
    int calculateStatistics( QProgressDialog* p );

  private:
    QgsZonalStatistics();

    class FeatureStats
    {
      public:
        FeatureStats( bool storeValues = false, bool storeValueCounts = false )
            : mStoreValues( storeValues )
            , mStoreValueCounts( storeValueCounts )
        {
          reset();
        }
        void reset() { sum = 0; count = 0; max = -FLT_MAX; min = FLT_MAX; valueCount.clear(); values.clear(); }
        void addValue( float value, double weight = 1.0 )
        {
          if ( weight < 1.0 )
          {
            sum += value * weight;
            count += weight;
          }
          else
          {
            sum += value;
            ++count;
          }
          min = qMin( min, value );
          max = qMax( max, value );
          if ( mStoreValueCounts )
            valueCount.insert( value, valueCount.value( value, 0 ) + 1 );
          if ( mStoreValues )
            values.append( value );
        }
        double sum;
        double count;
        float max;
        float min;
        QMap< float, int > valueCount;
        QList< float > values;

      private:
        bool mStoreValues;
        bool mStoreValueCounts;
    };

    /** Analysis what cells need to be considered to cover the bounding box of a feature
      @return 0 in case of success*/
    int cellInfoForBBox( const QgsRectangle& rasterBBox, const QgsRectangle& featureBBox, double cellSizeX, double cellSizeY,
                         int& offsetX, int& offsetY, int& nCellsX, int& nCellsY ) const;

    /** Returns statistics by considering the pixels where the center point is within the polygon (fast)*/
    void statisticsFromMiddlePointTest( void* band, const QgsGeometry* poly, int pixelOffsetX, int pixelOffsetY, int nCellsX, int nCellsY,
                                        double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, FeatureStats& stats );

    /** Returns statistics with precise pixel - polygon intersection test (slow) */
    void statisticsFromPreciseIntersection( void* band, const QgsGeometry* poly, int pixelOffsetX, int pixelOffsetY, int nCellsX, int nCellsY,
                                            double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, FeatureStats& stats );

    /** Tests whether a pixel's value should be included in the result*/
    bool validPixel( float value ) const;

    QString getUniqueFieldName( const QString& fieldName );

    QString mRasterFilePath;
    /** Raster band to calculate statistics from (defaults to 1)*/
    int mRasterBand;
    QgsVectorLayer* mPolygonLayer;
    QString mAttributePrefix;
    /** The nodata value of the input layer*/
    float mInputNodataValue;
    Statistics mStatistics;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsZonalStatistics::Statistics )

#endif // QGSZONALSTATISTICS_H
