/***************************************************************************
     qgsvectortransformsettingsdialog.cpp
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
#include "qgsvectortransformsettingsdialog.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgui.h"
#include "qgshelp.h"

QgsVectorTransformSettingsDialog::QgsVectorTransformSettingsDialog( const QString &input, const QString &output,
    int countGCPpoints, const QString &outputFilters, QWidget *parent )
  : QDialog( parent )
  , mSourceFile( input )
  , mCountGCPpoints( countGCPpoints )
{
  setupUi( this );
  QgsSettings settings;
  QgsGui::instance()->enableAutoGeometryRestore( this );


  mOutputVector->setStorageMode( QgsFileWidget::SaveFile );
  if ( output.isEmpty() )
  {
    mOutputVector->setFilePath( generateModifiedOutputFileName( mSourceFile ) );
  }
  else
  {
    mOutputVector->setFilePath( output );
  }

  mOutputVector->setStorageMode( QgsFileWidget::SaveFile );
  mOutputVector->setFilter( outputFilters );
  mOutputVector->setConfirmOverwrite( false );
  mOutputVector->setDialogTitle( tr( "Destination" ) );
  mOutputVector->setDefaultRoot( settings.value( QStringLiteral( "UI/lastVectorFileFilterDir" ), QDir::homePath() ).toString() );
  connect( mOutputVector, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( mOutputVector->filePath() );
    settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), tmplFileInfo.absolutePath() );
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

  connect( cmbTransformType, &QComboBox::currentTextChanged, this, &QgsVectorTransformSettingsDialog::cmbTransformType_currentIndexChanged );
  connect( mWorldFileCheckBox, &QCheckBox::stateChanged, this, &QgsVectorTransformSettingsDialog::mWorldFileCheckBox_stateChanged );

  cmbTransformType->addItem( tr( "Linear" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::Linear ) );
  cmbTransformType->addItem( tr( "Helmert" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::Helmert ) );
  cmbTransformType->addItem( tr( "Polynomial 1" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::PolynomialOrder1 ) );
  cmbTransformType->addItem( tr( "Polynomial 2" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::PolynomialOrder2 ) );
  cmbTransformType->addItem( tr( "Polynomial 3" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::PolynomialOrder3 ) );
  cmbTransformType->addItem( tr( "Thin Plate Spline" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::ThinPlateSpline ) );
  cmbTransformType->addItem( tr( "Projective" ), static_cast<int>( QgsGcpTransformerInterface::TransformMethod::Projective ) );


  QString targetCRSString = settings.value( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ) ).toString();
  QgsCoordinateReferenceSystem targetCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( targetCRSString );
  mCrsSelector->setCrs( targetCRS );

  mWorldFileCheckBox->setChecked( settings.value( QStringLiteral( "/Plugin-Georeferencer/word_file_checkbox" ), false ).toBool() );

  cbxLoadInQgisWhenDone->setChecked( settings.value( QStringLiteral( "/Plugin-GeoReferencer/loadinqgis" ), false ).toBool() );
  saveGcpCheckBox->setChecked( settings.value( QStringLiteral( "/Plugin-GeoReferencer/save_gcp_points" ), false ).toBool() );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorTransformSettingsDialog::showHelp );
}

void QgsVectorTransformSettingsDialog::getTransformSettings( QgsGeorefTransform::TransformMethod &tp,
    QString &output, QgsCoordinateReferenceSystem &proj, QString &pdfMapFile,
    QString &pdfReportFile, QString &gcpPoints, bool &zt, bool &loadInQgis,
    double &resX, double &resY )
{
  if ( cmbTransformType->currentIndex() == -1 )
    tp = QgsGcpTransformerInterface::TransformMethod::InvalidTransform;
  else
    tp = static_cast< QgsGcpTransformerInterface::TransformMethod >( cmbTransformType->currentData().toInt() );

  if ( mWorldFileCheckBox->isChecked() )
  {
    output.clear();
  }
  else
  {
    output = mOutputVector->filePath();
  }
  proj = mCrsSelector->crs();
  pdfMapFile = mPdfMap->filePath();
  pdfReportFile = mPdfReport->filePath();
  loadInQgis = cbxLoadInQgisWhenDone->isChecked();

  if ( saveGcpCheckBox->isChecked() )
  {
    gcpPoints = mOutputVector->filePath();
  }
}

void QgsVectorTransformSettingsDialog::resetSettings()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/lastvectortransformation" ), -1 );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ), QString() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/loadinqgis" ), false );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/save_gcp_points" ), false );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/word_file_checkbox" ), false );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/lastPDFReportDir" ), QDir::homePath() );
}

void QgsVectorTransformSettingsDialog::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  switch ( e->type() )
  {
    case QEvent::LanguageChange:
      retranslateUi( this );
      break;
    default:
      break;
  }
}

void QgsVectorTransformSettingsDialog::accept()
{
  if ( !mOutputVector->filePath().isEmpty() )
  {
    //if the file path is relative, make it relative to the  file directory
    QString outputVectorName = mOutputVector->filePath();
    QFileInfo FileInfo( mSourceFile );
    QFileInfo outputFileInfo( FileInfo.absoluteDir(), outputVectorName );

    if ( outputFileInfo.fileName().isEmpty() || !outputFileInfo.dir().exists() )
    {
      QMessageBox::warning( this, tr( "Destination" ), tr( "Invalid output file name." ) );
      return;
    }
    if ( outputFileInfo.filePath() == mSourceFile )
    {
      //can't overwrite input file
      QMessageBox::warning( this, tr( "Destination" ), tr( "Input file can not be overwritten." ) );
      return;
    }
    mOutputVector->setFilePath( outputFileInfo.absoluteFilePath() );
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/lasttransformation" ), cmbTransformType->currentIndex() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ), mCrsSelector->crs().authid() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/loadinqgis" ), cbxLoadInQgisWhenDone->isChecked() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/word_file_checkbox" ), mWorldFileCheckBox->isChecked() );
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/save_gcp_points" ), saveGcpCheckBox->isChecked() );


  QDialog::accept();
}

void QgsVectorTransformSettingsDialog::cmbTransformType_currentIndexChanged( const QString &text )
{
  if ( text == tr( "Linear" ) )
  {
    mWorldFileCheckBox->setEnabled( true );
  }
  else
  {
    mWorldFileCheckBox->setEnabled( false );
    // reset world file checkbox when transformation differ from Linear
    mWorldFileCheckBox->setChecked( false );
  }
}

void QgsVectorTransformSettingsDialog::mWorldFileCheckBox_stateChanged( int state )
{
  bool enableOutput = true;
  if ( state == Qt::Checked )
  {
    enableOutput = false;
  }
  label_2->setEnabled( enableOutput );
  mOutputVector->setEnabled( enableOutput );
}

bool QgsVectorTransformSettingsDialog::checkGCPpoints( int count, int &minGCPpoints )
{
  QgsGeorefTransform georefTransform;
  georefTransform.selectTransformParametrisation( ( QgsGeorefTransform::TransformMethod )count );
  minGCPpoints = georefTransform.minimumGCPCount();
  return ( mCountGCPpoints >= minGCPpoints );
}

QString QgsVectorTransformSettingsDialog::generateModifiedOutputFileName( const QString &filename )
{
  if ( filename.isEmpty() )
    return QString();

  QString modifiedFileName = filename;
  int lastDot = filename.lastIndexOf( "." );
  if ( filename.length() - lastDot <= 4 )
    modifiedFileName = modifiedFileName.insert( lastDot, tr( "_modified" ) );
  else
    modifiedFileName.append( tr( "_modified" ) );

  return modifiedFileName;
}

// Note this code is duplicated from qgisapp.cpp because
// I didn't want to make plugins on qgsapplication [TS]
QIcon QgsVectorTransformSettingsDialog::getThemeIcon( const QString &name )
{
  if ( QFile::exists( QgsApplication::activeThemePath() + name ) )
  {
    return QIcon( QgsApplication::activeThemePath() + name );
  }
  else if ( QFile::exists( QgsApplication::defaultThemePath() + name ) )
  {
    return QIcon( QgsApplication::defaultThemePath() + name );
  }
  else
  {
    QgsSettings settings;
    QString themePath = ":/icons/" + settings.value( QStringLiteral( "Themes" ) ).toString() + name;
    if ( QFile::exists( themePath ) )
    {
      return QIcon( themePath );
    }
    else
    {
      return QIcon( ":/icons/default" + name );
    }
  }
}

void QgsVectorTransformSettingsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_raster/georeferencer.html#defining-the-transformation-settings" ) );
}
