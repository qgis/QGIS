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
#include "heatmap.h"
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

HeatmapGui::HeatmapGui( QWidget* parent, Qt::WFlags fl, QMap<QString, QVariant>* temporarySettings )
    : QDialog( parent, fl ),
    mRows( 500 )
{
  setupUi( this );

  QgsDebugMsg( QString( "Creating Heatmap Dialog" ) );

  blockAllSignals( true );

  mHeatmapSessionSettings = temporarySettings;

  // Adding point layers to the inputLayerCombo
  QString defaultLayer = mHeatmapSessionSettings->value( QString( "lastInputLayer" ) ).toString();
  int defaultLayerIndex = 0;
  bool usingLastInputLayer = false;
  int currentIndex = -1;
  foreach ( QgsMapLayer *l, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( l );
    if ( !vl || vl->geometryType() != QGis::Point )
      continue;

    currentIndex++;
    inputLayerCombo->addItem( vl->name(), vl->id() );
    if ( vl->id() == defaultLayer )
    {
      // if this layer is the same layer as a heatmap was last generated using,
      // then default to this layer
      usingLastInputLayer = true;
      defaultLayerIndex = currentIndex;
    }
  }

  inputLayerCombo->setCurrentIndex( defaultLayerIndex );

  // Adding GDAL drivers with CREATE to formatCombo
  int myTiffIndex = -1;
  int myIndex = -1;
  GDALAllRegister();
  int nDrivers = GDALGetDriverCount();
  for ( int i = 0; i < nDrivers; i += 1 )
  {
    GDALDriver* nthDriver = GetGDALDriverManager()->GetDriver( i );
    char** driverMetadata = nthDriver->GetMetadata();
    // Only formats which allow creation of Float32 data types are valid
    if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) &&
         QString( nthDriver->GetMetadataItem( GDAL_DMD_CREATIONDATATYPES, NULL ) ).contains( "Float32" ) )
    {
      ++myIndex;
      QString myLongName = nthDriver->GetMetadataItem( GDAL_DMD_LONGNAME );
      // Add LongName text, shortname variant; GetDescription actually gets the shortname
      formatCombo->addItem( myLongName, QVariant( nthDriver->GetDescription() ) );
      // Add the drivers and their extensions to a map for filename correction
      mExtensionMap.insert( nthDriver->GetDescription(), nthDriver->GetMetadataItem( GDAL_DMD_EXTENSION ) );
      if ( myLongName == "GeoTIFF" )
      {
        myTiffIndex = myIndex;
      }
    }
  }
  //Restore choice of output format from last run
  QSettings s;
  int defaultFormatIndex = s.value( "/Heatmap/lastFormat", myTiffIndex ).toInt();
  formatCombo->setCurrentIndex( defaultFormatIndex );

  restoreSettings( usingLastInputLayer );
  updateBBox();
  updateSize();

  blockAllSignals( false );

  //finally set right the ok button
  enableOrDisableOkButton();
}

HeatmapGui::~HeatmapGui()
{
}

void HeatmapGui::blockAllSignals( bool b )
{
  bufferLineEdit->blockSignals( b );
  inputLayerCombo->blockSignals( b );
  rowsSpinBox->blockSignals( b );
  radiusFieldCombo->blockSignals( b );
  advancedGroupBox->blockSignals( b );
  kernelShapeCombo->blockSignals( b );
  rowsSpinBox->blockSignals( b );
  columnsSpinBox->blockSignals( b );
  cellXLineEdit->blockSignals( b );
  cellYLineEdit->blockSignals( b );
}

/*
 *
 * Private Slots
 *
 */
void HeatmapGui::on_buttonBox_accepted()
{
  saveSettings();
  accept();
}

