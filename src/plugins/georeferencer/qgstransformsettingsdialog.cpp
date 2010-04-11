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
/* $Id$ */

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

#include "qgsprojectionselector.h"

#include "qgsapplication.h"
#include "qgsrasterlayer.h"

#include "qgstransformsettingsdialog.h"

QgsTransformSettingsDialog::QgsTransformSettingsDialog( const QString &raster, const QString &output,
    int countGCPpoints, QWidget *parent )
    : QDialog( parent )
    , mModifiedRaster( raster )
    , mCountGCPpoints( countGCPpoints )
{
  setupUi( this );

  cmbTransformType->addItem( tr( "Linear" ) ,           (int)QgsGeorefTransform::Linear ) ;
  cmbTransformType->addItem( tr( "Helmert" ),           (int)QgsGeorefTransform::Helmert );
  cmbTransformType->addItem( tr( "Polynomial 1" ),      (int)QgsGeorefTransform::PolynomialOrder1 );
  cmbTransformType->addItem( tr( "Polynomial 2" ),      (int)QgsGeorefTransform::PolynomialOrder2 );
  cmbTransformType->addItem( tr( "Polynomial 3" ),      (int)QgsGeorefTransform::PolynomialOrder3 );
  cmbTransformType->addItem( tr( "Thin Plate Spline" ), (int)QgsGeorefTransform::ThinPlateSpline );
  
  leOutputRaster->setText( output );

  mRegExpValidator = new QRegExpValidator( QRegExp( "(^epsg:{1}\\s*\\d+)|(^\\+proj.*)", Qt::CaseInsensitive ), leTargetSRS );
  leTargetSRS->setValidator( mRegExpValidator );

  QSettings s;
  cmbTransformType->setCurrentIndex( s.value( "/Plugin-GeoReferencer/lasttransformation", -1 ).toInt() );
  cmbResampling->setCurrentIndex( s.value( "/Plugin-GeoReferencer/lastresampling", 0 ).toInt() );
  cmbCompressionComboBox->setCurrentIndex( s.value( "/Plugin-GeoReferencer/lastcompression", 0 ).toInt() );
  leTargetSRS->setText( s.value( "/Plugin-GeoReferencer/targetsrs" ).toString() );

  cbxUserResolution->setChecked( s.value( "/Plugin-Georeferencer/user_specified_resolution", false ).toBool() );
  bool ok;
  dsbHorizRes->setValue( s.value( "/Plugin-GeoReferencer/user_specified_resx",     1.0 ).toDouble( &ok ) );
  if ( !ok ) dsbHorizRes->setValue( 1.0 );
  dsbVerticalRes->setValue( s.value( "/Plugin-GeoReferencer/user_specified_resy", -1.0 ).toDouble( &ok ) );
  if ( !ok ) dsbHorizRes->setValue( -1.0 );
  // Activate spin boxes for vertical/horizontal resolution, if the option is checked
  dsbHorizRes->setEnabled( cbxUserResolution->isChecked() );
  dsbVerticalRes->setEnabled( cbxUserResolution->isChecked() );
  // Update activation of spinboxes, if the user specified resolution is checked/unchecked
  connect( cbxUserResolution, SIGNAL( toggled( bool ) ), dsbHorizRes, SLOT( setEnabled( bool ) ) );
  connect( cbxUserResolution, SIGNAL( toggled( bool ) ), dsbVerticalRes, SLOT( setEnabled( bool ) ) );

  cbxZeroAsTrans->setChecked( s.value( "/Plugin-GeoReferencer/zeroastrans", false ).toBool() );
  cbxLoadInQgisWhenDone->setChecked( s.value( "/Plugin-GeoReferencer/loadinqgis", false ).toBool() );

  tbnOutputRaster->setIcon( getThemeIcon( "/mPushButtonFileOpen.png" ) );
  tbnTargetSRS->setIcon( getThemeIcon( "/mPushButtonTargetSRSDisabled.png" ) );
}

void QgsTransformSettingsDialog::getTransformSettings( QgsGeorefTransform::TransformParametrisation &tp,
    QgsImageWarper::ResamplingMethod &rm,
    QString &comprMethod, QString &raster,
    QString &proj, bool &zt, bool &loadInQgis,
    double& resX, double& resY )
{
  if ( cmbTransformType->currentIndex() == -1 )
    tp = QgsGeorefTransform::InvalidTransform;
  else
    tp = ( QgsGeorefTransform::TransformParametrisation )cmbTransformType->currentIndex();

  rm = ( QgsImageWarper::ResamplingMethod )cmbResampling->currentIndex();
  comprMethod = cmbCompressionComboBox->currentText();
  raster = leOutputRaster->text();
  proj = leTargetSRS->text();
  zt = cbxZeroAsTrans->isChecked();
  loadInQgis = cbxLoadInQgisWhenDone->isChecked();
  resX = 0.0;
  resY = 0.0;
  if ( cbxUserResolution->isChecked() )
  {
    resX = dsbHorizRes->value();
    resY = dsbVerticalRes->value();
  }
}

