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

#include <QMessageBox>
#include <QRegularExpression>

#include "qgspointcloudlayersaveasdialog.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsdatums.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgspointcloudlayer.h"
#include "qgsmaplayerutils.h"
#include "qgsvectorlayer.h"

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
}

void QgsPointCloudLayerSaveAsDialog::setup()
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudLayerSaveAsDialog::mFormatComboBox_currentIndexChanged );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsPointCloudLayerSaveAsDialog::mCrsSelector_crsChanged );
  connect( mSelectAllAttributes, &QPushButton::clicked, this, &QgsPointCloudLayerSaveAsDialog::mSelectAllAttributes_clicked );
  connect( mDeselectAllAttributes, &QPushButton::clicked, this, &QgsPointCloudLayerSaveAsDialog::mDeselectAllAttributes_clicked );
  connect( mFilterGeometryLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsPointCloudLayerSaveAsDialog::mFilterGeometryLayerChanged );
  connect( mFilterGeometryGroupBox, &QgsCollapsibleGroupBox::toggled, this, &QgsPointCloudLayerSaveAsDialog::mFilterGeometryGroupBoxCheckToggled );
  connect( mMinimumZSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudLayerSaveAsDialog::mMinimumZSpinBoxValueChanged );
  connect( mMaximumZSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudLayerSaveAsDialog::mMaximumZSpinBoxValueChanged );

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
  const QList< QgsPointCloudLayerExporter::ExportFormat > supportedFormats = QgsPointCloudLayerExporter::supportedFormats();
  for ( const auto &format : supportedFormats )
    mFormatComboBox->addItem( getTranslatedNameForFormat( format ), static_cast< int >( format ) );

  QgsSettings settings;
  const int defaultFormat = settings.value( QStringLiteral( "UI/lastPointCloudFormat" ), 0 ).toInt();
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( defaultFormat ) );
  mFormatComboBox->blockSignals( false );
  mFormatComboBox_currentIndexChanged( 0 );

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

  // polygon layer filter group box
  mFilterGeometryLayerComboBox->setFilters( QgsMapLayerProxyModel::PolygonLayer );

  // ZRange group box
  mMinimumZSpinBox->setRange( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max() );
  mMaximumZSpinBox->setRange( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max() );
  if ( mLayer )
  {
    mMinimumZSpinBox->setValue( mLayer->statistics().minimum( QStringLiteral( "Z" ) ) );
    mMinimumZSpinBox->setClearValue( mMinimumZSpinBox->value() );
    mMaximumZSpinBox->setValue( mLayer->statistics().maximum( QStringLiteral( "Z" ) ) );
    mMaximumZSpinBox->setClearValue( mMaximumZSpinBox->value() );
  }

  // points limit group box
  mPointsLimitSpinBox->setMinimum( 1 );
  mPointsLimitSpinBox->setMaximum( std::numeric_limits<int>::max() );
  mPointsLimitSpinBox->setValue( 1e6 );
  mPointsLimitSpinBox->setClearValue( 1e6 );

  mFilename->setStorageMode( QgsFileWidget::SaveFile );
  mFilename->setDialogTitle( tr( "Save Layer As" ) );
  mFilename->setDefaultRoot( settings.value( QStringLiteral( "UI/lastPointCloudFileFilterDir" ), QDir::homePath() ).toString() );
  mFilename->setConfirmOverwrite( false );
  connect( mFilename, &QgsFileWidget::fileChanged, this, [ = ]( const QString & filePath )
  {
    QgsSettings settings;
    if ( !filePath.isEmpty() )
      mLastUsedFilename = filePath;

    const QFileInfo fileInfo( filePath );
    settings.setValue( QStringLiteral( "UI/lastPointCloudFileFilterDir" ), fileInfo.absolutePath() );
    const QString suggestedLayerName = QgsMapLayerUtils::launderLayerName( fileInfo.completeBaseName() );
    if ( mDefaultOutputLayerNameFromInputLayerName.isEmpty() )
    {
      leLayername->setDefaultValue( suggestedLayerName );
    }

    // if no layer name set, then automatically match the output layer name to the file name
    if ( leLayername->text().isEmpty() && !filePath.isEmpty() && leLayername->isEnabled() )
    {
      leLayername->setText( suggestedLayerName );
    }
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( !filePath.isEmpty() );
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

  mAddToCanvas->setEnabled( exportFormat() != QgsPointCloudLayerExporter::ExportFormat::Memory );

  if ( mLayer )
  {
    mDefaultOutputLayerNameFromInputLayerName = QgsMapLayerUtils::launderLayerName( mLayer->name() );
    leLayername->setDefaultValue( mDefaultOutputLayerNameFromInputLayerName );
    leLayername->setClearMode( QgsFilterLineEdit::ClearToDefault );
    if ( leLayername->isEnabled() )
      leLayername->setText( mDefaultOutputLayerNameFromInputLayerName );
  }

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( exportFormat() == QgsPointCloudLayerExporter::ExportFormat::Memory ||
      !mFilename->filePath().isEmpty() );
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
    msgBox.setWindowTitle( tr( "Save Point Cloud Layer As" ) );
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
                                    tr( "Save Point Cloud Layer As" ),
                                    tr( "The file already exists. Do you want to overwrite it?" ) ) == QMessageBox::NoButton )
        {
          return;
        }
        mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
      }
    }
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
  settings.setValue( QStringLiteral( "UI/lastPointCloudFileFilterDir" ), QFileInfo( filename() ).absolutePath() );
  settings.setValue( QStringLiteral( "UI/lastPointCloudFormat" ), static_cast< int >( exportFormat() ) );
  QDialog::accept();
}

