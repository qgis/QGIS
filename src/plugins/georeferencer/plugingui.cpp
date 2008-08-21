/***************************************************************************
 *   Copyright (C) 2005 by Lars Luthman
 *   larsl@users.sourceforge.net
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "plugingui.h"
#include "qgsgeorefdescriptiondialog.h"
#include "qgsleastsquares.h"
#include "qgspointdialog.h"
#include "qgsrasterlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"

//qt includes
#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

//standard includes

QgsGeorefPluginGui::QgsGeorefPluginGui() : QgsGeorefPluginGuiBase()
{
  
}

QgsGeorefPluginGui::QgsGeorefPluginGui(QgisInterface* theQgisInterface,
                                       QWidget* parent, Qt::WFlags fl)
  : QDialog(parent, fl), mIface(theQgisInterface), mPluginWindowsArranged(false)
{
  setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  mPointDialog = new QgsPointDialog(mIface, parent);
  //move point dialog to the left of the screen so that both windows are visible
  mPointDialog->move(0, mPointDialog->pos().y());
  mPointDialog->show();
}  


QgsGeorefPluginGui::~QgsGeorefPluginGui()
{
  delete mPointDialog;

  //restore size of qgis main window if arrange button was used
  if(mPluginWindowsArranged)
    {
      QWidget* mainWindow = QgsGeorefPluginGui::findMainWindow();
      if(mainWindow)
	{
	  mainWindow->resize(origSize);
	  mainWindow->move(origPos);
	}
    }
}


void QgsGeorefPluginGui::on_pbnClose_clicked()
{
  close();
}

void QgsGeorefPluginGui::on_pbnDescription_clicked()
{
  QgsGeorefDescriptionDialog d(0);
  d.exec();
}


void QgsGeorefPluginGui::on_pbnSelectRaster_clicked() {
  QSettings settings;
  QString dir = settings.value("/Plugin-GeoReferencer/rasterdirectory").toString();
  if (dir.isEmpty())
    dir = ".";
  QString filename = 
    QFileDialog::getOpenFileName(this,
				 tr("Choose a raster file"),
                 dir,
				 tr("Raster files (*.*)"));

  if(filename.isNull())
    {
      return;
    }
  leSelectRaster->setText(filename);

  // do we think that this is a valid raster?
  if (!QgsRasterLayer::isValidRasterFileName(leSelectRaster->text())) {
    QMessageBox::critical(this, tr("Error"), 
			  tr("The selected file is not a valid raster file."));
    return;
  }
  
  // remember the directory
  {
    QSettings settings;
    QFileInfo fileInfo(leSelectRaster->text());
    settings.setValue("/Plugin-GeoReferencer/rasterdirectory", 
			fileInfo.path());
  }
  
  // guess the world file name
  QString raster = leSelectRaster->text();
  int point = raster.lastIndexOf('.');
  QString worldfile;
  if (point != -1 && point != raster.length() - 1) {
    worldfile = raster.left(point + 1);
    worldfile += ("wld");
  }
  
  // check if there already is a world file
  if (!worldfile.isEmpty()) {
    if (QFile::exists(worldfile)) {
      int r = QMessageBox::question(this, tr("World file exists"),
                       tr("<p>The selected file already seems to have a ")+
                       tr("world file! Do you want to replace it with the ")+
		       tr("new world file?</p>"),
                       QMessageBox::Yes|QMessageBox::Default, 
                       QMessageBox::No|QMessageBox::Escape);
      if (r == QMessageBox::No)
        return;
      else
        QFile::remove(worldfile);
    }
  }
  
  // XXX This is horrible, but it works and I'm tired / ll
  {
    QSettings settings;
    QgsProject* prj = QgsProject::instance();
    mProjBehaviour = settings.value("/Projections/defaultBehaviour").toString();
    mProjectCRS = prj->readEntry("SpatialRefSys", "/ProjectCRSProj4String");
    mProjectCRSID = prj->readNumEntry("SpatialRefSys", "/ProjectCRSID");
    
    settings.setValue("/Projections/defaultBehaviour", "useProject");
    prj->writeEntry("SpatialRefSys", "/ProjectCRSProj4String", GEOPROJ4);
    prj->writeEntry("SpatialRefSys", "/ProjectCRSID", int(GEOCRS_ID));
    
    settings.setValue("/Projections/defaultBehaviour", mProjBehaviour);
    prj->writeEntry("SpatialRefSys", "/ProjectCRSProj4String", mProjectCRS);
    prj->writeEntry("SpatialRefSys", "/ProjectCRSID", mProjectCRSID);
  }

  mPointDialog->openImageFile(filename);
  mPointDialog->show();
}



void QgsGeorefPluginGui::on_mArrangeWindowsButton_clicked()
{
  if(mPointDialog && mIface)
    {
      QWidget* mainWindow = QgsGeorefPluginGui::findMainWindow();
      if(!mainWindow)
	{
	  return;
	}
      
      int myScreenWidth, myScreenHeight; //width and height of screen
      
      //store initial size and position of qgis window
      mPluginWindowsArranged = true;
      origSize = mainWindow->size();
      origPos = mainWindow->pos();
      
      //read the desktop geometry
      QDesktopWidget* desktop = QApplication::desktop();
      QRect screenGeometry = desktop->availableGeometry();
      myScreenWidth = screenGeometry.width();
      myScreenHeight = screenGeometry.height();
      
      int newPluginDialogHeight = qMax(int(myScreenHeight * 0.2), minimumHeight());
      int newPluginDialogWidth = qMax(int(myScreenWidth * 0.33), minimumWidth());
      int newPointDialogHeight = qMax(int(myScreenHeight * 0.60), mPointDialog->minimumHeight());
      int newPointDialogWidth = qMax(int(myScreenWidth * 0.33), mPointDialog->minimumWidth());
      int newMainWindowHeight = qMax(int(myScreenHeight * 0.90), mainWindow->minimumHeight());
      int newMainWindowWidth = qMax(int(myScreenWidth * 0.65), mainWindow->minimumHeight());
      
      //place main window
      mainWindow->setEnabled(false); //avoid getting two resize events for the main canvas
      mainWindow->resize(newMainWindowWidth, newMainWindowHeight);
      //Resize again to account for frame border width -- Probably a better way to do this.
      mainWindow->resize(newMainWindowWidth - (mainWindow->width() - newMainWindowWidth), newMainWindowHeight - (mainWindow->height() - newMainWindowHeight));
      mainWindow->move(myScreenWidth - newMainWindowWidth, int(myScreenHeight * 0.05));
      mainWindow->setEnabled(true);

      //place this dialog
      resize(newPluginDialogWidth, newPluginDialogHeight);
      resize(newPluginDialogWidth - (width() - newPluginDialogWidth), newPluginDialogHeight - (height() - newPluginDialogHeight));
      move(0, int(myScreenHeight * 0.05));

      //place point dialog
      mPointDialog->resize(newPointDialogWidth, newPointDialogHeight);
      mPointDialog->resize(newPointDialogWidth - (mPointDialog->width() - newPointDialogWidth), newPointDialogHeight - (mPointDialog->height() - newPointDialogHeight));
      mPointDialog->move(0, int(myScreenHeight * 0.35));
      
      
    }
}


QWidget* QgsGeorefPluginGui::findMainWindow()
{
  QWidget* result = 0;
  
  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  QWidgetList::iterator it = topLevelWidgets.begin();
  for(; it != topLevelWidgets.end(); ++it)
    {
      if((*it)->objectName() == "QgisApp")
	{
	  result = *it;
	  break;
	}
    }
  return result;
}
