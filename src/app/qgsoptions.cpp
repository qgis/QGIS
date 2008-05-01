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
#include "qgsspatialrefsys.h"

#include <QFileDialog>
#include <QSettings>
#include <QColorDialog>
#include <QLocale>
#include <QTextCodec>

#include <cassert>
#include <iostream>
#include <sqlite3.h>
#define ELLIPS_FLAT "NONE"
#define ELLIPS_FLAT_DESC "None / Planimetric"

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
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(this, SIGNAL(accepted()), this, SLOT(saveOptions()));

  qparent = parent;
  // read the current browser and set it
  QSettings settings;
#ifdef QGISDEBUG
  std::cout << "Standard Identify radius setting: " << QGis::DEFAULT_IDENTIFY_RADIUS << std::endl;
#endif
  double identifyValue = settings.value("/Map/identifyRadius",QGis::DEFAULT_IDENTIFY_RADIUS).toDouble();
#ifdef QGISDEBUG
  std::cout << "Standard Identify radius setting read from settings file: " << identifyValue << std::endl;
#endif
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
  QDir myThemeDir( QgsApplication::pkgDataPath()+"/themes/" );
  myThemeDir.setFilter(QDir::Dirs);
  QStringList myDirList = myThemeDir.entryList("*");
  cmbTheme->clear();
  for(int i=0; i < myDirList.count(); i++)
  {
    if(myDirList[i] != "." && myDirList[i] != "..")
    {
      cmbTheme->insertItem(myDirList[i]);
    }
  }

  // set the theme combo
  cmbTheme->setCurrentText(settings.readEntry("/Themes","default"));

  //set the state of the checkboxes
  chkAntiAliasing->setChecked(settings.value("/qgis/enable_anti_aliasing",false).toBool());

  // Slightly awkard here at the settings value is true to use QImage,
  // but the checkbox is true to use QPixmap
  chkUseQPixmap->setChecked(!(settings.value("/qgis/use_qimage_to_render", true).toBool()));
  chkAddedVisibility->setChecked(settings.value("/qgis/new_layers_visible",true).toBool());
  cbxHideSplash->setChecked(settings.value("/qgis/hideSplash",false).toBool());

  //set the colour for selections
  int myRed = settings.value("/qgis/default_selection_color_red",255).toInt();
  int myGreen = settings.value("/qgis/default_selection_color_green",255).toInt();
  int myBlue = settings.value("/qgis/default_selection_color_blue",0).toInt();
  pbnSelectionColour->setColor( QColor(myRed,myGreen,myBlue) );

  //set the default color for canvas background
  myRed = settings.value("/qgis/default_canvas_color_red",255).toInt();
  myGreen = settings.value("/qgis/default_canvas_color_green",255).toInt();
  myBlue = settings.value("/qgis/default_canvas_color_blue",255).toInt();
  pbnCanvasColor->setColor( QColor(myRed,myGreen,myBlue) );
  
  // set the default color for the measure tool
  myRed = settings.value("/qgis/default_measure_color_red",180).toInt();
  myGreen = settings.value("/qgis/default_measure_color_green",180).toInt();
  myBlue = settings.value("/qgis/default_measure_color_blue",180).toInt();
  pbnMeasureColour->setColor( QColor(myRed,myGreen,myBlue) );

  capitaliseCheckBox->setChecked(settings.value("qgis/capitaliseLayerName", QVariant(false)).toBool());

  chbAskToSaveProjectChanges->setChecked(settings.value("qgis/askToSaveProjectChanges", QVariant(true)).toBool());
  chbWarnOldProjectVersion->setChecked(settings.value("/qgis/warnOldProjectVersion", QVariant(true)).toBool());
  
  cmbWheelAction->setCurrentIndex(settings.value("/qgis/wheel_action", 0).toInt());
  spinZoomFactor->setValue(settings.value("/qgis/zoom_factor", 2).toDouble());

  cbxSplitterRedraw->setChecked(settings.value("/qgis/splitterRedraw", QVariant(true)).toBool());

  //
  // Locale settings 
  //
  QString mySystemLocale = QTextCodec::locale();
  lblSystemLocale->setText(tr("Detected active locale on your system: ") + mySystemLocale);
  QString myUserLocale = settings.value("locale/userLocale", "").toString();
  QStringList myI18nList = i18nList();
  cboLocale->addItems(myI18nList);
  if (myI18nList.contains(myUserLocale))
  {
    cboLocale->setCurrentText(myUserLocale);
  }
  bool myLocaleOverrideFlag = settings.value("locale/overrideFlag",false).toBool();
  grpLocale->setChecked(myLocaleOverrideFlag);

  //set elements in digitizing tab
  mLineWidthSpinBox->setValue(settings.value("/qgis/digitizing/line_width", 1).toInt());
  QColor digitizingColor;
  myRed = settings.value("/qgis/digitizing/line_color_red", 255).toInt();
  myGreen = settings.value("/qgis/digitizing/line_color_green", 0).toInt();
  myBlue = settings.value("/qgis/digitizing/line_color_blue", 0).toInt();
  mLineColourToolButton->setColor(QColor(myRed, myGreen, myBlue));

  //default snap mode
  mDefaultSnapModeComboBox->insertItem(0, tr("to vertex"));
  mDefaultSnapModeComboBox->insertItem(1, tr("to segment"));
  mDefaultSnapModeComboBox->insertItem(2, tr("to vertex and segment"));
  QString defaultSnapString = settings.value("/qgis/digitizing/default_snap_mode", "to vertex").toString();
  mDefaultSnapModeComboBox->setCurrentIndex(mDefaultSnapModeComboBox->findText(tr(defaultSnapString)));
  mDefaultSnappingToleranceSpinBox->setValue(settings.value("/qgis/digitizing/default_snapping_tolerance", 0).toDouble());
  mSearchRadiusVertexEditSpinBox->setValue(settings.value("/qgis/digitizing/search_radius_vertex_edit", 10).toDouble());

  //vertex marker
  mMarkerStyleComboBox->addItem(tr("Semi transparent circle"));
  mMarkerStyleComboBox->addItem(tr("Cross"));

  QString markerStyle = settings.value("/qgis/digitizing/marker_style", "SemiTransparentCircle").toString();
  if(markerStyle == "SemiTransparentCircle")
    {
      mMarkerStyleComboBox->setCurrentIndex(mMarkerStyleComboBox->findText(tr("Semi transparent circle")));
    }
  else if(markerStyle == "Cross")
    {
      mMarkerStyleComboBox->setCurrentIndex(mMarkerStyleComboBox->findText(tr("Cross")));
    }
}

