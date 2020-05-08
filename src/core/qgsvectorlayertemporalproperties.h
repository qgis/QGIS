/***************************************************************************
                         qgsvectorlayertemporalproperties.h
                         ---------------
    begin                : May 2020
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


#ifndef QGSVECTORLAYERTEMPORALPROPERTIES_H
#define QGSVECTORLAYERTEMPORALPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsrasterdataprovidertemporalcapabilities.h"

class QgsVectorLayer;

/**
 * \class QgsVectorLayerTemporalProperties
 * \ingroup core
 * Implementation of map layer temporal properties for vector layers.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorLayerTemporalProperties : public QgsMapLayerTemporalProperties
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorLayerTemporalProperties, with the specified \a parent object.
     *
     * The \a enabled argument specifies whether the temporal properties are initially enabled or not (see isActive()).
     */
    QgsVectorLayerTemporalProperties( QObject *parent SIP_TRANSFERTHIS = nullptr, bool enabled = false );

    bool isVisibleInTemporalRange( const QgsDateTimeRange &range ) const override;

    /**
     * Mode of the vector temporal properties
     **/
    enum TemporalMode
    {
      ModeFixedTemporalRange = 0, //!< Mode when temporal properties have fixed start and end datetimes.
      ModeFeatureDateTimeInstantFromField, //!< Mode when features have a datetime instant taken from a single field
      ModeFeatureDateTimeStartAndEndFromFields, //!< Mode when features have separate fields for start and end times
      ModeRedrawLayerOnly, //!< Redraw the layer when temporal range changes, but don't apply any filtering. Useful when symbology or rule based renderer expressions depend on the time range.
    };

    /**
     * Returns the temporal properties mode.
     *
     *\see setMode()
    **/
    TemporalMode mode() const;

    /**
     * Sets the temporal properties \a mode.
     *
     *\see mode()
    **/
    void setMode( TemporalMode mode );

    /**
     * Sets a temporal \a range to apply to the whole layer. All features from
     * the layer will be rendered whenever the current datetime range of
     * a render context intersects the specified \a range.
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFixedTemporalRange
     *
     * \see fixedTemporalRange()
     */
    void setFixedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed temporal range for the layer.
     *
     * \warning To be used only when mode() is
     * QgsVectorLayerTemporalProperties::ModeFixedTemporalRange
     *
     * \see setFixedTemporalRange()
    **/
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Returns the name of the start datetime field, which
     * contains the start time for the feature's time spans.
     *
     * If mode() is ModeFeatureDateTimeInstantFromField, then this field
     * represents both the start AND end times.
     *
     * \see setStartField()
     * \see endField()
     */
    QString startField() const;

    /**
     * Sets the name of the start datetime \a field, which
     * contains the start time for the feature's time spans.
     *
     * If mode() is ModeFeatureDateTimeInstantFromField, then this field
     * represents both the start AND end times.
     *
     * \see startField()
     * \see setEndField()
     */
    void setStartField( const QString &field );

    /**
     * Returns the name of the end datetime field, which
     * contains the end time for the feature's time spans.
     *
     * \see setEndField()
     * \see startField()
     */
    QString endField() const;

    /**
     * Sets the name of the end datetime \a field, which
     * contains the end time for the feature's time spans.
     *
     * \see endField()
     * \see setStartField()
     */
    void setEndField( const QString &field );

    /**
     * Creates a QGIS expression filter string for filtering features from \a layer
     * to those within the specified time \a range.
     *
     * The returned expression string considers the mode() and other related
     * settings (such as startField()) when building the filter string.
     *
     * \warning Note that ModeFixedTemporalRange is intentional NOT handled by this method
     * and if mode() is ModeFixedTemporalRange then an empty string will be returned. Use
     * isVisibleInTemporalRange() when testing whether features from a layer set to the
     * ModeFixedTemporalRange should ALL be filtered out.
     */
    QString createFilterString( QgsVectorLayer *layer, const QgsDateTimeRange &range ) const;

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities ) override;

  private:

    //! Temporal layer mode.
    TemporalMode mMode = ModeFixedTemporalRange;

    //! Represents fixed temporal range.
    QgsDateTimeRange mFixedRange;

    QString mStartFieldName;
    QString mEndFieldName;

};

#endif // QGSVECTORLAYERTEMPORALPROPERTIES_H
