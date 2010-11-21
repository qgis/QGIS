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
#include "globe_plugin.h"

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

#include <osg/DisplaySettings>

//constructor
QgsGlobePluginDialog::QgsGlobePluginDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  getStereoConfig(); //default values from OSG
  loadStereoConfig();
  setStereoConfig(); //overwrite with values from QSettings
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
  setStereoConfig();
  saveStereoConfig();
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
  //showMessageBox("index_changed " + stereoMode);
}

void QgsGlobePluginDialog::showMessageBox( QString text )
{
  QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.exec();
}

void QgsGlobePluginDialog::getStereoConfig()
{
  //stereoMode ignored

  screenDistance->setValue( osg::DisplaySettings::instance()->getScreenDistance() );
}

void QgsGlobePluginDialog::setStereoConfig()
{
  //http://www.openscenegraph.org/projects/osg/wiki/Support/UserGuides/StereoConfig
  //http://www.openscenegraph.org/documentation/OpenSceneGraphReferenceDocs/a00181.html

  QString stereoMode = comboStereoMode->currentText();
  if("OFF" == stereoMode)
  {
    osg::DisplaySettings::instance()->setStereo( false );
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
    else if("HORIZONTAL_SPLIT" == stereoMode)
    {
      osg::DisplaySettings::instance()->setStereoMode( osg::DisplaySettings::HORIZONTAL_SPLIT );
    }
    else if("QUAD_BUFFER" == stereoMode)
    {
      osg::DisplaySettings::instance()->setStereoMode( osg::DisplaySettings::QUAD_BUFFER );
    }
    else
    {
      //should never get here
      QMessageBox msgBox;
      msgBox.setText("This stereo mode has not been implemented yet. Defaulting to ANAGLYPHIC");
      msgBox.exec();
    }
  }

  osg::DisplaySettings::instance()->setScreenDistance( screenDistance->value() );
}

void QgsGlobePluginDialog::loadStereoConfig()
{
  if ( settings.contains( "/Plugin-Globe/stereoMode" ) )
  {
    comboStereoMode->setCurrentIndex( comboStereoMode->findText( settings.value( "/Plugin-Globe/stereoMode" ).toString() ) );
  }
  if ( settings.contains( "/Plugin-Globe/screenDistance" ) )
  {
    screenDistance->setValue( settings.value( "/Plugin-Globe/screenDistance" ).toDouble() );
  }
}

void QgsGlobePluginDialog::saveStereoConfig()
{
  settings.setValue( "/Plugin-Globe/stereoMode", comboStereoMode->currentText() );
  settings.setValue( "/Plugin-Globe/screenDistance", screenDistance->value() );
}
