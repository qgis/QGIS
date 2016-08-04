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
#include <gdal.h>
#include <cpl_string.h>
#include <cpl_conv.h>

//qt includes
#include <QComboBox>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

//standard includes

HeatmapGui::HeatmapGui( QWidget* parent, Qt::WindowFlags fl, QMap<QString, QVariant>* temporarySettings )
    : QDialog( parent, fl )
    , mRows( 500 )
{
  setupUi( this );

  QgsDebugMsg( QString( "Creating Heatmap Dialog" ) );

  blockAllSignals( true );

  mKernelShapeCombo->addItem( tr( "Quartic (biweight)" ), Heatmap::Quartic );
  mKernelShapeCombo->addItem( tr( "Triangular" ), Heatmap::Triangular );
  mKernelShapeCombo->addItem( tr( "Uniform" ), Heatmap::Uniform );
  mKernelShapeCombo->addItem( tr( "Triweight" ), Heatmap::Triweight );
  mKernelShapeCombo->addItem( tr( "Epanechnikov" ), Heatmap::Epanechnikov );

  mOutputValuesComboBox->addItem( tr( "Raw values" ), Heatmap::Raw );
  mOutputValuesComboBox->addItem( tr( "Scaled by kernel size" ), Heatmap::Scaled );

  mHeatmapSessionSettings = temporarySettings;

  // Adding point layers to the inputLayerCombo
  QString lastUsedLayer = mHeatmapSessionSettings->value( QString( "lastInputLayer" ) ).toString();
  bool usingLastInputLayer = false;

  mInputLayerCombo->setFilters( QgsMapLayerProxyModel::PointLayer );
  QgsMapLayer* defaultLayer = QgsMapLayerRegistry::instance()->mapLayer( lastUsedLayer );
  if ( defaultLayer )
  {
    mInputLayerCombo->setLayer( defaultLayer );
    usingLastInputLayer = true;
  }

  mRadiusFieldCombo->setFilters( QgsFieldProxyModel::Numeric );
  mWeightFieldCombo->setFilters( QgsFieldProxyModel::Numeric );
  connect( mInputLayerCombo, SIGNAL( layerChanged( QgsMapLayer* ) ), mRadiusFieldCombo, SLOT( setLayer( QgsMapLayer* ) ) );
  connect( mInputLayerCombo, SIGNAL( layerChanged( QgsMapLayer* ) ), mWeightFieldCombo, SLOT( setLayer( QgsMapLayer* ) ) );
  mRadiusFieldCombo->setLayer( mInputLayerCombo->currentLayer() );
  mWeightFieldCombo->setLayer( mInputLayerCombo->currentLayer() );

  // Adding GDAL drivers with CREATE to mFormatCombo
  int myTiffIndex = -1;
  int myIndex = -1;
  GDALAllRegister();
  int nDrivers = GDALGetDriverCount();
  for ( int i = 0; i < nDrivers; i += 1 )
  {
    GDALDriverH nthDriver = GDALGetDriver( i );
    char **driverMetadata = GDALGetMetadata( nthDriver, nullptr );
    // Only formats which allow creation of Float32 data types are valid
    if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) &&
         QString( GDALGetMetadataItem( nthDriver, GDAL_DMD_CREATIONDATATYPES, nullptr ) ).contains( "Float32" ) )
    {
      ++myIndex;
      QString myLongName = GDALGetMetadataItem( nthDriver, GDAL_DMD_LONGNAME, nullptr );
      // Add LongName text, shortname variant; GetDescription actually gets the shortname
      mFormatCombo->addItem( myLongName, QVariant( GDALGetDescription( nthDriver ) ) );
      // Add the drivers and their extensions to a map for filename correction
      mExtensionMap.insert( GDALGetDescription( nthDriver ), GDALGetMetadataItem( nthDriver, GDAL_DMD_EXTENSION, nullptr ) );
      if ( myLongName == "GeoTIFF" )
      {
        myTiffIndex = myIndex;
      }
    }
  }
  //Restore choice of output format from last run
  QSettings s;
  int defaultFormatIndex = s.value( "/Heatmap/lastFormat", myTiffIndex ).toInt();
  mFormatCombo->setCurrentIndex( defaultFormatIndex );

  restoreSettings( usingLastInputLayer );
  updateBBox();
  updateSize();

  mAddToCanvas->setChecked( s.value( "/Heatmap/addToCanvas", true ).toBool() );

  blockAllSignals( false );

  //finally set right the ok button
  enableOrDisableOkButton();
}

