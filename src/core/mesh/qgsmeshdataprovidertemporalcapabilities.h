/***************************************************************************
                         qgsmeshdataprovidertemporalcapabilities.h
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHDATAPROVIDERTEMPORALCAPABILITIES_H
#define QGSMESHDATAPROVIDERTEMPORALCAPABILITIES_H

#include "qgsdataprovidertemporalcapabilities.h"
#include "qgsrange.h"
#include "qgsmeshdataset.h"


/**
 * \class QgsMeshDataProviderTemporalCapabilities
 * \ingroup core
 * Class for handling properties relating to a mesh data provider's temporal capabilities.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsMeshDataProviderTemporalCapabilities: public QgsDataProviderTemporalCapabilities
{
  public:
    /**
     * Constructor for QgsMeshDataProviderTemporalCapabilities
     */
    QgsMeshDataProviderTemporalCapabilities();

    /**
     * Returns the last dataset index that has the time equal or less or than \a timeSinceGlobalReference (in milliseconds)from the dataset \a group
     *
     * Returns invalid dataset index if the time is less than the first dataset's time
     */
    QgsMeshDatasetIndex datasetIndexTimeInMilliseconds( int group, qint64 timeSinceGlobalReference ) const;

    /**
     * Returns the last dataset index that has the time equal or less or than \a timeSinceGlobalReference (in hours)from the dataset \a group
     *
     * Returns invalid dataset index if the time is less than the first dataset's time
     */
    QgsMeshDatasetIndex datasetIndex( int group, double  timeSinceGlobalReferenceInHours ) const;

    /**
     * Adds a \a reference date/time from a dataset \a group
     *
     * \note must be used only by the mesh data provider
     */
    void addGroupReferenceDateTime( int group, const QDateTime &reference ) SIP_SKIP;

    /**
     * Adds a \a time (in milliseonds) from a dataset contained in \a group
     *
     * \note must be used only by the mesh data provider,
     * all dataset need to be added one after one
     */
    void addDatasetTimeInMilliseconds( int group, qint64 time ) SIP_SKIP;

    /**
     * Adds a \a time (in provider unit) from a dataset contained in \a group
     *
     * \note must be used only by the mesh data provider,
     * all dataset need to be added one after one
     */
    void addDatasetTime( int group, double  time ) SIP_SKIP;

    /**
     * Returns whether the reference time is set
     */
    bool hasReferenceTime() const;

    /**
     * Returns the reference time
     */
    QDateTime referenceTime() const;

    /**
     * Returns the time extent using the internal reference time
     * and the first and last times available from the all the dataset
     */
    QgsDateTimeRange timeExtent() const;

    /**
     * Returns the time extent using an external \a reference date time
     * and the first and last times available from the all the dataset
     */
    QgsDateTimeRange timeExtent( const QDateTime &reference ) const;

    /**
     * Sets the temporal unit (\a temporalUnit) used to read data by the data provider
     *
     * Temporal units supported are milliseconds, seconds, minutes, hors, days and weeks
     */
    void setTemporalUnit( const QgsUnitTypes::TemporalUnit &temporalUnit );

    /**
     * Returns the temporal unit used to read data by the data provider
     *
     * Temporal units supported are milliseconds, seconds, minutes, hors, days and weeks
     */
    QgsUnitTypes::TemporalUnit temporalUnit() const;

    /**
    * Returns the relative time in milliseconds of the dataset
    */
    qint64 datasetTime( const QgsMeshDatasetIndex &index ) const;

    /**
    * Clears alls stored reference times and dataset times
    */
    void clear();

  private:

    //! Holds the global reference date/time value if exists (min of all groups), otherwise it is invalid
    QDateTime mGlobalReferenceDateTime;

    //! Holds the reference time of each dataset groups
    QHash<int, QDateTime> mGroupsReferenceDateTime;

    //! Holds the time of each dataset in milliseconds.
    //! The times are from the dataset groups reference time if any,
    //! otherwise from 0
    QHash<int, QList<qint64>> mDatasetTimeSinceGroupReference;

    QgsUnitTypes::TemporalUnit mTemporalUnit = QgsUnitTypes::TemporalHours;
};

#endif // QGSMESHDATAPROVIDERTEMPORALCAPABILITIES_H
