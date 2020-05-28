/***************************************************************************
  qgsdb2dataitemguiprovider.h
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

#ifndef QGSDB2DATAITEMGUIPROVIDER_H
#define QGSDB2DATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"


class QgsDb2DataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "DB2" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static void refreshConnection( QgsDataItem *item );
};

#endif // QGSDB2DATAITEMGUIPROVIDER_H