HeatmapGui::~HeatmapGui()
{
}

void HeatmapGui::blockAllSignals( bool b )
{
  mBufferSizeLineEdit->blockSignals( b );
  mInputLayerCombo->blockSignals( b );
  mRowsSpinBox->blockSignals( b );
  mRadiusFieldCombo->blockSignals( b );
  mAdvancedGroupBox->blockSignals( b );
  mKernelShapeCombo->blockSignals( b );
  mColumnsSpinBox->blockSignals( b );
  mCellXLineEdit->blockSignals( b );
  mCellYLineEdit->blockSignals( b );
  mOutputValuesComboBox->blockSignals( b );
}

/*
 *
 * Private Slots
 *
 */
void HeatmapGui::on_mButtonBox_accepted()
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
      mAdvancedGroupBox->setChecked( true );
    }

    mRadiusFieldCombo->setLayer( mInputLayerCombo->currentLayer() );
    mWeightFieldCombo->setLayer( mInputLayerCombo->currentLayer() );

    // Radius controls
    mBufferSizeLineEdit->setText( mHeatmapSessionSettings->value( QString( "lastRadius" ) ).toString() );
    mBufferUnitCombo->setCurrentIndex( mHeatmapSessionSettings->value( QString( "lastRadiusUnit" ) ).toInt() );

    // Raster size controls
    mRows = mHeatmapSessionSettings->value( QString( "lastRows" ) ).toInt();
    mRowsSpinBox->setValue( mRows );

    // Data defined radius controls
    if ( mHeatmapSessionSettings->value( QString( "useRadius" ) ).toBool() )
    {
      mRadiusFieldCheckBox->setChecked( true );
      mRadiusFieldUnitCombo->setCurrentIndex( mHeatmapSessionSettings->value( QString( "radiusFieldUnit" ) ).toInt() );
      mRadiusFieldCombo->setField( mHeatmapSessionSettings->value( QString( "radiusField" ) ).toString() );
    }

    // Data defined weight controls
    if ( mHeatmapSessionSettings->value( QString( "useWeight" ) ).toBool() )
    {
      mWeightFieldCheckBox->setChecked( true );
      mWeightFieldCombo->setField( mHeatmapSessionSettings->value( QString( "weightField" ) ).toString() );
    }
  }
  else
  {
    // Default to estimated radius
    mBufferSizeLineEdit->setText( QString::number( estimateRadius() ) );
  }

  // Kernel setting - not layer specific
  if ( mHeatmapSessionSettings->value( QString( "lastKernel" ) ).toInt() )
  {
    mKernelShapeCombo->setCurrentIndex( mKernelShapeCombo->findData(
                                          ( Heatmap::KernelShape )( mHeatmapSessionSettings->value( QString( "lastKernel" ) ).toInt() ) ) );
    mDecayLineEdit->setText( mHeatmapSessionSettings->value( QString( "decayRatio" ) ).toString() );
    mDecayLineEdit->setEnabled( mAdvancedGroupBox->isChecked() &&
                                ( Heatmap::KernelShape )( mKernelShapeCombo->itemData( mKernelShapeCombo->currentIndex() ).toInt() ) == Heatmap::Triangular );
  }
  mOutputValuesComboBox->setCurrentIndex( mOutputValuesComboBox->findData(
                                            ( Heatmap::OutputValues )( mHeatmapSessionSettings->value( QString( "lastOutputValues" ), "0" ).toInt() ) ) );

}

