/***************************************************************************
                         qgsvectorlayertemporalproperties.h
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

#ifndef QGSVECTORLAYERTEMPORALPROPERTIES_H
#define QGSVECTORLAYERTEMPORALPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsmaplayertemporalproperties.h"

/**
 * \class QgsVectorLayerTemporalProperties
 * \ingroup core
 * Implementation of map layer temporal properties for vector layers.
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsVectorLayerTemporalProperties : public QgsMapLayerTemporalProperties
{
  public:

    /**
     * Constructor for QgsVectorLayerTemporalProperties.
     */
    QgsVectorLayerTemporalProperties( bool enabled = false );

    virtual ~QgsVectorLayerTemporalProperties() = default;

    enum TemporalMode
    {
      ModeFixedTemporalRange,
      ModeTemporalRangeFromDateTimeFields,
      ModeTemporalRangeFromExpressions,
      ModeTemporalRangeFromFixedReferenceTimeAndOffsetField
    };

    /**
     * Return the temporal properties mode
     *
     *\see setMode()
    **/
    TemporalMode mode() const;

    /**
     * Set the temporal properties mode
     *
     *\see mode()
    **/
    void setMode( TemporalMode mode );

    /**
     * Sets the temporal \a range to apply to the whole layer. Allfeatures from
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
     * Return fixed temporal range for these properties
     *
     * \warning To be used only when mode() is
     * QgsVectorLayerTemporalProperties::ModeFixedTemporalRange
     *
     *\see setFixedTemporalRange()
    **/
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Sets the start date \a field. Features will only be rendered if
     * their datetime value contained in this field is earlier than
     * the current datetime range of the render context.
     *
     * \warning This setting is only effective when mode() is
     * QgsVectorLayerTemporalProperties::ModeTemporalRangeFromDateTimeFields
     *
     * \see startTimeField()
     */
    void setStartTimeField( const QString &field );

    /**
     * Returns start time field
     *
     * \warning To be used only when mode() is
     * QgsVectorLayerTemporalProperties::ModeTemporalRangeFromDateTimeFields
     *
     *\see ssetStartTimeField()
    **/
    QString startTimeField() const;


  private:

    //! Temporal layer mode.
    TemporalMode mMode;

    //! Represents datetime range member.
    QgsDateTimeRange mRange;

    //!  Start time field for the layer
    QString mStartTimeField;

};

#endif // QGSVECTORLAYERTEMPORALPROPERTIES_H
