/***************************************************************************
                         qgstemporalcontroller.h
                         ---------------
    begin                : March 2020
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

#ifndef QGSTEMPORALCONTROLLER_H
#define QGSTEMPORALCONTROLLER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include <QObject>

/**
 * \class QgsTemporalController
 * \ingroup core
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsTemporalController : public QObject
{

  public:

    /**
     * Constructor for QgsTemporalController.
     *
    */
    QgsTemporalController();

    virtual ~QgsTemporalController() = default;

  signals:
    void updateTemporalRange( const QgsDateTimeRange &range );

};

#endif // QGSTEMPORALCONTROLLER_H
