/***************************************************************************
    qgsgcplistwidget.cpp - Widget for GCP list display
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#include <QHeaderView>

#include "qgsgcplistwidget.h"
#include "qgsgcplistmodel.h"

#include "qgspointdialog.h"

#include <iostream> //debugging

QgsGCPListWidget::QgsGCPListWidget(QWidget *parent) : QWidget(parent)
{
  setupUi(this);
  initialize();
}

void QgsGCPListWidget::initialize()
{
  mGCPListModel = new QgsGCPListModel;
  mGCPTableView->setModel(mGCPListModel);
  mGCPTableView->setSortingEnabled(true);
  mGCPTableView->verticalHeader()->hide();

  connect(mGCPTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(itemDoubleClicked(const QModelIndex &)));
}

void QgsGCPListWidget::setGCPList(QgsGCPList *theGCPList)
{
  mGCPListModel->setGCPList(theGCPList);
}

void QgsGCPListWidget::setGeorefTransform(QgsGeorefTransform *theGeorefTransform)
{
  mGCPListModel->setGeorefTransform(theGeorefTransform);
}

void QgsGCPListWidget::itemDoubleClicked(const QModelIndex &index)
{
  QStandardItem *item = mGCPListModel->item(index.row(), 0);
  bool ok;
  int id = item->text().toInt(&ok);

  if (ok)
  {
   emit jumpToGCP(id);
  }
}


