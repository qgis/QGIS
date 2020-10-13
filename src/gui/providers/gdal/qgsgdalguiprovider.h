/***************************************************************************
      qgsgdalguiprovider.h  - GUI for QGIS Data provider for GDAL rasters
                             -------------------
    begin                : June, 2019
    copyright            : (C) 2019 by Peter Petrik
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


#ifndef QGSGDALGUIPROVIDER_H
#define QGSGDALGUIPROVIDER_H

class QgsProviderMetadata;

#include "qgsproviderguimetadata.h"
#include "qgsgdalsourceselect.h"
#include "qgsdataitemguiprovider.h"
#include "qgsproviderguiregistry.h"
#include <QObject>
#include <QPointer>

#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsGdalItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT

  public:
    QgsGdalItemGuiProvider();
    ~QgsGdalItemGuiProvider();

    QString name() override;
    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  protected slots:
    void onDeleteLayer( QgsDataItemGuiContext context );
};

class QgsGdalGuiProviderMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsGdalGuiProviderMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
};

///@endcond
#endif
