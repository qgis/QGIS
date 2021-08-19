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

#include <QString>
#include <QMap>

#include <limits>
#include <cfloat>

#include "qgis_analysis.h"
#include "qgsfeedback.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfields.h"

class QgsGeometry;
class QgsVectorLayer;
class QgsRasterLayer;
class QgsRasterInterface;
class QgsRasterDataProvider;
class QgsRectangle;
class QgsField;
class QgsFeatureSink;
class QgsFeatureSource;

/**
 * \ingroup analysis
 * \brief A class that calculates raster statistics (count, sum, mean) for a polygon or multipolygon layer and appends the results as attributes.
*/
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
      Variance = 2048, //!< Variance of pixel values
      All = Count | Sum | Mean | Median | StDev | Max | Min | Range | Minority | Majority | Variety | Variance
    };
    Q_DECLARE_FLAGS( Statistics, Statistic )

    //! Error codes for calculation
    enum Result
    {
      Success = 0, //!< Success
      LayerTypeWrong = 1, //!< Layer is not a polygon layer
      LayerInvalid, //!< Layer is invalid
      RasterInvalid, //!< Raster layer is invalid
      RasterBandInvalid, //!< The raster band does not exist on the raster layer
      FailedToCreateField = 8, //!< Output fields could not be created
      Canceled = 9 //!< Algorithm was canceled
    };

    /**
     * Convenience constructor for QgsZonalStatistics, using an input raster layer.
     *
     * The raster layer must exist for the lifetime of the zonal statistics calculation.
     *
     * \warning Constructing QgsZonalStatistics using this method is not thread safe, and
     * the constructor which accepts a QgsRasterInterface should be used instead.
     */
    QgsZonalStatistics( QgsVectorLayer *polygonLayer,
                        QgsRasterLayer *rasterLayer,
                        const QString &attributePrefix = QString(),
                        int rasterBand = 1,
                        QgsZonalStatistics::Statistics stats = QgsZonalStatistics::Statistics( QgsZonalStatistics::Count | QgsZonalStatistics::Sum | QgsZonalStatistics::Mean ) );

    /**
     * Constructor for QgsZonalStatistics, using a QgsRasterInterface.
     *
     * The \a polygonLayer gives the vector layer containing the (multi)polygon features corresponding to the
     * different zones. This layer will be modified, adding extra attributes for each of the zonal statistics
     * calculated.
     *
     * Pixel values for each zone are taken from the raster \a rasterInterface. The constructor must also
     * be given various properties relating to the input raster, such as the raster CRS (\a rasterCrs),
     * and the size (X and Y) in map units for each raster pixel. The source raster band is specified
     * via \a rasterBand, where a value of 1 corresponds to the first band.
     *
     * If the CRS of the \a polygonLayer and \a rasterCrs differ, the calculation will automatically
     * reproject the zones to ensure valid results are calculated.
     *
     * The \a attributePrefix argument specifies an optional prefix to use when creating the
     * new fields for each calculated statistic.
     *
     * Finally, the calculated statistics can be set via the \a stats argument. A new field will be
     * added to \a polygonLayer for each statistic calculated.
     *
     * \warning The raster interface must exist for the lifetime of the zonal statistics calculation. For thread
     * safe use, always use a cloned raster interface.
     *
     * \since QGIS 3.2
     */
    QgsZonalStatistics( QgsVectorLayer *polygonLayer,
                        QgsRasterInterface *rasterInterface,
                        const QgsCoordinateReferenceSystem &rasterCrs,
                        double rasterUnitsPerPixelX,
                        double rasterUnitsPerPixelY,
                        const QString &attributePrefix = QString(),
                        int rasterBand = 1,
                        QgsZonalStatistics::Statistics stats = QgsZonalStatistics::Statistics( QgsZonalStatistics::Count | QgsZonalStatistics::Sum | QgsZonalStatistics::Mean ) );


    /**
     * Runs the calculation.
     */
    QgsZonalStatistics::Result calculateStatistics( QgsFeedback *feedback );

    /**
     * Returns the friendly display name for a \a statistic.
     * \see shortName()
     * \since QGIS 3.12
     */
    static QString displayName( QgsZonalStatistics::Statistic statistic );

    /**
     * Returns a short, friendly display name for a \a statistic, suitable for use in a field name.
     * \see displayName()
     * \since QGIS 3.12
     */
    static QString shortName( QgsZonalStatistics::Statistic statistic );

    /**
     * Calculates the specified \a statistics for the pixels of \a rasterBand
     * in \a rasterInterface (a raster layer dataProvider() ) within polygon \a geometry.
     *
     * Returns a map of statistic to result value.
     *
     * \since QGIS 3.16
     */
#ifndef SIP_RUN
    static QMap<QgsZonalStatistics::Statistic, QVariant> calculateStatistics( QgsRasterInterface *rasterInterface, const QgsGeometry &geometry, double cellSizeX, double cellSizeY, int rasterBand, QgsZonalStatistics::Statistics statistics );
#endif

///@cond PRIVATE
    // Required to fix https://github.com/qgis/QGIS/issues/43245 (SIP is failing to convert the enum to values)

    /**
     * Calculates the specified \a statistics for the pixels of \a rasterBand
     * in \a rasterInterface (a raster layer dataProvider() ) within polygon \a geometry.
     *
     * Returns a map of statistic to result value.
     *
     * \since QGIS 3.16
     */
    static QMap<int, QVariant> calculateStatisticsInt( QgsRasterInterface *rasterInterface, const QgsGeometry &geometry, double cellSizeX, double cellSizeY, int rasterBand, QgsZonalStatistics::Statistics statistics ) SIP_PYNAME( calculateStatistics );
/// @endcond

  private:
    QgsZonalStatistics() = default;

    class FeatureStats
    {
      public:
        FeatureStats( bool storeValues = false, bool storeValueCounts = false )
          : mStoreValues( storeValues )
          , mStoreValueCounts( storeValueCounts )
        {
        }

        void reset()
        {
          sum = 0;
          count = 0;
          max = std::numeric_limits<double>::lowest();
          min = std::numeric_limits<double>::max();
          valueCount.clear();
          values.clear();
        }

        void addValue( double value, double weight = 1.0 )
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
          min = std::min( min, value );
          max = std::max( max, value );
          if ( mStoreValueCounts )
            valueCount.insert( value, valueCount.value( value, 0 ) + 1 );
          if ( mStoreValues )
            values.append( value );
        }
        double sum = 0.0;
        double count = 0.0;
        double max = std::numeric_limits<double>::lowest();
        double min = std::numeric_limits<double>::max();
        QMap< double, int > valueCount;
        QList< double > values;

      private:
        bool mStoreValues = false;
        bool mStoreValueCounts = false;
    };

    QString getUniqueFieldName( const QString &fieldName, const QList<QgsField> &newFields );

    QgsRasterInterface *mRasterInterface = nullptr;
    QgsCoordinateReferenceSystem mRasterCrs;

    double mCellSizeX = 0;
    double mCellSizeY = 0;

    //! Raster band to calculate statistics
    int mRasterBand = 0;
    QgsVectorLayer *mPolygonLayer = nullptr;
    QString mAttributePrefix;
    Statistics mStatistics = QgsZonalStatistics::All;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsZonalStatistics::Statistics )

// clazy:excludeall=qstring-allocations

#endif // QGSZONALSTATISTICS_H
