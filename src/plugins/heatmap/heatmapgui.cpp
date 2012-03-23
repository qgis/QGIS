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
// qgis includes
#include "qgis.h"
#include "heatmapgui.h"
#include "qgscontexthelp.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdistancearea.h"

// GDAL includes
#include "gdal_priv.h"
#include "cpl_string.h"
#include "cpl_conv.h"

//qt includes
#include <QComboBox>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

//standard includes

HeatmapGui::HeatmapGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  QgsDebugMsg( tr( "Creating Heatmap Dialog" ) );

  // Adding point layers to the mInputVectorCombo
  foreach( QgsMapLayer *l, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( l );
    if ( !vl || vl->geometryType() != QGis::Point )
      continue;

    mInputVectorCombo->addItem( vl->name(), vl->id() );
  }

  // Adding GDAL drivers with CREATE to the mFormatCombo
  int myTiffIndex = -1;
  int myIndex = -1;
  GDALAllRegister();
  int nDrivers = GDALGetDriverCount();
  for ( int i = 0; i < nDrivers; i += 1 )
  {
    GDALDriver* nthDriver = GetGDALDriverManager()->GetDriver( i );
    char** driverMetadata = nthDriver->GetMetadata();
    if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
    {
      ++myIndex;
      QString myLongName = nthDriver->GetMetadataItem( GDAL_DMD_LONGNAME );
      // Add LongName text, shortname variant; GetDescription actually gets the shortname
      mFormatCombo->addItem( myLongName, QVariant( nthDriver->GetDescription() ) );
      // Add the drivers and their extensions to a map for filename correction
      mExtensionMap.insert( nthDriver->GetDescription(), nthDriver->GetMetadataItem( GDAL_DMD_EXTENSION ) );
      if ( myLongName == "GeoTIFF" )
      {
        myTiffIndex = myIndex;
      }
    }
  }
  mFormatCombo->setCurrentIndex( myTiffIndex );

  //finally set right the ok button
  enableOrDisableOkButton();
}

HeatmapGui::~HeatmapGui()
{
}

/*
 *
 * Private Slots
 *
 */
void HeatmapGui::on_mButtonBox_accepted()
{
  accept();
}

void HeatmapGui::on_mButtonBox_rejected()
{
  reject();
}

void HeatmapGui::on_mButtonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}

void HeatmapGui::on_mBrowseButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/Heatmap/lastOutputDir", "" ).toString();

  QString outputFilename = QFileDialog::getSaveFileName( 0, tr( "Save Heatmap as:" ), lastDir );
  if ( !outputFilename.isEmpty() )
  {
    mOutputRasterLineEdit->setText( outputFilename );
    QFileInfo outputFileInfo( outputFilename );
    QDir outputDir = outputFileInfo.absoluteDir();
    if ( outputDir.exists() )
    {
      s.setValue( "/Heatmap/lastOutputDir", outputFileInfo.absolutePath() );
    }
  }

  enableOrDisableOkButton();
}

void HeatmapGui::on_mOutputRasterLineEdit_editingFinished()
{
  enableOrDisableOkButton();
}

void HeatmapGui::on_advancedGroupBox_toggled( bool enabled )
{
  if ( enabled )
  {
    // if there are no layers point layers then show error dialog and toggle
    if( mInputVectorCombo->count() == 0 )
    {
      QMessageBox::information( 0, tr( "No valid layers found!" ), tr( "Advanced options cannot be enabled." ) );
      advancedGroupBox->setChecked( false );
      return;
    }
    // if there are layers then populate fields
    populateFields();
    updateBBox();

 }
}

void HeatmapGui::on_rowLineEdit_editingFinished()
{
  int rows = rowLineEdit->text().toInt();
  float ycellsize = mBBox.height() / rows;
  float xcellsize = ycellsize;
  int columns = mBBox.width() / xcellsize;

  updateSize( rows, columns, xcellsize, ycellsize );
}

void HeatmapGui::on_columnLineEdit_editingFinished()
{
  int columns = columnLineEdit->text().toInt();
  float xcellsize = mBBox.width() / columns;
  float ycellsize = xcellsize;
  int rows = mBBox.height() / ycellsize;

  updateSize( rows, columns, xcellsize, ycellsize );
}

void HeatmapGui::on_cellXLineEdit_editingFinished()
{
  float xcellsize = cellXLineEdit->text().toFloat();
  float ycellsize = xcellsize;
  int rows = mBBox.height() / ycellsize;
  int columns = mBBox.width() / xcellsize;

  updateSize( rows, columns, xcellsize, ycellsize );
}

