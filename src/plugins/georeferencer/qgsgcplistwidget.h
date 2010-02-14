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
/* $Id$ */
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
class QgsPoint;

class QgsGCPListWidget : public QTableView
{
  Q_OBJECT
public:
  QgsGCPListWidget(QWidget *parent = 0);

  void setGCPList(QgsGCPList *theGCPList);
  void setGeorefTransform(QgsGeorefTransform *theGeorefTransform);
  QgsGCPList *gcpList() { return mGCPList; }
  void updateGCPList();

public slots:
  // This slot is called by the list view if an item is double-clicked
  void itemDoubleClicked(QModelIndex index);
  void itemClicked(QModelIndex index);

signals:
  void jumpToGCP(uint theGCPIndex);
//  void replaceDataPoint(QgsGeorefDataPoint *pnt, int row);
  void pointEnabled(QgsGeorefDataPoint *pnt, int i);
  void deleteDataPoint(int index);

private slots:
  void updateItemCoords(QWidget *editor);
  void showContextMenu(QPoint);
  void removeRow();
  void editCell();
  void jumpToPoint();

private:
  void createActions();
  void createItemContextMenu();
  void adjustTableContent();

  QMenu *mItemContextMenu;

  QgsGCPList               *mGCPList;
  QgsGCPListModel          *mGCPListModel;

  QgsNonEditableDelegate   *mNonEditableDelegate;
  QgsDmsAndDdDelegate      *mDmsAndDdDelegate;
  QgsCoordDelegate         *mCoordDelegate;

  int mPrevRow;
  int mPrevColumn;
};

#endif
