/***************************************************************************
  qgspostgresprovidermetadatautils.h - QgsPostgresProviderMetadataUtils

 ---------------------
 begin                : 29.8.2022
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESPROVIDERMETADATAUTILS_H
#define QGSPOSTGRESPROVIDERMETADATAUTILS_H

#include "qgsabstractlayermetadataprovider.h"
#include "qgsrectangle.h"

class QgsFeedback;


/**
 * The QgsPostgresProviderMetadataUtils class
 * provides utility functions for QgsPostgresProviderMetadata and QgsPostgresRasterProviderMetadata data providers.
 */
class QgsPostgresProviderMetadataUtils
{
  public:

    static QList<QgsLayerMetadataProviderResult> searchLayerMetadata( const QgsMetadataSearchContext &searchContext, const QString &uri, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback );
    static bool saveLayerMetadata( const QgsMapLayerType &layerType, const QString &uri, const QgsLayerMetadata &metadata, QString &errorMessage );
};

#endif // QGSPOSTGRESPROVIDERMETADATAUTILS_H
