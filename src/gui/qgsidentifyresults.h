/***************************************************************************
                      qgsidentifyresults.h  -  description
                               ------------------
        begin                : Fri Oct 25 2002
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
#ifndef QGSIDENTIFYRESULTS_H
#define QGSIDENTIFYRESULTS_H

#include "ui_qgsidentifyresultsbase.h"
#include "qgsattributeaction.h"
#include <QWidget>
#include <vector>
#include <map>

class QCloseEvent;
class Q3ListViewItem;
class Q3PopupMenu;
class QAction;

/**
 *@author Gary E.Sherman
 */

class QgsIdentifyResults: public QDialog, private Ui::QgsIdentifyResultsBase
{
  Q_OBJECT;
  public:

  //! Constructor - takes it own copy of the QgsAttributeAction so
  // that it is independent of whoever created it.
  QgsIdentifyResults(const QgsAttributeAction& actions, QWidget *parent = 0, Qt::WFlags f = 0);

  ~QgsIdentifyResults();

  /** Add an attribute to the feature display node */
  void addAttribute(Q3ListViewItem *parent, QString field, QString value);

  /** Add an attribute */
  void addAttribute(QString field, QString value);

  /** Add a derived attribute (e.g. Length, Area) to the feature display node */
  void addDerivedAttribute(Q3ListViewItem *parent, QString field, QString value);

  /** Add an action to the feature display node */
  void addAction(Q3ListViewItem *parent, int id, QString field, QString value);

  /** Add a feature node to the feature display */
  Q3ListViewItem * addNode(QString label);
  /** Set the title for the identify results dialog */
  void setTitle(QString title);
  /** Set header column */
  void setColumnText ( int column, const QString & label );
  void saveWindowLocation();
  void restorePosition();  
  void closeEvent(QCloseEvent *e);
  void showAllAttributes();
  void adjustColumnWidths();

  /** Remove results */
  void clear();
  
  /** Set "No features ... " */
  void setMessage( QString shortMsg, QString longMsg );

  /** Set actions */
  void setActions ( const QgsAttributeAction& actions );
  
  //void accept();
  //void reject();

  public slots:

    void close();
    void popupContextMenu(Q3ListViewItem*, const QPoint&, int);
    void popupItemSelected(QAction* menuAction);

    /* Item in tree was clicked */
    void clicked ( Q3ListViewItem *lvi );

    //! Context help
    void on_buttonHelp_clicked();

 private:
  
  QgsAttributeAction mActions;
  int mClickedOnValue;
  QMenu* mActionPopup;
  std::vector<std::pair<QString, QString> > mValues;
  static const int context_id = -689216579;

  /**
   Keeps track of what derived-attribute (e.g. Length, Area)
   root nodes have been generated for each feature in this widget.

   First item:  Feature root node
   Second item: Derived-attribute root node for that feature
   */
  std::map<Q3ListViewItem *, Q3ListViewItem *> mDerivedAttributeRootNodes;
};

#endif
