/***************************************************************************
  qgsidentifyresults.h  -  description
  -------------------
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

#include "qgsidentifyresultsbase.uic.h"

/**
 *@author Gary E.Sherman
 */

class QgsIdentifyResults:public QgsIdentifyResultsBase
{
  Q_OBJECT;
  public:
  QgsIdentifyResults();
  ~QgsIdentifyResults();
  /** Add an attribute to the feature display node */
  void addAttribute(QListViewItem *parent, QString field, QString value);
  /** Add a feature node to the feature display */
  QListViewItem * addNode(QString label);
  /** Set the title for the identify results dialog */
  void setTitle(QString title);
  void saveWindowLocation();
  void restorePosition();  
  void close();
  void closeEvent(QCloseEvent *e);
  //void accept();
  //void reject();
};

#endif
