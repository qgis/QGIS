/***************************************************************************
    qgsbrowserdockwidget.h
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
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
#include <ui_qgsbrowserdockwidgetbase.h>

#include "qgsdataitem.h"

class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserTreeFilterProxyModel;

class APP_EXPORT QgsBrowserDockWidget : public QDockWidget, private Ui::QgsBrowserDockWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsBrowserDockWidget( QString name, QWidget *parent = 0 );
    //~QgsBrowserDockWidget();
    void addFavouriteDirectory( QString favDir );

  public slots:
    void addLayerAtIndex( const QModelIndex& index );
    void showContextMenu( const QPoint & );

    void addFavourite();
    void addFavouriteDirectory();
    void removeFavourite();

    void refresh();

    void showFilterWidget( bool visible );
    void setFilterSyntax( QAction * );
    void setCaseSensitive( bool caseSensitive );
    void setFilter();

    // layer menu items
    void addCurrentLayer();
    void addSelectedLayers();
    void showProperties();
    void toggleFastScan();

  protected:
    void refreshModel( const QModelIndex& index );

    void showEvent( QShowEvent * event ) override;

    void addLayer( QgsLayerItem *layerItem );

    QgsDockBrowserTreeView* mBrowserView;
    QgsBrowserModel* mModel;
    QgsBrowserTreeFilterProxyModel* mProxyModel;
    QString mInitPath;

  private:
};

#endif // QGSBROWSERDOCKWIDGET_H
