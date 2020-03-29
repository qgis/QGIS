/***************************************************************************
  qgsvectortileprovidermetadata.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEPROVIDERMETADATA_H
#define QGSVECTORTILEPROVIDERMETADATA_H


#include "qgsprovidermetadata.h"

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * This metadata class does not support creation of provider instances, because
 * vector tile layer currently does not have a concept of data providers. This class
 * is only used to create data item provider (for browser integration).
 */
class QgsVectorTileProviderMetadata : public QgsProviderMetadata
{
  public:
    QgsVectorTileProviderMetadata();
    QList< QgsDataItemProvider * > dataItemProviders() const override;

    //static QString staticKey();
};

///@endcond

#endif // QGSVECTORTILEPROVIDERMETADATA_H
