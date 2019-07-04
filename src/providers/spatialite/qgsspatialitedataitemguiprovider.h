/***************************************************************************
  qgsspatialitedataitemguiprovider.h
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

#ifndef QGSSPATIALITEDATAITEMGUIPROVIDER_H
#define QGSSPATIALITEDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsSLConnectionItem;


class QgsSpatiaLiteDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "SPATIALITE" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

    bool deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context ) override;

    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void createDatabase( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static bool handleDropConnectionItem( QgsSLConnectionItem *connItem, const QMimeData *data, Qt::DropAction );
};

#endif // QGSSPATIALITEDATAITEMGUIPROVIDER_H
