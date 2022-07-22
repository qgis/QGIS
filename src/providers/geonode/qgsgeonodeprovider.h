/***************************************************************************
                              qgsgeonodeprovider.h
                              ----------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSGEONODEPROVIDERMETADATA_H
#define QGSGEONODEPROVIDERMETADATA_H

#include <QList>

#include "qgis.h"
#include "qgsprovidermetadata.h"
#include "qgsgeonodedataitems.h"
#include "qgsapplication.h"

class QgsGeoNodeProviderMetadata: public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsGeoNodeProviderMetadata();

    QIcon icon() const override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
};

#endif // QGSGEONODEPROVIDERMETADATA_H
