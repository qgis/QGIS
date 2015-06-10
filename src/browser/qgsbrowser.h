/***************************************************************************
                 qgsbrowser.h  - Data sources browser
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBROWSER_H
#define QGSBROWSER_H

#include <QMainWindow>
#include <QMap>
#include <QModelIndex>
#include "ui_qgsbrowserbase.h"

class QgsBrowserModel;
class QgsLayerItem;
class QgsMapLayer;

class QgsBrowser : public QMainWindow, private Ui::QgsBrowserBase
{
    Q_OBJECT
  public:
    QgsBrowser( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
    ~QgsBrowser();

    // Expand to given path
    void expandPath( QString path );
    void setLayer( QgsVectorLayer* vLayer );


  public slots:
    void itemClicked( const QModelIndex& index );
    void itemDoubleClicked( const QModelIndex& index );
    void on_mActionSetProjection_triggered();
    void on_mActionWmsConnections_triggered();
    void on_mActionRefresh_triggered();
    void newVectorLayer();

    void saveWindowState();
    void restoreWindowState();

    void tabChanged();
    void updateCurrentTab();
    void stopRendering();

    // Refresh all leaf or expanded items
    void refresh( const QModelIndex& index = QModelIndex() );

  protected:
    void keyPressEvent( QKeyEvent * e ) override;
    void keyReleaseEvent( QKeyEvent * e ) override;

    bool layerClicked( QgsLayerItem* ptr );

    enum Tab
    {
      Metadata,
      Preview,
      Attributes
    };
    Tab activeTab();

    bool mDirtyMetadata, mDirtyPreview, mDirtyAttributes;

    QgsBrowserModel* mModel;
    QgsMapLayer *mLayer;
    QModelIndex mIndex;
    QWidget *mParamWidget;
    // last (selected) tab for each
    QMap<QString, int> mLastTab;
    QgsAttributeTableFilterModel* mAttributeTableFilterModel;
};

#endif // QGSBROWSER_H
