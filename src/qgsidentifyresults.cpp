/***************************************************************************
                          qgsidentifyresults.cpp  -  description
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
#include <qlistview.h>
#include "qgsidentifyresults.h"

QgsIdentifyResults::QgsIdentifyResults()
{
}

QgsIdentifyResults::~QgsIdentifyResults()
{
}

/** add an attribute and its value to the list */
void QgsIdentifyResults::addAttribute(QListViewItem * fnode, QString field, QString value)
{
  new QListViewItem(fnode, field, value);
}

/** Add a feature node to the list */
QListViewItem *QgsIdentifyResults::addNode(QString label)
{
  return (new QListViewItem(lstResults, label));
}

void QgsIdentifyResults::setTitle(QString title)
{
  setCaption("Identify Results - " + title);
}
