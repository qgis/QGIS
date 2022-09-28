/***************************************************************************
  qgsogrlayermetadataprovider.h - QgsOgrLayerMetadataProvider

 ---------------------
 begin                : 24.8.2022
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
#ifndef QGSOGRLAYERMETADATAPROVIDER_H
#define QGSOGRLAYERMETADATAPROVIDER_H

#define SIP_NO_FILE
#include <qgsabstractlayermetadataprovider.h>

class QgsOgrLayerMetadataProvider : public QgsAbstractLayerMetadataProvider
{
  public:
    QString id() const override;
    QgsLayerMetadataSearchResults search( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback = nullptr ) const override;
};

#endif // QGSOGRLAYERMETADATAPROVIDER_H
