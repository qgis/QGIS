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
#include <iostream>
#include <qlistview.h>
#include <qsettings.h>
#include <qpoint.h>
#include <qsize.h>
#include <qevent.h>

#include "qgsidentifyresults.h"

QgsIdentifyResults::QgsIdentifyResults()
{
}

QgsIdentifyResults::~QgsIdentifyResults()
{

}
// Slot called when user clicks the Close button
// (saves the current window size/position)
void QgsIdentifyResults::close()
{
  saveWindowLocation();
  hide();
}
// Save the current window size/position before closing 
// from window menu or X in titlebar
void QgsIdentifyResults::closeEvent(QCloseEvent *e)
{
  saveWindowLocation();
  e->accept();
}
// Restore last window position/size and show the window
void QgsIdentifyResults::restorePosition()
{

  QSettings settings;
  int ww = settings.readNumEntry("/qgis/Windows/Identify/w", 281);
  int wh = settings.readNumEntry("/qgis/Windows/Identify/h", 316);
  int wx = settings.readNumEntry("/qgis/Windows/Identify/x", 100);
  int wy = settings.readNumEntry("/qgis/Windows/Identify/y", 100);
  //std::cerr << "Setting geometry: " << wx << ", " << wy << ", " << ww << ", " << wh << std::endl;
  resize(ww,wh);
  move(wx,wy);
  QgsIdentifyResultsBase::show();
  //std::cerr << "Current geometry: " << x() << ", " << y() << ", " << width() << ", " << height() << std::endl; 
}
// Save the current window location (store in ~/.qt/qgisrc)
void QgsIdentifyResults::saveWindowLocation()
{
  QSettings settings;
  QPoint p = this->pos();
  QSize s = this->size();
  settings.writeEntry("/qgis/Windows/Identify/x", p.x());
  settings.writeEntry("/qgis/Windows/Identify/y", p.y());
  settings.writeEntry("/qgis/Windows/Identify/w", s.width());
  settings.writeEntry("/qgis/Windows/Identify/h", s.height());
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
