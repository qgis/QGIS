/***************************************************************************
  qgsnewmeshlayerdialog.cpp - QgsNewMeshLayerDialog

 ---------------------
 begin                : 22.6.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnewmeshlayerdialog.h"

#include <QPushButton>
#include <QMessageBox>

#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsmeshdataprovider.h"
#include "qgsproject.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"


QgsNewMeshLayerDialog::QgsNewMeshLayerDialog( QWidget *parent, Qt::WindowFlags fl ) : QDialog( parent, fl )
{
  QgsProviderMetadata *meta = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "mdal" ) );

  if ( !meta )
  {
    setLayout( new QVBoxLayout );
    layout()->addWidget( new QLabel( tr( "MDAL not available, unable to create a new mesh layer" ) ) );
    return;
  }

  setupUi( this );
  const QList<QgsMeshDriverMetadata> driverList = meta->meshDriversMetadata();

  for ( const QgsMeshDriverMetadata &driverMeta : driverList )
    if ( driverMeta.capabilities() & QgsMeshDriverMetadata::CanWriteMeshData )
    {
      mFormatComboBox->addItem( driverMeta.description(), driverMeta.name() );
    }

  mFormatComboBox->setCurrentIndex( -1 );
  mFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mMeshProjectComboBox->setFilters( QgsMapLayerProxyModel::MeshLayer );

  connect( mFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mEmptyMeshRadioButton, &QRadioButton::toggled, this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mMeshFileRadioButton, &QRadioButton::toggled, this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mMeshFromFileWidget, &QgsFileWidget::fileChanged, this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mMeshProjectComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsNewMeshLayerDialog::updateDialog );

  updateDialog();

}

void QgsNewMeshLayerDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mProjectionSelectionWidget->setCrs( crs );
}

void QgsNewMeshLayerDialog::setSourceMeshLayer( QgsMeshLayer *meshLayer )
{
  mMeshProjectComboBox->setLayer( meshLayer );
  mMeshProjectRadioButton->setChecked( true );
}

void QgsNewMeshLayerDialog::accept()
{
  if ( apply() )
    QDialog::accept();
}

void QgsNewMeshLayerDialog::updateDialog()
{
  updateSourceMeshframe();

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled(
    ! mFileWidget->filePath().isEmpty() &&
    mFormatComboBox->currentIndex() != -1 &&
    mSourceMeshFrameReady );
}

void QgsNewMeshLayerDialog::updateSourceMeshframe()
{
  mMeshProjectComboBox->setEnabled( false );
  mMeshFromFileWidget->setEnabled( false );
  if ( mEmptyMeshRadioButton->isChecked() )
  {
    mSourceMeshFromFile.reset();
    mSourceMeshFrameReady = true;
    mProjectionSelectionWidget->setEnabled( true );
  }
  else if ( mMeshProjectRadioButton->isChecked() )
  {
    mMeshProjectComboBox->setEnabled( true );
    mSourceMeshFromFile.reset();
    mSourceMeshFrameReady = mMeshProjectComboBox->currentLayer() != nullptr;
    mProjectionSelectionWidget->setEnabled( false );
  }
  else if ( mMeshFileRadioButton->isChecked() )
  {
    mMeshFromFileWidget->setEnabled( true );
    if ( !mSourceMeshFromFile || mSourceMeshFromFile->source() != mMeshFromFileWidget->filePath() )
    {
      QgsApplication::setOverrideCursor( Qt::WaitCursor );
      if ( !mMeshFromFileWidget->filePath().isEmpty() )
        mSourceMeshFromFile.reset( new QgsMeshLayer( mMeshFromFileWidget->filePath(), QString(), QStringLiteral( "mdal" ) ) );

      if ( mSourceMeshFromFile && !mSourceMeshFromFile->isValid() )
        mSourceMeshFromFile.reset();

      mProjectionSelectionWidget->setEnabled( false );

      mSourceMeshFrameReady = mSourceMeshFromFile != nullptr;

      QgsApplication::restoreOverrideCursor();
    }
  }

  updateSourceMeshInformation();
}

void QgsNewMeshLayerDialog::updateSourceMeshInformation()
{
  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );

  mInformationTextBrowser->clear();
  mInformationTextBrowser->document()->setDefaultStyleSheet( myStyle );
  if ( mMeshProjectRadioButton->isChecked() )
  {
    if ( mMeshProjectComboBox->currentLayer() )
      mInformationTextBrowser->setHtml( mMeshProjectComboBox->currentLayer()->htmlMetadata() );
  }

  if ( mMeshFileRadioButton->isChecked() )
  {
    if ( mSourceMeshFromFile )
      mInformationTextBrowser->setHtml( mSourceMeshFromFile->htmlMetadata() );
  }

  mInformationTextBrowser->setOpenLinks( false );
};

bool QgsNewMeshLayerDialog::apply()
{
  bool result = false;
  QString fileName = mFileWidget->filePath();
  QString format = mFormatComboBox->currentData().toString();

  QgsMesh mesh;
  QgsCoordinateReferenceSystem crs;

  QgsMeshLayer *source = nullptr;

  if ( mEmptyMeshRadioButton->isChecked() )
  {
    crs = mProjectionSelectionWidget->crs();
  }
  else if ( mMeshProjectRadioButton->isChecked() )
  {
    source = qobject_cast<QgsMeshLayer *>( mMeshProjectComboBox->currentLayer() );
  }
  else if ( mMeshFromFileWidget )
  {
    source = mSourceMeshFromFile.get();
  }

  if ( source )
  {
    crs = source->crs();
    source->dataProvider()->populateMesh( &mesh );
  }

  const QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "mdal" ) );
  if ( providerMetadata )
  {
    result = providerMetadata->createMeshData( mesh, fileName, format, crs );
    if ( result )
    {
      std::unique_ptr<QgsMeshLayer> newMeshLayer = std::make_unique<QgsMeshLayer>( fileName, mLayerNameLineEdit->text(), QStringLiteral( "mdal" ) );

      if ( newMeshLayer->isValid() )
        QgsProject::instance()->addMapLayer( newMeshLayer.release(), true, true );
      return true;
    }
  }

  QMessageBox::warning( this, windowTitle(), tr( "Unable to create a new mesh layer" ) );
  return false;
}

