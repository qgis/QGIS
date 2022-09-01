/***************************************************************************
  qgspostgreslayermetadataprovider.h - QgsPostgresLayerMetadataProvider

 ---------------------
 begin                : 17.8.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESLAYERMETADATAPROVIDER_H
#define QGSPOSTGRESLAYERMETADATAPROVIDER_H

#include "qgsabstractlayermetadataprovider.h"

class QgsPostgresLayerMetadataProvider : public QgsAbstractLayerMetadataProvider
{
  public:

    QString id() const override;

    QgsLayerMetadataSearchResults search( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback = nullptr ) const override;
};

#endif // QGSPOSTGRESLAYERMETADATAPROVIDER_H
