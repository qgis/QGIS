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
#include "qgsprojectionselector.h"
#include <qapplication.h>
/**
 * \class QgsLayerProjectionSelector - Set user layerprojectionselector and preferences
 * Constructor
 */
QgsLayerProjectionSelector::QgsLayerProjectionSelector(QWidget *parent, 
    const char * name, bool modal) : QgsLayerProjectionSelectorBase(parent, name, modal)
{

 qApp->restoreOverrideCursor();
}
//! Destructor
QgsLayerProjectionSelector::~QgsLayerProjectionSelector()
{}

void QgsLayerProjectionSelector::pbnOK_clicked()
{
 accept();
}

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
