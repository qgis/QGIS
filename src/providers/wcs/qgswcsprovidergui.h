/***************************************************************************
  qgswcsprovidergui.h
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswcsprovider.h"
#include "qgsproviderguimetadata.h"

#include"qgsprovidersourcewidgetprovider.h"

class QgsWcsProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsWcsProviderGuiMetadata();

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsProviderSourceWidgetProvider *> sourceWidgetProviders() override;
};


class QgsWcsSourceWidgetProvider : public QgsProviderSourceWidgetProvider
{
  public:
    QgsWcsSourceWidgetProvider();
    QString providerKey() const override;
    bool canHandleLayer( QgsMapLayer *layer ) const override;
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent = nullptr ) override;

};


