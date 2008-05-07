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
  QString dir = settings.readEntry("/Plugin-GeoReferencer/rasterdirectory");
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
    settings.writeEntry("/Plugin-GeoReferencer/rasterdirectory", 
			fileInfo.dirPath());
  }
  
  // guess the world file name
  QString raster = leSelectRaster->text();
  int point = raster.findRev('.');
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
    mProjBehaviour = settings.readEntry("/Projections/defaultBehaviour");
    mProjectSRS = prj->readEntry("SpatialRefSys", "/ProjectSRSProj4String");
    mProjectSRSID = prj->readNumEntry("SpatialRefSys", "/ProjectSRSID");
    
    settings.writeEntry("/Projections/defaultBehaviour", "useProject");
    prj->writeEntry("SpatialRefSys", "/ProjectSRSProj4String", GEOPROJ4);
    prj->writeEntry("SpatialRefSys", "/ProjectSRSID", int(GEOSRS_ID));
    
    settings.writeEntry("/Projections/defaultBehaviour", mProjBehaviour);
    prj->writeEntry("SpatialRefSys", "/ProjectSRSProj4String", mProjectSRS);
    prj->writeEntry("SpatialRefSys", "/ProjectSRSID", mProjectSRSID);
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
      
      int width, height; //width and height of screen
      
      //store initial size and position of qgis window
      mPluginWindowsArranged = true;
      origSize = mainWindow->size();
      origPos = mainWindow->pos();
      
      
      //qt distinguishes between geometry with and without window frame
      int widthIncrMainWindow, heightIncrMainWindow;
      int widthIncrPointDialog, heightIncrPointDialog;
      int widthIncrThis, heightIncrThis;
      
      //read the desktop geometry
      QDesktopWidget* desktop = QApplication::desktop();
      QRect screenGeometry = desktop->availableGeometry();
      width = screenGeometry.width();
      height = screenGeometry.height();
      
      int leftRightBorder; //border between plugin/point dialogs on the left and qgis main window on the right
      int pluginPointDialogBorder; //border on y-axis between plugin dialog and point dialog
      
      
      leftRightBorder = width/3;
      pluginPointDialogBorder = height/5;
      
      //consider minimum heights of plugin dialog and mPointDialog
      int minPluginDialogHeight = minimumHeight() + (frameSize().height() - this->height()); 
      int minPointDialogHeight = mPointDialog->minimumHeight() +	\
	(mPointDialog->frameSize().height() - mPointDialog->height());
      
      if((height - pluginPointDialogBorder) < minPointDialogHeight)
	{
	  pluginPointDialogBorder = (height - minPointDialogHeight);
	} 
      if(pluginPointDialogBorder < minPluginDialogHeight)
	{
	  pluginPointDialogBorder = minPluginDialogHeight;
	}
      
      //consider minimum widths of plugin/point dialogs and qgis main window
      int minPluginDialogWidth = minimumWidth() + (frameSize().width() - this->width());
      int minPointDialogWidth = mPointDialog->minimumWidth() +		\
	(mPointDialog->frameSize().width() - mPointDialog->width());
      int minMainWindowWidth = mainWindow->minimumWidth() +		\
	(mainWindow->frameSize().width() - mainWindow->width());
      
      if(leftRightBorder < minPointDialogWidth)
	{
	  leftRightBorder = minPointDialogWidth;
	}
      if(leftRightBorder < minPluginDialogWidth)
	{
	  leftRightBorder = minPluginDialogWidth;
	}
      if((width - leftRightBorder) < minMainWindowWidth)
	{
	  leftRightBorder = width - minMainWindowWidth;
	}
      
      //place main window
      widthIncrMainWindow = (width -leftRightBorder) - mainWindow->frameSize().width();
      heightIncrMainWindow = height - mainWindow->frameSize().height();
      mainWindow->setEnabled(false); //avoid getting two resize events for the main canvas
      mainWindow->resize(mainWindow->width() + widthIncrMainWindow, mainWindow->height() + heightIncrMainWindow);
      mainWindow->move(leftRightBorder, 0);
      mainWindow->setEnabled(true);
      
      //place point dialog
      widthIncrPointDialog = leftRightBorder - mPointDialog->frameSize().width();
      heightIncrPointDialog = height - pluginPointDialogBorder - mPointDialog->frameSize().height();
      mPointDialog->resize(mPointDialog->width() + widthIncrPointDialog, mPointDialog->height() + heightIncrPointDialog);
      mPointDialog->move(0, pluginPointDialogBorder);
      
      //place this dialog
      widthIncrThis = leftRightBorder - frameSize().width();
      heightIncrThis = pluginPointDialogBorder - frameSize().height();
      resize(this->width() + widthIncrThis, this->height() + heightIncrThis);
      move(0, 0);
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
