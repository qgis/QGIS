/***************************************************************************
*    qgsglobeplugindialog.cpp - settings dialog for the globe plugin
*     --------------------------------------
*    Date                 : 11-Nov-2010
*    Copyright            : (C) 2010 by Marco Bernasocchi
*    Email                : marco at bernawebdesign.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsglobeplugindialog.h"
#include "globe_plugin.h"

#include <qgsapplication.h>
#include <qgsnetworkaccessmanager.h>
#include <qgsproject.h>
#include <qgslogger.h>

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMenu>
#include <QSettings>
#include <QTimer>

#include <osg/DisplaySettings>
#include <osgViewer/Viewer>
#include <osgEarth/Version>
#include <osgEarthUtil/EarthManipulator>

QgsGlobePluginDialog::QgsGlobePluginDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  QMenu* addImageryMenu = new QMenu( this );

  QMenu* tmsImageryMenu = new QMenu( this );
  tmsImageryMenu->addAction( "Readymap: NASA BlueMarble Imagery", this, SLOT( addTMSImagery() ) )->setData( "http://readymap.org/readymap/tiles/1.0.0/1/" );
  tmsImageryMenu->addAction( "Readymap: NASA BlueMarble, ocean only", this, SLOT( addTMSImagery() ) )->setData( "http://readymap.org/readymap/tiles/1.0.0/2/" );
  tmsImageryMenu->addAction( "Readymap: High resolution insets from various world locations", this, SLOT( addTMSImagery() ) )->setData( "http://readymap.org/readymap/tiles/1.0.0/3/" );
  tmsImageryMenu->addAction( "Readymap: Global Land Cover Facility 15m Landsat", this, SLOT( addTMSImagery() ) )->setData( "http://readymap.org/readymap/tiles/1.0.0/6/" );
  tmsImageryMenu->addAction( "Readymap: NASA BlueMarble + Landsat + Ocean Masking Layer", this, SLOT( addTMSImagery() ) )->setData( "http://readymap.org/readymap/tiles/1.0.0/7/" );
  tmsImageryMenu->addAction( tr( "Custom..." ), this, SLOT( addCustomTMSImagery() ) );
  addImageryMenu->addAction( tr( "TMS" ) )->setMenu( tmsImageryMenu );

  QMenu* wmsImageryMenu = new QMenu( this );
  wmsImageryMenu->addAction( tr( "Custom..." ), this, SLOT( addCustomWMSImagery() ) );
  addImageryMenu->addAction( tr( "WMS" ) )->setMenu( wmsImageryMenu );

  QMenu* fileImageryMenu = new QMenu( this );
  QString worldtif = QDir::cleanPath( QgsApplication::pkgDataPath() + "/globe/world.tif" );
  if ( QgsApplication::isRunningFromBuildDir() )
  {
    worldtif = QDir::cleanPath( QgsApplication::buildSourcePath() + "/src/plugins/globe/images/world.tif" );
  }
  fileImageryMenu->addAction( tr( "world.tif" ), this, SLOT( addRasterImagery() ) )->setData( worldtif );
  fileImageryMenu->addAction( tr( "Custom..." ), this, SLOT( addCustomRasterImagery() ) );
  addImageryMenu->addAction( tr( "Raster" ) )->setMenu( fileImageryMenu );

  mAddImageryButton->setMenu( addImageryMenu );


  QMenu* addElevationMenu = new QMenu( this );

  QMenu* tmsElevationMenu = new QMenu( this );
  tmsElevationMenu->addAction( "Readymap: SRTM 90m Elevation Data", this, SLOT( addTMSElevation() ) )->setData( "http://readymap.org/readymap/tiles/1.0.0/9/" );
  tmsElevationMenu->addAction( tr( "Custom..." ), this, SLOT( addCustomTMSElevation() ) );
  addElevationMenu->addAction( tr( "TMS" ) )->setMenu( tmsElevationMenu );

  QMenu* fileElevationMenu = new QMenu( this );
  fileElevationMenu->addAction( tr( "Custom..." ), this, SLOT( addCustomRasterElevation() ) );
  addElevationMenu->addAction( tr( "Raster" ) )->setMenu( fileElevationMenu );

  mAddElevationButton->setMenu( addElevationMenu );


  comboBoxStereoMode->addItem( "OFF", -1 );
  comboBoxStereoMode->addItem( "ANAGLYPHIC", osg::DisplaySettings::ANAGLYPHIC );
  comboBoxStereoMode->addItem( "QUAD_BUFFER", osg::DisplaySettings::ANAGLYPHIC );
  comboBoxStereoMode->addItem( "HORIZONTAL_SPLIT", osg::DisplaySettings::HORIZONTAL_SPLIT );
  comboBoxStereoMode->addItem( "VERTICAL_SPLIT", osg::DisplaySettings::VERTICAL_SPLIT );

  lineEditAASamples->setValidator( new QIntValidator( lineEditAASamples ) );

#if OSGEARTH_VERSION_LESS_THAN( 2, 5, 0 )
  mSpinBoxVerticalScale->setVisible( false );
#endif

  connect( checkBoxSkyAutoAmbient, SIGNAL( toggled( bool ) ), horizontalSliderMinAmbient, SLOT( setEnabled( bool ) ) );
  connect( checkBoxDateTime, SIGNAL( toggled( bool ) ), dateTimeEditSky, SLOT( setEnabled( bool ) ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked( bool ) ), this, SLOT( apply() ) );

  restoreSavedSettings();
  readProjectSettings();
}

void QgsGlobePluginDialog::restoreSavedSettings()
{
  QSettings settings;

  // Video settings
  comboBoxStereoMode->setCurrentIndex( comboBoxStereoMode->findText( settings.value( "/Plugin-Globe/stereoMode", "OFF" ).toString() ) );
  spinBoxStereoScreenDistance->setValue( settings.value( "/Plugin-Globe/spinBoxStereoScreenDistance",
                                         osg::DisplaySettings::instance()->getScreenDistance() ).toDouble() );
  spinBoxStereoScreenWidth->setValue( settings.value( "/Plugin-Globe/spinBoxStereoScreenWidth",
                                      osg::DisplaySettings::instance()->getScreenWidth() ).toDouble() );
  spinBoxStereoScreenHeight->setValue( settings.value( "/Plugin-Globe/spinBoxStereoScreenHeight",
                                       osg::DisplaySettings::instance()->getScreenHeight() ).toDouble() );
  spinBoxStereoEyeSeparation->setValue( settings.value( "/Plugin-Globe/spinBoxStereoEyeSeparation",
                                        osg::DisplaySettings::instance()->getEyeSeparation() ).toDouble() );
  spinBoxSplitStereoHorizontalSeparation->setValue( settings.value( "/Plugin-Globe/spinBoxSplitStereoHorizontalSeparation",
      osg::DisplaySettings::instance()->getSplitStereoHorizontalSeparation() ).toInt() );
  spinBoxSplitStereoVerticalSeparation->setValue( settings.value( "/Plugin-Globe/spinBoxSplitStereoVerticalSeparation",
      osg::DisplaySettings::instance()->getSplitStereoVerticalSeparation() ).toInt() );
  comboBoxSplitStereoHorizontalEyeMapping->setCurrentIndex( settings.value( "/Plugin-Globe/comboBoxSplitStereoHorizontalEyeMapping",
      osg::DisplaySettings::instance()->getSplitStereoHorizontalEyeMapping() ).toInt() );
  comboBoxSplitStereoVerticalEyeMapping->setCurrentIndex( settings.value( "/Plugin-Globe/comboBoxSplitStereoVerticalEyeMapping",
      osg::DisplaySettings::instance()->getSplitStereoVerticalEyeMapping() ).toInt() );
  groupBoxAntiAliasing->setChecked( settings.value( "/Plugin-Globe/anti-aliasing", false ).toBool() );
  lineEditAASamples->setText( settings.value( "/Plugin-Globe/anti-aliasing-level", "" ).toString() );

  horizontalSliderMinAmbient->setEnabled( checkBoxSkyAutoAmbient->isChecked() );
  dateTimeEditSky->setEnabled( checkBoxDateTime->isChecked() );

  // Advanced
  sliderScrollSensitivity->setValue( settings.value( "/Plugin-Globe/scrollSensitivity", 20 ).toInt() );
  checkBoxInvertScroll->setChecked( settings.value( "/Plugin-Globe/invertScrollWheel", 0 ).toInt() );
  checkBoxFrustumHighlighting->setChecked( settings.value( "/Plugin-Globe/frustum-highlighting", false ).toBool() );
  checkBoxFeatureIdentification->setChecked( settings.value( "/Plugin-Globe/feature-identification", false ).toBool() );
}

void QgsGlobePluginDialog::on_buttonBox_accepted()
{
  apply();
  accept();
}

void QgsGlobePluginDialog::on_buttonBox_rejected()
{
  restoreSavedSettings();
  readProjectSettings();
  reject();
}

void QgsGlobePluginDialog::apply()
{
  QSettings settings;

  // Video settings
  settings.setValue( "/Plugin-Globe/stereoMode", comboBoxStereoMode->currentText() );
  settings.setValue( "/Plugin-Globe/stereoScreenDistance", spinBoxStereoScreenDistance->value() );
  settings.setValue( "/Plugin-Globe/stereoScreenWidth", spinBoxStereoScreenWidth->value() );
  settings.setValue( "/Plugin-Globe/stereoScreenHeight", spinBoxStereoScreenHeight->value() );
  settings.setValue( "/Plugin-Globe/stereoEyeSeparation", spinBoxStereoEyeSeparation->value() );
  settings.setValue( "/Plugin-Globe/SplitStereoHorizontalSeparation", spinBoxSplitStereoHorizontalSeparation->value() );
  settings.setValue( "/Plugin-Globe/SplitStereoVerticalSeparation", spinBoxSplitStereoVerticalSeparation->value() );
  settings.setValue( "/Plugin-Globe/SplitStereoHorizontalEyeMapping", comboBoxSplitStereoHorizontalEyeMapping->currentIndex() );
  settings.setValue( "/Plugin-Globe/SplitStereoVerticalEyeMapping", comboBoxSplitStereoVerticalEyeMapping->currentIndex() );
  settings.setValue( "/Plugin-Globe/anti-aliasing", groupBoxAntiAliasing->isChecked() );
  settings.setValue( "/Plugin-Globe/anti-aliasing-level", lineEditAASamples->text() );

  // Advanced settings
  settings.setValue( "/Plugin-Globe/scrollSensitivity", sliderScrollSensitivity->value() );
  settings.setValue( "/Plugin-Globe/invertScrollWheel", checkBoxInvertScroll->checkState() );
  settings.setValue( "/Plugin-Globe/frustum-highlighting", checkBoxFrustumHighlighting->isChecked() );
  settings.setValue( "/Plugin-Globe/feature-identification", checkBoxFeatureIdentification->isChecked() );

  writeProjectSettings();

  // Apply stereo settings
  int stereoMode = comboBoxStereoMode->itemData( comboBoxStereoMode->currentIndex() ).toInt();
  if ( stereoMode == -1 )
  {
    osg::DisplaySettings::instance()->setStereo( false );
  }
  else
  {
    osg::DisplaySettings::instance()->setStereo( true );
    osg::DisplaySettings::instance()->setStereoMode(
      static_cast<osg::DisplaySettings::StereoMode>( stereoMode ) );
    osg::DisplaySettings::instance()->setEyeSeparation( spinBoxStereoEyeSeparation->value() );
    osg::DisplaySettings::instance()->setScreenDistance( spinBoxStereoScreenDistance->value() );
    osg::DisplaySettings::instance()->setScreenWidth( spinBoxStereoScreenWidth->value() );
    osg::DisplaySettings::instance()->setScreenHeight( spinBoxStereoScreenHeight->value() );
    osg::DisplaySettings::instance()->setSplitStereoVerticalSeparation(
      spinBoxSplitStereoVerticalSeparation->value() );
    osg::DisplaySettings::instance()->setSplitStereoVerticalEyeMapping(
      static_cast<osg::DisplaySettings::SplitStereoVerticalEyeMapping>(
        comboBoxSplitStereoVerticalEyeMapping->currentIndex() ) );
    osg::DisplaySettings::instance()->setSplitStereoHorizontalSeparation(
      spinBoxSplitStereoHorizontalSeparation->value() );
    osg::DisplaySettings::instance()->setSplitStereoHorizontalEyeMapping(
      static_cast<osg::DisplaySettings::SplitStereoHorizontalEyeMapping>(
        comboBoxSplitStereoHorizontalEyeMapping->currentIndex() ) );
  }

  emit settingsApplied();
}

void QgsGlobePluginDialog::readProjectSettings()
{
  // Imagery settings
  mImageryTreeView->clear();
  foreach ( const LayerDataSource& ds, getImageryDataSources() )
  {
    QTreeWidgetItem* item = new QTreeWidgetItem( QStringList() << ds.type << ds.uri );
    item->setFlags( item->flags() & ~Qt::ItemIsDropEnabled );
    mImageryTreeView->addTopLevelItem( item );
  }
  mImageryTreeView->resizeColumnToContents( 0 );

  // Elevation settings
  mElevationTreeView->clear();
  foreach ( const LayerDataSource& ds, getElevationDataSources() )
  {
    QTreeWidgetItem* item = new QTreeWidgetItem( QStringList() << ds.type << ds.uri );
    item->setFlags( item->flags() & ~Qt::ItemIsDropEnabled );
    mElevationTreeView->addTopLevelItem( item );
  }
  mElevationTreeView->resizeColumnToContents( 0 );

#if OSGEARTH_VERSION_GREATER_OR_EQUAL( 2, 5, 0 )
  mSpinBoxVerticalScale->setValue( QgsProject::instance()->readDoubleEntry( "Globe-Plugin", "/verticalScale", 1 ) );
#endif

  // Map settings
  groupBoxSky->setChecked( QgsProject::instance()->readBoolEntry( "Globe-Plugin", "/skyEnabled", true ) );
  checkBoxDateTime->setChecked( QgsProject::instance()->readBoolEntry( "Globe-Plugin", "/overrideDateTime", false ) );
  dateTimeEditSky->setDateTime( QDateTime::fromString( QgsProject::instance()->readEntry( "Globe-Plugin", "/skyDateTime", QDateTime::currentDateTime().toString() ) ) );
  checkBoxSkyAutoAmbient->setChecked( QgsProject::instance()->readBoolEntry( "Globe-Plugin", "/skyAutoAmbient", true ) );
  horizontalSliderMinAmbient->setValue( QgsProject::instance()->readDoubleEntry( "Globe-Plugin", "/skyMinAmbient", 30. ) );
}

void QgsGlobePluginDialog::writeProjectSettings()
{
  // Imagery settings
  QgsProject::instance()->removeEntry( "Globe-Plugin", "/imageryDatasources/" );
  for ( int row = 0, nRows = mImageryTreeView->topLevelItemCount(); row < nRows; ++row )
  {
    QTreeWidgetItem* item = mImageryTreeView->topLevelItem( row );
    QString key = QString( "/imageryDatasources/L%1" ).arg( row );
    QgsProject::instance()->writeEntry( "Globe-Plugin", key + "/type", item->text( 0 ) );
    QgsProject::instance()->writeEntry( "Globe-Plugin", key + "/uri", item->text( 1 ) );
  }

  // Elevation settings
  QgsProject::instance()->removeEntry( "Globe-Plugin", "/elevationDatasources/" );
  for ( int row = 0, nRows = mElevationTreeView->topLevelItemCount(); row < nRows; ++row )
  {
    QTreeWidgetItem* item = mElevationTreeView->topLevelItem( row );
    QString key = QString( "/elevationDatasources/L%1" ).arg( row );
    QgsProject::instance()->writeEntry( "Globe-Plugin", key + "/type", item->text( 0 ) );
    QgsProject::instance()->writeEntry( "Globe-Plugin", key + "/uri", item->text( 1 ) );
  }

#if OSGEARTH_VERSION_GREATER_OR_EQUAL( 2, 5, 0 )
  QgsProject::instance()->writeEntry( "Globe-Plugin", "/verticalScale", mSpinBoxVerticalScale->value() );
#endif

  // Map settings
  QgsProject::instance()->writeEntry( "Globe-Plugin", "/skyEnabled/", groupBoxSky->isChecked() );
  QgsProject::instance()->writeEntry( "Globe-Plugin", "/overrideDateTime/", checkBoxDateTime->isChecked() );
  QgsProject::instance()->writeEntry( "Globe-Plugin", "/skyDateTime/", dateTimeEditSky->dateTime().toString() );
  QgsProject::instance()->writeEntry( "Globe-Plugin", "/skyAutoAmbient/", checkBoxSkyAutoAmbient->isChecked() );
  QgsProject::instance()->writeEntry( "Globe-Plugin", "/skyMinAmbient/", horizontalSliderMinAmbient->value() );
}

bool QgsGlobePluginDialog::validateRemoteUri( const QString& uri, QString& errMsg ) const
{
  QUrl url( uri );
  QgsNetworkAccessManager* nam = QgsNetworkAccessManager::instance();
  QNetworkReply* reply = nullptr;

  while ( true )
  {
    QNetworkRequest req( url );
    req.setRawHeader( "User-Agent" , "Wget/1.13.4" );
    reply = nam->get( req );
    QTimer timer;
    QEventLoop loop;
    QObject::connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
    QObject::connect( reply, SIGNAL( finished() ), &loop, SLOT( quit() ) );
    timer.setSingleShot( true );
    timer.start( 500 );
    loop.exec();
    if ( reply->isRunning() )
    {
      // Timeout
      reply->close();
      delete reply;
      errMsg = tr( "Timeout" );
      return false;
    }

    QUrl redirectUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    if ( redirectUrl.isValid() && url != redirectUrl )
    {
      delete reply;
      url = redirectUrl;
    }
    else
    {
      break;
    }
  }

  errMsg = reply->error() == QNetworkReply::NoError ? QString() : reply->errorString();
  delete reply;
  return errMsg.isEmpty();
}

/// MAP ///////////////////////////////////////////////////////////////////////

void QgsGlobePluginDialog::addImagery( const QString& type, const QString& uri )
{
  QTreeWidgetItem* item = new QTreeWidgetItem( QStringList() << type << uri );
  item->setFlags( item->flags() & ~Qt::ItemIsDropEnabled );
  mImageryTreeView->addTopLevelItem( item );
  mImageryTreeView->resizeColumnToContents( 0 );
}

void QgsGlobePluginDialog::addTMSImagery()
{
  addImagery( "TMS", qobject_cast<QAction*>( QObject::sender() )->data().toString() );
}

void QgsGlobePluginDialog::addCustomTMSImagery()
{
  QString url = QInputDialog::getText( this, tr( "Add TMS Imagery" ), tr( "TMS URL:" ) );
  if ( !url.isEmpty() )
  {
    QString validationError;
    if ( !validateRemoteUri( url, validationError ) )
    {
      QMessageBox::warning( this, tr( "Invalid URL" ), validationError );
    }
    else
    {
      addImagery( "TMS", url );
    }
  }
}

void QgsGlobePluginDialog::addCustomWMSImagery()
{
  QString url = QInputDialog::getText( this, tr( "Add WMS Imagery" ), tr( "URL:" ) );
  if ( !url.isEmpty() )
  {
    QString validationError;
    if ( !validateRemoteUri( url, validationError ) )
    {
      QMessageBox::warning( this, tr( "Invalid URL" ), validationError );
    }
    else
    {
      addImagery( "WMS", url );
    }
  }
}

void QgsGlobePluginDialog::addRasterImagery()
{
  addImagery( "Raster", qobject_cast<QAction*>( QObject::sender() )->data().toString() );
}

void QgsGlobePluginDialog::addCustomRasterImagery()
{
  QString filename = QFileDialog::getOpenFileName( this, tr( "Add Raster Imagery" ) );
  if ( !filename.isEmpty() )
  {
    addImagery( "Raster", filename );
  }
}

void QgsGlobePluginDialog::addElevation( const QString& type, const QString& uri )
{
  QTreeWidgetItem* item = new QTreeWidgetItem( QStringList() << type << uri );
  item->setFlags( item->flags() & ~Qt::ItemIsDropEnabled );
  mElevationTreeView->addTopLevelItem( item );
  mElevationTreeView->resizeColumnToContents( 0 );
}

void QgsGlobePluginDialog::addTMSElevation()
{
  addElevation( "TMS", qobject_cast<QAction*>( QObject::sender() )->data().toString() );
}

void QgsGlobePluginDialog::addCustomTMSElevation()
{
  QString url = QInputDialog::getText( this, tr( "Add TMS Elevation" ), tr( "TMS URL:" ) );
  if ( !url.isEmpty() )
  {
    QString validationError;
    if ( !validateRemoteUri( url, validationError ) )
    {
      QMessageBox::warning( this, tr( "Invalid URL" ), validationError );
    }
    else
    {
      addElevation( "TMS", url );
    }
  }
}

void QgsGlobePluginDialog::addCustomRasterElevation()
{
  QString filename = QFileDialog::getOpenFileName( this, tr( "Add Raster Elevation" ) );
  if ( !filename.isEmpty() )
  {
    addElevation( "Raster", filename );
  }
}

void QgsGlobePluginDialog::on_mRemoveImageryButton_clicked()
{
  delete mImageryTreeView->currentItem();
}

void QgsGlobePluginDialog::on_mRemoveElevationButton_clicked()
{
  delete mElevationTreeView->currentItem();
}

QList<QgsGlobePluginDialog::LayerDataSource> QgsGlobePluginDialog::getImageryDataSources() const
{
  int keysCount = QgsProject::instance()->subkeyList( "Globe-Plugin", "/imageryDatasources/" ).count();
  QList<LayerDataSource> datasources;
  for ( int i = 0; i < keysCount; ++i )
  {
    QString key = QString( "/imageryDatasources/L%1" ).arg( i );
    LayerDataSource datasource;
    datasource.type  = QgsProject::instance()->readEntry( "Globe-Plugin", key + "/type" );
    datasource.uri   = QgsProject::instance()->readEntry( "Globe-Plugin", key + "/uri" );
    datasources.append( datasource );
  }
  return datasources;
}

QList<QgsGlobePluginDialog::LayerDataSource> QgsGlobePluginDialog::getElevationDataSources() const
{
  int keysCount = QgsProject::instance()->subkeyList( "Globe-Plugin", "/elevationDatasources/" ).count();
  QList<LayerDataSource> datasources;
  for ( int i = 0; i < keysCount; ++i )
  {
    QString key = QString( "/elevationDatasources/L%1" ).arg( i );
    LayerDataSource datasource;
    datasource.type  = QgsProject::instance()->readEntry( "Globe-Plugin", key + "/type" );
    datasource.uri   = QgsProject::instance()->readEntry( "Globe-Plugin", key + "/uri" );
    datasources.append( datasource );
  }
  return datasources;
}

double QgsGlobePluginDialog::getVerticalScale() const
{
  return mSpinBoxVerticalScale->value();
}

bool QgsGlobePluginDialog::getSkyEnabled() const
{
  return QgsProject::instance()->readBoolEntry( "Globe-Plugin", "/skyEnabled", true );
}

QDateTime QgsGlobePluginDialog::getSkyDateTime() const
{
  bool overrideDateTime = QgsProject::instance()->readBoolEntry( "Globe-Plugin", "/overrideDateTime", false );
  if ( overrideDateTime )
  {
    return QDateTime::fromString( QgsProject::instance()->readEntry( "Globe-Plugin", "/skyDateTime", QDateTime::currentDateTime().toString() ) );
  }
  else
  {
    return QDateTime::currentDateTime();
  }
}

bool QgsGlobePluginDialog::getSkyAutoAmbience() const
{
  return QgsProject::instance()->readBoolEntry( "Globe-Plugin", "/skyAutoAmbient", true );
}

double QgsGlobePluginDialog::getSkyMinAmbient() const
{
  return QgsProject::instance()->readDoubleEntry( "Globe-Plugin", "/skyMinAmbient", 30. ) / 100.;
}

/// ADVANCED //////////////////////////////////////////////////////////////////

float QgsGlobePluginDialog::getScrollSensitivity() const
{
  return sliderScrollSensitivity->value() / 10;
}

bool QgsGlobePluginDialog::getInvertScrollWheel() const
{
  return checkBoxInvertScroll->checkState();
}

bool QgsGlobePluginDialog::getFrustumHighlighting() const
{
  return checkBoxFrustumHighlighting->isChecked();
}

bool QgsGlobePluginDialog::getFeatureIdenification() const
{
  return checkBoxFeatureIdentification->isChecked();
}

/// STEREO ////////////////////////////////////////////////////////////////////
void QgsGlobePluginDialog::on_pushButtonStereoResetDefaults_clicked()
{
  //http://www.openscenegraph.org/projects/osg/wiki/Support/UserGuides/StereoSettings
  comboBoxStereoMode->setCurrentIndex( comboBoxStereoMode->findText( "OFF" ) );
  spinBoxStereoScreenDistance->setValue( 0.5 );
  spinBoxStereoScreenHeight->setValue( 0.26 );
  spinBoxStereoScreenWidth->setValue( 0.325 );
  spinBoxStereoEyeSeparation->setValue( 0.06 );
  spinBoxSplitStereoHorizontalSeparation->setValue( 42 );
  spinBoxSplitStereoVerticalSeparation->setValue( 42 );
  comboBoxSplitStereoHorizontalEyeMapping->setCurrentIndex( 0 );
  comboBoxSplitStereoVerticalEyeMapping->setCurrentIndex( 0 );
}

void QgsGlobePluginDialog::on_comboBoxStereoMode_currentIndexChanged( int index )
{
  //http://www.openscenegraph.org/projects/osg/wiki/Support/UserGuides/StereoSettings
  //http://www.openscenegraph.org/documentation/OpenSceneGraphReferenceDocs/a00181.html

  int stereoMode = comboBoxStereoMode->itemData( index ).toInt();
  bool stereoEnabled = stereoMode != -1;
  bool verticalSplit = stereoMode == osg::DisplaySettings::VERTICAL_SPLIT;
  bool horizontalSplit = stereoMode == osg::DisplaySettings::HORIZONTAL_SPLIT;

  spinBoxStereoScreenDistance->setEnabled( stereoEnabled );
  spinBoxStereoScreenHeight->setEnabled( stereoEnabled );
  spinBoxStereoScreenWidth->setEnabled( stereoEnabled );
  spinBoxStereoEyeSeparation->setEnabled( stereoEnabled );
  spinBoxSplitStereoHorizontalSeparation->setEnabled( stereoEnabled && horizontalSplit );
  comboBoxSplitStereoHorizontalEyeMapping->setEnabled( stereoEnabled && horizontalSplit );
  spinBoxSplitStereoVerticalSeparation->setEnabled( stereoEnabled && verticalSplit );
  comboBoxSplitStereoVerticalEyeMapping->setEnabled( stereoEnabled && verticalSplit );
}
