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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <osg/DisplaySettings>

//constructor
QgsGlobePluginDialog::QgsGlobePluginDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  loadStereoConfig();  //values from settings, default values from OSG
  setStereoConfig(); //overwrite with values from QSettings
  updateStereoDialog(); //update the dialog gui
}

//destructor
QgsGlobePluginDialog::~QgsGlobePluginDialog()
{
}

QString QgsGlobePluginDialog::openFile()
{
  //see http://www.gdal.org/formats_list.html
  const char* filter = "GDAL files (*.dem *.tif *.tiff *.jpg *.jpeg *.asc);;DEM files (*.dem);;All files (*.*)";
  QString path = QFileDialog::getOpenFileName( this,
                  tr( "Open raster file" ),
                  QDir::homePath (),
                  tr( filter ) );

  return path;
}

bool QgsGlobePluginDialog::validateNetworkResource(QString uri)
{
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  QNetworkRequest request;
  request.setUrl(QUrl(uri));
  QNetworkReply *reply = manager->get(request);

  //wait for response syncronously
  QEventLoop eLoop;
  connect( manager, SIGNAL( finished( QNetworkReply * ) ),
           &eLoop, SLOT( quit() ) );
  eLoop.exec(QEventLoop::ExcludeUserInputEvents);

  if (QNetworkReply::HostNotFoundError != reply->error())
  //FIXME:should be the following line but reply->error() always give "unknown error"
  //if (QNetworkReply::NoError == reply->error())
  {
    QByteArray data = reply->readAll();
    QString req(data);
    return true;
  }
  else
  {
    showMessageBox( tr("Invalid URL: ") + reply->errorString() );
    return false;
  }
}

void QgsGlobePluginDialog::showMessageBox( QString text )
{
  QMessageBox msgBox;
  msgBox.setText(text);
  msgBox.exec();
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

  if ( globeRunning() )
  {
    restartGlobe();
  }
  accept();
}

void QgsGlobePluginDialog::on_buttonBox_rejected()
{
  loadStereoConfig();
  setStereoConfig();
  reject();
}

//ELEVATION
void QgsGlobePluginDialog::on_elevationCombo_currentIndexChanged(QString type)
{
  if("Raster" == type)
  {
    elevationActions->setCurrentIndex(0);
  }
  else
  {
    elevationActions->setCurrentIndex(1);
  }
}

void QgsGlobePluginDialog::on_elevationBrowse_clicked()
{
  elevationPath->setText( openFile() );
}

void QgsGlobePluginDialog::on_elevationTest_clicked()
{
  validateNetworkResource( elevationPath->text() );
}

void QgsGlobePluginDialog::on_resetStereoDefaults_clicked()
{
  //http://www.openscenegraph.org/projects/osg/wiki/Support/UserGuides/StereoSettings
  comboStereoMode->setCurrentIndex( comboStereoMode->findText("OFF") );
  screenDistance->setValue( 0.5 );
  screenHeight->setValue( 0.26 );
  screenWidth->setValue( 0.325 );
  eyeSeparation->setValue( 0.06);
  splitStereoHorizontalSeparation->setValue( 42 );
  splitStereoVerticalSeparation->setValue( 42 );
  splitStereoHorizontalEyeMapping->setCurrentIndex( 0 );
  splitStereoVerticalEyeMapping->setCurrentIndex( 0 );
}

void QgsGlobePluginDialog::on_comboStereoMode_currentIndexChanged(QString value)
{
  setStereoMode();
  updateStereoDialog();
}

void QgsGlobePluginDialog::on_eyeSeparation_valueChanged(double value)
{
  osg::DisplaySettings::instance()->setEyeSeparation( value );
}

void QgsGlobePluginDialog::on_screenDistance_valueChanged(double value)
{
  osg::DisplaySettings::instance()->setScreenDistance( value );
}

void QgsGlobePluginDialog::on_screenWidth_valueChanged(double value)
{
  osg::DisplaySettings::instance()->setScreenWidth( value );
}

void QgsGlobePluginDialog::on_screenHeight_valueChanged(double value)
{
  osg::DisplaySettings::instance()->setScreenHeight( value );
}

void QgsGlobePluginDialog::on_splitStereoHorizontalSeparation_valueChanged(int value)
{
  osg::DisplaySettings::instance()->setSplitStereoHorizontalSeparation( value );
}

void QgsGlobePluginDialog::on_splitStereoVerticalSeparation_valueChanged(int value)
{
  osg::DisplaySettings::instance()->setSplitStereoVerticalSeparation( value );
}

void QgsGlobePluginDialog::on_splitStereoHorizontalEyeMapping_currentIndexChanged(int value)
{
  osg::DisplaySettings::instance()->setSplitStereoHorizontalEyeMapping(
    (osg::DisplaySettings::SplitStereoHorizontalEyeMapping) value );
}

void QgsGlobePluginDialog::on_splitStereoVerticalEyeMapping_currentIndexChanged(int value)
{
  osg::DisplaySettings::instance()->setSplitStereoVerticalEyeMapping(
    (osg::DisplaySettings::SplitStereoVerticalEyeMapping) value );
}

