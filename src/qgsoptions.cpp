/***************************************************************************
                          qgsoptions.cpp
                    Set user options and preferences
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
#include <qsettings.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "qgsoptions.h"
#include "qgisapp.h"
/**
 * \class QgsOptions - Set user options and preferences
 * Constructor
 */
QgsOptions::QgsOptions(QWidget *parent, const char *name) : QgsOptionsBase(parent, name)
{
  qparent = parent;
    // read the current browser and set it
    QSettings settings;
    QString browser = settings.readEntry("/qgis/browser");
    cmbBrowser->setCurrentText(browser);
    // set the show splash option
    int identifyValue = settings.readNumEntry("/qgis/map/identifyRadius");
    spinBoxIdentifyValue->setValue(identifyValue);
    bool hideSplashFlag = false;
    if (settings.readEntry("/qgis/hideSplash")=="true")
    {
      hideSplashFlag =true;
    }
    cbxHideSplash->setChecked(hideSplashFlag);
    // set the current theme
    cmbTheme->setCurrentText(settings.readEntry("/qgis/theme"));
}
//! Destructor
QgsOptions::~QgsOptions(){}

void QgsOptions::themeChanged(const QString &newThemeName)
{
  // Slot to change the theme as user scrolls through the choices
  QString newt = newThemeName;
  ((QgisApp*)qparent)->setTheme(newt);
}
QString QgsOptions::theme()
{
  // returns the current theme (as selected in the cmbTheme combo box)
  return cmbTheme->currentText();
}
