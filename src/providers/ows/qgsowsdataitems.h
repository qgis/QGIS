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
#include "qgsdatasourceuri.h"
class QgsOWSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOWSConnectionItem();

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );

    virtual QList<QAction*> actions();

  public slots:
    void editConnection();
    void deleteConnection();

  private:
    void replacePath( QgsDataItem* item, QString before, QString after );
};

class QgsOWSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOWSRootItem();

    QVector<QgsDataItem*> createChildren();

    virtual QList<QAction*> actions();

    virtual QWidget * paramWidget();

  public slots:
    void connectionsChanged();

    void newConnection();
};

#endif // QGSOWSDATAITEMS_H
