/***************************************************************************
     qgstransformsettingsdialog.cpp
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "qgsprojectionselectiontreewidget.h"
#include "qgsapplication.h"
#include "qgsfilewidget.h"
#include "qgstransformsettingsdialog.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsvectorfilewriter.h"

QgsTransformSettingsDialog::QgsTransformSettingsDialog( QgsMapLayerType type, const QString &source, const QString &output, QWidget *parent )
  : QDialog( parent )
  , mType( type )
  , mSourceFile( source )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  QgsFileWidget *outputFile = mType == QgsMapLayerType::RasterLayer ? mRasterOutputFile : mVectorOutputFile;
  outputFile->setStorageMode( QgsFileWidget::SaveFile );
  if ( output.isEmpty() )
  {
    outputFile->setFilePath( generateModifiedFileName( mSourceFile ) );
  }
  else
  {
    outputFile->setFilePath( output );
  }

  switch ( mType )
  {
    case QgsMapLayerType::VectorLayer:
      mOutputSettingsStackedWidget->setCurrentWidget( mVectorOutputSettings );
      mOutputSettingsStackedWidget->removeWidget( mRasterOutputSettings );
      outputFile->setFilter( QgsVectorFileWriter::fileFilterString() );
      break;
    case QgsMapLayerType::RasterLayer:
      mOutputSettingsStackedWidget->setCurrentWidget( mRasterOutputSettings );
      mOutputSettingsStackedWidget->removeWidget( mVectorOutputSettings );
      outputFile->setFilter( tr( "TIF files" ) + " (*.tif *.tiff *.TIF *.TIFF)" );
      break;
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }
  mOutputSettingsStackedWidget->adjustSize();
  mOutputSettingsGroupBox->adjustSize();

  outputFile->setDialogTitle( tr( "Destination File" ) );
  const QString lastDestinationFolder = settingLastDestinationFolder.value();
  outputFile->setDefaultRoot( lastDestinationFolder.isEmpty() ? QDir::homePath() : lastDestinationFolder );
  connect( outputFile, &QgsFileWidget::fileChanged, this, [ = ]
  {
    settingLastDestinationFolder.setValue( QFileInfo( outputFile->filePath() ).absolutePath() );
  } );

  mPdfMap->setStorageMode( QgsFileWidget::SaveFile );
  mPdfMap->setFilter( tr( "PDF files" ) + " (*.pdf *.PDF)" );
  mPdfMap->setDialogTitle( tr( "Save Map File As" ) );
  const QString lastPdfFolder = settingLastPdfFolder.value();
  mPdfMap->setDefaultRoot( lastPdfFolder.isEmpty() ? QDir::homePath() : lastPdfFolder );
  connect( mPdfMap, &QgsFileWidget::fileChanged, this, [ = ]
  {
    settingLastPdfFolder.setValue( QFileInfo( mPdfMap->filePath() ).absolutePath() );
  } );

  mPdfReport->setStorageMode( QgsFileWidget::SaveFile );
  mPdfReport->setFilter( tr( "PDF files" ) + " (*.pdf *.PDF)" );
  mPdfReport->setDialogTitle( tr( "Save Report File As" ) );
  mPdfReport->setDefaultRoot( lastPdfFolder.isEmpty() ? QDir::homePath() : lastPdfFolder );
  connect( mPdfReport, &QgsFileWidget::fileChanged, this, [ = ]
  {
    settingLastPdfFolder.setValue( QFileInfo( mPdfMap->filePath() ).absolutePath() );
  } );

  connect( cmbTransformType, &QComboBox::currentTextChanged, this, &QgsTransformSettingsDialog::cmbTransformType_currentIndexChanged );
  connect( mWorldFileCheckBox, &QCheckBox::stateChanged, this, &QgsTransformSettingsDialog::mWorldFileCheckBox_stateChanged );

  cmbTransformType->addItem( tr( "Linear" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::Linear ) );
  cmbTransformType->addItem( tr( "Helmert" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::Helmert ) );
  cmbTransformType->addItem( tr( "Polynomial 1" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::PolynomialOrder1 ) );
  cmbTransformType->addItem( tr( "Polynomial 2" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::PolynomialOrder2 ) );
  cmbTransformType->addItem( tr( "Polynomial 3" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::PolynomialOrder3 ) );
  cmbTransformType->addItem( tr( "Thin Plate Spline" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::ThinPlateSpline ) );
  cmbTransformType->addItem( tr( "Projective" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::Projective ) );

  // Populate CompressionComboBox
  cmbCompressionComboBox->addItem( tr( "None" ), QStringLiteral( "None" ) );
  cmbCompressionComboBox->addItem( tr( "LZW" ), QStringLiteral( "LZW" ) );
  cmbCompressionComboBox->addItem( tr( "PACKBITS" ), QStringLiteral( "PACKBITS" ) );
  cmbCompressionComboBox->addItem( tr( "DEFLATE" ), QStringLiteral( "DEFLATE" ) );

  cmbResampling->addItem( tr( "Nearest Neighbour" ), static_cast< int >( QgsImageWarper::ResamplingMethod::NearestNeighbour ) );
  cmbResampling->addItem( tr( "Linear" ), static_cast< int >( QgsImageWarper::ResamplingMethod::Bilinear ) );
  cmbResampling->addItem( tr( "Cubic" ), static_cast< int >( QgsImageWarper::ResamplingMethod::Cubic ) );
  cmbResampling->addItem( tr( "Cubic Spline" ), static_cast< int >( QgsImageWarper::ResamplingMethod::CubicSpline ) );
  cmbResampling->addItem( tr( "Lanczos" ), static_cast< int >( QgsImageWarper::ResamplingMethod::Lanczos ) );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsTransformSettingsDialog::showHelp );
}

void QgsTransformSettingsDialog::setTargetCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrsSelector->setCrs( crs );
}

QgsCoordinateReferenceSystem QgsTransformSettingsDialog::targetCrs() const
{
  return mCrsSelector->crs();
}

bool QgsTransformSettingsDialog::createWorldFileOnly() const
{
  return mWorldFileCheckBox->isChecked();
}

void QgsTransformSettingsDialog::setCreateWorldFileOnly( bool enabled )
{
  mWorldFileCheckBox->setChecked( enabled );
  mWorldFileCheckBox_stateChanged( mWorldFileCheckBox->checkState() );
}

QgsGcpTransformerInterface::TransformMethod QgsTransformSettingsDialog::transformMethod() const
{
  if ( cmbTransformType->currentIndex() == -1 )
    return QgsGcpTransformerInterface::TransformMethod::InvalidTransform;
  else
    return static_cast< QgsGcpTransformerInterface::TransformMethod >( cmbTransformType->currentData().toInt() );
}

void QgsTransformSettingsDialog::setTransformMethod( QgsGcpTransformerInterface::TransformMethod method )
{
  if ( method == QgsGcpTransformerInterface::TransformMethod::InvalidTransform )
    cmbTransformType->setCurrentIndex( 0 );
  else
    cmbTransformType->setCurrentIndex( cmbTransformType->findData( static_cast< int >( method ) ) );
}

QgsImageWarper::ResamplingMethod QgsTransformSettingsDialog::resamplingMethod() const
{
  return static_cast< QgsImageWarper::ResamplingMethod >( cmbResampling->currentData().toInt() );
}

void QgsTransformSettingsDialog::setResamplingMethod( QgsImageWarper::ResamplingMethod method )
{
  cmbResampling->setCurrentIndex( cmbResampling->findData( static_cast< int >( method ) ) );
}

QString QgsTransformSettingsDialog::compressionMethod() const
{
  return cmbCompressionComboBox->currentData().toString();
}

void QgsTransformSettingsDialog::setCompressionMethod( const QString &method )
{
  cmbCompressionComboBox->setCurrentIndex( cmbCompressionComboBox->findData( method ) );
}

QString QgsTransformSettingsDialog::destinationFilename() const
{
  QgsFileWidget *outputFile = mType == QgsMapLayerType::RasterLayer ? mRasterOutputFile : mVectorOutputFile;
  return outputFile->filePath();
}

QString QgsTransformSettingsDialog::pdfReportFilename() const
{
  return mPdfReport->filePath();
}

void QgsTransformSettingsDialog::setPdfReportFilename( const QString &filename )
{
  mPdfReport->setFilePath( filename );
}

QString QgsTransformSettingsDialog::pdfMapFilename() const
{
  return mPdfMap->filePath();
}

void QgsTransformSettingsDialog::setPdfMapFilename( const QString &filename )
{
  mPdfMap->setFilePath( filename );
}

bool QgsTransformSettingsDialog::saveGcpPoints() const
{
  return saveGcpCheckBox->isChecked();
}

void QgsTransformSettingsDialog::setSaveGcpPoints( bool save )
{
  saveGcpCheckBox->setChecked( save );
}

bool QgsTransformSettingsDialog::useZeroForTransparent() const
{
  return cbxZeroAsTrans->isChecked();
}

void QgsTransformSettingsDialog::setUseZeroForTransparent( bool enabled )
{
  cbxZeroAsTrans->setChecked( enabled );
}

bool QgsTransformSettingsDialog::loadInProject() const
{
  return cbxLoadInProjectsWhenDone->isChecked();
}

void QgsTransformSettingsDialog::setLoadInProject( bool enabled )
{
  cbxLoadInProjectsWhenDone->setChecked( enabled );
}

void QgsTransformSettingsDialog::outputResolution(
  double &resX, double &resY )
{
  resX = 0.0;
  resY = 0.0;
  if ( cbxUserResolution->isChecked() )
  {
    resX = dsbHorizRes->value();
    resY = dsbVerticalRes->value();
  }
}

void QgsTransformSettingsDialog::setOutputResolution( double resX, double resY )
{
  cbxUserResolution->setChecked( !qgsDoubleNear( resX, 0 ) || !qgsDoubleNear( resY, 0 ) );
  dsbHorizRes->setValue( resX );
  dsbVerticalRes->setValue( resY );
}

void QgsTransformSettingsDialog::accept()
{
  QgsFileWidget *outputFile = mType == QgsMapLayerType::RasterLayer ? mRasterOutputFile : mVectorOutputFile;
  if ( !outputFile->filePath().isEmpty() )
  {
    //if the file path is relative, make it relative to the input file directory
    QString outputfilename = outputFile->filePath();
    QFileInfo sourceFileInfo( mSourceFile );
    QFileInfo outputFileInfo( sourceFileInfo.absoluteDir(), outputfilename );

    if ( outputFileInfo.fileName().isEmpty() || !outputFileInfo.dir().exists() )
    {
      QMessageBox::warning( this, tr( "Destination File" ), tr( "Invalid output file name." ) );
      return;
    }
    if ( outputFileInfo.filePath() == mSourceFile )
    {
      //can't overwrite input file
      QMessageBox::warning( this, tr( "Destination File" ), tr( "Input file can not be overwritten." ) );
      return;
    }
    outputFile->setFilePath( outputFileInfo.absoluteFilePath() );
  }

  QDialog::accept();
}

void QgsTransformSettingsDialog::cmbTransformType_currentIndexChanged( const QString & )
{
  if ( cmbTransformType->currentIndex() != -1
       && ( static_cast< QgsGcpTransformerInterface::TransformMethod >( cmbTransformType->currentData().toInt() ) == QgsGcpTransformerInterface::TransformMethod::Linear
            || static_cast< QgsGcpTransformerInterface::TransformMethod >( cmbTransformType->currentData().toInt() ) == QgsGcpTransformerInterface::TransformMethod::Helmert ) )
  {
    mWorldFileCheckBox->setEnabled( true );
  }
  else
  {
    // world file only option is only compatible with helmert/linear transforms
    mWorldFileCheckBox->setEnabled( false );
    mWorldFileCheckBox->setChecked( false );
  }
}

void QgsTransformSettingsDialog::mWorldFileCheckBox_stateChanged( int state )
{
  bool enableOutputRaster = true;
  if ( state == Qt::Checked )
  {
    enableOutputRaster = false;
  }
  label_2->setEnabled( enableOutputRaster );

  QgsFileWidget *outputFile = mType == QgsMapLayerType::RasterLayer ? mRasterOutputFile : mVectorOutputFile;
  outputFile->setEnabled( enableOutputRaster );
}

QString QgsTransformSettingsDialog::generateModifiedFileName( const QString &filename )
{
  if ( filename.isEmpty() )
    return QString();

  QString modifiedFileName = filename;
  QFileInfo modifiedFileInfo( modifiedFileName );
  int pos = modifiedFileName.size() - modifiedFileInfo.suffix().size() - 1;
  modifiedFileName.insert( pos, tr( "_modified", "QgsTransformSettingsDialog.cpp - used to modify a user given file name" ) );

  pos = modifiedFileName.size() - modifiedFileInfo.suffix().size();

  switch ( mType )
  {
    case QgsMapLayerType::VectorLayer:
      modifiedFileName.replace( pos, modifiedFileName.size(), QStringLiteral( "gpkg" ) );
      break;
    case QgsMapLayerType::RasterLayer:
      modifiedFileName.replace( pos, modifiedFileName.size(), QStringLiteral( "tif" ) );
      break;
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }


  return modifiedFileName;
}

void QgsTransformSettingsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_raster/georeferencer.html#defining-the-transformation-settings" ) );
}
