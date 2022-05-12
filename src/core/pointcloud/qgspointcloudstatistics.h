/***************************************************************************
                         qgspointcloudstatistics.h
                         --------------------
    begin                : May 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDSTATISTICS_H
#define QGSPOINTCLOUDSTATISTICS_H

#include "qgis_core.h"

#include <QVector>
#include <QSet>
#include <QVariant>
#include <QtMath>

class QgsPointCloudAttribute;
class IndexedPointCloudNode;

/**
 * \ingroup core
 * \class QgsPointCloudStatistics
 *
 * \brief Class used to store statistics of a point cloud dataset.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudStatistics
{
  public:
    struct AttributeStatistics
    {
      double minimum = std::numeric_limits<double>::max();
      double maximum = std::numeric_limits<double>::lowest();
      double mean = 0;
      double stDev = 0;
      int count = 0;
#ifndef SIP_RUN
      QMap<int, int> classCount;
#endif
      void cumulateStatistics( const AttributeStatistics &stats )
      {
        minimum = std::min( minimum, stats.minimum );
        maximum = std::max( maximum, stats.maximum );

        double newMean = ( mean * count + stats.mean * stats.count ) / ( count + stats.count );
        double delta1 = newMean - mean;
        double variance1 = stDev * stDev + delta1 * delta1 - 2 * count * delta1 * mean;
        double delta2 = newMean - stats.mean;
        double variance2 = stats.stDev * stats.stDev + delta2 * delta2 - 2 * stats.count * delta2 * stats.mean;
        stDev = ( variance1 * count + variance2 * stats.count ) / ( count + stats.count );
        stDev = std::sqrt( stDev );

        mean = newMean;
        count += stats.count;

        for ( int key : stats.classCount.keys() )
        {
          int c = classCount.value( key, 0 );
          c += stats.classCount[ key ];
          classCount[ key ] = c;
        }
      }
    };

    //! Constructor
    QgsPointCloudStatistics();

#ifndef SIP_RUN
    //! Constructor from statistics map
    QgsPointCloudStatistics( int sampledPointsCount, const QMap<QString, AttributeStatistics> &stats );
#endif

    //! Returns the number of points used to calculate the statistics
    int sampledPointsCount() const { return mSampledPointsCount; }

    //! Clears the statistics of all attributes
    void clear();

    //! Clears the statistics of given attributes \a attributes
    void clear( const QVector<QgsPointCloudAttribute> &attributes );

    //! Returns the calculated statistics of attribute \a attribute
    AttributeStatistics statisticsOf( const QString &attribute ) const;

    //! Returns a list of existing classes which are present for the specified \a attribute
    QList<int> classesOf( const QString &attribute ) const;

#ifndef SIP_RUN

    /**
     * Returns a map containing the count of each class of the attribute \a attribute
     * If no matching statistic is available then an empty map will be returned.
     */
    QMap<int, int> availableClasses( const QString &attribute ) const;
#endif

    /**
     * Returns the minimum value for the attribute \a attribute
     * If no matching statistic is available then NaN will be returned.
     */
    double minimum( const QString &attribute ) const;

    /**
     * Returns the maximum value for the attribute \a attribute
     * If no matching statistic is available then NaN will be returned.
     */
    double maximum( const QString &attribute ) const;

    /**
     * Returns the mean value for the attribute \a attribute
     * If no matching statistic is available then NaN will be returned.
     */
    double mean( const QString &attribute ) const;

    /**
     * Returns the standard deviation value for the attribute \a attribute
     * If no matching statistic is available then NaN will be returned.
     */
    double stDev( const QString &attribute ) const;

    //! Merges the current statistics with the statistics from \a stats
    void combineWith( const QgsPointCloudStatistics &stats );

#ifndef SIP_RUN
    //! Returns a map object containing all the statistics
    QMap<QString, AttributeStatistics> statisticsMap() const { return mStatisticsMap; };
#endif
  private:
    int mSampledPointsCount = 0;
    QMap<QString, AttributeStatistics> mStatisticsMap;
};

#endif // QGSPOINTCLOUDSTATISTICS_H
