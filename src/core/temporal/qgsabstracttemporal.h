/***************************************************************************
                         qgsabstracttemporal.h
                         ---------------
    begin                : January 2020
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


#ifndef QGSABSTRACTTEMPORAL_H
#define QGSABSTRACTTEMPORAL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include "qgsrange.h"

#include <QObject>
#include <QMutex>
#include <QSet>
#include <QDateTime>
#include <QList>

/**
 * \class QgsAbstractTemporal
 * \ingroup core
 * Base class for temporal based classes.
 *
 * Subclasses may wish to update the .
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsAbstractTemporal
{
  public:

    /**
     * Constructor for QgsAbstractTemporal.
     */
    QgsAbstractTemporal() ;

    virtual ~QgsAbstractTemporal() = default;

  private:

    /**
     * Represents datetime range.
     */
    QgsDateTimeRange *dateTimeRange = nullptr;

};

#endif

#endif // QGSABSTRACTTEMPORAL_H
