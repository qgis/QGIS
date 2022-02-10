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

#include "qgssettings.h"
#include "qgsprojectionselectiontreewidget.h"
#include "qgsapplication.h"
#include "qgsfilewidget.h"
#include "qgsrasterlayer.h"
#include "qgstransformsettingsdialog.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgui.h"
#include "qgshelp.h"

QgsTransformSettingsDialog::QgsTransformSettingsDialog( const QString &raster, const QString &output, QWidget *parent )
  : QDialog( parent )
  , mSourceRasterFile( raster )
{
  setupUi( this );
  QgsSettings settings;
  QgsGui::enableAutoGeometryRestore( this );


  mOutputRaster->setStorageMode( QgsFileWidget::SaveFile );
  if ( output.isEmpty() )
  {
    mOutputRaster->setFilePath( generateModifiedRasterFileName( mSourceRasterFile ) );
  }
  else
  {
    mOutputRaster->setFilePath( output );
  }
  mOutputRaster->setFilter( tr( "TIF files" ) + " (*.tif *.tiff *.TIF *.TIFF)" );
  mOutputRaster->setDialogTitle( tr( "Destination Raster" ) );
  mOutputRaster->setDefaultRoot( settings.value( QStringLiteral( "UI/lastRasterFileFilterDir" ), QDir::homePath() ).toString() );
  connect( mOutputRaster, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( mOutputRaster->filePath() );
    settings.setValue( QStringLiteral( "UI/lastRasterFileFilterDir" ), tmplFileInfo.absolutePath() );
  } );

  mPdfMap->setStorageMode( QgsFileWidget::SaveFile );
  mPdfMap->setFilter( tr( "PDF files" ) + " (*.pdf *.PDF)" );
  mPdfMap->setDialogTitle( tr( "Save Map File As" ) );
  mPdfMap->setDefaultRoot( settings.value( QStringLiteral( "/Plugin-GeoReferencer/lastPDFReportDir" ), QDir::homePath() ).toString() );
  connect( mPdfMap, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( mPdfMap->filePath() );
    settings.setValue( QStringLiteral( "Plugin-GeoReferencer/lastPDFReportDir" ), tmplFileInfo.absolutePath() );
  } );

  mPdfReport->setStorageMode( QgsFileWidget::SaveFile );
  mPdfReport->setFilter( tr( "PDF files" ) + " (*.pdf *.PDF)" );
  mPdfReport->setDialogTitle( tr( "Save Report File As" ) );
  mPdfReport->setDefaultRoot( settings.value( QStringLiteral( "/Plugin-GeoReferencer/lastPDFReportDir" ), QDir::homePath() ).toString() );
  connect( mPdfReport, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( mPdfReport->filePath() );
    settings.setValue( QStringLiteral( "Plugin-GeoReferencer/lastPDFReportDir" ), tmplFileInfo.absolutePath() );
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

  cmbResampling->addItem( tr( "Nearest Neighbour" ), QgsImageWarper::ResamplingMethod::NearestNeighbour );
  cmbResampling->addItem( tr( "Linear" ), QgsImageWarper::ResamplingMethod::Bilinear );
  cmbResampling->addItem( tr( "Cubic" ), QgsImageWarper::ResamplingMethod::Cubic );
  cmbResampling->addItem( tr( "Cubic Spline" ), QgsImageWarper::ResamplingMethod::CubicSpline );
  cmbResampling->addItem( tr( "Lanczos" ), QgsImageWarper::ResamplingMethod::Lanczos );

  cmbTransformType->setCurrentIndex( settings.value( QStringLiteral( "/Plugin-GeoReferencer/lasttransformation" ), -1 ).toInt() );
  cmbResampling->setCurrentIndex( settings.value( QStringLiteral( "/Plugin-GeoReferencer/lastresampling" ), 0 ).toInt() );
  cmbCompressionComboBox->setCurrentIndex( settings.value( QStringLiteral( "/Plugin-GeoReferencer/lastcompression" ), 0 ).toInt() );

  cbxUserResolution->setChecked( settings.value( QStringLiteral( "/Plugin-Georeferencer/user_specified_resolution" ), false ).toBool() );
  bool ok;
  dsbHorizRes->setValue( settings.value( QStringLiteral( "/Plugin-GeoReferencer/user_specified_resx" ), .0 ).toDouble( &ok ) );
  if ( !ok )
    dsbHorizRes->setValue( 1.0 );
  dsbVerticalRes->setValue( settings.value( QStringLiteral( "/Plugin-GeoReferencer/user_specified_resy" ), -1.0 ).toDouble( &ok ) );
  if ( !ok )
    dsbHorizRes->setValue( -1.0 );

  cbxZeroAsTrans->setChecked( settings.value( QStringLiteral( "/Plugin-GeoReferencer/zeroastrans" ), false ).toBool() );
  cbxLoadInQgisWhenDone->setChecked( settings.value( QStringLiteral( "/Plugin-GeoReferencer/loadinqgis" ), false ).toBool() );
  saveGcpCheckBox->setChecked( settings.value( QStringLiteral( "/Plugin-GeoReferencer/save_gcp_points" ), false ).toBool() );

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

void QgsTransformSettingsDialog::getTransformSettings( QgsGeorefTransform::TransformMethod &tp,
    QgsImageWarper::ResamplingMethod &rm,
    QString &comprMethod, QString &raster, QString &pdfMapFile, QString &pdfReportFile, bool &saveGcpPoints, bool &zt, bool &loadInQgis,
    double &resX, double &resY )
{
  if ( cmbTransformType->currentIndex() == -1 )
    tp = QgsGcpTransformerInterface::TransformMethod::InvalidTransform;
  else
    tp = static_cast< QgsGcpTransformerInterface::TransformMethod >( cmbTransformType->currentData().toInt() );

  rm = static_cast< QgsImageWarper::ResamplingMethod >( cmbResampling->currentData().toInt() );
  comprMethod = cmbCompressionComboBox->currentData().toString();
  raster = mOutputRaster->filePath();

  pdfMapFile = mPdfMap->filePath();
  pdfReportFile = mPdfReport->filePath();
  zt = cbxZeroAsTrans->isChecked();
  loadInQgis = cbxLoadInQgisWhenDone->isChecked();
  resX = 0.0;
  resY = 0.0;
  if ( cbxUserResolution->isChecked() )
  {
    resX = dsbHorizRes->value();
    resY = dsbVerticalRes->value();
  }
  saveGcpPoints = saveGcpCheckBox->isChecked();
}

void QgsTransformSettingsDialog::accept()
{
  if ( !mOutputRaster->filePath().isEmpty() )
  {
    //if the file path is relative, make it relative to the raster file directory
    QString outputRasterName = mOutputRaster->filePath();
    QFileInfo rasterFileInfo( mSourceRasterFile );
    QFileInfo outputFileInfo( rasterFileInfo.absoluteDir(), outputRasterName );

    if ( outputFileInfo.fileName().isEmpty() || !outputFileInfo.dir().exists() )
    {
      QMessageBox::warning( this, tr( "Destination Raster" ), tr( "Invalid output file name." ) );
      return;
    }
    if ( outputFileInfo.filePath() == mSourceRasterFile )
    {
      //can't overwrite input file
      QMessageBox::warning( this, tr( "Destination Raster" ), tr( "Input raster can not be overwritten." ) );
      return;
    }
    mOutputRaster->setFilePath( outputFileInfo.absoluteFilePath() );
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/lasttransformation" ), cmbTransformType->currentIndex() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/lastresampling" ), cmbResampling->currentIndex() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/lastcompression" ), cmbCompressionComboBox->currentIndex() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ), mCrsSelector->crs().authid() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/zeroastrans" ), cbxZeroAsTrans->isChecked() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/loadinqgis" ), cbxLoadInQgisWhenDone->isChecked() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/user_specified_resolution" ), cbxUserResolution->isChecked() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/user_specified_resx" ), dsbHorizRes->value() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/user_specified_resy" ), dsbVerticalRes->value() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/save_gcp_points" ), saveGcpCheckBox->isChecked() );

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
  mOutputRaster->setEnabled( enableOutputRaster );
}

QString QgsTransformSettingsDialog::generateModifiedRasterFileName( const QString &raster )
{
  if ( raster.isEmpty() )
    return QString();

  QString modifiedFileName = raster;
  QFileInfo modifiedFileInfo( modifiedFileName );
  int pos = modifiedFileName.size() - modifiedFileInfo.suffix().size() - 1;
  modifiedFileName.insert( pos, tr( "_modified", "Georeferencer:QgsOpenRasterDialog.cpp - used to modify a user given file name" ) );

  pos = modifiedFileName.size() - modifiedFileInfo.suffix().size();
  modifiedFileName.replace( pos, modifiedFileName.size(), QStringLiteral( "tif" ) );

  return modifiedFileName;
}

void QgsTransformSettingsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_raster/georeferencer.html#defining-the-transformation-settings" ) );
}
