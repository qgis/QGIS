/***************************************************************************
  qgsgeonodedataitemguiprovider.h
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

#ifndef QGSGEONODEDATAITEMGUIPROVIDER_H
#define QGSGEONODEDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsGeoNodeDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "GeoNode" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
};

#endif // QGSGEONODEDATAITEMGUIPROVIDER_H
