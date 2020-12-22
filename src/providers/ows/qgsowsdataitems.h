/***************************************************************************
    qgsowsdataitems.h
    ---------------------
    begin                : May 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOWSDATAITEMS_H
#define QGSOWSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgswkbtypes.h"
#include "qgsdataitemprovider.h"

class QgsOWSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSConnectionItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    void replacePath( QgsDataItem *item, QString before, QString after );
};

class QgsOWSRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsOWSRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 11; }

};

//! Provider for OWS data item
class QgsOwsDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

#endif // QGSOWSDATAITEMS_H
