/***************************************************************************
  qgsgdalclouddataitemguiprovider.h
  --------------------------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGDALCLOUDDATAITEMGUIPROVIDER_H
#define QGSGDALCLOUDDATAITEMGUIPROVIDER_H

///@cond PRIVATE
#include "qgsgdalclouddataitems.h"
#define SIP_NO_FILE

#include "qgsdataitemguiprovider.h"
#include "qgsgdalutils.h"

class QgsGdalCloudConnectionItem;
class QgsGdalCloudRootItem;

class QgsGdalCloudDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QString name() override { return QStringLiteral( "GDAL Cloud" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void editConnection( QgsGdalCloudConnectionItem *item );
    static void duplicateConnection( QgsGdalCloudConnectionItem *item );
    static void newConnection( QgsDataItem *item, const QgsGdalUtils::VsiNetworkFileSystemDetails &driver );
    static void saveConnections();
    static void loadConnections( QgsGdalCloudRootItem *item );
};

///@endcond

#endif // QGSGDALCLOUDDATAITEMGUIPROVIDER_H
