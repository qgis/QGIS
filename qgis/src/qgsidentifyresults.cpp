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
#include <qlabel.h>
#include <qpopupmenu.h>

#include "qgsidentifyresults.h"

QgsIdentifyResults::QgsIdentifyResults(const QgsAttributeAction& aa, QWidget *parent, 
	                               const char * name, WFlags f) :
    QgsIdentifyResultsBase ( parent, name, f),
    mActions(aa), mClickedOnValue(0), mActionPopup(0)
{
    lstResults->setResizeMode(QListView::AllColumns);

    connect ( lstResults, SIGNAL(clicked(QListViewItem *)), this, SLOT(clicked(QListViewItem *)));
}

QgsIdentifyResults::~QgsIdentifyResults()
{
  saveWindowLocation();
  delete mActionPopup;
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

// Popup (create if necessary) a context menu that contains a list of
// actions that can be applied to the data in the identify results
// dialog box.
void QgsIdentifyResults::popupContextMenu(QListViewItem* item, 
					  const QPoint& p, int i)
{
  // if the user clicked below the end of the attribute list, just return
  if (item == NULL)
    return;
  
  // The assumption is made that an instance of QgsIdentifyResults is
  // created for each new Identify Results dialog box, and that the
  // contents of the popup menu doesn't change during the time that
  // such a dialog box is around.
  if (mActionPopup == 0)
  {
    mActionPopup = new QPopupMenu();

    QLabel* popupLabel = new QLabel( mActionPopup );
    popupLabel->setText( tr("<center>Run action</center>") );
    mActionPopup->insertItem(popupLabel);
    mActionPopup->insertSeparator();

    QgsAttributeAction::aIter	iter = mActions.begin();
    for (int j = 0; iter != mActions.end(); ++iter, ++j)
    {
      int id = mActionPopup->insertItem(iter->name(), this, 
					SLOT(popupItemSelected(int)));
      mActionPopup->setItemParameter(id, j);
    }
  }
  // Save the attribute values as these are needed for substituting into
  // the action. 
  // A little bit complicated because the user could of right-clicked
  // on a parent or a child in the dialog box. We also want to keep
  // track of which row in the identify results table was actually
  // clicked on. This is stored as an index into the mValues vector.

  QListViewItem* parent = item->parent();
  QListViewItem* child;

  if (item->parent() == 0)
    child = item->firstChild();
  else
    child = parent->firstChild();

  mValues.clear();
  int j = 0;
  while (child != 0)
    {
      if ( child->text(2) != "action" ) {
	  mValues.push_back(std::make_pair(child->text(0), child->text(1)));
	  // Need to do the comparison on the text strings rather than the
	  // pointers because if the user clicked on the parent, we need
	  // to pick up which child that actually is (the parent in the
	  // identify results dialog box is just one of the children
	  // that has been chosen by some method).
	  if (child->text(0) == item->text(0))
	    mClickedOnValue = j;
	  ++j;
      }
      child = child->nextSibling();
    }

  mActionPopup->popup(p);
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
void QgsIdentifyResults::addAttribute(QString field, QString value)
{
  new QListViewItem(lstResults, field, value);
}

void QgsIdentifyResults::addAction(QListViewItem * fnode, int id, QString field, QString value)
{
  QListViewItem *item = new QListViewItem(fnode, field, value, "action", QString::number(id) );

  QString appDir;
#if defined(WIN32) || defined(Q_OS_MACX)
  appDir = qApp->applicationDirPath();
#else
  appDir = PREFIX;
#endif

  QString img = appDir + "/share/qgis/themes/default/action.png";
  
  QPixmap pm ( img );
  item->setPixmap ( 0, pm ); 
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

void QgsIdentifyResults::setColumnText ( int column, const QString & label )
{
    lstResults->setColumnText ( column, label );
}

// Run the action that was selected in the popup menu
void QgsIdentifyResults::popupItemSelected(int id)
{
  mActions.doAction(id, mValues, mClickedOnValue);
}

/** Expand all the identified features (show their attributes). */
void QgsIdentifyResults::showAllAttributes() {
  QListViewItemIterator qlvii(lstResults);
  for ( ; *qlvii; ++qlvii)
    lstResults->setOpen(*qlvii, true);
}

void QgsIdentifyResults::clear()
{
    lstResults->clear();
}

void QgsIdentifyResults::setMessage( QString shortMsg, QString longMsg )
{
    new QListViewItem(lstResults, shortMsg, longMsg );
}

void QgsIdentifyResults::setActions( const QgsAttributeAction& actions  )
{
    mActions = actions;
}

void QgsIdentifyResults::clicked ( QListViewItem *item )
{
    if ( !item ) return;
    
    if ( item->text(2) != "action" ) return;
    
    int id = item->text(3).toInt();
    
    QListViewItem* parent = item->parent();
    QListViewItem* child;

    if (item->parent() == 0)
      child = item->firstChild();
    else
      child = parent->firstChild();

    mValues.clear();

    int j = 0;
    
    while (child != 0)
    {
      if ( child->text(2) != "action" ) {
	  mValues.push_back(std::make_pair(child->text(0), child->text(1)));

	  if (child->text(0) == item->text(0))
	      mClickedOnValue = j;
	  
	  ++j;
      }
      child = child->nextSibling();
    }

    mActions.doAction(id, mValues, mClickedOnValue);
}
