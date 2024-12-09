/***************************************************************************
    qgsmbtilesvectortileguiprovider.h
     --------------------------------------
    Date                 : March 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMBTILESVECTORTILEGUIPROVIDER_H
#define QGSMBTILESVECTORTILEGUIPROVIDER_H

#include "qgsproviderguimetadata.h"
#include "qgsprovidersourcewidgetprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsMbtilesVectorTileSourceWidgetProvider : public QgsProviderSourceWidgetProvider
{
  public:
    QgsMbtilesVectorTileSourceWidgetProvider();
    QString providerKey() const override;
    bool canHandleLayer( QgsMapLayer *layer ) const override;
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent = nullptr ) override;
};

class QgsMbtilesVectorTileGuiProviderMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsMbtilesVectorTileGuiProviderMetadata();
    QList<QgsProviderSourceWidgetProvider *> sourceWidgetProviders() override;
};

///@endcond
#endif // QGSMBTILESVECTORTILEGUIPROVIDER_H
