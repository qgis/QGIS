/***************************************************************************
                          qgslayerprojectionselector.cpp
                    Set user layerprojectionselector and preferences
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
#include "qgslayerprojectionselector.h"
#include <QApplication>

/**
 * \class QgsLayerProjectionSelector - Set user layerprojectionselector and preferences
 * Constructor
 */
QgsLayerProjectionSelector::QgsLayerProjectionSelector(QWidget *parent, 
                                                       Qt::WFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);
  
  // Set up text edit pane
  QString format("<h2>%1</h2>%2 %3");
  QString header = tr("Define this layer's projection:");
  QString sentence1 = tr("This layer appears to have no projection specification.");
  QString sentence2 = tr("By default, this layer will now have its projection set to that of the project"
			 ", but you may override this by selecting a different projection below.");
  textEdit->setHtml(format.arg(header).arg(sentence1)
		    .arg(sentence2));

  connect(pbnOK, SIGNAL(clicked()), this, SLOT( accept()));

  QApplication::restoreOverrideCursor();
}

//! Destructor
QgsLayerProjectionSelector::~QgsLayerProjectionSelector()
{}

void QgsLayerProjectionSelector::setSelectedSRSName(QString theName)
{
  projectionSelector->setSelectedSRSName(theName);
}

void QgsLayerProjectionSelector::setSelectedSRSID(long theID)
{
  projectionSelector->setSelectedSRSID(theID);
}

QString QgsLayerProjectionSelector::getCurrentProj4String()
{
  //@NOTE dont use getSelectedWKT as that just returns the name part!
  return projectionSelector->getCurrentProj4String();
}

long QgsLayerProjectionSelector::getCurrentSRSID()
{
  //@NOTE dont use getSelectedWKT as that just returns the name part!
  return projectionSelector->getCurrentSRSID();
}

long QgsLayerProjectionSelector::getCurrentEpsg()
{
  return projectionSelector->getCurrentEpsg();
}

void QgsLayerProjectionSelector::setOgcWmsCrsFilter(QSet<QString> crsFilter)
{
  projectionSelector->setOgcWmsCrsFilter(crsFilter);
}

