/***************************************************************************
                         qgsrasterdataprovidertemporalcapabilities.h
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H
#define QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsdataprovidertemporalcapabilities.h"

/**
 * \class QgsRasterDataProviderTemporalCapabilities
 * \ingroup core
 * Implementation of data provider temporal properties for QgsRasterDataProviders.
 *
 * Data provider temporal capabilities reflect the temporal capabilities of a QgsDataProvider.
 * Unlike QgsMapLayerTemporalProperties, these settings are not user-configurable,
 * and should only be set by the QgsDataProvider itself.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsRasterDataProviderTemporalCapabilities : public QgsDataProviderTemporalCapabilities
{
  public:

    /**
     * Constructor for QgsRasterDataProviderTemporalProperties.
     *
     * The \a enabled argument specifies whether the data provider has temporal capabilities.
     */
    QgsRasterDataProviderTemporalCapabilities( bool enabled = false );

    virtual ~QgsRasterDataProviderTemporalCapabilities() = default;

    /**
     * Mode of the temporal capabilities
     **/
    enum TemporalMode
    {
      ModeFixedTemporalRange = 0, //!< Mode when temporal capabilities have fixed start and end datetimes.
      ModeTemporalRangeFromDataProvider = 1 //!< Mode when temporal capabilities provides the temporal range .
    };

    /**
     * Returns the temporal mode.
     *
     *\see setMode()
    **/
    TemporalMode mode() const;

    /**
     * Sets the temporal \a mode.
     *
     *\see mode()
    **/
    void setMode( TemporalMode mode );

    /**
     * Mode to used to fetch the data from provider. This is applicable for wms based layers.
     *
     **/
    enum FetchMode
    {
      Earliest = 0, //!< Use the start datetime in the temporal range.
      Latest = 1, //!< Use the end datetime in the temporal range.
      Range = 2 //!< Use the datetimes in temporal range as range.
    };

    /**
     * Returns the temporal capabilities fetch mode.
     *
     *\see setFetchMode()
    **/
    FetchMode fetchMode() const;

    /**
     * Sets the temporal properties fetch \a mode.
     *
     *\see fetchMode()
    **/
    void setFetchMode( FetchMode mode );

    //! Stores the capabilities time interval duration in the temporal ranges.
    enum TimeInterval
    {
      Seconds = 0,  //! For seconds
      Minutes = 1,  //! For minutes
      Hours = 2,    //! For hours
      Days = 3,     //! For days
      Months = 4,   //! For months
      Years = 5,    //! For years
      None = 6     //! if there is no time interval
    };

    /**
     * Returns the temporal interval.
     *
     *\see setTimeInterval()
    **/
    TimeInterval timeInterval() const;

    /**
     * Sets the temporal time \a interval.
     *
     *\see timeInterval()
    **/
    void setTimeInterval( TimeInterval interval );

    /**
     * Sets the fixed datetime \a range for the temporal properties.
     *
     * \see fixedTemporalRange()
    */
    void setFixedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed datetime range for these temporal properties.
     *
     * \see setFixedTemporalRange()
    */
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Sets the fixed reference datetime \a range. This is to be used for
     * bi-temporal based data. Where data can have both nominal and reference times.
     *
     * \see fixedReferenceTemporalRange()
    */
    void setFixedReferenceTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed reference datetime range.
     *
     * \see setFixedReferenceTemporalRange()
    */
    const QgsDateTimeRange &fixedReferenceTemporalRange() const;

    /**
     * Sets the requested temporal \a range to retrieve when
     * returning data from the associated data provider.
     *
     * \note this is not normally manually set, and is intended for use by
     * QgsRasterLayerRenderer to automatically set the requested temporal range
     *  on a clone of the data provider during a render job.
     *
     * \see requestedTemporalRange()
    */
    void setRequestedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the requested temporal range.
     * Intended to be used by the provider in fetching data.
     *
     * \see setRequestedTemporalRange()
    */
    const QgsDateTimeRange &requestedTemporalRange() const;

    /**
     * Sets the requested reference temporal \a range to retrieve when
     * returning data from the associated data provider.
     *
     * \note this is not normally manually set, and is intended for use by
     * QgsRasterLayerRenderer to automatically set the requested temporal range
     *  on a clone of the data provider during a render job.
     *
     * \see requestedReferenceTemporalRange()
    */
    void setRequestedReferenceTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the requested reference temporal range.
     * Intended to be used by the provider in fetching data.
     *
     * \see setRequestedReferenceTemporalRange()
    */
    const QgsDateTimeRange &requestedReferenceTemporalRange() const;

    /**
     * Sets the time enabled status.
     * This enables whether time part in the temporal range should be
     * used when updated the temporal range of these capabilities.
     *
     * This is useful in some temporal layers who use dates only.
     *
     * \see isTimeEnabled()
     */
    void setEnableTime( bool enabled );

    /**
     * Returns the temporal property status.
     *
     * \see setEnableTime()
    */
    bool isTimeEnabled() const;

    /**
     * Sets the usage status of the reference range.
     *
     * \see isReferenceEnable()
     */
    void setReferenceEnable( bool enabled );

    /**
     * Returns the enabled status of the reference range.
     *
     * \see setReferenceEnable()
    */
    bool isReferenceEnable() const;

  private:

    /**
     * Represents fixed data provider datetime range.
     *
     * This is for determining the providers lower and upper datetime bounds,
     * any updates on the mRange should get out the range bound defined
     * by this member.
     *
     */
    QgsDateTimeRange mFixedRange;

    /**
     * If the stored time part in temporal ranges should be taked into account.
     *
     * This is to enable data providers that use dates only and no datetime, to
     * configure their temporal properties to consider their state.
     *
     * eg. some WMS-T providers only require date with "YYYY-MM-DD" format with
     *  no time part.
     */
    bool mEnableTime = true;

    //! Represents the requested temporal range.
    QgsDateTimeRange mRequestedRange;

    //! Represents the requested reference temporal range.
    QgsDateTimeRange mRequestedReferenceRange;

    /**
     * Stores the fixed reference temporal range
     */
    QgsDateTimeRange mFixedReferenceRange;

    //! If reference range has been enabled to be used in these properties
    bool mReferenceEnable = false;

    //! Data fetch mode.
    FetchMode mFetchMode = Earliest;

    //! Temporal capabilities mode.
    TemporalMode mMode = ModeTemporalRangeFromDataProvider;

    //! Member for time interval.
    TimeInterval mTimeInteval = None;

};

#endif // QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H