void HeatmapGui::on_cellYLineEdit_editingFinished()
{
  float ycellsize = cellYLineEdit->text().toFloat();
  float xcellsize = ycellsize;
  int rows = mBBox.height() / ycellsize;
  int columns = mBBox.width() / xcellsize;

  updateSize( rows, columns, xcellsize, ycellsize );
}

void HeatmapGui::on_radiusFieldUnitCombo_currentIndexChanged( int index )
{
  updateBBox();
  QgsDebugMsg( tr( "Unit index set to %1").arg( index ) );
}

void HeatmapGui::on_mRadiusUnitCombo_currentIndexChanged( int index )
{
  QgsDebugMsg( tr( "Unit index set to %1").arg( index ) );
  updateBBox();
}

void HeatmapGui::on_mInputVectorCombo_currentIndexChanged( int index )
{
  if( advancedGroupBox->isChecked() )
  {
    populateFields();
    updateBBox();
  }
  QgsDebugMsg( tr("Input vector index changed to %1").arg( index ) );
}

void HeatmapGui::on_radiusFieldCombo_currentIndexChanged( int index )
{
  updateBBox();
  QgsDebugMsg( tr("Radius Field index changed to %1").arg( index ) );
}

void HeatmapGui::on_mBufferLineEdit_editingFinished()
{
  updateBBox();
}
/*
 *
 * Private Functions
 *
 */
