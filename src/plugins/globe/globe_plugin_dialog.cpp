/*
 * $Id$ 
 */
/***************************************************************************
    globe_plugin_dialog.cpp - settings dialog for the globe plugin
     --------------------------------------
    Date                 : 11-Nov-2010
    Copyright            : (C) 2010 by Marco Bernasocchi
    Email                : marco at bernawebdesign.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "globe_plugin_dialog.h"

#include <qgsapplication.h>
#include <qgslogger.h>
#include <qgscontexthelp.h>

#include <QtAlgorithms>
#include <QtDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <osgViewer/Viewer>

//constructor
QgsGlobePluginDialog::QgsGlobePluginDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  stereoMode = settings.value( "/Plugin-Globe/stereoMode", "OFF" ).toString();
  comboStereoMode->setCurrentIndex( comboStereoMode->findText( stereoMode ) );
}

//destructor
QgsGlobePluginDialog::~QgsGlobePluginDialog()
{
}

QString QgsGlobePluginDialog::openFile()
{ 
  QString path = QFileDialog::getOpenFileName( this,
                 tr( "Open earth file" ),
                 earthFile,
                 tr( "Earth files (*.earth)" ) );

  return path;
}

void QgsGlobePluginDialog::setStereoMode()
{
  if("OFF" == stereoMode)
  {
    osg::DisplaySettings::instance()->setStereo( false );
    }
  else if("ADVANCED" == stereoMode)
  {
    //osg::DisplaySettings::instance()->set
    }
  else
  {
    osg::DisplaySettings::instance()->setStereo( true );
    
    if("ANAGLYPHIC" == stereoMode)
    {
      osg::DisplaySettings::instance()->setStereoMode( osg::DisplaySettings::ANAGLYPHIC );
    }
    else if("VERTICAL_SPLIT" == stereoMode)
    {
      osg::DisplaySettings::instance()->setStereoMode( osg::DisplaySettings::VERTICAL_SPLIT );
    }
    else
    {
      showMessageBox("This stereo mode has not been implemented yet. Defaulting to ANAGLYPHIC");
    }
  }
}

void QgsGlobePluginDialog::restartGlobe()
{
  //showMessageBox("TODO: restart globe");
}

bool QgsGlobePluginDialog::globeRunning()
{
  //TODO: method that tells if the globe plugin is running
  return true;
}

void QgsGlobePluginDialog::on_buttonBox_accepted()
{
  /*
   * 
  // Validate input settings
  QString srcUrl( inputSrcDataset->text() );
  QString srcLayer( comboSrcLayer->currentText() );

  if ( srcUrl.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Input OGR dataset is missing!" ) );
    return;
  }

  if ( srcLayer.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Input OGR layer name is missing!" ) );
    return;
  }

  // Validate output settings
  QString dstFormat( comboDstFormats->currentText() );
  QString dstUrl( inputDstDataset->text() );
  QString dstLayer( inputDstLayer->text() );
  if ( dstLayer.isEmpty() )
    dstLayer = srcLayer;

  if ( dstFormat.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Target OGR format not selected!" ) );
    return;
  }

  if ( dstUrl.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Output OGR dataset is missing!" ) );
    return;
  }

  if ( dstLayer.isEmpty() )
  {
    QMessageBox::warning( this,
                          tr( "OGR Layer Converter" ),
                          tr( "Output OGR layer name is missing!" ) );
    return;
  }

  // TODO: SRS transformation support
  //QString srcSrs("EPSG:");
  //QString dstSrs("EPSG:");
  //srcSrs += inputSrcSrs->text();
  //dstSrs += inputDstSrs->text();

  // Execute layer translation
  bool success = false;

  // TODO: Use try-catch to display more meaningful error messages from Translator
  Translator worker( srcUrl, dstUrl, dstFormat );
  worker.setSourceLayer( srcLayer );
  worker.setTargetLayer( dstLayer );
  success = worker.translate();

  if ( success )
  {
    QMessageBox::information( this,
                              tr( "OGR Layer Converter" ),
                              tr( "Successfully translated layer '%1'" ).arg( srcLayer ) );
  }
  else
  {
    QMessageBox::information( this,
                              tr( "OGR Layer Converter" ),
                              tr( "Failed to translate layer '%1'" ).arg( srcLayer ) );
  }

  // Close dialog box
  */
  setStereoMode();
  
  if ( globeRunning() )
  {
    restartGlobe();
  } 
  accept();
}

void QgsGlobePluginDialog::on_buttonBox_rejected()
{
  reject();
}

void QgsGlobePluginDialog::on_comboStereoMode_currentIndexChanged( QString mode )
{
  stereoMode = mode;
  settings.setValue( "/Plugin-Globe/stereoMode", stereoMode );
}

void QgsGlobePluginDialog::showMessageBox( QString text )
{
  QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.exec();
}