void QgsGlobePluginDialog::loadStereoConfig()
{
  comboStereoMode->setCurrentIndex( comboStereoMode->findText( settings.value( "/Plugin-Globe/stereoMode",
    "OFF" ).toString() ) );
  screenDistance->setValue( settings.value( "/Plugin-Globe/screenDistance",
    osg::DisplaySettings::instance()->getScreenDistance() ).toDouble() );
  screenWidth->setValue( settings.value( "/Plugin-Globe/screenWidth",
    osg::DisplaySettings::instance()->getScreenWidth() ).toDouble() );
  screenHeight->setValue( settings.value( "/Plugin-Globe/screenHeight",
    osg::DisplaySettings::instance()->getScreenHeight() ).toDouble() );
  eyeSeparation->setValue( settings.value( "/Plugin-Globe/eyeSeparation",
    osg::DisplaySettings::instance()->getEyeSeparation() ).toDouble() );
  splitStereoHorizontalSeparation->setValue( settings.value( "/Plugin-Globe/splitStereoHorizontalSeparation",
    osg::DisplaySettings::instance()->getSplitStereoHorizontalSeparation() ).toInt() );
  splitStereoVerticalSeparation->setValue( settings.value( "/Plugin-Globe/splitStereoVerticalSeparation",
    osg::DisplaySettings::instance()->getSplitStereoVerticalSeparation() ).toInt() );
  splitStereoHorizontalEyeMapping->setCurrentIndex( settings.value( "/Plugin-Globe/splitStereoHorizontalEyeMapping",
    osg::DisplaySettings::instance()->getSplitStereoHorizontalEyeMapping() ).toInt() );
  splitStereoVerticalEyeMapping->setCurrentIndex( settings.value( "/Plugin-Globe/splitStereoVerticalEyeMapping",
    osg::DisplaySettings::instance()->getSplitStereoVerticalEyeMapping() ).toInt() );
}

void QgsGlobePluginDialog::setStereoMode()
{
    //http://www.openscenegraph.org/projects/osg/wiki/Support/UserGuides/StereoSettings
  //http://www.openscenegraph.org/documentation/OpenSceneGraphReferenceDocs/a00181.html

  QString stereoMode = comboStereoMode->currentText() ;
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
}

void QgsGlobePluginDialog::setStereoConfig()
{
  //SETTING THE VALUES IN THE OEGearth instance
  setStereoMode();
  osg::DisplaySettings::instance()->setScreenDistance( screenDistance->value() );
  osg::DisplaySettings::instance()->setScreenWidth( screenWidth->value() );
  osg::DisplaySettings::instance()->setScreenHeight( screenHeight->value() );
  osg::DisplaySettings::instance()->setEyeSeparation( eyeSeparation->value() );
  osg::DisplaySettings::instance()->setSplitStereoHorizontalSeparation( splitStereoHorizontalSeparation->value() );
  osg::DisplaySettings::instance()->setSplitStereoVerticalSeparation( splitStereoVerticalSeparation->value() );
  osg::DisplaySettings::instance()->setSplitStereoHorizontalEyeMapping(
    (osg::DisplaySettings::SplitStereoHorizontalEyeMapping) splitStereoHorizontalEyeMapping->currentIndex() );
  osg::DisplaySettings::instance()->setSplitStereoVerticalEyeMapping(
    (osg::DisplaySettings::SplitStereoVerticalEyeMapping) splitStereoVerticalEyeMapping->currentIndex() );

}

void QgsGlobePluginDialog::saveStereoConfig()
{
  settings.setValue( "/Plugin-Globe/stereoMode", comboStereoMode->currentText() );
  settings.setValue( "/Plugin-Globe/screenDistance", screenDistance->value() );
  settings.setValue( "/Plugin-Globe/screenWidth", screenWidth->value() );
  settings.setValue( "/Plugin-Globe/screenHeight", screenHeight->value() );
  settings.setValue( "/Plugin-Globe/eyeSeparation", eyeSeparation->value() );
  settings.setValue( "/Plugin-Globe/splitStereoHorizontalSeparation", splitStereoHorizontalSeparation->value() );
  settings.setValue( "/Plugin-Globe/splitStereoVerticalSeparation", splitStereoVerticalSeparation->value() );
  settings.setValue( "/Plugin-Globe/splitStereoHorizontalEyeMapping", splitStereoHorizontalEyeMapping->currentIndex() );
  settings.setValue( "/Plugin-Globe/splitStereoVerticalEyeMapping", splitStereoVerticalEyeMapping->currentIndex() );
}

void QgsGlobePluginDialog::updateStereoDialog()
{
  QString stereoMode = comboStereoMode->currentText() ;
  screenDistance->setEnabled( true );
  screenHeight->setEnabled( true );
  screenWidth->setEnabled( true );
  eyeSeparation->setEnabled( true );
  splitStereoHorizontalSeparation->setEnabled( false );
  splitStereoVerticalSeparation->setEnabled( false );
  splitStereoHorizontalEyeMapping->setEnabled( false );
  splitStereoVerticalEyeMapping->setEnabled( false );

  if("OFF" == stereoMode)
  {
    screenDistance->setEnabled( false );
    screenHeight->setEnabled( false );
    screenWidth->setEnabled( false );
    eyeSeparation->setEnabled( false );
  }
  else if("ANAGLYPHIC" == stereoMode)
  {
    //nothing to do
  }
  else if("VERTICAL_SPLIT" == stereoMode)
  {
    splitStereoVerticalSeparation->setEnabled( true );
    splitStereoVerticalEyeMapping->setEnabled( true );
  }
  else if("HORIZONTAL_SPLIT" == stereoMode)
  {
    splitStereoHorizontalSeparation->setEnabled( true );
    splitStereoHorizontalEyeMapping->setEnabled( true );
  }
  else if("QUAD_BUFFER" == stereoMode)
  {
    //nothing to do
  }
  else
  {
    //should never get here
    QMessageBox msgBox;
    msgBox.setText("This stereo mode has not been implemented yet.");
    msgBox.exec();
  }
}
