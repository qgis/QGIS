/***************************************************************************
  qgstiledmeshdataitemguiprovider.h
  --------------------------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDMESHDATAITEMGUIPROVIDER_H
#define QGSTILEDMESHDATAITEMGUIPROVIDER_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsdataitemguiprovider.h"


class QgsTiledMeshDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "Tiled Mesh" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static void newConnection( QgsDataItem *item );
    static void newArcGISConnection( QgsDataItem *item );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );

};

///@endcond

#endif // QGSTILEDMESHDATAITEMGUIPROVIDER_H
