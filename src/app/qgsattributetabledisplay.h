/***************************************************************************
                          qgsattributetabledisplay.h  -  description
                             -------------------
    begin                : Sat Nov 23 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSATTRIBUTETABLEDISPLAY_H
#define QGSATTRIBUTETABLEDISPLAY_H

#include "ui_qgsattributetablebase.h"

class QDockWidget;
class QgsAttributeTable;
class QgsVectorLayer;
class QgisApp;
class QgsAttributeActions;

/**
  *@author Gary E.Sherman
  */

class QgsAttributeTableDisplay : public QDialog, private Ui::QgsAttributeTableBase
{
  Q_OBJECT
  public:
    static QgsAttributeTableDisplay *attributeTable(QgsVectorLayer *layer);

    ~QgsAttributeTableDisplay();

    void fillTable();

  protected:
    QgsAttributeTableDisplay(QgsVectorLayer* layer);

    QgsVectorLayer* mLayer;

    void doSearch(QString searchString);
    void setAttributeActions(const QgsAttributeAction &actions);
    void selectRowsWithId(const QgsFeatureIds &ids);

    virtual void closeEvent(QCloseEvent *ev);

    /** array of feature IDs that match last searched condition */
    QgsFeatureIds mSearchIds;

  protected slots:
    void selectedToTop();
    void invertSelection();
    void removeSelection();
    void copySelectedRowsToClipboard();
    void zoomMapToSelectedRows();
    void search();
    void advancedSearch();
    void searchShowResultsChanged(int item);
    void showHelp();
    void toggleEditing();

    void attributeAdded(int idx);
    void attributeDeleted(int idx);

  public slots:
    void changeFeatureAttribute(int row, int column);
    void editingToggled();
    void selectionChanged();
 
  signals:
    void editingToggled(QgsMapLayer *);

  private:
    /** Set the icon theme for this dialog */
    void setTheme();

    void restorePosition();
    void saveWindowLocation();

    QString mSearchString;

    QDockWidget *mDock;

    static const int context_id = 831088384;

    static QMap<QgsVectorLayer *, QgsAttributeTableDisplay *> smTables;
};

#endif
