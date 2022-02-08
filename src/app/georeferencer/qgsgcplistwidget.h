/***************************************************************************
    qgsgcplistwidget.h - Widget for GCP list display
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGS_GCP_LIST_WIDGET_H
#define QGS_GCP_LIST_WIDGET_H

#include <QTableView>

class QgsDoubleSpinBoxDelegate;
class QgsNonEditableDelegate;
class QgsDmsAndDdDelegate;
class QgsCoordDelegate;

class QgsGCPList;
class QgsGCPListModel;
class QgsGeorefTransform;
class QgsGeorefDataPoint;
class QgsPointXY;

class QgsGCPListWidget : public QTableView
{
    Q_OBJECT
  public:
    explicit QgsGCPListWidget( QWidget *parent = nullptr );

    void setGCPList( QgsGCPList *theGCPList );
    void setGeorefTransform( QgsGeorefTransform *georefTransform );
    QgsGCPList *gcpList() { return mGCPList; }
    void updateGCPList();
    void closeEditors();

    void keyPressEvent( QKeyEvent *e ) override;

  public slots:
    // This slot is called by the list view if an item is double-clicked
    void itemDoubleClicked( const QModelIndex &index );
    void itemClicked( QModelIndex index );

  signals:
    void jumpToGCP( const QgsPointXY &point );
    void pointEnabled( QgsGeorefDataPoint *pnt, int i );
    void deleteDataPoint( int index );

  private slots:
    void updateItemCoords( QWidget *editor );
    void showContextMenu( QPoint );
    void removeRow();
    void editCell();
    void jumpToPoint();

  private:
    void createActions();
    void createItemContextMenu();
    void adjustTableContent();

    QgsGCPList               *mGCPList = nullptr;
    QgsGCPListModel          *mGCPListModel = nullptr;

    QgsNonEditableDelegate   *mNonEditableDelegate = nullptr;
    QgsDmsAndDdDelegate      *mDmsAndDdDelegate = nullptr;
    QgsCoordDelegate         *mCoordDelegate = nullptr;

    int mPrevRow = 0;
    int mPrevColumn = 0;
};

#endif
