/***************************************************************************
  qgsarcgisrestdataitemguiprovider.h
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

#ifndef QGSARCGISRESTDATAITEMGUIPROVIDER_H
#define QGSARCGISRESTDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"
#include "qgsmimedatautils.h"


class QgsArcGisRestDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT

  public:

    QgsArcGisRestDataItemGuiProvider() = default;

    QString name() override
    {
      return QStringLiteral( "afs_items" );
    }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static void refreshConnection( QgsDataItem *item );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );
    void addFilteredLayer( const QgsMimeDataUtils::Uri &uri, QgsDataItemGuiContext context );
};


#endif // QGSARCGISRESTDATAITEMGUIPROVIDER_H
