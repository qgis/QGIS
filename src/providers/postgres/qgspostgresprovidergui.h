/***************************************************************************
  qgspostgresprovidergui.h
  ------------------------
  Date                 : October 2019
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

#ifndef QGSPOSTGRESPROVIDERGUI_H
#define QGSPOSTGRESPROVIDERGUI_H

#include <QList>
#include <QMainWindow>
#include <memory>

#include "qgsproviderguimetadata.h"

class QgsPostgresProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsPostgresProviderGuiMetadata();

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() override;
    QList<const QgsMapLayerConfigWidgetFactory *> mapLayerConfigWidgetFactories() override;

  private:
    std::unique_ptr<QgsMapLayerConfigWidgetFactory> mRasterTemporalWidgetFactory;
};

#endif // QGSPOSTGRESPROVIDERGUI_H
