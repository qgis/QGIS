/***************************************************************************
                              qgswfsprovidermetadata.h
                              -------------------
  begin                : November 2022
  copyright            : (C) 2006 by Marco Hugentobler
                         (C) 2016-2022 by Even Rouault
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSPROVIDERMETADATA_H
#define QGSWFSPROVIDERMETADATA_H

#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgsprovidermetadata.h"

class QgsWfsProviderMetadata final: public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsWfsProviderMetadata();
    QIcon icon() const override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
    QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    QString suggestGroupNameForUri( const QString &uri ) const override;
};

#endif // QGSWFSPROVIDERMETADATA_H
