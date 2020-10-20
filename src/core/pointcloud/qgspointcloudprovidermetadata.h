/***************************************************************************
                         qgspointcloudprovidermetadata.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDPROVIDERMETADATA_H
#define QGSPOINTCLOUDPROVIDERMETADATA_H


#include "qgsprovidermetadata.h"

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * This metadata class does not support creation of provider instances, because
 * point cloud layer currently does not have a concept of data providers. This class
 * is only used to create data item provider (for browser integration).
 */
class QgsPointCloudProviderMetadata : public QgsProviderMetadata
{
  public:
    QgsPointCloudProviderMetadata();
    QList< QgsDataItemProvider * > dataItemProviders() const override;
};

///@endcond

#endif // QGSPOINTCLOUDPROVIDERMETADATA_H