void HeatmapGui::enableOrDisableOkButton()
{
  bool enabled = true;
  QString filename = mOutputRasterLineEdit->text();
  QFileInfo theFileInfo( filename );
  if ( filename.isEmpty() || !theFileInfo.dir().exists() || ( mInputVectorCombo->count() == 0 ) )
  {
    enabled = false;
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void HeatmapGui::populateFields()
{
  QgsVectorLayer* inputLayer = inputVectorLayer();
  // The fields
  QgsVectorDataProvider* provider = inputLayer->dataProvider();
  QgsFieldMap fieldMap = provider->fields();
  // Populate fields
  radiusFieldCombo->clear();
  weightFieldCombo->clear();

  QMap<int, QgsField>::const_iterator i = fieldMap.constBegin();
  while ( i != fieldMap.constEnd() )
  {
    radiusFieldCombo->addItem( i.value().name(), QVariant( i.key() ) );
    weightFieldCombo->addItem( i.value().name(), QVariant( i.key() ) );
    ++i;
  }

}

void HeatmapGui::updateSize( int rows, int columns, float cellx, float celly )
{
  rowLineEdit->setText( QString::number( rows ) );
  columnLineEdit->setText( QString::number( columns ) );
  cellXLineEdit->setText( QString::number( cellx ) );
  cellYLineEdit->setText( QString::number( celly ) );
}

void HeatmapGui::updateBBox()
{
  // Set the row/cols and cell sizes here
  QgsVectorLayer *inputLayer = inputVectorLayer();

  mBBox = inputLayer->extent();
  QgsCoordinateReferenceSystem layerCrs = inputLayer->crs();

  float radiusInMapUnits;
  if( useRadius->isChecked() )
  {
    float maxInField = inputLayer->maximumValue( radiusFieldCombo->itemData( radiusFieldCombo->currentIndex() ).toInt() ).toFloat();

    if( radiusFieldUnitCombo->currentIndex() == HeatmapGui::Meters )
    {
      radiusInMapUnits = mapUnitsOf( maxInField, layerCrs );
    }
    else if( radiusFieldUnitCombo->currentIndex() == HeatmapGui::MapUnits )
    {
      radiusInMapUnits = maxInField;
    }
  }
  else
  {
    float radiusValue = mBufferLineEdit->text().toFloat();
    if( mRadiusUnitCombo->currentIndex() == HeatmapGui::Meters )
    {
      radiusInMapUnits = mapUnitsOf( radiusValue, layerCrs );
    }
    else if( mRadiusUnitCombo->currentIndex() == HeatmapGui::MapUnits )
    {
      radiusInMapUnits = radiusValue;
    }
  }
  // get the distance converted into map units
  mBBox.setXMinimum( mBBox.xMinimum() - radiusInMapUnits );
  mBBox.setYMinimum( mBBox.yMinimum() - radiusInMapUnits );
  mBBox.setXMaximum(  mBBox.xMaximum() + radiusInMapUnits);
  mBBox.setYMaximum( mBBox.yMaximum() + radiusInMapUnits );
  float xcellsize;
  float ycellsize;
  int rows = 500;
  ycellsize = mBBox.height() / rows;
  xcellsize = ycellsize;
  int columns = mBBox.width() / xcellsize;

  if( advancedGroupBox->isChecked() )
  {
    updateSize( rows, columns, xcellsize, ycellsize );
  }
 
}

float HeatmapGui::mapUnitsOf( float meters, QgsCoordinateReferenceSystem layerCrs )
{
  // converter function to transform metres input to mapunits
  // so that bounding box can be updated
  QgsDistanceArea da;
  da.setSourceCrs( layerCrs.srsid() );
  da.setEllipsoid( layerCrs.ellipsoidAcronym() );
  if ( da.geographic() )
  {
    da.setProjectionsEnabled( true );
  }
  double unitDistance = da.measureLine( QgsPoint( 0.0, 0.0 ), QgsPoint( 0.0, 1.0 ) );
  QgsDebugMsg( tr( "Converted %1 meters to %2 mapunits" ).arg( meters ).arg( meters/unitDistance ) );
  return  meters/unitDistance;
}
/*
 *
 * Public functions
 *
 */

bool HeatmapGui::weighted()
{
  return useWeight->isChecked();
}

bool HeatmapGui::variableRadius()
{
  return useRadius->isChecked();
}

float HeatmapGui::radius()
{
  QString dummyText;
  float radius;
  dummyText = mBufferLineEdit->text();
  radius = dummyText.toInt();
  return radius;
}

int HeatmapGui::radiusUnit()
{
  if( useRadius->isChecked() )
  {
    return radiusFieldUnitCombo->currentIndex();
  }
  return mRadiusUnitCombo->currentIndex();
}

float HeatmapGui::decayRatio()
{
  QString dummyText;
  float decayRatio;
  dummyText = mDecayLineEdit->text();
  decayRatio = dummyText.toFloat();
  return decayRatio;
}

QVariant HeatmapGui::radiusField()
{
  int radiusindex;
  radiusindex = radiusFieldUnitCombo->currentIndex();
  return radiusFieldUnitCombo->itemData( radiusindex );
}

QVariant HeatmapGui::weightField()
{
  int weightindex;
  weightindex = weightFieldCombo->currentIndex();
  return weightFieldCombo->itemData( weightindex );
}

QString HeatmapGui::outputFilename()
{
  QString outputFileName;
  QString outputFormat;

  outputFileName = mOutputRasterLineEdit->text();
  QFileInfo myFileInfo( outputFileName );
  if ( outputFileName.isEmpty() || !myFileInfo.dir().exists() )
  {
    QMessageBox::information( 0, tr( "Invalid output filename" ), tr( "Please enter a valid output file path and name." ) );
    return NULL;
  }

  // The output format
  outputFormat = mFormatCombo->itemData( mFormatCombo->currentIndex() ).toString();
  // append the file format if the suffix is empty
  QString suffix = myFileInfo.suffix();
  if ( suffix.isEmpty() )
  {
    QMap<QString, QString>::const_iterator it = mExtensionMap.find( outputFormat );
    if ( it != mExtensionMap.end() && it.key() == outputFormat )
    {
      // making sure that there is really a extension value available
      // Some drivers donot seem to have any extension at all
      if ( it.value() != NULL || it.value() != "" )
      {
        outputFileName.append( "." );
        outputFileName.append( it.value() );
      }
    }
  }

  return outputFileName;
}

QString HeatmapGui::ouputFormat()
{
  return mFormatCombo->itemData( mFormatCombo->currentIndex() ).toString();
}

QgsVectorLayer* HeatmapGui::inputVectorLayer()
{
  QString myLayerId = mInputVectorCombo->itemData( mInputVectorCombo->currentIndex() ).toString();

  QgsVectorLayer* inputLayer = qobject_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( myLayerId ) );
  if ( !inputLayer )
  {
    QMessageBox::information( 0, tr( "Layer not found" ), tr( "Layer %1 not found." ).arg( myLayerId ) );
    return NULL;
  }
  return inputLayer;
}

int HeatmapGui::rows()
{
  return rowLineEdit->text().toInt();
}

int HeatmapGui::columns()
{
  return columnLineEdit->text().toInt();
}

float HeatmapGui::cellSizeX()
{
  return cellXLineEdit->text().toFloat();
}

float HeatmapGui::cellSizeY()
{
  return cellYLineEdit->text().toFloat();
}
