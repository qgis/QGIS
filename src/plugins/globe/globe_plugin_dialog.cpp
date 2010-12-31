/*
* $Id$
*/
/***************************************************************************
*    globe_plugin_dialog.cpp - settings dialog for the globe plugin
*     --------------------------------------
*    Date                 : 11-Nov-2010
*    Copyright            : (C) 2010 by Marco Bernasocchi
*    Email                : marco at bernawebdesign.ch
* /***************************************************************************
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
#include <qgsproject.h>
#include <qgsmaplayerregistry.h>
#include <qgsmaplayer.h>
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
QgsGlobePluginDialog::QgsGlobePluginDialog( QgsOsgViewer* viewer, QWidget* parent, Qt::WFlags fl )
: mViewer(viewer), QDialog( parent, fl )
{
  setupUi( this );
  loadStereoConfig();  //values from settings, default values from OSG
  setStereoConfig(); //overwrite with values from QSettings
  updateStereoDialog(); //update the dialog gui

  elevationPath->setText( QDir::homePath() );
}

//destructor
QgsGlobePluginDialog::~QgsGlobePluginDialog()
{
}

QString QgsGlobePluginDialog::openRasterFile()
{
  //see http://www.gdal.org/formats_list.html
  const char* filter = "GDAL files (*.dem *.tif *.tiff *.jpg *.jpeg *.asc) \
  ;;DEM files (*.dem) \
  ;;All files (*.*)";
QString path = QFileDialog::getOpenFileName( this,
                                            tr( "Open raster file" ),
                                            QDir::homePath (),
                                            tr( filter ) );

return path;
}

bool QgsGlobePluginDialog::validateResource( QString type, QString uri, QString &error )
{
  if ( "Raster" == type )
  {
    QFileInfo file( uri );
    if ( file.isFile() && file.isReadable() )
    {
      return true;
    }
    else
    {
      error = tr("Invalid Path: The file is either unreadable or does not exist");
      return false;
    }
  }
  else
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
      error = tr("Invalid URL: ") + reply->errorString();
      return false;
    }
  }
}

void QgsGlobePluginDialog::showMessageBox( QString text )
{
  QMessageBox msgBox;
  msgBox.setText(text);
  msgBox.exec();
}

void QgsGlobePluginDialog::on_buttonBox_accepted()
{
  setStereoConfig();
  saveStereoConfig();

  saveElevationDatasources();
  accept();
}

void QgsGlobePluginDialog::on_buttonBox_rejected()
{
  loadStereoConfig();
  setStereoConfig();
  readElevationDatasources();
  reject();
}

//ELEVATION
void QgsGlobePluginDialog::on_elevationCombo_currentIndexChanged(QString type)
{
  elevationPath->setEnabled(true);
  if("Raster" == type)
  {
    elevationActions->setCurrentIndex(0);
    elevationPath->setText( QDir::homePath() );
  }
  else if ( "Worldwind" == type )
  {
    elevationActions->setCurrentIndex(1);
    elevationPath->setText("http://tileservice.worldwindcentral.com/getTile?bmng.topo.bathy.200401");
    elevationPath->setEnabled(false);
  }
  else if ( "TMS" == type )
  {
    elevationActions->setCurrentIndex(1);
    elevationPath->setText("http://demo.pelicanmapping.com/rmweb/data/srtm30_plus_tms/tms.xml");
  }
}

void QgsGlobePluginDialog::on_elevationBrowse_clicked()
{
  QString newPath = openRasterFile();
  if ( ! newPath.isEmpty() )
  {
    elevationPath->setText( newPath );
  }
}

void QgsGlobePluginDialog::on_elevationAdd_clicked()
{
  QString errorText;
  bool validationResult = validateResource( elevationCombo->currentText(), elevationPath->text(), errorText );

  QMessageBox msgBox;
  msgBox.setText( errorText );
  msgBox.setInformativeText( tr( "Do you want to add the datasource anyway?" ) );
  msgBox.setIcon( QMessageBox::Warning );
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);

  if ( validationResult || msgBox.exec() == QMessageBox::Ok )
  {
    int i = elevationDatasourcesWidget->rowCount();
    QTableWidgetItem *type = new QTableWidgetItem(elevationCombo->currentText());
    QTableWidgetItem *uri = new QTableWidgetItem(elevationPath->text());
    QTableWidgetItem* cache = new QTableWidgetItem();
    cache->setCheckState( (elevationCombo->currentText() == "Worldwind") ? Qt::Checked : Qt::Unchecked ); //worldwind_cache will be active
    elevationDatasourcesWidget->setRowCount(1+i);
    elevationDatasourcesWidget->setItem(i, 0, type);
    elevationDatasourcesWidget->setItem(i, 1, cache);
    elevationDatasourcesWidget->setItem(i, 2, uri);
    elevationDatasourcesWidget->setCurrentItem(type, QItemSelectionModel::Clear);
  }
}

void QgsGlobePluginDialog::on_elevationRemove_clicked()
{
  elevationDatasourcesWidget->removeRow( elevationDatasourcesWidget->currentRow() );
}

void QgsGlobePluginDialog::on_elevationUp_clicked()
{
  moveRow( elevationDatasourcesWidget, true );
}

void QgsGlobePluginDialog::on_elevationDown_clicked()
{
  moveRow( elevationDatasourcesWidget, false );
}

void QgsGlobePluginDialog::moveRow(QTableWidget* widget, bool up)
{
  //moves QTableWidget row up or down
  if ( widget->selectedItems().count() > 0 )
  {
    const int sourceRow = widget->currentItem()->row();
    const int destRow = (up ? sourceRow-1 : sourceRow+1);
    if ( destRow >= 0 && destRow < widget->rowCount() )
    {
      // take whole rows
      QList<QTableWidgetItem*> sourceItems = takeRow(widget, sourceRow);
      QList<QTableWidgetItem*> destItems = takeRow(widget, destRow);

      // set back in reverse order
      setRow(widget, sourceRow, destItems);
      setRow(widget, destRow, sourceItems);
      widget->selectRow(destRow);
    }
  }
}

QList<QTableWidgetItem*> QgsGlobePluginDialog::takeRow(QTableWidget* widget, int row)
{
  // takes and returns the whole row
  QList<QTableWidgetItem*> rowItems;
  for (int col = 0; col < widget->columnCount(); ++col)
  {
    rowItems << widget->takeItem(row, col);
  }
  return rowItems;
}

void QgsGlobePluginDialog::setRow(QTableWidget* widget, int row, const QList<QTableWidgetItem*>& rowItems)
{
  // sets the whole row
  for (int col = 0; col < widget->columnCount(); ++col)
  {
    widget->setItem(row, col, rowItems.at(col));
  }
}

void QgsGlobePluginDialog::readElevationDatasources()
{
  //showMessageBox("reading");
  // clear the widget
  elevationDatasourcesWidget->clearContents();
  int keysCount = QgsProject::instance()->subkeyList("Globe-Plugin", "/elevationDatasources/").count();
  for (int i = 0; i < keysCount; ++i) {
    QString iNum;
    iNum.setNum(i);
    QTableWidgetItem *type = new QTableWidgetItem(
      QgsProject::instance()->readEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/type"));
    QTableWidgetItem *uri = new QTableWidgetItem(
      QgsProject::instance()->readEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/uri"));
    bool cache = QgsProject::instance()->readBoolEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/cache");

    elevationDatasourcesWidget->setRowCount(1+i);
    elevationDatasourcesWidget->setItem(i, 0, type);
    QTableWidgetItem* chkBoxItem = new QTableWidgetItem();
    chkBoxItem->setCheckState( cache ? Qt::Checked : Qt::Unchecked );
    elevationDatasourcesWidget->setItem(i, 1, chkBoxItem);
    elevationDatasourcesWidget->setItem(i, 2, uri);
  }
}

void QgsGlobePluginDialog::saveElevationDatasources()
{
  bool somethingChanged = false;
  int keysCount = QgsProject::instance()->subkeyList("Globe-Plugin", "/elevationDatasources/").count();
  int rowsCount = elevationDatasourcesWidget->rowCount();

  for (int i = 0; i < rowsCount; ++i) {
    QString iNum;
    iNum.setNum(i);
    QString typeKey   = QgsProject::instance()->readEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/type");
    QString uriKey    =  QgsProject::instance()->readEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/uri");
    bool    cacheKey  = QgsProject::instance()->readBoolEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/cache");

    QString type  = elevationDatasourcesWidget->item(i, 0)->text();
    QString uri   = elevationDatasourcesWidget->item(i, 2)->text();
    bool    cache = elevationDatasourcesWidget->item(i, 1)->checkState();

    if ( typeKey != type || uriKey != uri || cacheKey != cache )
    {
      somethingChanged = true;
      QgsProject::instance()->writeEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/type", type);
      QgsProject::instance()->writeEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/uri", uri);
      QgsProject::instance()->writeEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/cache", cache);
      QgsDebugMsg( "editing at "+iNum );
    }
    else
    {
      QgsDebugMsg( "nothing to do at "+iNum );
    }
  }

  if (keysCount > rowsCount )
  {
    //elminate superfluous keys
    somethingChanged = true;
    for (int i = rowsCount; i < keysCount; ++i) {
      QString iNum;
      iNum.setNum(i);
      QgsDebugMsg( "deleting "+iNum );
      QgsProject::instance()->removeEntry("Globe-Plugin", "/elevationDatasources/L"+iNum+"/");
    }
  }

  if ( somethingChanged )
  {
    QgsDebugMsg( "emitting elevationDatasourcesChanged" );
    emit elevationDatasourcesChanged();
  }
}
//END ELEVATION

//MODEL
QList<QgsVectorLayer*> QgsGlobePluginDialog::pointLayers()
{
  QList<QgsVectorLayer*> list;
  QMap< QString, QgsMapLayer *> layers = QgsMapLayerRegistry::instance()->mapLayers();
  QMapIterator<QString, QgsMapLayer *> it(layers);
  while (it.hasNext())
  {
     it.next();
     QgsMapLayer* layer = it.value();
     if (layer->type() == QgsMapLayer::VectorLayer)
     {
       QgsVectorLayer* vectorLayer = static_cast<QgsVectorLayer*>(layer);
       if ( vectorLayer->geometryType() == QGis::Point )
       {
          list.append( vectorLayer );
       }
     }
  }
  return list;
}

void QgsGlobePluginDialog::updatePointLayers()
{
  modelLayerCombo->clear();
  QList<QgsVectorLayer*> layers = pointLayers();
  QListIterator<QgsVectorLayer*> it(layers);
  while (it.hasNext())
  {
     QgsVectorLayer* layer = it.next();
     modelLayerCombo->addItem( layer->name() );
  }
}

QgsVectorLayer* QgsGlobePluginDialog::modelLayer()
{
  QList<QgsVectorLayer*> layers = pointLayers();
  return ( modelLayerCombo->currentIndex() == -1 ) ? NULL : layers.at( modelLayerCombo->currentIndex() );
}

void QgsGlobePluginDialog::on_modelBrowse_clicked()
{
  //see http://www.openscenegraph.org/projects/osg/wiki/Support/UserGuides/Plugins
  const char* filter = "Model files (*.3dc *.asc *.3ds *.ac *.bsp *.dae *.dw *.dxf *.fbx *.gem *.geo *.iv *.wrl *.ive *.logo *.lwo *.lw *.geo *.lws *.md2 *.obj *.ogr *.flt *.osg *.shp *.stl *.sta *.wrl *.x) \
  ;;All files (*.*)";
  QString path = QFileDialog::getOpenFileName( this,
                                            tr( "Open 3D model file" ),
                                            QDir::homePath (),
                                            tr( filter ) );

  if ( ! path.isEmpty() )
  {
    modelPathLineEdit->setText( path );
  }
}
//END MODEL

//STEREO
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
  mViewer->getDatabasePager()->clear();
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
//END STEREO
