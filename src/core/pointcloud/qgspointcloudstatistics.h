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
#include "qgsstatisticalsummary.h"

#include <QVector>
#include <QSet>
#include <QVariant>

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
#ifndef SIP_RUN
    struct AttributeStatistics
    {
      double minimum = std::numeric_limits<double>::max();
      double maximum = std::numeric_limits<double>::lowest();
      int count = 0;
      QMap<int, int> classCount;

      void cumulateStatistics( const AttributeStatistics &stats )
      {
        minimum = std::min( minimum, stats.minimum );
        maximum = std::max( maximum, stats.maximum );
        count += stats.count;

        for ( int key : stats.classCount.keys() )
        {
          int c = classCount.value( key, 0 );
          c += stats.classCount[ key ];
          classCount[ key ] = c;
        }
      }
    };
#endif

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

#ifndef SIP_RUN
    //! Returns the calculated statistics of attribute \a attribute
    AttributeStatistics statisticsOf( const QString &attribute ) const;
#endif

    //! Returns the statistic \a statistic of \a attribute and NaN in case the attribute doesn't exist
    double statisticOf( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const;

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
