/***************************************************************************
                          qgspointcloudlayersaveasdialog.h
 Dialog to select destination, type and crs to save as pointcloud layers
                             -------------------
    begin                : July 2022
    copyright            : (C) 2022 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslogger.h"
#include "qgspointcloudlayersaveasdialog.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgssettings.h"
#include "qgsmapcanvas.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <QSpinBox>
#include <QRegularExpression>
#include "gdal.h"
#include "qgsdatums.h"
#include "qgsiconutils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"

#include "qgspointcloudlayer.h"


QgsPointCloudLayerSaveAsDialog::QgsPointCloudLayerSaveAsDialog( QgsPointCloudLayer *layer, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mLayer( layer )
  , mActionOnExistingFile( QgsVectorFileWriter::CreateOrOverwriteFile )
{
  if ( layer )
  {
    mSelectedCrs = layer->crs();
    mLayerExtent = layer->extent();
  }
  setup();

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
}

void QgsPointCloudLayerSaveAsDialog::setup()
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudLayerSaveAsDialog::mFormatComboBox_currentIndexChanged );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsPointCloudLayerSaveAsDialog::mCrsSelector_crsChanged );
  connect( mSelectAllAttributes, &QPushButton::clicked, this, &QgsPointCloudLayerSaveAsDialog::mSelectAllAttributes_clicked );
  connect( mDeselectAllAttributes, &QPushButton::clicked, this, &QgsPointCloudLayerSaveAsDialog::mDeselectAllAttributes_clicked );

#ifdef Q_OS_WIN
  mHelpButtonBox->setVisible( false );
  mButtonBox->addButton( QDialogButtonBox::Help );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsPointCloudLayerSaveAsDialog::showHelp );
#else
  connect( mHelpButtonBox, &QDialogButtonBox::helpRequested, this, &QgsPointCloudLayerSaveAsDialog::showHelp );
#endif
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsPointCloudLayerSaveAsDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsPointCloudLayerSaveAsDialog::reject );

  mFormatComboBox->blockSignals( true );
  mFormatComboBox->addItem( tr( "Temporary Scratch Layer" ), QStringLiteral( "memory" ) );
  mFormatComboBox->addItem( tr( "LAZ point cloud" ), QStringLiteral( "LAZ" ) );
  mFormatComboBox->addItem( tr( "GeoPackage" ), QStringLiteral( "GPKG" ) );
  mFormatComboBox->addItem( tr( "ESRI Shapefile" ), QStringLiteral( "SHP" ) );
  mFormatComboBox->addItem( tr( "AutoCAD DXF" ), QStringLiteral( "DXF" ) );

  QgsSettings settings;
  QString format = settings.value( QStringLiteral( "UI/lastVectorFormat" ), "memory" ).toString();
  mFormatComboBox->blockSignals( false );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( format ) );

  mCrsSelector->setCrs( mSelectedCrs );
  mCrsSelector->setLayerCrs( mSelectedCrs );
  mCrsSelector->setMessage( tr( "Select the coordinate reference system for the vector file. "
                                "The data points will be transformed from the layer coordinate reference system." ) );


  // attributes
  if ( mLayer )
  {
    const auto attributes = mLayer->attributes();
    QStringList availableAttributes;
    for ( int i = 0; i < attributes.count(); ++i )
    {
      const QString attribute = attributes.at( i ).name();
      if ( attribute.compare( QLatin1String( "X" ), Qt::CaseInsensitive ) &&
           attribute.compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) &&
           attribute.compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) )
      {
        availableAttributes.append( attribute );
      }
    }

    mAttributeTable->setRowCount( availableAttributes.count() );
    QStringList horizontalHeaders = QStringList() << tr( "Attribute" );
    mAttributeTable->setColumnCount( horizontalHeaders.size() );
    mAttributeTable->setHorizontalHeaderLabels( horizontalHeaders );

    for ( int i = 0; i < availableAttributes.count(); ++i )
    {
      QTableWidgetItem *item = new QTableWidgetItem( availableAttributes.at( i ) );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
      item->setCheckState( Qt::Checked );
      mAttributeTable->setItem( i, 0, item );
    }
    mAttributeTable->resizeColumnsToContents();
  }

  // extent group box
  mExtentGroupBox->setOutputCrs( mSelectedCrs );
  mExtentGroupBox->setOriginalExtent( mLayerExtent, mSelectedCrs );
  mExtentGroupBox->setOutputExtentFromOriginal();
  mExtentGroupBox->setCheckable( true );
  mExtentGroupBox->setChecked( false );
  mExtentGroupBox->setCollapsed( true );

  // ZRange group box
  mMinimumZSpinBox->setRange( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max() );
  mMaximumZSpinBox->setRange( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max() );
  if ( mLayer )
  {
    mMinimumZSpinBox->setValue( mLayer->statistics().minimum( QStringLiteral( "Z" ) ) );
    mMaximumZSpinBox->setValue( mLayer->statistics().maximum( QStringLiteral( "Z" ) ) );
  }

  mFilename->setStorageMode( QgsFileWidget::SaveFile );
  mFilename->setDialogTitle( tr( "Save Layer As" ) );
  mFilename->setDefaultRoot( settings.value( QStringLiteral( "UI/lastVectorFileFilterDir" ), QDir::homePath() ).toString() );
  mFilename->setConfirmOverwrite( false );
  connect( mFilename, &QgsFileWidget::fileChanged, this, [ = ]( const QString & filePath )
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( filePath );
    settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), tmplFileInfo.absolutePath() );
    if ( !filePath.isEmpty() && leLayername->isEnabled() )
    {
      QFileInfo fileInfo( filePath );
      leLayername->setText( fileInfo.completeBaseName() );
    }

  } );

  try
  {
    const QgsDatumEnsemble ensemble = mSelectedCrs.datumEnsemble();
    if ( ensemble.isValid() )
    {
      mCrsSelector->setSourceEnsemble( ensemble.name() );
    }
  }
  catch ( QgsNotSupportedException & )
  {
  }

  mCrsSelector->setShowAccuracyWarnings( true );

  mAddToCanvas->setEnabled( true );
}

void QgsPointCloudLayerSaveAsDialog::accept()
{
  if ( QFile::exists( filename() ) )
  {
    QgsVectorFileWriter::EditionCapabilities caps =
      QgsVectorFileWriter::editionCapabilities( filename() );
    bool layerExists = QgsVectorFileWriter::targetLayerExists( filename(),
                       layername() );
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setWindowTitle( tr( "Save Vector Layer As" ) );
    QPushButton *overwriteFileButton = msgBox.addButton( tr( "Overwrite File" ), QMessageBox::ActionRole );
    QPushButton *overwriteLayerButton = msgBox.addButton( tr( "Overwrite Layer" ), QMessageBox::ActionRole );
    QPushButton *appendToLayerButton = msgBox.addButton( tr( "Append to Layer" ), QMessageBox::ActionRole );
    msgBox.setStandardButtons( QMessageBox::Cancel );
    msgBox.setDefaultButton( QMessageBox::Cancel );
    overwriteFileButton->hide();
    overwriteLayerButton->hide();
    appendToLayerButton->hide();
    if ( layerExists )
    {
      if ( !( caps & QgsVectorFileWriter::CanAppendToExistingLayer ) &&
           ( caps & QgsVectorFileWriter::CanDeleteLayer ) &&
           ( caps & QgsVectorFileWriter::CanAddNewLayer ) )
      {
        msgBox.setText( tr( "The layer already exists. Do you want to overwrite the whole file or overwrite the layer?" ) );
        overwriteFileButton->setVisible( true );
        overwriteLayerButton->setVisible( true );
      }
      else if ( !( caps & QgsVectorFileWriter::CanAppendToExistingLayer ) )
      {
        msgBox.setText( tr( "The file already exists. Do you want to overwrite it?" ) );
        overwriteFileButton->setVisible( true );
      }
      else if ( ( caps & QgsVectorFileWriter::CanDeleteLayer ) &&
                ( caps & QgsVectorFileWriter::CanAddNewLayer ) )
      {
        msgBox.setText( tr( "The layer already exists. Do you want to overwrite the whole file, overwrite the layer or append features to the layer?" ) );
        appendToLayerButton->setVisible( true );
        overwriteFileButton->setVisible( true );
        overwriteLayerButton->setVisible( true );
      }
      else
      {
        msgBox.setText( tr( "The layer already exists. Do you want to overwrite the whole file or append features to the layer?" ) );
        appendToLayerButton->setVisible( true );
        overwriteFileButton->setVisible( true );
      }

      int ret = msgBox.exec();
      if ( ret == QMessageBox::Cancel )
        return;
      if ( msgBox.clickedButton() == overwriteFileButton )
        mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
      else if ( msgBox.clickedButton() == overwriteLayerButton )
        mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteLayer;
      else if ( msgBox.clickedButton() == appendToLayerButton )
        mActionOnExistingFile = QgsVectorFileWriter::AppendToLayerNoNewFields;
    }
    else // !layerExists
    {
      if ( ( caps & QgsVectorFileWriter::CanAddNewLayer ) )
      {
        mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteLayer;
      }
      else
      {
        // should not reach here, layer does not exist and cannot add new layer
        if ( QMessageBox::question( this,
                                    tr( "Save Vector Layer As" ),
                                    tr( "The file already exists. Do you want to overwrite it?" ) ) == QMessageBox::NoButton )
        {
          return;
        }
        mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
      }
    }
  }

  if ( mActionOnExistingFile == QgsVectorFileWriter::AppendToLayerNoNewFields )
  {
//    if ( QgsVectorFileWriter::areThereNewFieldsToCreate( filename(), layername(), mLayer, selectedAttributes() ) )
//    {
//      if ( QMessageBox::question( this,
//                                  tr( "Save Vector Layer As" ),
//                                  tr( "The existing layer has additional fields. Do you want to add the missing fields to the layer?" ) ) == QMessageBox::Yes )
//      {
//        mActionOnExistingFile = QgsVectorFileWriter::AppendToLayerAddFields;
//      }
//    }
  }
  else if ( mActionOnExistingFile == QgsVectorFileWriter::CreateOrOverwriteFile && QFile::exists( filename() ) )
  {
    const QList<QgsProviderSublayerDetails> sublayers = QgsProviderRegistry::instance()->querySublayers( filename() );
    QStringList layerList;
    layerList.reserve( sublayers.size() );
    for ( const QgsProviderSublayerDetails &sublayer : sublayers )
    {
      layerList.append( sublayer.name() );
    }
    if ( layerList.length() > 1 )
    {
      layerList.sort( Qt::CaseInsensitive );
      QMessageBox msgBox;
      msgBox.setIcon( QMessageBox::Warning );
      msgBox.setWindowTitle( tr( "Overwrite File" ) );
      msgBox.setText( tr( "This file contains %1 layers that will be lost!\n" ).arg( QLocale().toString( layerList.length() ) ) );
      msgBox.setDetailedText( tr( "The following layers will be permanently lost:\n\n%1" ).arg( layerList.join( "\n" ) ) );
      msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
      if ( msgBox.exec() == QMessageBox::Cancel )
        return;
    }
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), QFileInfo( filename() ).absolutePath() );
  settings.setValue( QStringLiteral( "UI/lastVectorFormat" ), format() );
  QDialog::accept();
}

void QgsPointCloudLayerSaveAsDialog::mFormatComboBox_currentIndexChanged( int idx )
{
  Q_UNUSED( idx )

  mFilename->setEnabled( true );
  mFilename->setFilter( QgsVectorFileWriter::filterForDriver( format() ) );

  // if output filename already defined we need to replace old suffix
  // to avoid double extensions like .gpkg.shp
  if ( !mFilename->filePath().isEmpty() )
  {
    QRegularExpression rx( "\\.(.*?)[\\s]" );
    QString ext;
    ext = rx.match( QgsVectorFileWriter::filterForDriver( format() ) ).captured( 1 );
    if ( !ext.isEmpty() )
    {
      QFileInfo fi( mFilename->filePath() );
      mFilename->setFilePath( QStringLiteral( "%1/%2.%3" ).arg( fi.path(), fi.baseName(), ext ) );
    }
  }

  const QString sFormat( format() );
  if ( sFormat == QLatin1String( "DXF" ) )
  {
    mAttributesSelection->setEnabled( false );
  }
  else
  {
    mAttributesSelection->setEnabled( true );
  }

  leLayername->setEnabled( sFormat == QLatin1String( "memory" ) ||
                           sFormat == QLatin1String( "GPKG" ) );

  leLayername->setMaxLength( 32767 ); // default length

  if ( !leLayername->isEnabled() )
    leLayername->setText( QString() );
  else if ( leLayername->text().isEmpty() &&
            !mFilename->filePath().isEmpty() )
  {
    QString layerName = QFileInfo( mFilename->filePath() ).baseName();
    leLayername->setText( layerName );
  }

  mFilename->setEnabled( sFormat != QLatin1String( "memory" ) );
}

void QgsPointCloudLayerSaveAsDialog::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mSelectedCrs = crs;
  mExtentGroupBox->setOutputCrs( mSelectedCrs );
}

QString QgsPointCloudLayerSaveAsDialog::filename() const
{
  return mFilename->filePath();
}

QString QgsPointCloudLayerSaveAsDialog::layername() const
{
  return leLayername->text();
}

QString QgsPointCloudLayerSaveAsDialog::format() const
{
  return mFormatComboBox->currentData().toString();
}

QgsCoordinateReferenceSystem QgsPointCloudLayerSaveAsDialog::crsObject() const
{
  return mSelectedCrs;
}

QStringList QgsPointCloudLayerSaveAsDialog::attributes() const
{
  QStringList attributes;

  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    if ( mAttributeTable->item( i, 0 )->checkState() == Qt::Checked )
    {
      attributes.append( mAttributeTable->item( i, 0 )->text() );
    }
  }

  return attributes;
}

bool QgsPointCloudLayerSaveAsDialog::addToCanvas() const
{
  return mAddToCanvas->isChecked() && mAddToCanvas->isEnabled();
}

void QgsPointCloudLayerSaveAsDialog::setAddToCanvas( bool enabled )
{
  mAddToCanvas->setChecked( enabled );
}

void QgsPointCloudLayerSaveAsDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  mExtentGroupBox->setCurrentExtent( canvas->mapSettings().visibleExtent(), canvas->mapSettings().destinationCrs() );
}

bool QgsPointCloudLayerSaveAsDialog::hasFilterExtent() const
{
  return mExtentGroupBox->isChecked();
}

QgsRectangle QgsPointCloudLayerSaveAsDialog::filterExtent() const
{
  return mExtentGroupBox->outputExtent();
}

bool QgsPointCloudLayerSaveAsDialog::hasAttributes() const
{
  return mAttributesSelection->isChecked() && mAttributesSelection->isEnabled();
}

bool QgsPointCloudLayerSaveAsDialog::hasZRange() const
{
  return mZRangeGroupBox->isChecked();
}

QgsDoubleRange QgsPointCloudLayerSaveAsDialog::zRange() const
{
  return QgsDoubleRange( mMinimumZSpinBox->value(), mMaximumZSpinBox->value() );
}

void QgsPointCloudLayerSaveAsDialog::mSelectAllAttributes_clicked()
{
  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    mAttributeTable->item( i, 0 )->setCheckState( Qt::Checked );
  }
}

void QgsPointCloudLayerSaveAsDialog::mDeselectAllAttributes_clicked()
{
  {
    for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    {
      mAttributeTable->item( i, 0 )->setCheckState( Qt::Unchecked );
    }
  }
}

void QgsPointCloudLayerSaveAsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-new-layers-from-an-existing-layer" ) );
}