void QgsPointCloudLayerSaveAsDialog::mFormatComboBox_currentIndexChanged( int idx )
{
  Q_UNUSED( idx )

  const QgsPointCloudLayerExporter::ExportFormat format = exportFormat();

  switch ( format )
  {
    case QgsPointCloudLayerExporter::ExportFormat::Memory:
    case QgsPointCloudLayerExporter::ExportFormat::Gpkg:
    case QgsPointCloudLayerExporter::ExportFormat::Shp:
    case QgsPointCloudLayerExporter::ExportFormat::Csv:
      mAttributesSelection->setEnabled( true );
      break;

    case QgsPointCloudLayerExporter::ExportFormat::Las:
    case QgsPointCloudLayerExporter::ExportFormat::Dxf:
      mAttributesSelection->setEnabled( false );
      break;
  }

  switch ( format )
  {
    case QgsPointCloudLayerExporter::ExportFormat::Memory:
    case QgsPointCloudLayerExporter::ExportFormat::Gpkg:
      leLayername->setEnabled( true );
      break;
      \
    case QgsPointCloudLayerExporter::ExportFormat::Shp:
    case QgsPointCloudLayerExporter::ExportFormat::Las:
    case QgsPointCloudLayerExporter::ExportFormat::Dxf:
    case QgsPointCloudLayerExporter::ExportFormat::Csv:
      leLayername->setEnabled( false );
      break;
  }

  switch ( format )
  {
    case QgsPointCloudLayerExporter::ExportFormat::Memory:
      mWasAddToCanvasForced = !mAddToCanvas->isChecked();
      mAddToCanvas->setEnabled( false );
      mAddToCanvas->setChecked( true );
      mFilename->setEnabled( false );
      break;

    case QgsPointCloudLayerExporter::ExportFormat::Gpkg:
    case QgsPointCloudLayerExporter::ExportFormat::Shp:
    case QgsPointCloudLayerExporter::ExportFormat::Las:
    case QgsPointCloudLayerExporter::ExportFormat::Dxf:
    case QgsPointCloudLayerExporter::ExportFormat::Csv:
      mAddToCanvas->setEnabled( true );
      if ( mWasAddToCanvasForced )
      {
        mAddToCanvas->setChecked( !mAddToCanvas->isChecked() );
        mWasAddToCanvasForced = false;
      }
      mFilename->setEnabled( true );
      break;
  }

  if ( mFilename->isEnabled() )
  {
    mFilename->setFilter( getFilterForFormat( format ) );

    // if output filename already defined we need to replace old suffix
    // to avoid double extensions like .gpkg.shp
    if ( !mLastUsedFilename.isEmpty() )
    {
      QRegularExpression rx( "\\.(.*?)[\\s]" );
      QString ext;
      ext = rx.match( getFilterForFormat( format ) ).captured( 1 );
      if ( !ext.isEmpty() )
      {
        QFileInfo fi( mLastUsedFilename );
        mFilename->setFilePath( QStringLiteral( "%1/%2.%3" ).arg( fi.path(), fi.baseName(), ext ) );
      }
    }
  }

  if ( !mFilename->isEnabled() )
    mFilename->setFilePath( QString() );

  if ( !leLayername->isEnabled() )
  {
    leLayername->setText( QString() );
  }
  else if ( leLayername->text().isEmpty() )
  {
    QString layerName = mDefaultOutputLayerNameFromInputLayerName;
    if ( layerName.isEmpty() && !mFilename->filePath().isEmpty() )
    {
      layerName = QFileInfo( mFilename->filePath() ).baseName();
      leLayername->setDefaultValue( layerName );
    }
    if ( layerName.isEmpty() )
    {
      layerName = tr( "new_layer" );
    }
    leLayername->setText( layerName );
  }

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( format == QgsPointCloudLayerExporter::ExportFormat::Memory ||
      !mFilename->filePath().isEmpty() );
}

