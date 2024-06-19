/***************************************************************************
  qgswfsdataitemguiprovider.h
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

#ifndef QGSWFSDATAITEMGUIPROVIDER_H
#define QGSWFSDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsWfsDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "WFS" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void refreshConnection( QgsDataItem *item );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );

};

#endif // QGSWFSDATAITEMGUIPROVIDER_H