void HeatmapGui::saveSettings()
{
  // Save persistent settings
  QSettings s;
  s.setValue( "/Heatmap/lastFormat", QVariant( mFormatCombo->currentIndex() ) );
  s.setValue( "/Heatmap/addToCanvas", mAddToCanvas->isChecked() );

  // Store temporary settings, which only apply to this session
  mHeatmapSessionSettings->insert( QString( "lastInputLayer" ), QVariant( mInputLayerCombo->currentLayer()->id() ) );
  mHeatmapSessionSettings->insert( QString( "lastRadius" ), QVariant( mBufferSizeLineEdit->text().toDouble() ) );
  mHeatmapSessionSettings->insert( QString( "lastRadiusUnit" ), QVariant( mBufferUnitCombo->currentIndex() ) );
  mHeatmapSessionSettings->insert( QString( "advancedEnabled" ), QVariant( mAdvancedGroupBox->isChecked() ) );
  mHeatmapSessionSettings->insert( QString( "lastRows" ), QVariant( mRowsSpinBox->value() ) );
  mHeatmapSessionSettings->insert( QString( "lastKernel" ), QVariant( mKernelShapeCombo->itemData( mKernelShapeCombo->currentIndex() ).toInt() ) );
  mHeatmapSessionSettings->insert( QString( "useRadius" ), QVariant( mRadiusFieldCheckBox->isChecked() ) );
  mHeatmapSessionSettings->insert( QString( "radiusField" ), QVariant( mRadiusFieldCombo->currentField() ) );
  mHeatmapSessionSettings->insert( QString( "radiusFieldUnit" ), QVariant( mRadiusFieldUnitCombo->currentIndex() ) );
  mHeatmapSessionSettings->insert( QString( "useWeight" ), QVariant( mWeightFieldCheckBox->isChecked() ) );
  mHeatmapSessionSettings->insert( QString( "weightField" ), QVariant( mWeightFieldCombo->currentField() ) );
  mHeatmapSessionSettings->insert( QString( "decayRatio" ), QVariant( mDecayLineEdit->text() ) );
  mHeatmapSessionSettings->insert( QString( "lastOutputValues" ), QVariant( mOutputValuesComboBox->itemData( mOutputValuesComboBox->currentIndex() ).toInt() ) );
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
  QString lastDir = s.value( "/Heatmap/lastOutputDir", QDir::homePath() ).toString();

  QString outputFilename = QFileDialog::getSaveFileName( nullptr, tr( "Save Heatmap as:" ), lastDir );
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

void HeatmapGui::on_mAdvancedGroupBox_toggled( bool enabled )
{
  if ( enabled )
  {
    // if there are no layers point layers then show error dialog and toggle
    if ( mInputLayerCombo->count() == 0 )
    {
      QMessageBox::information( nullptr, tr( "No valid layers found!" ), tr( "Advanced options cannot be enabled." ) );
      mAdvancedGroupBox->setChecked( false );
      return;
    }

    updateBBox();
    mDecayLineEdit->setEnabled(( Heatmap::KernelShape )( mKernelShapeCombo->itemData( mKernelShapeCombo->currentIndex() ).toInt() ) == Heatmap::Triangular );

  }
}

void HeatmapGui::on_mRowsSpinBox_valueChanged()
{
  mRows = mRowsSpinBox->value();
  mYcellsize = mBBox.height() / mRows;
  mXcellsize = mYcellsize;
  mColumns = max( qRound( mBBox.width() / mXcellsize ) + 1, 1 );

  updateSize();
}

void HeatmapGui::on_mColumnsSpinBox_valueChanged()
{
  mColumns = mColumnsSpinBox->value();
  mXcellsize = mBBox.width() / ( mColumns - 1 );
  mYcellsize = mXcellsize;
  mRows = max( qRound( mBBox.height() / mYcellsize ), 1 );

  updateSize();
}

void HeatmapGui::on_mCellXLineEdit_editingFinished()
{
  mXcellsize = mCellXLineEdit->text().toDouble();
  mYcellsize = mXcellsize;
  mRows = max( qRound( mBBox.height() / mYcellsize ) + 1, 1 );
  mColumns = max( qRound( mBBox.width() / mXcellsize ) + 1, 1 );

  updateSize();
}

void HeatmapGui::on_mCellYLineEdit_editingFinished()
{
  mYcellsize = mCellYLineEdit->text().toDouble();
  mXcellsize = mYcellsize;
  mRows = max( qRound( mBBox.height() / mYcellsize ) + 1, 1 );
  mColumns = max( qRound( mBBox.width() / mXcellsize ) + 1, 1 );

  updateSize();
}

void HeatmapGui::on_mRadiusFieldUnitCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  updateBBox();
  // DebugMsg to avoid index not used warning
  QgsDebugMsg( QString( "Unit index set to %1" ).arg( index ) );
}

