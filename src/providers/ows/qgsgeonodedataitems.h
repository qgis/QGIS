/***************************************************************************
    qgsgeonodedataitems.h
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEONODEDATAITEMS_H
#define QGSGEONODEDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgsgeonodeconnection.h"


//! Provider for Geonode root data item
class QgsGeoNodeDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "GeoNode" ); }

    virtual int capabilities() override { return QgsDataProvider::Net; }

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#endif //QGSGEONODEDATAITEMS_H
