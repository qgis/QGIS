/***************************************************************************
    qgsvtpkvectortileguiprovider.h
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


#ifndef QGSVTPKVECTORTILEGUIPROVIDER_H
#define QGSVTPKVECTORTILEGUIPROVIDER_H

#include "qgsproviderguimetadata.h"
#include "qgsprovidersourcewidgetprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsVtpkVectorTileSourceWidgetProvider : public QgsProviderSourceWidgetProvider
{
  public:
    QgsVtpkVectorTileSourceWidgetProvider();
    QString providerKey() const override;
    bool canHandleLayer( QgsMapLayer *layer ) const override;
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent = nullptr ) override;
};

class QgsVtpkVectorTileGuiProviderMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsVtpkVectorTileGuiProviderMetadata();
    QList<QgsProviderSourceWidgetProvider *> sourceWidgetProviders() override;
};

///@endcond
#endif // QGSVTPKVECTORTILEGUIPROVIDER_H
