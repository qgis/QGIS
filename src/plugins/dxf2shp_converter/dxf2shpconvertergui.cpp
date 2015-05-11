/***************************************************************************
 *Copyright (C) 2008 Paolo L. Scala, Barbara Rita Barricelli, Marco Padula *
 * CNR, Milan Unit (Information Technology),                               *
 * Construction Technologies Institute.\n";                                *
 *                                                                         *
 * email : Paolo L. Scala <scala@itc.cnr.it>                               *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "dxf2shpconvertergui.h"
#include "qgscontexthelp.h"

#include "builder.h"
#include "dxflib/src/dl_dxf.h"

//qt includes
#include <qmessagebox.h>
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QDir>

#include "qgslogger.h"

dxf2shpConverterGui::dxf2shpConverterGui( QWidget *parent, Qt::WindowFlags fl ):
    QDialog( parent, fl )
{
  setupUi( this );
  restoreState();
}

dxf2shpConverterGui::~dxf2shpConverterGui()
{
  QSettings settings;
  settings.setValue( "/Plugin-DXF/geometry", saveGeometry() );
}

void dxf2shpConverterGui::on_buttonBox_accepted()
{
  QString inf = name->text();
  QString outd = dirout->text();

  if ( inf.isEmpty() )
  {
    QMessageBox::information( this, tr( "Warning" ), tr( "Please specify a file to convert." ) );
    return;
  }

  if ( outd.isEmpty() )
  {
    QMessageBox::information( this, tr( "Warning" ), tr( "Please specify an output file" ) );
    return;
  }

  QApplication::setOverrideCursor( Qt::BusyCursor );

  int type = SHPT_POINT;

  if ( polyline->isChecked() )
    type = SHPT_ARC;

  if ( polygon->isChecked() )
    type = SHPT_POLYGON;

  if ( point->isChecked() )
    type = SHPT_POINT;

  Builder *parser = new Builder( outd, type, convertTextCheck->isChecked(), convertInsertCheck->isChecked() );

  DL_Dxf *dxf_Main = new DL_Dxf();

  if ( !dxf_Main->in( inf.toStdString(), parser ) )
  {
    // if file open failed
    delete dxf_Main;
    QgsDebugMsg( "Aborting: The input file could not be opened." );
    QApplication::restoreOverrideCursor();
    return;
  }

  delete dxf_Main;

  parser->print_shpObjects();

  emit createLayer( parser->outputShp(), "Data layer" );

  if ( convertTextCheck->isChecked() && parser->textObjectsSize() > 0 )
  {
    emit createLayer( parser->outputTShp(), "Text layer" );
  }

  if ( convertInsertCheck->isChecked() && parser->insertObjectsSize() > 0 )
  {
    emit createLayer( parser->outputIShp(), "Insert layer" );
  }

  delete parser;

  QApplication::restoreOverrideCursor();

  accept();
}

void dxf2shpConverterGui::on_buttonBox_rejected()
{
  reject();
}

void dxf2shpConverterGui::on_buttonBox_helpRequested()
{
  QString s = tr( "Fields description:\n"
                  "* Input DXF file: path to the DXF file to be converted\n"
                  "* Output Shp file: desired name of the shape file to be created\n"
                  "* Shp output file type: specifies the type of the output shape file\n"
                  "* Export text labels checkbox: if checked, an additional shp points layer will be created, "
                  "and the associated dbf table will contain information about the \"TEXT\" fields found"
                  " in the dxf file, and the text strings themselves\n\n"
                  "---\n"
                  "Developed by Paolo L. Scala, Barbara Rita Barricelli, Marco Padula\n"
                  "CNR, Milan Unit (Information Technology), Construction Technologies Institute.\n"
                  "For support send a mail to scala@itc.cnr.it\n" );

  QMessageBox::information( this, "Help", s );
}

void dxf2shpConverterGui::on_btnBrowseForFile_clicked()
{
  getInputFileName();
}

void dxf2shpConverterGui::on_btnBrowseOutputDir_clicked()
{
  getOutputDir();
}

void dxf2shpConverterGui::getInputFileName()
{
  QSettings settings;
  QString s = QFileDialog::getOpenFileName( this,
              tr( "Choose a DXF file to open" ),
              settings.value( "/Plugin-DXF/text_path", "./" ).toString(),
              tr( "DXF files" ) + " (*.dxf)" );

  if ( !s.isEmpty() )
  {
    name->setText( s );
    settings.setValue( "/Plugin-DXF/text_path", QFileInfo( s ).absolutePath() );
  }
}

void dxf2shpConverterGui::getOutputDir()
{
  QSettings settings;
  QString s = QFileDialog::getSaveFileName( this,
              tr( "Choose a file name to save to" ),
              settings.value( "/UI/lastShapefileDir", "./" ).toString(),
              tr( "Shapefile" ) + " (*.shp)" );

  if ( !s.isEmpty() )
  {
    if ( !s.toLower().endsWith( ".shp" ) )
    {
      s += ".shp";
    }
    dirout->setText( s );
    settings.setValue( "/UI/lastShapefileDir", QFileInfo( s ).absolutePath() );
  }
}

void dxf2shpConverterGui::restoreState()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Plugin-DXF/geometry" ).toByteArray() );
}