void HeatmapGui::restoreSettings( bool usingLastInputLayer )
{
  // Temporary settings, which are cleared on exit from QGIS
  // If we are using the same layer as last run, restore layer specific settings
  if ( usingLastInputLayer )
  {
    // Advanced checkbox
    if ( mHeatmapSessionSettings->value( QString( "advancedEnabled" ) ).toBool() )
    {
      advancedGroupBox->setChecked( true );
    }
    populateFields();

    // Radius controls
    bufferLineEdit->setText( mHeatmapSessionSettings->value( QString( "lastRadius" ) ).toString() );
    radiusUnitCombo->setCurrentIndex( mHeatmapSessionSettings->value( QString( "lastRadiusUnit" ) ).toInt() );

    // Raster size controls
    mRows = mHeatmapSessionSettings->value( QString( "lastRows" ) ).toInt();
    rowsSpinBox->setValue( mRows );

    // Data defined radius controls
    if ( mHeatmapSessionSettings->value( QString( "useRadius" ) ).toBool() )
    {
      radiusFieldCheckBox->setChecked( true );
      radiusFieldUnitCombo->setCurrentIndex( mHeatmapSessionSettings->value( QString( "radiusFieldUnit" ) ).toInt() );
      // Default to same radius field as last run
      QString prevRadiusField = mHeatmapSessionSettings->value( QString( "radiusField" ) ).toString();
      for ( int idx = 0; idx < radiusFieldCombo->count(); ++idx )
      {
        if ( radiusFieldCombo->itemText( idx ) == prevRadiusField )
        {
          radiusFieldCombo->setCurrentIndex( idx );
          break;
        }
      }
    }

    // Data defined weight controls
    if ( mHeatmapSessionSettings->value( QString( "useWeight" ) ).toBool() )
    {
      weightFieldCheckBox->setChecked( true );
      // Default to same weight field as last run
      QString prevWeightField = mHeatmapSessionSettings->value( QString( "weightField" ) ).toString();
      for ( int idx = 0; idx < weightFieldCombo->count(); ++idx )
      {
        if ( weightFieldCombo->itemText( idx ) == prevWeightField )
        {
          weightFieldCombo->setCurrentIndex( idx );
          break;
        }
      }
    }
  }
  else
  {
    // Default to estimated radius
    bufferLineEdit->setText( QString::number( estimateRadius() ) );
  }

  // Kernel setting - not layer specific
  if ( mHeatmapSessionSettings->value( QString( "lastKernel" ) ).toInt() )
  {
    kernelShapeCombo->setCurrentIndex( mHeatmapSessionSettings->value( QString( "lastKernel" ) ).toInt() );
    decayLineEdit->setText( mHeatmapSessionSettings->value( QString( "decayRatio" ) ).toString() );
    decayLineEdit->setEnabled( advancedGroupBox->isChecked() && kernelShapeCombo->currentIndex() == Heatmap::Triangular );
  }
}

void HeatmapGui::saveSettings()
{
  // Save persistent settings
  QSettings s;
  s.setValue( "/Heatmap/lastFormat", QVariant( formatCombo->currentIndex() ) );

  // Store temporary settings, which only apply to this session
  mHeatmapSessionSettings->insert( QString( "lastInputLayer" ), QVariant( inputLayerCombo->itemData( inputLayerCombo->currentIndex() ) ) );
  mHeatmapSessionSettings->insert( QString( "lastRadius" ), QVariant( bufferLineEdit->text().toDouble() ) );
  mHeatmapSessionSettings->insert( QString( "lastRadiusUnit" ), QVariant( radiusUnitCombo->currentIndex() ) );
  mHeatmapSessionSettings->insert( QString( "advancedEnabled" ), QVariant( advancedGroupBox->isChecked() ) );
  mHeatmapSessionSettings->insert( QString( "lastRows" ), QVariant( rowsSpinBox->value() ) );
  mHeatmapSessionSettings->insert( QString( "lastKernel" ), QVariant( kernelShapeCombo->currentIndex() ) );
  mHeatmapSessionSettings->insert( QString( "useRadius" ), QVariant( radiusFieldCheckBox->isChecked() ) );
  mHeatmapSessionSettings->insert( QString( "radiusField" ), QVariant( radiusFieldCombo->currentText() ) );
  mHeatmapSessionSettings->insert( QString( "radiusFieldUnit" ), QVariant( radiusFieldUnitCombo->currentIndex() ) );
  mHeatmapSessionSettings->insert( QString( "useWeight" ), QVariant( weightFieldCheckBox->isChecked() ) );
  mHeatmapSessionSettings->insert( QString( "weightField" ), QVariant( weightFieldCombo->currentText() ) );
  mHeatmapSessionSettings->insert( QString( "decayRatio" ), QVariant( decayLineEdit->text() ) );
}

