/***************************************************************************
                          qgsgenericprojectionselector.cpp
                    Set user defined CRS using projection selector widget
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
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
#include "qgsgenericprojectionselector.h"
#include <QApplication>

/**
 * \class QgsGenericProjectionSelector 
 * \brief A generic dialog to prompt the user for a Coordinate Reference System
 */
QgsGenericProjectionSelector::QgsGenericProjectionSelector(QWidget *parent, 
                                                       Qt::WFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);
  //we will show this only when a message is set
  textEdit->hide();
}

void QgsGenericProjectionSelector::setMessage(QString theMessage)
{
  //short term kludge to make the layer selector default to showing
  //a layer projection selection message. If you want the selector
  if (theMessage.isEmpty())
  {
    // Set up text edit pane
    QString format("<h2>%1</h2>%2 %3");
    QString header = tr("Define this layer's projection:");
    QString sentence1 = tr("This layer appears to have no projection specification.");
    QString sentence2 = tr("By default, this layer will now have its projection set to that of the project"
        ", but you may override this by selecting a different projection below.");
    textEdit->setHtml(format.arg(header).arg(sentence1)
        .arg(sentence2));
  }
  else
  {
    textEdit->setHtml(theMessage);
  }
  textEdit->show();

}
//! Destructor
QgsGenericProjectionSelector::~QgsGenericProjectionSelector()
{}

void QgsGenericProjectionSelector::setSelectedCRSName(QString theName)
{
  projectionSelector->setSelectedCRSName(theName);
}

void QgsGenericProjectionSelector::setSelectedCRSID(long theID)
{
  projectionSelector->setSelectedCRSID(theID);
}

void QgsGenericProjectionSelector::setSelectedEpsg(long theID)
{
  projectionSelector->setSelectedEpsg(theID);
}

QString QgsGenericProjectionSelector::getSelectedProj4String()
{
  //@NOTE dont use getSelectedWKT as that just returns the name part!
  return projectionSelector->getSelectedProj4String();
}

long QgsGenericProjectionSelector::getSelectedCRSID()
{
  //@NOTE dont use getSelectedWKT as that just returns the name part!
  return projectionSelector->getSelectedCRSID();
}

long QgsGenericProjectionSelector::getSelectedEpsg()
{
  return projectionSelector->getSelectedEpsg();
}

void QgsGenericProjectionSelector::setOgcWmsCrsFilter(QSet<QString> crsFilter)
{
  projectionSelector->setOgcWmsCrsFilter(crsFilter);
}

