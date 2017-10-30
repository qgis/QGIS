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
    QgsOWSConnectionItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif

  private:
    void replacePath( QgsDataItem *item, QString before, QString after );
};

class QgsOWSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
    virtual QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();

    void newConnection();
#endif
};

#endif // QGSOWSDATAITEMS_H
