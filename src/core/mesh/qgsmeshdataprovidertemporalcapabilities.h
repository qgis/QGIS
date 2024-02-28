/***************************************************************************
                         qgsmeshdataprovidertemporalcapabilities.h
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
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
#include "qgis.h"

#define  INVALID_MESHLAYER_TIME -99999

/**
 * \class QgsMeshDataProviderTemporalCapabilities
 * \ingroup core
 * \brief Class for handling properties relating to a mesh data provider's temporal capabilities.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsMeshDataProviderTemporalCapabilities: public QgsDataProviderTemporalCapabilities
{
  public:

    /**
     * Method for selection of temporal mesh dataset from a range time.
     */
    enum MatchingTemporalDatasetMethod
    {
      FindClosestDatasetBeforeStartRangeTime, //! Finds the closest dataset which have its time before the requested start range time
      FindClosestDatasetFromStartRangeTime //! Finds the closest dataset before or after the requested start range time
    };

    /**
     * Constructor for QgsMeshDataProviderTemporalCapabilities
     */
    QgsMeshDataProviderTemporalCapabilities();

    /**
     * Returns the last dataset with time less than or equal to \a timeSinceGlobalReference
     *
     * Returns invalid dataset index if \a timeSinceGlobalReference is outside the time extent of the dataset group
     *
     * \note for non temporal dataset group, \a timeSinceGlobalReference is not used and the unique dataset is returned
     */
    QgsMeshDatasetIndex datasetIndexClosestBeforeRelativeTime( int group, qint64 timeSinceGlobalReference ) const;

    /**
     * Returns the closest dataset index from the \a timeSinceGlobalReference
     *
     *Returns invalid dataset index if \a timeSinceGlobalReference is outside the time extent of the dataset group
     *
     * \note for non temporal dataset group, \a timeSinceGlobalReference is not used and the unique dataset is returned
     */
    QgsMeshDatasetIndex datasetIndexClosestFromRelativeTime( int group, qint64 timeSinceGlobalReference ) const;

    /**
     * Adds a \a reference date/time from a dataset \a group
     *
     * \note must be used only by the mesh data provider
     */
    void addGroupReferenceDateTime( int group, const QDateTime &reference ) SIP_SKIP;

    /**
     * Adds a \a time (in milliseconds) from a dataset contained in \a group
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
    void setTemporalUnit( Qgis::TemporalUnit temporalUnit );

    /**
     * Returns the temporal unit used to read data by the data provider
     */
    Qgis::TemporalUnit temporalUnit() const;

    /**
    * Returns the relative time in milliseconds of the dataset
    */
    qint64 datasetTime( const QgsMeshDatasetIndex &index ) const;

    /**
    * Clears all stored reference times and dataset times
    */
    void clear();

    /**
    * Returns the duration of the first time step of the dataset group with index \a group
    *
    * The value is -1 if the dataset group is not present or if it contains only one dataset (non temporal dataset)
    */
    qint64 firstTimeStepDuration( int group ) const;

  private:

    //! Holds the global reference date/time value if exists (min of all groups), otherwise it is invalid
    QDateTime mGlobalReferenceDateTime;

    //! Holds the reference time of each dataset groups
    QHash<int, QDateTime> mGroupsReferenceDateTime;

    /**
     * Holds the time of each dataset in milliseconds.
     * The times are from the dataset groups reference time if any,
     * otherwise from 0
     * Non Temporal dataset (static) have empty list
     */
    QHash<int, QList<qint64>> mDatasetTimeSinceGroupReference;

    Qgis::TemporalUnit mTemporalUnit = Qgis::TemporalUnit::Hours;
};

#endif // QGSMESHDATAPROVIDERTEMPORALCAPABILITIES_H
