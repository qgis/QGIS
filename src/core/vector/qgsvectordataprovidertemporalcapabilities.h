/***************************************************************************
                         qgsvectordataprovidertemporalcapabilities.h
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORDATAPROVIDERTEMPORALCAPABILITIES_H
#define QGSVECTORDATAPROVIDERTEMPORALCAPABILITIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsdataprovidertemporalcapabilities.h"

/**
 * \class QgsVectorDataProviderTemporalCapabilities
 * \ingroup core
 * \brief Implementation of data provider temporal properties for QgsVectorDataProviders.
 *
 * Data provider temporal capabilities reflect the temporal capabilities of a QgsDataProvider.
 * Unlike QgsMapLayerTemporalProperties, these settings are not user-configurable,
 * and should only be set by the QgsDataProvider itself.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorDataProviderTemporalCapabilities : public QgsDataProviderTemporalCapabilities
{
  public:

    /**
     * Constructor for QgsVectorDataProviderTemporalCapabilities.
     *
     * The \a enabled argument specifies whether the data provider has temporal capabilities.
     */
    QgsVectorDataProviderTemporalCapabilities( bool enabled = false );

    /**
     * Returns the temporal properties mode.
     *
     *\see setMode()
    */
    Qgis::VectorDataProviderTemporalMode mode() const;

    /**
     * Sets the temporal properties \a mode.
     *
     *\see mode()
    */
    void setMode( Qgis::VectorDataProviderTemporalMode mode );

    /**
     * Sets the datetime \a range extent from which temporal data is available from the provider.
     *
     * \see availableTemporalRange()
    */
    void setAvailableTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the datetime range extent from which temporal data is available from the provider.
     *
     * \see setAvailableTemporalRange()
    */
    const QgsDateTimeRange &availableTemporalRange() const;

    /**
     * Returns the name of the start datetime field, which contains the start time for the feature's time spans.
     *
     * If mode() is ProviderStoresFeatureDateTimeInstantInField, then this field stores both the start AND end times.
     *
     * \see setStartField()
     * \see endField()
     */
    QString startField() const;

    /**
     * Sets the name of the start datetime \a field, which stores the start time for the feature's time spans.
     *
     * If mode() is ModeFeatureDateTimeInstantFromField, then this field stores both the start AND end times.
     *
     * \see startField()
     * \see setEndField()
     */
    void setStartField( const QString &field );

    /**
     * Returns the name of the end datetime field, which stores the end time for the feature's time spans.
     *
     * \see setEndField()
     * \see startField()
     */
    QString endField() const;

    /**
     * Sets the name of the end datetime \a field, which stores the end time for the feature's time spans.
     *
     * \see endField()
     * \see setStartField()
     */
    void setEndField( const QString &field );

  private:

    /**
     * Represents available data provider datetime range.
     *
     * This is for determining the providers lower and upper datetime bounds,
     * any updates on the mRange should get out the range bound defined
     * by this member.
     *
     */
    QgsDateTimeRange mAvailableTemporalRange;

    Qgis::VectorDataProviderTemporalMode mMode = Qgis::VectorDataProviderTemporalMode::HasFixedTemporalRange;

    QString mStartField;
    QString mEndField;

};

#endif // QGSVECTORDATAPROVIDERTEMPORALCAPABILITIES_H
