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
#include "qgis.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsunittypes.h"

class QgsVectorLayer;
class QgsFields;

/**
 * \class QgsVectorLayerTemporalContext
 * \ingroup core
 * \brief Encapsulates the context in which a QgsVectorLayer's temporal capabilities
 * will be applied
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorLayerTemporalContext
{
  public:

    /**
     * Returns the associated layer.
     *
     * \see setLayer()
     */
    QgsVectorLayer *layer() const;

    /**
     * Sets the associated \a layer.
     *
     * \see layer()
     */
    void setLayer( QgsVectorLayer *layer );

  private:

    QgsVectorLayer *mLayer = nullptr;
};

/**
 * \class QgsVectorLayerTemporalProperties
 * \ingroup core
 * \brief Implementation of map layer temporal properties for vector layers.
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
    QgsDateTimeRange calculateTemporalExtent( QgsMapLayer *layer ) const override SIP_SKIP;

    /**
     * Returns the temporal properties mode.
     *
     *\see setMode()
    */
    Qgis::VectorTemporalMode mode() const;

    /**
     * Sets the temporal properties \a mode.
     *
     *\see mode()
    */
    void setMode( Qgis::VectorTemporalMode mode );

    /**
     * Returns the temporal limit mode (to include or exclude begin/end limits).
     *
     * \see setLimitMode()
     * \since QGIS 3.22
    */
    Qgis::VectorTemporalLimitMode limitMode() const;

    /**
     * Sets the temporal \a limit mode (to include or exclude begin/end limits).
     *
     * \see limitMode()
     * \since QGIS 3.22
    */
    void setLimitMode( Qgis::VectorTemporalLimitMode mode );

    /**
     * Returns flags associated to the temporal property.
     */
    QgsTemporalProperty::Flags flags() const override;

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
    */
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
     * Returns the expression for the start time for the feature's time spans.
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromExpressions
     *
     * \see setStartExpression()
     * \see endExpression()
     */
    QString startExpression() const;

    /**
     * Sets the \a expression to use for the start time for the feature's time spans.
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromExpressions
     *
     * \see startExpression()
     * \see setEndExpression()
     */
    void setStartExpression( const QString &expression );

    /**
     * Returns the expression for the end time for the feature's time spans.
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromExpressions
     *
     * \see setEndExpression()
     * \see startExpression()
     */
    QString endExpression() const;

    /**
     * Sets the \a expression to use for the end time for the feature's time spans.
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromExpressions
     *
     * \see endExpression()
     * \see setStartExpression()
     */
    void setEndExpression( const QString &endExpression );

    /**
     * Returns the name of the duration field, which
     * contains the duration of the event.
     *
     * Units are specified by durationUnits()
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndDurationFromFields
     *
     * \see setDurationField()
     * \see durationUnits()
     */
    QString durationField() const;

    /**
     * Sets the name of the duration \a field, which
     * contains the duration of the event.
     *
     * Units are specified by setDurationUnits()
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndDurationFromFields
     *
     * \see durationField()
     * \see setDurationUnits()
     */
    void setDurationField( const QString &field );

    /**
     * Returns the units of the event's duration.
     *
     * \see setDurationUnits()
     */
    QgsUnitTypes::TemporalUnit durationUnits() const;

    /**
     * Sets the \a units of the event's duration.
     *
     * \see durationUnits()
     */
    void setDurationUnits( QgsUnitTypes::TemporalUnit units );

    /**
     * Returns the fixed duration length, which contains the duration of the event.
     *
     * Units are specified by durationUnits()
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField
     *
     * \see setFixedDuration()
     * \see durationUnits()
     */
    double fixedDuration() const;

    /**
     * Sets the fixed event \a duration, which contains the duration of the event.
     *
     * Units are specified by setDurationUnits()
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField
     *
     * \see fixedDuration()
     * \see setDurationUnits()
     */
    void setFixedDuration( double duration );

    /**
     * Returns TRUE if features will be accumulated over time (i.e. all features which
     * occur before or within the map's temporal range should be rendered).
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField
     *
     * \see setAccumulateFeatures()
     */
    bool accumulateFeatures() const;

    /**
     * Sets whether features will be accumulated over time (i.e. all features which
     * occur before or within the map's temporal range should be rendered).
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField
     *
     * \see accumulateFeatures()
     */
    void setAccumulateFeatures( bool accumulate );

    /**
     * Creates a QGIS expression filter string for filtering features within
     * the specified \a context to those within the specified time \a range.
     *
     * The returned expression string considers the mode() and other related
     * settings (such as startField()) when building the filter string.
     *
     * \warning Note that ModeFixedTemporalRange is intentional NOT handled by this method
     * and if mode() is ModeFixedTemporalRange then an empty string will be returned. Use
     * isVisibleInTemporalRange() when testing whether features from a layer set to the
     * ModeFixedTemporalRange should ALL be filtered out.
     */
    QString createFilterString( const QgsVectorLayerTemporalContext &context, const QgsDateTimeRange &range ) const;

    /**
     * Attempts to setup the temporal properties by scanning a set of \a fields
     * and looking for standard naming conventions (e.g. "begin_date").
     */
    void guessDefaultsFromFields( const QgsFields &fields );

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities ) override;

  private:

    //! Temporal layer mode.
    Qgis::VectorTemporalMode mMode = Qgis::VectorTemporalMode::FixedTemporalRange;

    //! How to handle the limits of the timeframe (include or exclude)
    Qgis::VectorTemporalLimitMode mLimitMode = Qgis::VectorTemporalLimitMode::IncludeBeginExcludeEnd;

    //! Represents fixed temporal range.
    QgsDateTimeRange mFixedRange;

    QString mStartFieldName;
    QString mEndFieldName;
    QString mDurationFieldName;
    QgsUnitTypes::TemporalUnit mDurationUnit = QgsUnitTypes::TemporalMinutes;

    double mFixedDuration = 0;

    bool mAccumulateFeatures = false;

    QString mStartExpression;
    QString mEndExpression;

};

#endif // QGSVECTORLAYERTEMPORALPROPERTIES_H
