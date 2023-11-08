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

#include "qgsfields.h"
#include "qgscoordinatereferencesystem.h"
#include "qgshttpheaders.h"

#include <QReadWriteLock>

class QgsFeedback;

#define SIP_NO_FILE
///@cond PRIVATE

/**
 * \brief This class holds data shared between QgsSensorThingsProvider and QgsSensorThingsFeatureSource instances.
 */
class QgsSensorThingsSharedData
{

  public:
    QgsSensorThingsSharedData( const QString &uri );

    /**
    * Parses and processes a \a url.
    */
    static QUrl parseUrl( const QUrl &url, bool *isTestEndpoint = nullptr );

  private:

    friend class QgsSensorThingsProvider;
    mutable QReadWriteLock mReadWriteLock{ QReadWriteLock::Recursive };

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mRootUri;

    Qgis::SensorThingsEntity mEntityType = Qgis::SensorThingsEntity::Invalid;

    Qgis::WkbType mGeometryType = Qgis::WkbType::Unknown;
    QgsFields mFields;
    QgsCoordinateReferenceSystem mSourceCRS;
};

///@endcond PRIVATE

#endif // QGSSENSORTHINGSSHAREDDATA_H
