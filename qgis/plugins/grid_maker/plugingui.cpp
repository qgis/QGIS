/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "plugingui.h"

//qt includes
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfile.h>
#include "graticulecreator.h"
//standard includes
#include <iostream>

QgsGridMakerPluginGui::QgsGridMakerPluginGui() : QgsGridMakerPluginGuiBase()
{
}
QgsGridMakerPluginGui::QgsGridMakerPluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
    : QgsGridMakerPluginGuiBase( parent, name, modal, fl )
{
}
QgsGridMakerPluginGui::~QgsGridMakerPluginGui()
{}

void QgsGridMakerPluginGui::pbnOK_clicked()
{
  //check input file exists
  //
  std::cout << "GrativuleCreator called with: " <<
  leOutputShapeFile->text().local8Bit() <<
  leLongitudeInterval->text().local8Bit() <<
  leLatitudeInterval->text().local8Bit() <<
  leOriginLongitude->text().local8Bit() <<
  leOriginLatitude->text().local8Bit() <<
  leEndPointLongitude->text().local8Bit() <<
  leEndPointLatitude->text().local8Bit()
  << std::endl;

  if (leOutputShapeFile->text().isEmpty())
  {
    QMessageBox::warning( 0, "QGIS - Grid Maker",
        QString("Please enter the file name before pressing OK!." ));
    return;
  }

  bool myFlag=false; //presumed guilty

  double myLongitudeInterval =  leLongitudeInterval->text().toDouble(&myFlag);
  if (!myFlag)
  {
    QMessageBox::warning( 0, "QGIS - Grid Maker",
        QString("Longitude Interval is invalid - please correct and try again." ));
    return;
  }
  myFlag=false;//reset test flag
  double myLatitudeInterval =  leLatitudeInterval->text().toDouble(&myFlag);
  if (!myFlag)
  {
    QMessageBox::warning( 0, "QGIS - Grid Maker",
        QString("Latitude Interval is invalid - please correct and try again." ));
    return;
  }
  myFlag=false;//reset test flag
  double myLongitudeOrigin =  leOriginLongitude->text().toDouble(&myFlag);
  if (!myFlag)
  {
    QMessageBox::warning( 0, "QGIS - Grid Maker",
        QString("Longitude Origin is invalid - please correct and try again.." ));
    return;
  }
  myFlag=false;//reset test flag
  double myLatitudeOrigin =  leOriginLatitude->text().toDouble(&myFlag);
  if (!myFlag)
  {
    QMessageBox::warning( 0, "QGIS - Grid Maker",
        QString("Latitude Origin is invalid - please correct and try again." ));
    return;
  }
  myFlag=false;//reset test flag
  double myEndPointLongitude = leEndPointLongitude->text().toDouble(&myFlag);
  if (!myFlag)
  {
    QMessageBox::warning( 0, "QGIS - Grid Maker",
        QString("End Point Longitude is invalid - please correct and try again." ));
    return;
  }
  myFlag=false;//reset test flag
  double myEndPointLatitude = leEndPointLatitude->text().toDouble(&myFlag);
  if (!myFlag)
  {
    QMessageBox::warning( 0, "QGIS - Grid Maker",
        QString("End Point Latitude is invalid - please correct and try again." ));
    return;
  }
  

  GraticuleCreator *  myGraticuleCreator = new GraticuleCreator(
        leOutputShapeFile->text(),
        myLongitudeInterval,
        myLatitudeInterval,
        myLongitudeOrigin,
        myLatitudeOrigin,
        myEndPointLongitude,
        myEndPointLatitude
      );
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  //emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //

  delete myGraticuleCreator;
  emit drawVectorLayer(leOutputShapeFile->text(),QString("Graticule"),QString("ogr"));
  //close the dialog
  done(1);
}


void QgsGridMakerPluginGui::pbnSelectOutputFile_clicked()
{
  std::cout << " Gps File Importer Gui::pbnSelectOutputFile_clicked() " << std::endl;
  QString myOutputFileNameQString = QFileDialog::getSaveFileName(
          ".",
          "ESRI Shapefile (*.shp)",
          this,
          "save file dialog"
          "Choose a filename to save under" );

  if (myOutputFileNameQString.right(4) != ".shp")
    myOutputFileNameQString += ".shp";

  leOutputShapeFile->setText(myOutputFileNameQString);
  if ( leOutputShapeFile->text()==""  )
  {
    pbnOK->setEnabled(false);
  }
  else
  {
    pbnOK->setEnabled(true);
  }
}


void QgsGridMakerPluginGui::pbnCancel_clicked()
{
  close(1);
}

