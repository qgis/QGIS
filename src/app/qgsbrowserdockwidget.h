/***************************************************************************
    qgsbrowserdockwidget.h
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBROWSERDOCKWIDGET_H
#define QGSBROWSERDOCKWIDGET_H

#include <QDockWidget>

class QgsBrowserModel;
class QModelIndex;
class QTreeView;
class QgsLayerItem;

class QgsBrowserDockWidget : public QDockWidget
{
    Q_OBJECT
  public:
    explicit QgsBrowserDockWidget( QWidget *parent = 0 );

  signals:

  public slots:
    void addLayerAtIndex( const QModelIndex& index );
    void showContextMenu( const QPoint & );

    void addFavourite();
    void removeFavourite();

    void refresh();

    // layer menu items
    void addCurrentLayer();
    void addSelectedLayers();
    void showProperties();

  protected:

    void refreshModel( const QModelIndex& index );

    void showEvent( QShowEvent * event );

    void addLayer( QgsLayerItem *layerItem );

    QTreeView* mBrowserView;
    QgsBrowserModel* mModel;
};

#endif // QGSBROWSERDOCKWIDGET_H