void QgsTransformSettingsDialog::resetSettings()
{
  QSettings s;
  s.setValue( "/Plugin-GeoReferencer/lasttransformation", -1 );
  s.setValue( "/Plugin-GeoReferencer/lastresampling", 0 );
  s.setValue( "/Plugin-GeoReferencer/lastcompression", 0 );
  s.setValue( "/Plugin-GeoReferencer/targetsrs", QString() );
  s.setValue( "/Plugin-GeoReferencer/zeroastrans", false );
  s.setValue( "/Plugin-GeoReferencer/loadinqgis", false );
  s.setValue( "/Plugin-GeoReferencer/user_specified_resolution", false );
  s.setValue( "/Plugin-GeoReferencer/user_specified_resx",  1.0 );
  s.setValue( "/Plugin-GeoReferencer/user_specified_resy", -1.0 );
}

void QgsTransformSettingsDialog::changeEvent( QEvent *e )
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

void QgsTransformSettingsDialog::accept()
{
  int minGCPpoints;
  if ( checkGCPpoints( cmbTransformType->currentIndex(), minGCPpoints ) )
  {
    if ( leOutputRaster->text().isEmpty() && cmbTransformType->currentIndex() != 0 )
    {
      QMessageBox::information( this, tr( "Info" ), tr( "Please set output name" ) );
      return;
    }
    QDialog::accept();
  }
  else
  {
    QMessageBox::information( this, tr( "Info" ), tr( "%1 requires at least %2 GCPs. Please define more" )
                              .arg( cmbTransformType->currentText() ).arg( minGCPpoints ) );
    QSettings s;
    cmbTransformType->setCurrentIndex( s.value( "/Plugin-GeoReferencer/lasttransformation", -1 ).toInt() );
    return;
  }

  QSettings s;
  s.setValue( "/Plugin-GeoReferencer/lasttransformation", cmbTransformType->currentIndex() );
  s.setValue( "/Plugin-GeoReferencer/lastresampling", cmbResampling->currentIndex() );
  s.setValue( "/Plugin-GeoReferencer/lastcompression", cmbCompressionComboBox->currentIndex() );
  s.setValue( "/Plugin-GeoReferencer/targetsrs", leTargetSRS->text() );
  s.setValue( "/Plugin-GeoReferencer/zeroastrans", cbxZeroAsTrans->isChecked() );
  s.setValue( "/Plugin-GeoReferencer/loadinqgis", cbxLoadInQgisWhenDone->isChecked() );
  s.setValue( "/Plugin-GeoReferencer/user_specified_resolution", cbxUserResolution->isChecked() );
  s.setValue( "/Plugin-GeoReferencer/user_specified_resx", dsbHorizRes->value() );
  s.setValue( "/Plugin-GeoReferencer/user_specified_resy", dsbVerticalRes->value() );
}

void QgsTransformSettingsDialog::on_tbnOutputRaster_clicked()
{
  QString selectedFile = generateModifiedRasterFileName( mModifiedRaster );
  QString rasterFileName = QFileDialog::getSaveFileName( this, tr( "Save raster" ),
                           selectedFile, "GeoTIFF (*.tif *.tiff *.TIF *.TIFF)" );

  if ( rasterFileName.isEmpty() )
    return;

  leOutputRaster->setText( rasterFileName );
  leOutputRaster->setToolTip( rasterFileName );
}

void QgsTransformSettingsDialog::on_tbnTargetSRS_clicked()
{
  QDialog srsSelector;
  QVBoxLayout *layout = new QVBoxLayout;
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Close );

  QgsProjectionSelector *projSelector = new QgsProjectionSelector( 0 );
  layout->addWidget( projSelector );
  layout->addWidget( buttonBox );
  srsSelector.setLayout( layout );

  connect( buttonBox, SIGNAL( accepted() ), &srsSelector, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), &srsSelector, SLOT( reject() ) );

  if ( srsSelector.exec() )
  {
    QString srs;
    // If the selected target SRS has an EPSG ID, use this as identification
    if ( projSelector->selectedAuthId().isEmpty() )
    {
      srs = projSelector->selectedProj4String();
    }
    else
    {
      srs = projSelector->selectedAuthId();
    }
    leTargetSRS->setText( srs );
  }
}

void QgsTransformSettingsDialog::on_leTargetSRS_textChanged( const QString &text )
{
  QString t = text;
  int s = t.size();
  if ( text.isEmpty() )
  {
    tbnTargetSRS->setIcon( getThemeIcon( "/mPushButtonTargetSRSDisabled.png" ) );
  }
  else if ( mRegExpValidator->validate( t, s ) == QValidator::Acceptable )
  {
    tbnTargetSRS->setIcon( getThemeIcon( "/mPushButtonTargetSRSEnabled.png" ) );
  }
}

bool QgsTransformSettingsDialog::checkGCPpoints( int count, int &minGCPpoints )
{
  QgsGeorefTransform georefTransform;
  georefTransform.selectTransformParametrisation(( QgsGeorefTransform::TransformParametrisation )count );
  minGCPpoints = georefTransform.getMinimumGCPCount();
  return ( mCountGCPpoints >= minGCPpoints );
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
  modifiedFileName.replace( pos, modifiedFileName.size(), "tif" );

  return modifiedFileName;
}

// Note this code is duplicated from qgisapp.cpp because
// I didnt want to make plugins on qgsapplication [TS]
QIcon QgsTransformSettingsDialog::getThemeIcon( const QString &theName )
{
  if ( QFile::exists( QgsApplication::activeThemePath() + theName ) )
  {
    return QIcon( QgsApplication::activeThemePath() + theName );
  }
  else if ( QFile::exists( QgsApplication::defaultThemePath() + theName ) )
  {
    return QIcon( QgsApplication::defaultThemePath() + theName );
  }
  else
  {
    return QIcon( ":/icons" + theName );
  }
}