void HeatmapGui::on_mBufferUnitCombo_currentIndexChanged( int index )
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

  if ( mBufferUnitCombo->currentIndex() == HeatmapGui::LayerUnits )
  {
    // layer units selected, so convert estimate from map units
    QgsCoordinateReferenceSystem layerCrs = inputLayer->crs();
    estimate /= mapUnitsOf( 1, layerCrs );
  }

  // Make estimate pretty by rounding off to first digit only (eg 356->300, 0.567->0.5)
  double tens = pow( 10, floor( log10( estimate ) ) );
  return floor( estimate / tens + 0.5 ) * tens;
}

void HeatmapGui::on_mInputLayerCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  // Set initial value for radius field based on layer's extent
  mBufferSizeLineEdit->setText( QString::number( estimateRadius() ) );

  updateBBox();

  QgsDebugMsg( QString( "Input vector index changed to %1" ).arg( index ) );
}

void HeatmapGui::on_mRadiusFieldCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  updateBBox();
  QgsDebugMsg( QString( "Radius Field index changed to %1" ).arg( index ) );
}

void HeatmapGui::on_mBufferSizeLineEdit_editingFinished()
{
  updateBBox();
}

void HeatmapGui::on_mKernelShapeCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  // Only enable the decay edit if the kernel shape is set to triangular
  mDecayLineEdit->setEnabled( index == Heatmap::Triangular );
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
  if ( filename.isEmpty() || !theFileInfo.dir().exists() || ( mInputLayerCombo->count() == 0 ) )
  {
    enabled = false;
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void HeatmapGui::updateSize()
{
  blockAllSignals( true );
  mRowsSpinBox->setValue( mRows );
  mColumnsSpinBox->setValue( mColumns );
  mCellXLineEdit->setText( QString::number( mXcellsize ) );
  mCellYLineEdit->setText( QString::number( mYcellsize ) );
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
  if ( mRadiusFieldCheckBox->isChecked() )
  {
    int idx = inputLayer->fields().indexFromName( mRadiusFieldCombo->currentField() );
    double maxInField = inputLayer->maximumValue( idx ).toDouble();

    if ( mRadiusFieldUnitCombo->currentIndex() == HeatmapGui::LayerUnits )
    {
      radiusInMapUnits = mapUnitsOf( maxInField, layerCrs );
    }
    else if ( mRadiusFieldUnitCombo->currentIndex() == HeatmapGui::MapUnits )
    {
      radiusInMapUnits = maxInField;
    }
  }
  else
  {
    double radiusValue = mBufferSizeLineEdit->text().toDouble();
    if ( mBufferUnitCombo->currentIndex() == HeatmapGui::LayerUnits )
    {
      radiusInMapUnits = mapUnitsOf( radiusValue, layerCrs );
    }
    else if ( mBufferUnitCombo->currentIndex() == HeatmapGui::MapUnits )
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

double HeatmapGui::mapUnitsOf( double dist, const QgsCoordinateReferenceSystem& layerCrs ) const
{
  // converter function to transform layer input to mapunits
  // so that bounding box can be updated
  QgsDistanceArea da;
  da.setSourceCrs( layerCrs.srsid() );
  da.setEllipsoid( layerCrs.ellipsoidAcronym() );
  if ( da.geographic() )
  {
    da.setEllipsoidalMode( true );
  }
  double unitDistance = da.measureLine( QgsPoint( 0.0, 0.0 ), QgsPoint( 0.0, 1.0 ) );
  QgsDebugMsg( QString( "Converted %1 layer to %2 map units" ).arg( dist ).arg( dist / unitDistance ) );
  return  dist / unitDistance;
}
/*
 *
 * Public functions
 *
 */

bool HeatmapGui::weighted() const
{
  return mWeightFieldCheckBox->isChecked();
}

bool HeatmapGui::variableRadius() const
{
  return mRadiusFieldCheckBox->isChecked();
}

double HeatmapGui::radius() const
{
  double radius = mBufferSizeLineEdit->text().toDouble();
  if ( mBufferUnitCombo->currentIndex() == HeatmapGui::LayerUnits )
  {
    radius = mapUnitsOf( radius, inputVectorLayer()->crs() );
  }
  return radius;
}

int HeatmapGui::radiusUnit() const
{
  if ( mRadiusFieldCheckBox->isChecked() )
  {
    return mRadiusFieldUnitCombo->currentIndex();
  }
  return mBufferUnitCombo->currentIndex();
}

Heatmap::KernelShape HeatmapGui::kernelShape() const
{
  return ( Heatmap::KernelShape ) mKernelShapeCombo->itemData( mKernelShapeCombo->currentIndex() ).toInt();
}

Heatmap::OutputValues HeatmapGui::outputValues() const
{
  return ( Heatmap::OutputValues ) mOutputValuesComboBox->itemData( mOutputValuesComboBox->currentIndex() ).toInt();
}

double HeatmapGui::decayRatio() const
{
  return mDecayLineEdit->text().toDouble();
}

int HeatmapGui::radiusField() const
{
  QgsVectorLayer *inputLayer = inputVectorLayer();
  if ( !inputLayer )
    return 0;

  return inputLayer->fields().indexFromName( mRadiusFieldCombo->currentField() );
}

int HeatmapGui::weightField() const
{
  QgsVectorLayer *inputLayer = inputVectorLayer();
  if ( !inputLayer )
    return 0;

  return inputLayer->fields().indexFromName( mWeightFieldCombo->currentField() );
}

bool HeatmapGui::addToCanvas() const
{
  return mAddToCanvas->isChecked();
}

QString HeatmapGui::outputFilename() const
{
  QString outputFileName;
  QString outputFormat;

  outputFileName = mOutputRasterLineEdit->text();
  QFileInfo myFileInfo( outputFileName );
  if ( outputFileName.isEmpty() || !myFileInfo.dir().exists() )
  {
    QMessageBox::information( nullptr, tr( "Invalid output filename" ), tr( "Please enter a valid output file path and name." ) );
    return QString::null;
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
      // Some drivers don't seem to have any extension at all
      if ( !it.value().isEmpty() )
      {
        outputFileName.append( '.' );
        outputFileName.append( it.value() );
      }
    }
  }

  return outputFileName;
}

QString HeatmapGui::outputFormat() const
{
  return mFormatCombo->itemData( mFormatCombo->currentIndex() ).toString();
}

QgsVectorLayer* HeatmapGui::inputVectorLayer() const
{
  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( mInputLayerCombo->currentLayer() );
  return layer;
}

