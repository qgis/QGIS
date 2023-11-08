/***************************************************************************
      qgssensorthingsshareddata.h
      ----------------
    begin                : November 2023
    copyright            : (C) 2013 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORTHINGSSHAREDDATA_H
#define QGSSENSORTHINGSSHAREDDATA_H

#include "qgsdatasourceuri.h"
#include "qgsfields.h"
#include "qgscoordinatereferencesystem.h"

#include <QReadWriteLock>

class QgsFeedback;

#define SIP_NO_FILE

/**
 * \brief This class holds data shared between QgsSensorThingsProvider and QgsSensorThingsFeatureSource instances.
 */
class QgsSensorThingsSharedData
{

  public:
    QgsSensorThingsSharedData( const QgsDataSourceUri &uri );

  private:

    friend class QgsSensorThingsProvider;
    mutable QReadWriteLock mReadWriteLock{ QReadWriteLock::Recursive };

    QgsDataSourceUri mDataSource;

    Qgis::WkbType mGeometryType = Qgis::WkbType::Unknown;
    QgsFields mFields;
    QgsCoordinateReferenceSystem mSourceCRS;
};

#endif // QGSSENSORTHINGSSHAREDDATA_H
