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
#ifdef WIN32
#include "qgsidentifyresultsbase.h"
#else
#include "qgsidentifyresultsbase.uic.h"
#endif

#include "qgsattributeaction.h"
#include <vector>
#include <map>

class QPopupMenu;

/**
 *@author Gary E.Sherman
 */

class QgsIdentifyResults:public QgsIdentifyResultsBase
{
  Q_OBJECT;
  public:

  //! Constructor - takes it own copy of the QgsAttributeAction so
  // that it is independent of whoever created it.
  QgsIdentifyResults(const QgsAttributeAction&, QWidget *parent = 0, const char * name = 0, 
	             WFlags f = Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title 
		     | Qt::WStyle_Dialog | Qt::WStyle_Tool);

  ~QgsIdentifyResults();
  /** Add an attribute to the feature display node */
  void addAttribute(QListViewItem *parent, QString field, QString value);
  /** Add an attribute */
  void addAttribute(QString field, QString value);

  /** Add an action to the feature display node */
  void addAction(QListViewItem *parent, int id, QString field, QString value);

  /** Add a feature node to the feature display */
  QListViewItem * addNode(QString label);
  /** Set the title for the identify results dialog */
  void setTitle(QString title);
  /** Set header column */
  void setColumnText ( int column, const QString & label );
  void saveWindowLocation();
  void restorePosition();  
  void close();
  void closeEvent(QCloseEvent *e);
  void popupContextMenu(QListViewItem*, const QPoint&, int);
  void showAllAttributes();

  /** Remove results */
  void clear();
  
  /** Set "No features ... " */
  void setMessage( QString shortMsg, QString longMsg );

  /** Set actions */
  void setActions ( const QgsAttributeAction& actions );
  
  //void accept();
  //void reject();

  public slots:

    void popupItemSelected(int id);

    /* Item in tree was clicked */
    void clicked ( QListViewItem *lvi );

 private:
  
  QgsAttributeAction mActions;
  int mClickedOnValue;
  QPopupMenu* mActionPopup;
  std::vector<std::pair<QString, QString> > mValues;
};

#endif