void HeatmapGui::on_buttonBox_rejected()
{
  reject();
}

void HeatmapGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}

void HeatmapGui::on_browseButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/Heatmap/lastOutputDir", "" ).toString();

  QString outputFilename = QFileDialog::getSaveFileName( 0, tr( "Save Heatmap as:" ), lastDir );
  if ( !outputFilename.isEmpty() )
  {
    outputRasterLineEdit->setText( outputFilename );
    QFileInfo outputFileInfo( outputFilename );
    QDir outputDir = outputFileInfo.absoluteDir();
    if ( outputDir.exists() )
    {
      s.setValue( "/Heatmap/lastOutputDir", outputFileInfo.absolutePath() );
    }
  }

  enableOrDisableOkButton();
}

void HeatmapGui::on_outputRasterLineEdit_editingFinished()
{
  enableOrDisableOkButton();
}

void HeatmapGui::on_advancedGroupBox_toggled( bool enabled )
{
  if ( enabled )
  {
    // if there are no layers point layers then show error dialog and toggle
    if ( inputLayerCombo->count() == 0 )
    {
      QMessageBox::information( 0, tr( "No valid layers found!" ), tr( "Advanced options cannot be enabled." ) );
      advancedGroupBox->setChecked( false );
      return;
    }
    // if there are layers then populate fields
    populateFields();
    updateBBox();
    decayLineEdit->setEnabled( kernelShapeCombo->currentIndex() == Heatmap::Triangular );
  }
}

void HeatmapGui::on_rowsSpinBox_valueChanged()
{
  mRows = rowsSpinBox->value();
  mYcellsize = mBBox.height() / mRows;
  mXcellsize = mYcellsize;
  mColumns = max( qRound( mBBox.width() / mXcellsize ) + 1, 1 );

  updateSize();
}

void HeatmapGui::on_columnsSpinBox_valueChanged()
{
  mColumns = columnsSpinBox->value();
  mXcellsize = mBBox.width() / ( mColumns - 1 );
  mYcellsize = mXcellsize;
  mRows = max( qRound( mBBox.height() / mYcellsize ), 1 );

  updateSize();
}

void HeatmapGui::on_cellXLineEdit_editingFinished()
{
  mXcellsize = cellXLineEdit->text().toDouble();
  mYcellsize = mXcellsize;
  mRows = max( qRound( mBBox.height() / mYcellsize ) + 1, 1 );
  mColumns = max( qRound( mBBox.width() / mXcellsize ) + 1, 1 );

  updateSize();
}

void HeatmapGui::on_cellYLineEdit_editingFinished()
{
  mYcellsize = cellYLineEdit->text().toDouble();
  mXcellsize = mYcellsize;
  mRows = max( qRound( mBBox.height() / mYcellsize ) + 1, 1 );
  mColumns = max( qRound( mBBox.width() / mXcellsize ) + 1, 1 );

  updateSize();
}

void HeatmapGui::on_radiusFieldUnitCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  updateBBox();
  // DebugMsg to avoid index not used warning
  QgsDebugMsg( QString( "Unit index set to %1" ).arg( index ) );
}

void HeatmapGui::on_radiusUnitCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QgsDebugMsg( QString( "Unit index set to %1" ).arg( index ) );
  updateBBox();
}

/*
 * Estimate a good default radius for the heatmap, based on the
 * bounding box size of the layer
 */
double HeatmapGui::estimateRadius()
{

  QgsVectorLayer *inputLayer = inputVectorLayer();

  // No input layer? Default to radius of 100
  if ( !inputLayer )
    return 100;

  // Find max dimension of layer bounding box
  QgsRectangle mExtent = inputLayer->extent();
  double maxExtent = max( mExtent.width(), mExtent.height() );

  // Return max dimension divided by 30. This is fairly arbitrary
  // but approximately corresponds to the default value chosen by ArcMap
  // TODO - a better solution is to let the data define the radius
  // choice by setting the radius equal to the average Nearest
  // Neighbour Index for the closest n points

  double estimate = maxExtent / 30;

  if ( radiusUnitCombo->currentIndex() == HeatmapGui::Meters )
  {
    // metres selected, so convert estimate from map units
    QgsCoordinateReferenceSystem layerCrs = inputLayer->crs();
    estimate = estimate / mapUnitsOf( 1, layerCrs );
  }

  // Make estimate pretty by rounding off to first digit only (eg 356->300, 0.567->0.5)
  double tens = pow( 10, floor( log10( estimate ) ) );
  return floor( estimate / tens + 0.5 ) * tens;
}

