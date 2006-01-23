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
#include "qgsapplication.h"
#include "qgsoptions.h"
#include "qgis.h"
#include "qgisapp.h"
#include "qgslayerprojectionselector.h"
#include "qgssvgcache.h"
#include <QFileDialog>
#include <QSettings>
#include <QColorDialog>
#include <cassert>
#include <iostream>
#include <sqlite3.h>
/**
 * \class QgsOptions - Set user options and preferences
 * Constructor
 */
QgsOptions::QgsOptions(QWidget *parent, Qt::WFlags fl) :
  QDialog(parent, fl)
{
  setupUi(this);
  connect(cmbTheme, SIGNAL(activated(const QString&)), this, SLOT(themeChanged(const QString&)));
  connect(cmbTheme, SIGNAL(highlighted(const QString&)), this, SLOT(themeChanged(const QString&)));
  connect(cmbTheme, SIGNAL(textChanged(const QString&)), this, SLOT(themeChanged(const QString&)));
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(saveOptions()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  qparent = parent;
  // read the current browser and set it
  QSettings settings;
  QString browser = settings.readEntry("/qgis/browser");
  cmbBrowser->setCurrentText(browser);
  std::cout << "Standard Identify radius setting: " << QGis::DEFAULT_IDENTIFY_RADIUS << std::endl;
  int identifyValue = settings.readNumEntry("/Map/identifyRadius",QGis::DEFAULT_IDENTIFY_RADIUS);
  std::cout << "Standard Identify radius setting read from settings file: " << identifyValue << std::endl;
  spinBoxIdentifyValue->setValue(identifyValue);

  // set the current theme
  cmbTheme->setCurrentText(settings.readEntry("/Themes"));
  // set the display update threshold
  spinBoxUpdateThreshold->setValue(settings.readNumEntry("/Map/updateThreshold"));
  //set the default projection behaviour radio buttongs
  if (settings.readEntry("/Projections/defaultBehaviour")=="prompt")
  {
    radPromptForProjection->setChecked(true);
  }
  else if (settings.readEntry("/Projections/defaultBehaviour")=="useProject")
  {
    radUseProjectProjection->setChecked(true);
  }
  else //useGlobal
  {
    radUseGlobalProjection->setChecked(true);
  }
  mGlobalSRSID = settings.readNumEntry("/Projections/defaultProjectionSRSID",GEOSRS_ID);
  //! @todo changes this control name in gui to txtGlobalProjString
  QString myProjString = QgsSpatialRefSys::getProj4FromSrsId(mGlobalSRSID);
  txtGlobalWKT->setText(myProjString);
  
  // populate combo box with ellipsoids
  getEllipsoidList();
  QString myEllipsoidId = settings.readEntry("/qgis/measure/ellipsoid", "WGS84");
  cmbEllipsoid->setCurrentText(getEllipsoidName(myEllipsoidId));
  // add the themes to the combo box on the option dialog
  QDir myThemeDir(QgsApplication::themePath());
  myThemeDir.setFilter(QDir::Dirs);
  QStringList myDirList = myThemeDir.entryList("*");
  for(int i=0; i < myDirList.count(); i++)
  {
    if(myDirList[i] != "." && myDirList[i] != "..")
    {
      cmbTheme->insertItem(myDirList[i]);
    }
  }
  // set the theme combo
  cmbTheme->setCurrentText(settings.readEntry("/Themes","default"));
  //set teh state of the checkboxes
  chkAntiAliasing->setChecked(settings.value("/qgis/enable_anti_aliasing",false).toBool());
  chkAddedVisibility->setChecked(!settings.value("/qgis/new_layers_visible",false).toBool());
  cbxHideSplash->setChecked(settings.value("/qgis/hideSplash",false).toBool());
  //set the colour for selections
  int myRed = settings.value("/qgis/default_selection_color_red",255).toInt();
  int myGreen = settings.value("/qgis/default_selection_color_green",255).toInt();
  int myBlue = settings.value("/qgis/default_selection_color_blue",255).toInt();
  pbnSelectionColour->setPaletteBackgroundColor(QColor(myRed,myGreen,myBlue));
  //set teh default color for canvas background
  myRed = settings.value("/qgis/default_canvas_color_red",255).toInt();
  myGreen = settings.value("/qgis/default_canvas_color_green",255).toInt();
  myBlue = settings.value("/qgis/default_canvas_color_blue",255).toInt();
  pbnCanvasColor->setPaletteBackgroundColor(QColor(myRed,myGreen,myBlue));
}

//! Destructor
QgsOptions::~QgsOptions(){}

void QgsOptions::on_pbnSelectionColour_clicked()
{
  QColor color = QColorDialog::getColor(pbnSelectionColour->paletteBackgroundColor(),this);
  if (color.isValid())
  {
    pbnSelectionColour->setPaletteBackgroundColor(color);
  }
}

void QgsOptions::on_pbnCanvasColor_clicked()
{
  QColor color = QColorDialog::getColor(pbnCanvasColor->paletteBackgroundColor(),this);
  if (color.isValid())
  {
    pbnCanvasColor->setPaletteBackgroundColor(color);
  }
}
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

void QgsOptions::saveOptions()
{
  QSettings settings;
  settings.writeEntry("/qgis/browser", cmbBrowser->currentText());
  settings.writeEntry("/Map/identifyRadius", spinBoxIdentifyValue->value());
  settings.writeEntry("/qgis/hideSplash",cbxHideSplash->isChecked());
  settings.writeEntry("/qgis/new_layers_visible",!chkAddedVisibility->isChecked());
  settings.writeEntry("/qgis/enable_anti_aliasing",chkAntiAliasing->isChecked());
  if(cmbTheme->currentText().length() == 0)
  {
    settings.writeEntry("/Themes", "default");
  }else{
    settings.writeEntry("/Themes",cmbTheme->currentText());
  }
  settings.writeEntry("/Map/updateThreshold", spinBoxUpdateThreshold->value());
  //check behaviour so default projection when new layer is added with no
  //projection defined...
  if (radPromptForProjection->isChecked())
  {
    //
    settings.writeEntry("/Projections/defaultBehaviour", "prompt");
  }
  else if(radUseProjectProjection->isChecked())
  {
    //
    settings.writeEntry("/Projections/defaultBehaviour", "useProject");
  }
  else //assumes radUseGlobalProjection is checked
  {
    //
    settings.writeEntry("/Projections/defaultBehaviour", "useGlobal");
  }
  settings.writeEntry("/Projections/defaultProjectionSRSID",(int)mGlobalSRSID);

  settings.writeEntry("/qgis/measure/ellipsoid", getEllipsoidAcronym(cmbEllipsoid->currentText()));
  //set the colour for selections
  QColor myColor = pbnSelectionColour->paletteBackgroundColor();
  int myRed = settings.writeEntry("/qgis/default_selection_color_red",myColor.red());
  int myGreen = settings.writeEntry("/qgis/default_selection_color_green",myColor.green());
  int myBlue = settings.writeEntry("/qgis/default_selection_color_blue",myColor.blue());
  //set teh default color for canvas background
  myColor = pbnCanvasColor->paletteBackgroundColor();
  myRed = settings.writeEntry("/qgis/default_canvas_color_red",myColor.red());
  myGreen = settings.writeEntry("/qgis/default_canvas_color_green",myColor.green());
  myBlue = settings.writeEntry("/qgis/default_canvas_color_blue",myColor.blue());
  
  //all done
  accept();
}







void QgsOptions::on_btnFindBrowser_clicked()
{
  QString filter;
#ifdef WIN32
  filter = "Applications (*.exe)";
#else
  filter = "All Files (*)";
#endif
  QString browser = QFileDialog::getOpenFileName(
          this,
          "Choose a browser",
          "./",
          filter );
  if(browser.length() > 0)
  {
    cmbBrowser->setCurrentText(browser);
  }
}


void QgsOptions::on_pbnSelectProjection_clicked()
{
  QSettings settings;
  QgsLayerProjectionSelector * mySelector = new QgsLayerProjectionSelector(this);
  mySelector->setSelectedSRSID(mGlobalSRSID);
  if(mySelector->exec())
  {
#ifdef QGISDEBUG
    std::cout << "------ Global Default Projection Selection Set ----------" << std::endl;
#endif
    mGlobalSRSID = mySelector->getCurrentSRSID();  
    //! @todo changes this control name in gui to txtGlobalProjString
    txtGlobalWKT->setText(mySelector->getCurrentProj4String());
#ifdef QGISDEBUG
    std::cout << "------ Global Default Projection now set to ----------\n" << mGlobalSRSID << std::endl;
#endif
  }
  else
  {
#ifdef QGISDEBUG
    std::cout << "------ Global Default Projection Selection change cancelled ----------" << std::endl;
#endif
    QApplication::restoreOverrideCursor();
  }

}
// Return state of the visibility flag for newly added layers. If

bool QgsOptions::newVisible()
{
  return !chkAddedVisibility->isChecked();
}

void QgsOptions::getEllipsoidList()
{
  // (copied from qgscustomprojectiondialog.cpp)

  // 
  // Populate the ellipsoid combo
  // 
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(QgsApplication::qgisUserDbFilePath(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }

  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select * from tbl_ellipsoid order by name";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      cmbEllipsoid->insertItem((char *)sqlite3_column_text(myPreparedStatement,1));
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}

QString QgsOptions::getEllipsoidAcronym(QString theEllipsoidName)
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open(QgsApplication::qgisUserDbFilePath(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select acronym from tbl_ellipsoid where name='" + theEllipsoidName + "'";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    sqlite3_step(myPreparedStatement) == SQLITE_ROW;
    myName = QString((char *)sqlite3_column_text(myPreparedStatement,0));
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myName;

}

QString QgsOptions::getEllipsoidName(QString theEllipsoidAcronym)
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  QString       myName;
  //check the db is available
  myResult = sqlite3_open(QgsApplication::qgisUserDbFilePath(), &myDatabase);
  if(myResult) 
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl; 
    // XXX This will likely never happen since on open, sqlite creates the 
    //     database if it does not exist.
    assert(myResult == 0);
  }
  // Set up the query to retreive the projection information needed to populate the ELLIPSOID list
  QString mySql = "select name from tbl_ellipsoid where acronym='" + theEllipsoidAcronym + "'";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    sqlite3_step(myPreparedStatement) == SQLITE_ROW;
    myName = QString((char *)sqlite3_column_text(myPreparedStatement,0));
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myName;

}
