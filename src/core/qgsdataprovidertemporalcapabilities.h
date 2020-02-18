/***************************************************************************
                         qgsdataprovidertemporalcapabilities.h
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


#ifndef QGSDATAPROVIDERTEMPORALCAPABILITIES_H
#define QGSDATAPROVIDERTEMPORALCAPABILITIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgstemporalproperty.h"

#include <QDomElement>

/**
 * \class QgsDataProviderTemporalCapabilities
 * \ingroup core
 * Class for handling data providers temporal capabilities.
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsDataProviderTemporalCapabilities : public QgsTemporalProperty
{
  public:

    /**
     * Constructor for QgsDataProviderTemporalCapabilities.
     */
    QgsDataProviderTemporalCapabilities( bool enabled = false );

    virtual ~QgsDataProviderTemporalCapabilities() = default;
};

#endif // QGSDATAPROVIDERTEMPORALCAPABILITIES_H
