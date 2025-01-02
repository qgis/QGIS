/***************************************************************************
  qgsstacdataitemguiprovider.h
  --------------------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACDATAITEMGUIPROVIDER_H
#define QGSSTACDATAITEMGUIPROVIDER_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsdataitemguiprovider.h"
#include "qgis_gui.h"


class GUI_EXPORT QgsStacDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QgsStacDataItemGuiProvider() = default;

    QString name() override { return QStringLiteral( "STAC" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void refreshConnection( QgsDataItem *item );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );

    static void showDetails( QgsDataItem *item );
    static void downloadAssets( QgsDataItem *item, QgsDataItemGuiContext context );
};

///@endcond

#endif // QGSSTACDATAITEMGUIPROVIDER_H
