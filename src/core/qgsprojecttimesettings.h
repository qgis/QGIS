/***************************************************************************
                         qgsprojecttimesettings.h
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
#ifndef QGSPROJECTTIMESETTINGS_H
#define QGSPROJECTTIMESETTINGS_H

#include "qgis_core.h"
#include "qgsrange.h"
#include <QObject>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;

/**
 * Contains temporal settings and properties for the project,
 * this may be used when animating maps or showing temporal layers.
 *
 * \ingroup core
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProjectTimeSettings : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectTimeSettings with the specified \a parent object.
     */
    QgsProjectTimeSettings( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Resets the settings to a default state.
     */
    void reset();

    /**
     * Returns the project's temporal range, which indicates the earliest
     * and latest datetime ranges associated with the project.
     *
     * \note This is a manual, use-set property, and does not necessarily
     * coincide with the earliest and latest temporal ranges set for
     * individual layers in the project.
     *
     * \see setTemporalRange()
     * \see temporalRangeChanged()
     */
    QgsDateTimeRange temporalRange() const;

    /**
     * Sets the project's temporal \a range, which indicates the earliest
     * and latest datetime ranges associated with the project.
     *
     * \note This is a manual, use-set property, and does not necessarily
     * coincide with the earliest and latest temporal ranges set for
     * individual layers in the project.
     *
     * \see temporalRange()
     * \see temporalRangeChanged()
     */
    void setTemporalRange( const QgsDateTimeRange &range );

    /**
     * Reads the settings's state from a DOM \a element.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the settings.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

  signals:

    /**
     * Emitted when the temporal range changes.
     *
     * \see temporalRange()
     * \see setTemporalRange()
     */
    void temporalRangeChanged();

  private:

    QgsDateTimeRange mRange;
};


#endif // QGSPROJECTTIMESETTINGS_H
