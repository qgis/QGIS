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
 * \class QgsPointCloudAttributeStatistics
 *
 * \brief Class used to store statistics of one attribute of a point cloud dataset.
 *
 * \since QGIS 3.26
 */
struct CORE_EXPORT QgsPointCloudAttributeStatistics
{
  double minimum = std::numeric_limits<double>::max();
  double maximum = std::numeric_limits<double>::lowest();
  double mean = 0;
  double stDev = 0;
  int count = 0;
#ifndef SIP_RUN
  QMap<int, int> classCount;
  //! Updates the current point cloud statistics to hold the cumulation of the current statistics and \a stats
  void cumulateStatistics( const QgsPointCloudAttributeStatistics &stats );
#endif
};

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
    //! Constructor
    QgsPointCloudStatistics();

#ifndef SIP_RUN
    //! Constructor from statistics map
    QgsPointCloudStatistics( int sampledPointsCount, const QMap<QString, QgsPointCloudAttributeStatistics> &stats );
#endif

    //! Returns the number of points used to calculate the statistics
    int sampledPointsCount() const { return mSampledPointsCount; }

    //! Clears the statistics of all attributes
    void clear();

    //! Clears the statistics of given attributes \a attributes
    void clear( const QVector<QgsPointCloudAttribute> &attributes );

    //! Returns the calculated statistics of attribute \a attribute
    QgsPointCloudAttributeStatistics statisticsOf( const QString &attribute ) const;

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

    //! Converts the current statistics object into JSON object
    QByteArray toStatisticsJson() const;

    //! Creates a statistics object from the JSON object \a stats
    static QgsPointCloudStatistics fromStatisticsJson( QByteArray stats );

#ifndef SIP_RUN
    //! Returns a map object containing all the statistics
    QMap<QString, QgsPointCloudAttributeStatistics> statisticsMap() const { return mStatisticsMap; };
#endif
  private:
    int mSampledPointsCount = 0;
    QMap<QString, QgsPointCloudAttributeStatistics> mStatisticsMap;

    //! Converts statistics object \a stats into a JSON object
    static QJsonObject attributeStatisticsToJson( const QgsPointCloudAttributeStatistics &stats );

    //! Creates a statistics object from the JSON object \a stats
    static QgsPointCloudAttributeStatistics fromAttributeStatisticsJson( QJsonObject &stats );
};

#endif // QGSPOINTCLOUDSTATISTICS_H