void QgsPointCloudLayerSaveAsDialog::mFilterGeometryGroupBoxCheckToggled( bool checked )
{
  if ( checked )
    mFilterGeometryLayerChanged( mFilterGeometryLayerComboBox->currentLayer() );
}

void QgsPointCloudLayerSaveAsDialog::mFilterGeometryLayerChanged( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = dynamic_cast< QgsVectorLayer * >( layer );
  mSelectedFeaturesCheckBox->setChecked( false );
  mSelectedFeaturesCheckBox->setEnabled( hasFilterLayer() && vlayer && vlayer->selectedFeatureCount() );
}

void QgsPointCloudLayerSaveAsDialog::mMinimumZSpinBoxValueChanged( const double value )
{
  mMaximumZSpinBox->setMinimum( value );
}

void QgsPointCloudLayerSaveAsDialog::mMaximumZSpinBoxValueChanged( const double value )
{
  mMinimumZSpinBox->setMaximum( value );
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

QgsPointCloudLayerExporter::ExportFormat QgsPointCloudLayerSaveAsDialog::exportFormat() const
{
  return static_cast< QgsPointCloudLayerExporter::ExportFormat >( mFormatComboBox->currentData().toInt() );
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
  return mAddToCanvas->isChecked();
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

bool QgsPointCloudLayerSaveAsDialog::hasFilterLayer() const
{
  return mFilterGeometryGroupBox->isChecked() && mFilterGeometryLayerComboBox->count() > 0;
}

QgsMapLayer *QgsPointCloudLayerSaveAsDialog::filterLayer() const
{
  return mFilterGeometryLayerComboBox->currentLayer();
}

bool QgsPointCloudLayerSaveAsDialog::filterLayerSelectedOnly() const
{
  return hasFilterLayer() && mSelectedFeaturesCheckBox->isChecked();
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

bool QgsPointCloudLayerSaveAsDialog::hasPointsLimit() const
{
  return mPointsLimitGroupBox->isChecked();
}

int QgsPointCloudLayerSaveAsDialog::pointsLimit() const
{
  return mPointsLimitSpinBox->value();
}

QgsVectorFileWriter::ActionOnExistingFile QgsPointCloudLayerSaveAsDialog::creationActionOnExistingFile() const
{
  return mActionOnExistingFile;
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

QString QgsPointCloudLayerSaveAsDialog::getFilterForFormat( QgsPointCloudLayerExporter::ExportFormat format )
{
  switch ( format )
  {
    case QgsPointCloudLayerExporter::ExportFormat::Las:
      return QStringLiteral( "LAZ point cloud (*.laz *.LAZ);;LAS point cloud (*.las *.LAS)" );
    case QgsPointCloudLayerExporter::ExportFormat::Gpkg:
      return QStringLiteral( "GeoPackage (*.gpkg *.GPKG)" );
    case QgsPointCloudLayerExporter::ExportFormat::Dxf:
      return QStringLiteral( "AutoCAD DXF (*.dxf *.dxf)" );
    case QgsPointCloudLayerExporter::ExportFormat::Shp:
      return QStringLiteral( "ESRI Shapefile (*.shp *.SHP)" );
    case QgsPointCloudLayerExporter::ExportFormat::Csv:
      return QStringLiteral( "Comma separated values (*.csv *.CSV)" );
    case QgsPointCloudLayerExporter::ExportFormat::Memory:
      break;
  }
  return QString();
}

QString QgsPointCloudLayerSaveAsDialog::getTranslatedNameForFormat( QgsPointCloudLayerExporter::ExportFormat format )
{
  switch ( format )
  {
    case QgsPointCloudLayerExporter::ExportFormat::Memory:
      return tr( "Temporary Scratch Layer" );
    case QgsPointCloudLayerExporter::ExportFormat::Gpkg:
      return tr( "GeoPackage" );
    case QgsPointCloudLayerExporter::ExportFormat::Dxf:
      return tr( "AutoCAD DXF" );
    case QgsPointCloudLayerExporter::ExportFormat::Shp:
      return tr( "ESRI Shapefile" );
    case QgsPointCloudLayerExporter::ExportFormat::Las:
      return tr( "LAS/LAZ point cloud" );
    case QgsPointCloudLayerExporter::ExportFormat::Csv:
      return tr( "Comma separated values" );
  }
  return QString();
}
