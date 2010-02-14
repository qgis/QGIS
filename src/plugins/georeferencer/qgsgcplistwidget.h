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
/* $Id */
#ifndef QGS_GCP_LIST_WIDGET_H
#define QGS_GCP_LIST_WIDGET_H

#include <QWidget>
#include "ui_qgsgcplistwidgetbase.h"
#include "qgsgcplist.h"

//class QgsGCPList;
class QgsGCPListModel;
class QgsGeorefTransform;

class QgsGCPListWidget : public QWidget, private Ui::QgsGCPListWidgetBase {
  Q_OBJECT
public:
  QgsGCPListWidget(QWidget *parent = 0);

  void setGCPList(QgsGCPList *theGCPList);
  void setGeorefTransform(QgsGeorefTransform *theGeorefTransform);
public slots:
  // This slot is called by the list view if an item is double-clicked
  void itemDoubleClicked(const QModelIndex &);
signals:
  void jumpToGCP(uint theGCPIndex);
private:
  void initialize();

  QgsGCPList      *mGCPList;
  QgsGCPListModel *mGCPListModel;
};

#endif
