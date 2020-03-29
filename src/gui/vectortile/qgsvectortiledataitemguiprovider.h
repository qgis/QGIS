/***************************************************************************
  qgsvectortiledataitemguiprovider.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEDATAITEMGUIPROVIDER_H
#define QGSVECTORTILEDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"


class QgsVectorTileDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "Vector Tiles" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static void newConnection( QgsDataItem *item );
    static void saveXyzTilesServers();
    static void loadXyzTilesServers( QgsDataItem *item );

};


#endif // QGSVECTORTILEDATAITEMGUIPROVIDER_H
