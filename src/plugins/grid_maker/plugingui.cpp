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
#include <QFileDialog>
#include <QMessageBox>

#include "graticulecreator.h"
#include "qgscontexthelp.h"
#include "qgslogger.h"

//standard includes
#include <iostream>

QgsGridMakerPluginGui::QgsGridMakerPluginGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  pbnOK = buttonBox->button( QDialogButtonBox::Ok );
  pbnOK->setEnabled( false );
}

QgsGridMakerPluginGui::~QgsGridMakerPluginGui()
{}

void QgsGridMakerPluginGui::on_buttonBox_accepted()
{
  //check input file exists
  //
  QgsLogger::debug( "GrativuleCreator called with: " +
                    leOutputShapeFile->text() + " " +
                    leXInterval->text() + " " +
                    leYInterval->text() + " " +
                    leXLowerLeft->text() + " " +
                    leYLowerLeft->text() + " " +
                    leXUpperRight->text() + " " +
                    leYUpperRight->text() );

  if ( leOutputShapeFile->text().isEmpty() )
  {
    QMessageBox::warning( 0, tr( "QGIS - Grid Maker" ),
                          tr( "Please enter the file name before pressing OK!" ) );
    return;
  }


  double myXInterval =  leXInterval->text().toDouble();
  double myYInterval =  leYInterval->text().toDouble();

  if ( myXInterval == 0.0 || myYInterval == 0.0 )
  {
    QMessageBox::warning( 0, tr( "QGIS - Grid Maker" ),
                          tr( "Please enter intervals before pressing OK!" ) );
    return;
  }

  double myXOrigin =  leXLowerLeft->text().toDouble();
  double myYOrigin =  leYLowerLeft->text().toDouble();
  double myEndPointX = leXUpperRight->text().toDouble();
  double myEndPointY = leYUpperRight->text().toDouble();


  if ( radPoint->isChecked() )
  {
    GraticuleCreator  myGraticuleCreator( leOutputShapeFile->text() );
    myGraticuleCreator.generatePointGraticule(
      myXInterval,
      myYInterval,
      myXOrigin,
      myYOrigin,
      myEndPointX,
      myEndPointY
    );
  }
  else
  {
    GraticuleCreator  myGraticuleCreator( leOutputShapeFile->text() );
    myGraticuleCreator.generatePolygonGraticule(
      myXInterval,
      myYInterval,
      myXOrigin,
      myYOrigin,
      myEndPointX,
      myEndPointY
    );
  }
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to
  // add it to the view using:
  // emit drawRasterLayer(QString("layer name"));
  // or for a vector layer
  //emit drawVectorLayer(QString("path name"),QString("layer name"),QString("provider name (either ogr or postgres"));
  //

  emit drawVectorLayer( leOutputShapeFile->text(), QString( "Graticule" ), QString( "ogr" ) );
  //close the dialog
  accept();
}


void QgsGridMakerPluginGui::on_pbnSelectOutputFile_clicked()
{
  QgsLogger::debug( " Gps File Importer Gui::pbnSelectOutputFile_clicked()" );
  QString myOutputFileNameQString = QFileDialog::getSaveFileName(
                                      this,
                                      tr( "Choose a file name to save under" ),
                                      ".",
                                      tr( "ESRI Shapefile (*.shp)" ) );

  if ( myOutputFileNameQString.right( 4 ) != ".shp" )
    myOutputFileNameQString += ".shp";

  leOutputShapeFile->setText( myOutputFileNameQString );
  if ( leOutputShapeFile->text() == "" )
  {
    pbnOK->setEnabled( false );
  }
  else
  {
    pbnOK->setEnabled( true );
  }
}


void QgsGridMakerPluginGui::on_buttonBox_rejected()
{
  reject();
}

void QgsGridMakerPluginGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}
