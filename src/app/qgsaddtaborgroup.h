/***************************************************************************
                          qgsaddtaborgroup.h
        Add a tab or a group for the tab and group display of fields
                             -------------------
    begin                : 2012-07-30
    copyright            : (C) 2012 by Denis Rouzaud
    email                : denis dot rouzaud at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADDTABORGROUP
#define QGSADDTABORGROUP

#include "ui_qgsaddtaborgroupbase.h"
#include "qgisgui.h"

class QTreeWidgetItem;
class QgsVectorLayer;

class APP_EXPORT QgsAddTabOrGroup : public QDialog, private Ui::QgsAddTabOrGroupBase
{
    Q_OBJECT

  public:
    typedef QPair<QString, QTreeWidgetItem*> TabPair;

  public:
    QgsAddTabOrGroup( QgsVectorLayer *lyr, QList< TabPair > tabList, QWidget *parent = 0 );
    ~QgsAddTabOrGroup();

    QString name();

    QTreeWidgetItem* tab();

    bool tabButtonIsChecked();

  public slots:
    void on_mGroupButton_toggled( bool checked );
    void on_mTabButton_toggled( bool checked );

  protected:
    QgsVectorLayer *mLayer;
    QList< TabPair > mTabs;
};

#endif
