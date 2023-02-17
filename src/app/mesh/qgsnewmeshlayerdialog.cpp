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
#include "qgshelp.h"
#include "qgsgui.h"


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
  QgsGui::enableAutoGeometryRestore( this );
  const QList<QgsMeshDriverMetadata> driverList = meta->meshDriversMetadata();

  for ( const QgsMeshDriverMetadata &driverMeta : driverList )
    if ( driverMeta.capabilities() & QgsMeshDriverMetadata::CanWriteMeshData )
    {
      const QString description = driverMeta.description();
      const QString driverName = driverMeta.name();
      const QString suffix = driverMeta.writeMeshFrameOnFileSuffix();
      mFormatComboBox->addItem( description, driverName );
      mDriverSuffixes.insert( driverMeta.name(), suffix );
      mDriverFileFilters.insert( driverMeta.name(), tr( "%1" ).arg( description ) + QStringLiteral( " (*." ) + suffix + ')' );
    }

  const QStringList filters = mDriverFileFilters.values();
  mFormatComboBox->setCurrentIndex( -1 );
  mFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mFileWidget->setFilter( filters.join( QLatin1String( ";;" ) ) );
  mMeshProjectComboBox->setFilters( QgsMapLayerProxyModel::MeshLayer );

  connect( mFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsNewMeshLayerDialog::onFormatChanged );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsNewMeshLayerDialog::onFilePathChanged );
  connect( mInitializeMeshGroupBox, &QGroupBox::toggled, this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mMeshFileRadioButton, &QRadioButton::toggled, this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mMeshFromFileWidget, &QgsFileWidget::fileChanged, this, &QgsNewMeshLayerDialog::updateDialog );
  connect( mMeshProjectComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsNewMeshLayerDialog::updateDialog );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-mesh-layer" ) );
  } );

  updateDialog();
}

void QgsNewMeshLayerDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mProjectionSelectionWidget->setCrs( crs );
}

void QgsNewMeshLayerDialog::setSourceMeshLayer( QgsMeshLayer *meshLayer, bool fromExistingAsDefault )
{
  mMeshProjectComboBox->setLayer( meshLayer );
  mMeshProjectRadioButton->setChecked( true );
  mInitializeMeshGroupBox->setChecked( fromExistingAsDefault );
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
  if ( !mInitializeMeshGroupBox->isChecked() )
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

void QgsNewMeshLayerDialog::onFormatChanged()
{
  const QString currentDriverName = mFormatComboBox->currentData().toString();
  if ( currentDriverName.isEmpty() )
    return;

  const QString currentFilter = mDriverFileFilters.value( currentDriverName );
  mFileWidget->setSelectedFilter( currentFilter );

  const QString newSuffix = mDriverSuffixes.value( currentDriverName );

  QString currentFilePath = mFileWidget->filePath();
  if ( currentFilePath.isEmpty() )
    return;
  const QFileInfo fileInfo( currentFilePath );
  const QString currentSuffix = fileInfo.suffix();

  if ( !currentSuffix.isEmpty() )
    currentFilePath =  currentFilePath.mid( 0, currentFilePath.lastIndexOf( '.' ) );

  if ( currentFilePath.right( 1 ) == QString( '.' ) )
    currentFilePath.remove( currentFilePath.count() - 1, 1 );

  currentFilePath.append( '.' + newSuffix );

  mFileWidget->setFilePath( currentFilePath );

  updateDialog();
}

void QgsNewMeshLayerDialog::onFilePathChanged()
{
  const QFileInfo fileInfo( mFileWidget->filePath() );
  const QString &currentSuffix = fileInfo.suffix();

  const QStringList drivers = mDriverSuffixes.keys();
  for ( const QString &driverName : drivers )
  {
    if ( mDriverSuffixes.value( driverName ) == currentSuffix )
    {
      whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( driverName ) );
    }
  }

  updateDialog();
}

void QgsNewMeshLayerDialog::updateSourceMeshInformation()
{
  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );

  mInformationTextBrowser->clear();
  mInformationTextBrowser->document()->setDefaultStyleSheet( myStyle );
  if ( mInitializeMeshGroupBox->isChecked() )
  {
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
  }
};

bool QgsNewMeshLayerDialog::apply()
{
  bool result = false;
  const QString fileName = mFileWidget->filePath();
  const QString format = mFormatComboBox->currentData().toString();

  QgsMesh mesh;
  QgsCoordinateReferenceSystem crs;

  QgsMeshLayer *source = nullptr;

  if ( !mInitializeMeshGroupBox->isChecked() )
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
      QString layerName = mLayerNameLineEdit->text();
      if ( layerName.isEmpty() )
      {
        layerName = fileName;
        QFileInfo fileInfo( fileName );
        layerName = fileInfo.completeBaseName();
      }
      std::unique_ptr<QgsMeshLayer> newMeshLayer = std::make_unique<QgsMeshLayer>( fileName, layerName, QStringLiteral( "mdal" ) );

      if ( newMeshLayer->crs() != crs )
        newMeshLayer->setCrs( crs );

      if ( newMeshLayer->isValid() )
      {
        mNewLayer = newMeshLayer.get();
        QgsProject::instance()->addMapLayer( newMeshLayer.release(), true, true );
        return true;
      }
    }
  }

  QMessageBox::warning( this, windowTitle(), tr( "Unable to create a new mesh layer with format \"%1\"" ).arg( mFormatComboBox->currentText() ) );
  return false;
}

QgsMeshLayer *QgsNewMeshLayerDialog::newLayer() const
{
  return mNewLayer;
}

