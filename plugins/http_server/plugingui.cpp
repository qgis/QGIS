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
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qspinbox.h> 
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qsettings.h>
//standard includes

PluginGui::PluginGui() : PluginGuiBase()
{

}

    PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{
  loadDefaults();
}  

PluginGui::~PluginGui()
{
  loadDefaults();
}

void PluginGui::pbnOK_clicked()
{
  saveDefaults();
  bool myFlag = cboxEnableServer->isChecked();
  int myPortInt = spinPort->value();
  emit enabledChanged(myFlag);
  emit portChanged(myPortInt);
  
  //disconnect any connections to this dialog
  disconnect( this, 0, 0, 0 );
  //close the dialog
  done(1);
} 
void PluginGui::pbnApply_clicked()
{
  saveDefaults();
  bool myFlag = cboxEnableServer->isChecked();
  int myPortInt = spinPort->value();
  emit enabledChanged(myFlag);
  emit portChanged(myPortInt);
}
void PluginGui::pbnCancel_clicked()
{
  close(1);
}

void PluginGui::newConnect(QString theMessage)
{
  teLogs->append( theMessage  );
}
void PluginGui::endConnect(QString theMessage)
{
  teLogs->append( theMessage);
}
void PluginGui::wroteToClient(QString theMessage)
{
  teLogs->append( theMessage );
}
void PluginGui::requestReceived(QString theString)
{
  teDebug->append(theString);
}

void PluginGui::cboxEnableServer_toggled( bool )
{

}


void PluginGui::spinPort_valueChanged( int )
{

}
void PluginGui::setPort(int thePortInt)
{
  spinPort->setValue(thePortInt);
}
void PluginGui::setEnabled(bool theEnabledFlag)
{
  cboxEnableServer->setChecked(theEnabledFlag);
}


void PluginGui::pbnProjectsDir_clicked()
{
  QFileDialog* myFileDialog = new QFileDialog( this, "Select Projects Dir", TRUE );
  myFileDialog->setMode( QFileDialog::DirectoryOnly );
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    leProjectsDir->setText(myFileDialog->selectedFile());
  }
}

void PluginGui::pbnDefaultProject_clicked()
{
  QFileDialog* myFileDialog = new QFileDialog( this, "Select Default Project File", TRUE );
  myFileDialog->setMode( QFileDialog::ExistingFile);
  myFileDialog->setFilter( "QGIS Project Files (*.qgs)" );
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    leDefaultProject->setText(myFileDialog->selectedFile());
  }
}

void PluginGui::pbnLayersDir_clicked()
{
  QFileDialog* myFileDialog = new QFileDialog( this, "Select Layers Dir", TRUE );
  myFileDialog->setMode( QFileDialog::DirectoryOnly );
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    leLayersDir->setText(myFileDialog->selectedFile());
  }
}


void PluginGui::pbnCssFile_clicked()
{
  QFileDialog* myFileDialog = new QFileDialog( this, "Select Css File", TRUE );
  myFileDialog->setMode( QFileDialog::ExistingFile);
  myFileDialog->setFilter( "Cascading style sheets (*.css)" );
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    leCssFile->setText(myFileDialog->selectedFile());
  }
}


void PluginGui::pbnLogFile_clicked()
{
  QFileDialog* myFileDialog = new QFileDialog( this, "Select Log File", TRUE );
  myFileDialog->setMode( QFileDialog::AnyFile );
  myFileDialog->setFilter( "Log file (*.log)" );
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    leLogFile->setText(myFileDialog->selectedFile());
  }

}


void PluginGui::saveDefaults()
{
  QSettings myQSettings;  
  myQSettings.writeEntry("/qgis/http_server/serverName",leServerName->text());
  myQSettings.writeEntry("/qgis/http_server/projectsDir",leProjectsDir->text());
  myQSettings.writeEntry("/qgis/http_server/defaultProject",leDefaultProject->text());
  myQSettings.writeEntry("/qgis/http_server/layersDir",leLayersDir->text());
  myQSettings.writeEntry("/qgis/http_server/cssFile",leCssFile->text());
  myQSettings.writeEntry("/qgis/http_server/logFile",leLogFile->text());
  myQSettings.writeEntry("/qgis/http_server/alwaysStartFlag",cboxAlwaysStart->isChecked());
}

void PluginGui::loadDefaults()
{
  QSettings myQSettings;  
  QString myServerNameString = myQSettings.readEntry("/qgis/http_server/serverName");
  leServerName->setText(myServerNameString);
  
  QString myProjectsDirString = myQSettings.readEntry("/qgis/http_server/projectsDir");
  leProjectsDir->setText(myProjectsDirString);
  
  QString myDefaultProjectString = myQSettings.readEntry("/qgis/http_server/defaultProject");
  leDefaultProject->setText(myDefaultProjectString);
  
  QString myLayersDirsString = myQSettings.readEntry("/qgis/http_server/layersDir");
  leLayersDir->setText(myLayersDirsString);
  
  QString myCssFileString = myQSettings.readEntry("/qgis/http_server/cssFile");
  leCssFile->setText(myCssFileString);
  
  QString myLogFileString = myQSettings.readEntry("/qgis/http_server/logFile");
  leLogFile->setText(myLogFileString);
  
  QString myAlwaysStartFlag = myQSettings.readEntry("/qgis/http_server/alwaysStartFlag");
  if (myAlwaysStartFlag=="true")
  {
    cboxAlwaysStart->setChecked(true);
  }
  else
  {
    cboxAlwaysStart->setChecked(false);
  }
}
