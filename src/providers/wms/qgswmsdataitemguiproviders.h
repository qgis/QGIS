/***************************************************************************
  qgswmsdataitemguiproviders.h
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

#ifndef QGSWMSDATAITEMGUIPROVIDERS_H
#define QGSWMSDATAITEMGUIPROVIDERS_H

#include "qgsdataitemguiprovider.h"

class QgsWMSConnectionItem;
class QgsXyzLayerItem;

class QgsWmsDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QString name() override { return QStringLiteral( "WMS" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

    QWidget *createParamWidget( QgsDataItem *root, QgsDataItemGuiContext ) override;

  private:
    static void refreshConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void duplicateConnection( QgsDataItem *item );
    static void newConnection( QgsDataItem *item );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );
};


class QgsXyzDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QString name() override { return QStringLiteral( "XYZ Tiles" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

    QWidget *createParamWidget( QgsDataItem *root, QgsDataItemGuiContext ) override;

  private:
    static void editConnection( QgsDataItem *item );
    static void duplicateConnection( QgsDataItem *item );
    static void newConnection( QgsDataItem *item );
    static void saveXyzTilesServers();
    static void loadXyzTilesServers( QgsDataItem *item );
};


#endif // QGSWMSDATAITEMGUIPROVIDERS_H
