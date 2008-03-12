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


class QgsAttributeTable;
class QgsVectorLayer;
class QgisApp;

/**
  *@author Gary E.Sherman
  */

class QgsAttributeTableDisplay:public QDialog, private Ui::QgsAttributeTableBase
{
  Q_OBJECT
  public:
    /**
     \param qgisApp   This should be the QgisApp that spawned this table.
                      Otherwise the Copy button on this QgsAttributeTableDisplay
                      will not work.
     */
    QgsAttributeTableDisplay(QgsVectorLayer* layer, QgisApp * qgisApp);
    ~QgsAttributeTableDisplay();

    QgsAttributeTable *table();
    void setTitle(QString title);
  protected:
    QgsVectorLayer* mLayer;

    QgisApp * mQgisApp;

    void doSearch(const QString& searchString);

    virtual void closeEvent(QCloseEvent* ev);
    void showHelp();

    /** array of feature IDs that match last searched condition */
    QgsFeatureIds mSearchIds;

  protected slots:
    void deleteAttributes();
    void addAttribute();
    void startEditing();
    void stopEditing();
    void selectedToTop();
    void invertSelection();
    void removeSelection();
    void copySelectedRowsToClipboard();
    void zoomMapToSelectedRows();
    void search();
    void advancedSearch();
    void searchShowResultsChanged(int item);
    void on_btnHelp_clicked();

  public slots:
    void changeFeatureAttribute(int row, int column);

  signals:
    void deleted();

  private:
    /** Set the icon theme for this dialog */
    void setTheme();

    void restorePosition();
    void saveWindowLocation();

    QString mSearchString;

    static const int context_id = 831088384;
};

#endif
