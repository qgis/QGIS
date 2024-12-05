/***************************************************************************
  qgswmsprovidergui.h
  --------------------------------------
  Date                 : September 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSPROVIDERGUI_H
#define QGSWMSPROVIDERGUI_H

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"
#include <memory>

class QgsWmsProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsWmsProviderGuiMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsProviderSourceWidgetProvider *> sourceWidgetProviders() override;

    void registerGui( QMainWindow *widget ) override;
    QList<const QgsMapLayerConfigWidgetFactory *> mapLayerConfigWidgetFactories() override;

  private:
    std::unique_ptr<QgsMapLayerConfigWidgetFactory> mWmstConfigWidgetFactory;
};

#endif // QGSWMSPROVIDERGUI_H