void HeatmapGui::on_inputLayerCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  // Set initial value for radius field based on layer's extent
  bufferLineEdit->setText( QString::number( estimateRadius() ) );

  updateBBox();

  if ( advancedGroupBox->isChecked() )
  {
    populateFields();
  }
  QgsDebugMsg( QString( "Input vector index changed to %1" ).arg( index ) );
}

void HeatmapGui::on_radiusFieldCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  updateBBox();
  QgsDebugMsg( QString( "Radius Field index changed to %1" ).arg( index ) );
}

void HeatmapGui::on_bufferLineEdit_editingFinished()
{
  updateBBox();
}

void HeatmapGui::on_kernelShapeCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  // Only enable the decay edit if the kernel shape is set to triangular
  decayLineEdit->setEnabled( index == Heatmap::Triangular );
}

/*
 *
 * Private Functions
 *
 */
void HeatmapGui::enableOrDisableOkButton()
{
  bool enabled = true;
  QString filename = outputRasterLineEdit->text();
  QFileInfo theFileInfo( filename );
  if ( filename.isEmpty() || !theFileInfo.dir().exists() || ( inputLayerCombo->count() == 0 ) )
  {
    enabled = false;
  }
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void HeatmapGui::populateFields()
{
  QgsVectorLayer* inputLayer = inputVectorLayer();
  if ( !inputLayer )
    return;

  // The fields
  QgsVectorDataProvider* provider = inputLayer->dataProvider();
  const QgsFields& fields = provider->fields();
  // Populate fields
  radiusFieldCombo->clear();
  weightFieldCombo->clear();

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    radiusFieldCombo->addItem( fields[idx].name(), QVariant( idx ) );
    weightFieldCombo->addItem( fields[idx].name(), QVariant( idx ) );
  }

}

void HeatmapGui::updateSize()
{
  blockAllSignals( true );
  rowsSpinBox->setValue( mRows );
  columnsSpinBox->setValue( mColumns );
  cellXLineEdit->setText( QString::number( mXcellsize ) );
  cellYLineEdit->setText( QString::number( mYcellsize ) );
  blockAllSignals( false );
}

void HeatmapGui::updateBBox()
{
  // Set the row/cols and cell sizes here
  QgsVectorLayer *inputLayer = inputVectorLayer();
  if ( !inputLayer )
    return;

  mBBox = inputLayer->extent();
  QgsCoordinateReferenceSystem layerCrs = inputLayer->crs();

  double radiusInMapUnits = 0.0;
  if ( radiusFieldCheckBox->isChecked() )
  {
    double maxInField = inputLayer->maximumValue( radiusFieldCombo->itemData( radiusFieldCombo->currentIndex() ).toInt() ).toDouble();

    if ( radiusFieldUnitCombo->currentIndex() == HeatmapGui::Meters )
    {
      radiusInMapUnits = mapUnitsOf( maxInField, layerCrs );
    }
    else if ( radiusFieldUnitCombo->currentIndex() == HeatmapGui::MapUnits )
    {
      radiusInMapUnits = maxInField;
    }
  }
  else
  {
    double radiusValue = bufferLineEdit->text().toDouble();
    if ( radiusUnitCombo->currentIndex() == HeatmapGui::Meters )
    {
      radiusInMapUnits = mapUnitsOf( radiusValue, layerCrs );
    }
    else if ( radiusUnitCombo->currentIndex() == HeatmapGui::MapUnits )
    {
      radiusInMapUnits = radiusValue;
    }
  }
  // get the distance converted into map units
  mBBox.setXMinimum( mBBox.xMinimum() - radiusInMapUnits );
  mBBox.setYMinimum( mBBox.yMinimum() - radiusInMapUnits );
  mBBox.setXMaximum( mBBox.xMaximum() + radiusInMapUnits );
  mBBox.setYMaximum( mBBox.yMaximum() + radiusInMapUnits );

  // Leave number of rows the same, and calculate new corresponding cell size and number of columns
  mYcellsize = mBBox.height() / ( mRows - 1 );
  mXcellsize = mYcellsize;
  mColumns = max( mBBox.width() / mXcellsize + 1, 1 );
  updateSize();
}

