/***************************************************************************
     qgsidentifycontext.h
     --------------------
    Date                 : November 2020
    Copyright            : (C) 2020 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSIDENTIFYCONTEXT_H
#define QGSIDENTIFYCONTEXT_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsrange.h"

/**
 * \ingroup core
 * \class QgsIdentifyContext
 * \brief Identify contexts are used to encapsulate the settings to be used to perform
 * an identify action.
 *
 * \since QGIS 3.16.1
 */
class CORE_EXPORT QgsIdentifyContext
{
  public:

    //! Constructor for QgsIdentifyContext
    QgsIdentifyContext() = default;

    /**
     * Set datetime \a range to be used with the identify action.
     *
     * \see temporalRange()
     * \see isTemporal()
    */
    void setTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the datetime range to be used with the identify action.
     *
     * \see setTemporalRange()
     * \see isTemporal()
    */
    const QgsDateTimeRange &temporalRange() const;

    /**
     * Returns TRUE if the temporal range setting is enabled.
    */
    bool isTemporal() const;

    /**
     * Returns the range of z-values to identify within, or an infinite range if no filtering by
     * z should be applied.
     *
     * \see setZRange()
     * \since QGIS 3.38
     */
    QgsDoubleRange zRange() const;

    /**
     * Sets the \a range of z-values to identify within.
     *
     * Set to an infinite range if no filtering by z should be applied.
     *
     * \see zRange()
     * \since QGIS 3.38
     */
    void setZRange( const QgsDoubleRange &range );

  private:

    QgsDateTimeRange mTemporalRange;
    QgsDoubleRange mZRange;

};

#endif // QGSEXPRESSIONCONTEXT_H
