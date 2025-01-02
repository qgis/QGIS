/***************************************************************************
  qgssensorthingsdataitemguiprovider.h
  --------------------------------------
    begin                : December 2023
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

#ifndef QGSSENSORTHINGSDATAITEMGUIPROVIDER_H
#define QGSSENSORTHINGSDATAITEMGUIPROVIDER_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsdataitemguiprovider.h"


class QgsSensorThingsDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QString name() override { return QStringLiteral( "SensorThings" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void editConnection( QgsDataItem *item );
    static void duplicateConnection( QgsDataItem *item );
    static void newConnection( QgsDataItem *item );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );
};

///@endcond

#endif // QGSSENSORTHINGSDATAITEMGUIPROVIDER_H