double HeatmapGui::mapUnitsOf( double meters, QgsCoordinateReferenceSystem layerCrs )
{
  // converter function to transform metres input to mapunits
  // so that bounding box can be updated
  QgsDistanceArea da;
  da.setSourceCrs( layerCrs.srsid() );
  da.setEllipsoid( layerCrs.ellipsoidAcronym() );
  if ( da.geographic() )
  {
    da.setEllipsoidalMode( true );
  }
  double unitDistance = da.measureLine( QgsPoint( 0.0, 0.0 ), QgsPoint( 0.0, 1.0 ) );
  QgsDebugMsg( QString( "Converted %1 meters to %2 mapunits" ).arg( meters ).arg( meters / unitDistance ) );
  return  meters / unitDistance;
}
/*
 *
 * Public functions
 *
 */

bool HeatmapGui::weighted()
{
  return weightFieldCheckBox->isChecked();
}

bool HeatmapGui::variableRadius()
{
  return radiusFieldCheckBox->isChecked();
}

double HeatmapGui::radius()
{
  double radius = bufferLineEdit->text().toDouble();
  if ( radiusUnitCombo->currentIndex() == HeatmapGui::Meters )
  {
    radius = mapUnitsOf( radius, inputVectorLayer()->crs() );
  }
  return radius;
}

int HeatmapGui::radiusUnit()
{
  if ( radiusFieldCheckBox->isChecked() )
  {
    return radiusFieldUnitCombo->currentIndex();
  }
  return radiusUnitCombo->currentIndex();
}

int HeatmapGui::kernelShape()
{
  return kernelShapeCombo->currentIndex();
}

double HeatmapGui::decayRatio()
{
  return decayLineEdit->text().toDouble();
}

int HeatmapGui::radiusField()
{
  int radiusindex;
  radiusindex = radiusFieldCombo->currentIndex();
  return radiusFieldCombo->itemData( radiusindex ).toInt();
}

int HeatmapGui::weightField()
{
  int weightindex;
  weightindex = weightFieldCombo->currentIndex();
  return weightFieldCombo->itemData( weightindex ).toInt();
}

QString HeatmapGui::outputFilename()
{
  QString outputFileName;
  QString outputFormat;

  outputFileName = outputRasterLineEdit->text();
  QFileInfo myFileInfo( outputFileName );
  if ( outputFileName.isEmpty() || !myFileInfo.dir().exists() )
  {
    QMessageBox::information( 0, tr( "Invalid output filename" ), tr( "Please enter a valid output file path and name." ) );
    return QString::null;
  }

  // The output format
  outputFormat = formatCombo->itemData( formatCombo->currentIndex() ).toString();
  // append the file format if the suffix is empty
  QString suffix = myFileInfo.suffix();
  if ( suffix.isEmpty() )
  {
    QMap<QString, QString>::const_iterator it = mExtensionMap.find( outputFormat );
    if ( it != mExtensionMap.end() && it.key() == outputFormat )
    {
      // making sure that there is really a extension value available
      // Some drivers don't seem to have any extension at all
      if ( !it.value().isEmpty() )
      {
        outputFileName.append( "." );
        outputFileName.append( it.value() );
      }
    }
  }

  return outputFileName;
}

QString HeatmapGui::outputFormat()
{
  return formatCombo->itemData( formatCombo->currentIndex() ).toString();
}

QgsVectorLayer* HeatmapGui::inputVectorLayer()
{
  QString myLayerId = inputLayerCombo->itemData( inputLayerCombo->currentIndex() ).toString();

  QgsVectorLayer* inputLayer = qobject_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( myLayerId ) );
  if ( !inputLayer )
  {
    QMessageBox::information( 0, tr( "Layer not found" ), tr( "Layer %1 not found." ).arg( myLayerId ) );
    return 0;
  }
  return inputLayer;
}