//! Destructor
QgsOptions::~QgsOptions(){}

void QgsOptions::on_pbnSelectionColour_clicked()
{
  QColor color = QColorDialog::getColor(pbnSelectionColour->color(), this);
  if (color.isValid())
  {
    pbnSelectionColour->setColor(color);
  }
}

void QgsOptions::on_pbnCanvasColor_clicked()
{
  QColor color = QColorDialog::getColor(pbnCanvasColor->color(), this);
  if (color.isValid())
  {
    pbnCanvasColor->setColor(color);
  }
}

void QgsOptions::on_pbnMeasureColour_clicked()
{
  QColor color = QColorDialog::getColor(pbnMeasureColour->color(), this);
  if (color.isValid())
  {
    pbnMeasureColour->setColor(color);
  }
}

void QgsOptions::on_mLineColourToolButton_clicked()
{
  QColor color = QColorDialog::getColor(mLineColourToolButton->color(), this);
  if (color.isValid())
  {
    mLineColourToolButton->setColor(color);
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
  settings.writeEntry("/Map/identifyRadius", spinBoxIdentifyValue->value());
  settings.writeEntry("/qgis/hideSplash",cbxHideSplash->isChecked());
  settings.writeEntry("/qgis/new_layers_visible",chkAddedVisibility->isChecked());
  settings.writeEntry("/qgis/enable_anti_aliasing",chkAntiAliasing->isChecked());
  settings.writeEntry("/qgis/use_qimage_to_render", !(chkUseQPixmap->isChecked()));
  settings.setValue("qgis/capitaliseLayerName", capitaliseCheckBox->isChecked());
  settings.setValue("qgis/askToSaveProjectChanges", chbAskToSaveProjectChanges->isChecked());
  settings.setValue("qgis/warnOldProjectVersion", chbWarnOldProjectVersion->isChecked());

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
  QColor myColor = pbnSelectionColour->color();
  int myRed = settings.writeEntry("/qgis/default_selection_color_red",myColor.red());
  int myGreen = settings.writeEntry("/qgis/default_selection_color_green",myColor.green());
  int myBlue = settings.writeEntry("/qgis/default_selection_color_blue",myColor.blue());

  //set the default color for canvas background
  myColor = pbnCanvasColor->color();
  myRed = settings.writeEntry("/qgis/default_canvas_color_red",myColor.red());
  myGreen = settings.writeEntry("/qgis/default_canvas_color_green",myColor.green());
  myBlue = settings.writeEntry("/qgis/default_canvas_color_blue",myColor.blue());

  //set the default color for the measure tool
  myColor = pbnMeasureColour->color();
  settings.setValue("/qgis/default_measure_color_red",myColor.red());
  settings.setValue("/qgis/default_measure_color_green",myColor.green());
  settings.setValue("/qgis/default_measure_color_blue",myColor.blue());

  settings.writeEntry("/qgis/wheel_action", cmbWheelAction->currentIndex());
  settings.writeEntry("/qgis/zoom_factor", spinZoomFactor->value());

  settings.setValue("/qgis/splitterRedraw", cbxSplitterRedraw->isChecked());

  //digitizing
  settings.setValue("/qgis/digitizing/line_width", mLineWidthSpinBox->value());
  QColor digitizingColor = mLineColourToolButton->color();
  settings.setValue("/qgis/digitizing/line_color_red", digitizingColor.red());
  settings.setValue("/qgis/digitizing/line_color_green", digitizingColor.green());
  settings.setValue("/qgis/digitizing/line_color_blue", digitizingColor.blue());

  //default snap mode
  QString defaultSnapModeString;
  if(mDefaultSnapModeComboBox->currentText() == tr("to vertex"))
    {
      defaultSnapModeString = "to vertex";
    }
  else if(mDefaultSnapModeComboBox->currentText() == tr("to segment"))
    {
      defaultSnapModeString = "to segment";
    }
  else if(mDefaultSnapModeComboBox->currentText() == tr("to vertex and segment"))
    {
      defaultSnapModeString = "to vertex and segment";
    }
  settings.setValue("/qgis/digitizing/default_snap_mode", defaultSnapModeString);
  settings.setValue("/qgis/digitizing/default_snapping_tolerance", mDefaultSnappingToleranceSpinBox->value());
  settings.setValue("/qgis/digitizing/search_radius_vertex_edit", mSearchRadiusVertexEditSpinBox->value());

  QString markerComboText = mMarkerStyleComboBox->currentText();
  if(markerComboText == tr("Semi transparent circle"))
    {
      settings.setValue("/qgis/digitizing/marker_style", "SemiTransparentCircle");
    }
  else if(markerComboText == tr("Cross"))
    {
      settings.setValue("/qgis/digitizing/marker_style", "Cross");
    }

  //
  // Locale settings 
  //
  settings.setValue("locale/userLocale", cboLocale->currentText());
  settings.setValue("locale/overrideFlag", grpLocale->isChecked());
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

void QgsOptions::on_chkAntiAliasing_stateChanged()
{
  // We can't have the anti-aliasing turned on when QPixmap is being
  // used (we we can. but it then doesn't do anti-aliasing, and this
  // will confuse people).
  if (chkAntiAliasing->isChecked())
    chkUseQPixmap->setChecked(false);

}

void QgsOptions::on_chkUseQPixmap_stateChanged()
{
  // We can't have the anti-aliasing turned on when QPixmap is being
  // used (we we can. but it then doesn't do anti-aliasing, and this
  // will confuse people).
  if (chkUseQPixmap->isChecked())
    chkAntiAliasing->setChecked(false);

}

// Return state of the visibility flag for newly added layers. If

bool QgsOptions::newVisible()
{
  return chkAddedVisibility->isChecked();
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


  cmbEllipsoid->insertItem(ELLIPS_FLAT_DESC);
  //check the db is available
  myResult = sqlite3_open(QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase);
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
  QString       myName(ELLIPS_FLAT);
  //check the db is available
  myResult = sqlite3_open(QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase);
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
    if (sqlite3_step(myPreparedStatement) == SQLITE_ROW)
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
  QString       myName(ELLIPS_FLAT_DESC);
  //check the db is available
  myResult = sqlite3_open(QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase);
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
    if (sqlite3_step(myPreparedStatement) == SQLITE_ROW)
      myName = QString((char *)sqlite3_column_text(myPreparedStatement,0));
  }
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return myName;

}

QStringList QgsOptions::i18nList()
{
  QStringList myList;
  myList << "en_US"; //there is no qm file for this so we add it manually
  QString myI18nPath = QgsApplication::i18nPath();
  QDir myDir(myI18nPath,"*.qm");
  QStringList myFileList = myDir.entryList();
  QStringListIterator myIterator(myFileList);
  while (myIterator.hasNext()) 
  {
    QString myFileName = myIterator.next();
    myList << myFileName.replace("qgis_","").replace(".qm","");
  }
  return myList;
}
