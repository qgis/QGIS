/***************************************************************************
                          qgsvectorlayersaveasdialog.h
 Dialog to select destination, type and crs for ogr layers
                             -------------------
    begin                : Mon Mar 22 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
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
#include "qgsvectorlayersaveasdialog.h"
#include "moc_qgsvectorlayersaveasdialog.cpp"
#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgssettings.h"
#include "qgsmapcanvas.h"
#include "qgsgui.h"
#include "qgsmaplayerutils.h"
#include "qgshelp.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <QSpinBox>
#include <QRegularExpression>
#include <limits>
#include "gdal.h"
#include "qgsdatums.h"
#include "qgsiconutils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( long srsid, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mSelectedCrs( QgsCoordinateReferenceSystem::fromSrsId( srsid ) )
  , mActionOnExistingFile( QgsVectorFileWriter::CreateOrOverwriteFile )
{
  setup();
}

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( QgsVectorLayer *layer, QgsVectorLayerSaveAsDialog::Options options, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mLayer( layer )
  , mActionOnExistingFile( QgsVectorFileWriter::CreateOrOverwriteFile )
  , mOptions( options )
{
  if ( layer )
  {
    mSelectedCrs = layer->crs();
    mLayerExtent = layer->extent();
  }
  setup();

  if ( layer )
  {
    mDefaultOutputLayerNameFromInputLayerName = QgsMapLayerUtils::launderLayerName( layer->name() );
    leLayername->setDefaultValue( mDefaultOutputLayerNameFromInputLayerName );
    leLayername->setClearMode( QgsFilterLineEdit::ClearToDefault );
    if ( leLayername->isEnabled() )
      leLayername->setText( mDefaultOutputLayerNameFromInputLayerName );
  }

  if ( !( mOptions & Option::Symbology ) )
  {
    mSymbologyExportLabel->hide();
    mSymbologyExportComboBox->hide();
    mScaleLabel->hide();
    mScaleWidget->hide();
  }

  if ( !( mOptions & Option::DestinationCrs ) )
  {
    mCrsLabel->hide();
    mCrsSelector->hide();
  }
  if ( !( mOptions & Option::Fields ) )
    mAttributesSelection->hide();

  if ( !( mOptions & Option::SelectedOnly ) )
    mSelectedOnly->hide();

  if ( !( mOptions & Option::AddToCanvas ) )
    mAddToCanvas->hide();

  if ( !( mOptions & Option::GeometryType ) )
    mGeometryGroupBox->hide();

  if ( !( mOptions & Option::Extent ) )
    mExtentGroupBox->hide();

  if ( !( mOptions & Option::Metadata ) )
  {
    mCheckPersistMetadata->setChecked( false );
    mCheckPersistMetadata->hide();
  }

  mSelectedOnly->setEnabled( layer && layer->selectedFeatureCount() != 0 );
  mButtonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
}

void QgsVectorLayerSaveAsDialog::setup()
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsVectorLayerSaveAsDialog::mFormatComboBox_currentIndexChanged );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsVectorLayerSaveAsDialog::mCrsSelector_crsChanged );
  connect( mSymbologyExportComboBox, &QComboBox::currentTextChanged, this, &QgsVectorLayerSaveAsDialog::mSymbologyExportComboBox_currentIndexChanged );
  connect( mGeometryTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsVectorLayerSaveAsDialog::mGeometryTypeComboBox_currentIndexChanged );
  connect( mSelectAllAttributes, &QPushButton::clicked, this, &QgsVectorLayerSaveAsDialog::mSelectAllAttributes_clicked );
  connect( mDeselectAllAttributes, &QPushButton::clicked, this, &QgsVectorLayerSaveAsDialog::mDeselectAllAttributes_clicked );
  connect( mUseAliasesForExportedName, &QCheckBox::stateChanged, this, &QgsVectorLayerSaveAsDialog::mUseAliasesForExportedName_stateChanged );
  connect( mReplaceRawFieldValues, &QCheckBox::stateChanged, this, &QgsVectorLayerSaveAsDialog::mReplaceRawFieldValues_stateChanged );
  connect( mAttributeTable, &QTableWidget::itemChanged, this, &QgsVectorLayerSaveAsDialog::mAttributeTable_itemChanged );

#ifdef Q_OS_WIN
  mHelpButtonBox->setVisible( false );
  mButtonBox->addButton( QDialogButtonBox::Help );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorLayerSaveAsDialog::showHelp );
#else
  connect( mHelpButtonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorLayerSaveAsDialog::showHelp );
#endif
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsVectorLayerSaveAsDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsVectorLayerSaveAsDialog::reject );

  const QList<QgsVectorFileWriter::DriverDetails> drivers = QgsVectorFileWriter::ogrDriverList();
  mFormatComboBox->blockSignals( true );
  for ( const QgsVectorFileWriter::DriverDetails &driver : drivers )
  {
    mFormatComboBox->addItem( driver.longName, driver.driverName );
  }

  QgsSettings settings;
  QString format = settings.value( QStringLiteral( "UI/lastVectorFormat" ), "GPKG" ).toString();
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( format ) );
  mFormatComboBox->blockSignals( false );

  const auto addGeomItem = [this]( Qgis::WkbType type ) {
    mGeometryTypeComboBox->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), static_cast<quint32>( type ) );
  };

  //add geometry types to combobox
  mGeometryTypeComboBox->addItem( tr( "Automatic" ), -1 );
  addGeomItem( Qgis::WkbType::Point );
  addGeomItem( Qgis::WkbType::LineString );
  addGeomItem( Qgis::WkbType::Polygon );
  mGeometryTypeComboBox->addItem( QgsWkbTypes::translatedDisplayString( Qgis::WkbType::GeometryCollection ), static_cast<quint32>( Qgis::WkbType::GeometryCollection ) );
  addGeomItem( Qgis::WkbType::NoGeometry );
  mGeometryTypeComboBox->setCurrentIndex( mGeometryTypeComboBox->findData( -1 ) );

  mEncodingComboBox->addItems( QgsVectorDataProvider::availableEncodings() );

  QString enc = settings.value( QStringLiteral( "UI/encoding" ), "System" ).toString();
  int idx = mEncodingComboBox->findText( enc );
  if ( idx < 0 )
  {
    mEncodingComboBox->insertItem( 0, enc );
    idx = 0;
  }

  mCrsSelector->setCrs( mSelectedCrs );
  mCrsSelector->setLayerCrs( mSelectedCrs );
  mCrsSelector->setMessage( tr( "Select the coordinate reference system for the vector file. "
                                "The data points will be transformed from the layer coordinate reference system." ) );

  mEncodingComboBox->setCurrentIndex( idx );
  mFormatComboBox_currentIndexChanged( mFormatComboBox->currentIndex() );

  //symbology export combo box
  mSymbologyExportComboBox->addItem( tr( "No Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::NoSymbology ) );
  mSymbologyExportComboBox->addItem( tr( "Feature Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::PerFeature ) );
  mSymbologyExportComboBox->addItem( tr( "Symbol Layer Symbology" ), QVariant::fromValue( Qgis::FeatureSymbologyExport::PerSymbolLayer ) );
  mSymbologyExportComboBox_currentIndexChanged( mSymbologyExportComboBox->currentText() );

  // extent group box
  mExtentGroupBox->setOutputCrs( mSelectedCrs );
  mExtentGroupBox->setOriginalExtent( mLayerExtent, mSelectedCrs );
  mExtentGroupBox->setOutputExtentFromOriginal();
  mExtentGroupBox->setCheckable( true );
  mExtentGroupBox->setChecked( false );
  mExtentGroupBox->setCollapsed( true );

  mFilename->setStorageMode( QgsFileWidget::SaveFile );
  mFilename->setDialogTitle( tr( "Save Layer As" ) );
  mFilename->setDefaultRoot( settings.value( QStringLiteral( "UI/lastVectorFileFilterDir" ), QDir::homePath() ).toString() );
  mFilename->setConfirmOverwrite( false );
  connect( mFilename, &QgsFileWidget::fileChanged, this, [=]( const QString &filePath ) {
    QgsSettings settings;
    QFileInfo tmplFileInfo( filePath );
    settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), tmplFileInfo.absolutePath() );

    const QFileInfo fileInfo( filePath );
    const QString suggestedLayerName = QgsMapLayerUtils::launderLayerName( fileInfo.completeBaseName() );
    if ( mDefaultOutputLayerNameFromInputLayerName.isEmpty() )
      leLayername->setDefaultValue( suggestedLayerName );

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
}

QList<QPair<QLabel *, QWidget *>> QgsVectorLayerSaveAsDialog::createControls( const QMap<QString, QgsVectorFileWriter::Option *> &options )
{
  QList<QPair<QLabel *, QWidget *>> controls;
  QMap<QString, QgsVectorFileWriter::Option *>::ConstIterator it;

  for ( it = options.constBegin(); it != options.constEnd(); ++it )
  {
    QgsVectorFileWriter::Option *option = it.value();
    QWidget *control = nullptr;
    switch ( option->type )
    {
      case QgsVectorFileWriter::Int:
      {
        QgsVectorFileWriter::IntOption *opt = dynamic_cast<QgsVectorFileWriter::IntOption *>( option );
        if ( opt )
        {
          QSpinBox *sb = new QSpinBox();
          sb->setObjectName( it.key() );
          sb->setMaximum( std::numeric_limits<int>::max() ); // the default is 99
          sb->setValue( opt->defaultValue );
          control = sb;
        }
        break;
      }

      case QgsVectorFileWriter::Set:
      {
        QgsVectorFileWriter::SetOption *opt = dynamic_cast<QgsVectorFileWriter::SetOption *>( option );
        if ( opt )
        {
          QComboBox *cb = new QComboBox();
          cb->setObjectName( it.key() );
          for ( const QString &val : std::as_const( opt->values ) )
          {
            cb->addItem( val, val );
          }
          if ( opt->allowNone )
            cb->addItem( tr( "<Default>" ), QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
          int idx = cb->findText( opt->defaultValue );
          if ( idx == -1 )
            idx = cb->findData( QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
          cb->setCurrentIndex( idx );
          control = cb;
        }
        break;
      }

      case QgsVectorFileWriter::String:
      {
        QgsVectorFileWriter::StringOption *opt = dynamic_cast<QgsVectorFileWriter::StringOption *>( option );
        if ( opt )
        {
          QLineEdit *le = new QLineEdit( opt->defaultValue );
          le->setObjectName( it.key() );
          control = le;
        }
        break;
      }

      case QgsVectorFileWriter::Hidden:
        control = nullptr;
        break;
    }

    if ( control )
    {
      QLabel *label = new QLabel( it.key() );

      // Pack the tooltip in some html element, so it gets linebreaks.
      label->setToolTip( QStringLiteral( "<p>%1</p>" ).arg( option->docString.toHtmlEscaped() ) );
      control->setToolTip( QStringLiteral( "<p>%1</p>" ).arg( option->docString.toHtmlEscaped() ) );

      controls << QPair<QLabel *, QWidget *>( label, control );
    }
  }

  return controls;
}

void QgsVectorLayerSaveAsDialog::accept()
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 9, 0 )
  if ( format() == QLatin1String( "OpenFileGDB" ) )
  {
    // The OpenFileGDB driver supports 64-bit integer fields starting with GDAL 3.9,
    // if selecting the TARGET_ARCGIS_VERSION=ARCGIS_PRO_3_2_OR_LATER option
    bool targetAll = true;
    for ( const QString &layerOption : layerOptions() )
    {
      if ( layerOption == QLatin1String( "TARGET_ARCGIS_VERSION=ARCGIS_PRO_3_2_OR_LATER" ) )
      {
        targetAll = false;
      }
    }
    if ( targetAll )
    {
      for ( int i = 0; i < mLayer->fields().size(); ++i )
      {
        QgsField fld = mLayer->fields().at( i );
        if ( fld.type() == QMetaType::Type::LongLong )
        {
          if ( QMessageBox::question( this, tr( "Save Vector Layer As" ), tr( "The layer contains at least one 64-bit integer field, which, with the current settings, can only be exported as a Real field. It could be exported as a 64-bit integer field if the TARGET_ARCGIS_VERSION layer option is set to ARCGIS_PRO_3_2_OR_LATER. Do you want to continue and export it as a Real field?" ) ) != QMessageBox::Yes )
          {
            return;
          }
          break;
        }
      }
    }
  }
  else if ( format() == QLatin1String( "FileGDB" ) )
  {
    // The FileGDB driver based on the ESRI SDK doesn't support 64-bit integers
    for ( int i = 0; i < mLayer->fields().size(); ++i )
    {
      QgsField fld = mLayer->fields().at( i );
      if ( fld.type() == QMetaType::Type::LongLong )
      {
        if ( QMessageBox::question( this, tr( "Save Vector Layer As" ), tr( "The layer contains at least one 64-bit integer field, which cannot be exported as such when using this output driver. 64-bit integer fields could be supported by selecting the %1 format and setting its TARGET_ARCGIS_VERSION layer option to ARCGIS_PRO_3_2_OR_LATER. Do you want to continue and export it as a Real field?" ).arg( tr( "ESRI File Geodatabase" ) ) ) != QMessageBox::Yes )
        {
          return;
        }
        break;
      }
    }
  }
#endif

  if ( QFile::exists( fileName() ) )
  {
    QgsVectorFileWriter::EditionCapabilities caps = QgsVectorFileWriter::editionCapabilities( fileName() );
    bool layerExists = QgsVectorFileWriter::targetLayerExists( fileName(), layerName() );
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
      if ( !( caps & QgsVectorFileWriter::CanAppendToExistingLayer ) && ( caps & QgsVectorFileWriter::CanDeleteLayer ) && ( caps & QgsVectorFileWriter::CanAddNewLayer ) )
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
      else if ( ( caps & QgsVectorFileWriter::CanDeleteLayer ) && ( caps & QgsVectorFileWriter::CanAddNewLayer ) )
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
        if ( QMessageBox::question( this, tr( "Save Vector Layer As" ), tr( "The file already exists. Do you want to overwrite it?" ) ) != QMessageBox::Yes )
        {
          return;
        }
        mActionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
      }
    }
  }

  if ( mActionOnExistingFile == QgsVectorFileWriter::AppendToLayerNoNewFields )
  {
    if ( QgsVectorFileWriter::areThereNewFieldsToCreate( fileName(), layerName(), mLayer, selectedAttributes() ) )
    {
      if ( QMessageBox::question( this, tr( "Save Vector Layer As" ), tr( "The existing layer has additional fields. Do you want to add the missing fields to the layer?" ) ) == QMessageBox::Yes )
      {
        mActionOnExistingFile = QgsVectorFileWriter::AppendToLayerAddFields;
      }
    }
  }
  else if ( mActionOnExistingFile == QgsVectorFileWriter::CreateOrOverwriteFile && QFile::exists( fileName() ) )
  {
    const QList<QgsProviderSublayerDetails> sublayers = QgsProviderRegistry::instance()->querySublayers( fileName() );
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
  settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), QFileInfo( fileName() ).absolutePath() );
  settings.setValue( QStringLiteral( "UI/lastVectorFormat" ), format() );
  settings.setValue( QStringLiteral( "UI/encoding" ), encoding() );
  QDialog::accept();
}

void QgsVectorLayerSaveAsDialog::mFormatComboBox_currentIndexChanged( int idx )
{
  Q_UNUSED( idx )

  mFilename->setEnabled( true );
  QString filter = QgsVectorFileWriter::filterForDriver( format() );
  // A bit of hack to solve https://github.com/qgis/QGIS/issues/54566
  // to be able to select an existing File Geodatabase, we add in the filter
  // the "gdb" file that is found in all File Geodatabase .gdb directory
  // to allow the user to select it. We need to detect this particular case
  // in QgsFileWidget::openFileDialog() to remove this gdb file from the
  // selected filename
  if ( format() == QLatin1String( "OpenFileGDB" ) || format() == QLatin1String( "FileGDB" ) )
    filter = QStringLiteral( "%1 (*.gdb *.GDB gdb)" ).arg( tr( "ESRI File Geodatabase" ) );
  mFilename->setFilter( filter );

  // if output filename already defined we need to replace old suffix
  // to avoid double extensions like .gpkg.shp
  if ( !mFilename->filePath().isEmpty() )
  {
    const thread_local QRegularExpression rx( "\\.(.*?)[\\s]" );
    const QString ext = rx.match( filter ).captured( 1 );
    if ( !ext.isEmpty() )
    {
      QFileInfo fi( mFilename->filePath() );
      mFilename->setFilePath( QStringLiteral( "%1/%2.%3" ).arg( fi.path(), fi.baseName(), ext ) );
    }
  }

  bool selectAllFields = true;

  // Is it a format for which fields that have attached widgets of types
  // ValueMap, ValueRelation, etc. should be by default exported with their displayed
  // values
  bool isFormatForFieldsAsDisplayedValues = false;

  const QString sFormat( format() );
  if ( sFormat == QLatin1String( "DXF" ) || sFormat == QLatin1String( "DGN" ) )
  {
    mAttributesSelection->setVisible( false );
    selectAllFields = false;
  }
  else
  {
    if ( mOptions & Option::Fields )
    {
      mAttributesSelection->setVisible( true );
      isFormatForFieldsAsDisplayedValues = ( sFormat == QLatin1String( "CSV" ) || sFormat == QLatin1String( "XLS" ) || sFormat == QLatin1String( "XLSX" ) || sFormat == QLatin1String( "ODS" ) );
    }
  }

  // Show symbology options only for some formats
  if ( QgsVectorFileWriter::supportsFeatureStyles( sFormat ) && ( mOptions & Option::Symbology ) )
  {
    mSymbologyExportLabel->setVisible( true );
    mSymbologyExportComboBox->setVisible( true );
    mScaleLabel->setVisible( true );
    mScaleWidget->setVisible( true );
  }
  else
  {
    mSymbologyExportLabel->hide();
    mSymbologyExportComboBox->hide();
    mScaleLabel->hide();
    mScaleWidget->hide();
  }

  leLayername->setEnabled( sFormat == QLatin1String( "KML" ) || sFormat == QLatin1String( "GPKG" ) || sFormat == QLatin1String( "XLSX" ) || sFormat == QLatin1String( "ODS" ) || sFormat == QLatin1String( "FileGDB" ) || sFormat == QLatin1String( "OpenFileGDB" ) || sFormat == QLatin1String( "SQLite" ) || sFormat == QLatin1String( "SpatiaLite" ) );

  if ( sFormat == QLatin1String( "XLSX" ) )
    leLayername->setMaxLength( 31 );
  else if ( leLayername->isEnabled() )
    leLayername->setMaxLength( 32767 ); // default length

  if ( !leLayername->isEnabled() )
    leLayername->setText( QString() );
  else if ( leLayername->text().isEmpty() )
  {
    QString layerName = mDefaultOutputLayerNameFromInputLayerName;
    if ( layerName.isEmpty() && !mFilename->filePath().isEmpty() )
    {
      layerName = QFileInfo( mFilename->filePath() ).baseName();
      leLayername->setDefaultValue( layerName );
    }
    if ( layerName.isEmpty() )
      layerName = tr( "new_layer" );
    leLayername->setText( layerName );
  }

  if ( mLayer )
  {
    mAttributeTable->setRowCount( mLayer->fields().count() );

    QStringList horizontalHeaders = QStringList() << tr( "Name" ) << tr( "Export name" ) << tr( "Type" ) << tr( "Replace with displayed values" );
    mAttributeTable->setColumnCount( horizontalHeaders.size() );
    mAttributeTable->setHorizontalHeaderLabels( horizontalHeaders );

    bool foundFieldThatCanBeExportedAsDisplayedValue = false;
    for ( int i = 0; i < mLayer->fields().size(); ++i )
    {
      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, mLayer->fields()[i].name() );
      if ( setup.type() != QLatin1String( "TextEdit" ) && QgsGui::editorWidgetRegistry()->factory( setup.type() ) )
      {
        foundFieldThatCanBeExportedAsDisplayedValue = true;
        break;
      }
    }
    mAttributeTable->setColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ), !foundFieldThatCanBeExportedAsDisplayedValue );

    bool checkReplaceRawFieldValues = selectAllFields && isFormatForFieldsAsDisplayedValues;
    const QSignalBlocker signalBlockerAttributeTable( mAttributeTable );
    {
      for ( int i = 0; i < mLayer->fields().size(); ++i )
      {
        QgsField fld = mLayer->fields().at( i );
        Qt::ItemFlags flags = mLayer->providerType() != QLatin1String( "oracle" ) || !fld.typeName().contains( QLatin1String( "SDO_GEOMETRY" ) ) ? Qt::ItemIsEnabled : Qt::NoItemFlags;
        QTableWidgetItem *item = nullptr;
        item = new QTableWidgetItem( fld.name() );
        item->setFlags( flags | Qt::ItemIsUserCheckable );
        item->setCheckState( ( selectAllFields ) ? Qt::Checked : Qt::Unchecked );
        mAttributeTable->setItem( i, static_cast<int>( ColumnIndex::Name ), item );

        item = new QTableWidgetItem( fld.name() );
        item->setFlags( flags | Qt::ItemIsEditable );
        item->setData( Qt::UserRole, fld.displayName() );
        mAttributeTable->setItem( i, static_cast<int>( ColumnIndex::ExportName ), item );

        item = new QTableWidgetItem( fld.typeName() );
        item->setFlags( flags );
        mAttributeTable->setItem( i, static_cast<int>( ColumnIndex::Type ), item );

        if ( foundFieldThatCanBeExportedAsDisplayedValue )
        {
          const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, mLayer->fields()[i].name() );
          QgsEditorWidgetFactory *factory = nullptr;
          const QString widgetId( setup.type() );
          if ( flags == Qt::ItemIsEnabled && widgetId != QLatin1String( "TextEdit" ) && ( factory = QgsGui::editorWidgetRegistry()->factory( widgetId ) ) )
          {
            item = new QTableWidgetItem( tr( "Use %1" ).arg( factory->name() ) );
            item->setFlags( ( selectAllFields ) ? ( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable ) : Qt::ItemIsUserCheckable );
            const bool checkItem = ( selectAllFields && isFormatForFieldsAsDisplayedValues && ( widgetId == QLatin1String( "ValueMap" ) || widgetId == QLatin1String( "ValueRelation" ) || widgetId == QLatin1String( "CheckBox" ) || widgetId == QLatin1String( "RelationReference" ) ) );
            checkReplaceRawFieldValues &= checkItem;
            item->setCheckState( checkItem ? Qt::Checked : Qt::Unchecked );
            mAttributeTable->setItem( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ), item );
          }
          else
          {
            item = new QTableWidgetItem();
            item->setFlags( Qt::NoItemFlags );
            mAttributeTable->setItem( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ), item );
          }
        }
      }
    }

    whileBlocking( mReplaceRawFieldValues )->setChecked( checkReplaceRawFieldValues );
    mReplaceRawFieldValues->setEnabled( selectAllFields );
    mReplaceRawFieldValues->setVisible( foundFieldThatCanBeExportedAsDisplayedValue );

    mAttributeTable->resizeColumnsToContents();
  }

  QgsVectorFileWriter::MetaData driverMetaData;

  while ( mDatasourceOptionsGroupBox->layout()->count() )
  {
    QLayoutItem *item = mDatasourceOptionsGroupBox->layout()->takeAt( 0 );
    delete item->widget();
    delete item;
  }

  while ( mLayerOptionsGroupBox->layout()->count() )
  {
    QLayoutItem *item = mLayerOptionsGroupBox->layout()->takeAt( 0 );
    delete item->widget();
    delete item;
  }

  typedef QPair<QLabel *, QWidget *> LabelControlPair;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    if ( !driverMetaData.driverOptions.empty() )
    {
      mDatasourceOptionsGroupBox->setVisible( true );
      QList<QPair<QLabel *, QWidget *>> controls = createControls( driverMetaData.driverOptions );

      QFormLayout *datasourceLayout = dynamic_cast<QFormLayout *>( mDatasourceOptionsGroupBox->layout() );

      const auto constControls = controls;
      for ( LabelControlPair control : constControls )
      {
        datasourceLayout->addRow( control.first, control.second );
      }
    }
    else
    {
      mDatasourceOptionsGroupBox->setVisible( false );
    }

    if ( !driverMetaData.layerOptions.empty() )
    {
      mLayerOptionsGroupBox->setVisible( true );
      QList<QPair<QLabel *, QWidget *>> controls = createControls( driverMetaData.layerOptions );

      QFormLayout *layerOptionsLayout = dynamic_cast<QFormLayout *>( mLayerOptionsGroupBox->layout() );

      const auto constControls = controls;
      for ( LabelControlPair control : constControls )
      {
        layerOptionsLayout->addRow( control.first, control.second );
      }
    }
    else
    {
      mLayerOptionsGroupBox->setVisible( false );
    }

    if ( driverMetaData.compulsoryEncoding.isEmpty() )
    {
      mEncodingComboBox->setEnabled( true );
    }
    else
    {
      int idx = mEncodingComboBox->findText( driverMetaData.compulsoryEncoding );
      if ( idx >= 0 )
      {
        mEncodingComboBox->setCurrentIndex( idx );
        mEncodingComboBox->setDisabled( true );
      }
      else
      {
        mEncodingComboBox->setEnabled( true );
      }
    }
  }
  else
  {
    mEncodingComboBox->setEnabled( true );
  }

  GDALDriverH hDriver = GDALGetDriverByName( format().toUtf8().constData() );
  if ( hDriver )
  {
    const bool canReopen = GDALGetMetadataItem( hDriver, GDAL_DCAP_OPEN, nullptr );
    if ( mAddToCanvas->isEnabled() && !canReopen )
    {
      mAddToCanvasStateOnOpenCompatibleDriver = mAddToCanvas->isChecked();
      mAddToCanvas->setChecked( false );
      mAddToCanvas->setEnabled( false );
    }
    else if ( !mAddToCanvas->isEnabled() && canReopen )
    {
      mAddToCanvas->setChecked( mAddToCanvasStateOnOpenCompatibleDriver );
      mAddToCanvas->setEnabled( true );
    }
  }
}

void QgsVectorLayerSaveAsDialog::mUseAliasesForExportedName_stateChanged( int state )
{
  const QSignalBlocker signalBlocker( mAttributeTable );

  switch ( state )
  {
    case Qt::Unchecked:
    {
      // Check for modified entries
      bool modifiedEntries = false;
      for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
      {
        if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportName ) )->text()
             != mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportName ) )->data( Qt::UserRole ).toString() )
        {
          modifiedEntries = true;
          break;
        }
      }

      if ( modifiedEntries )
      {
        if ( QMessageBox::question( this, tr( "Modified names" ), tr( "Some names were modified and will be overridden. Do you want to continue?" ) )
             == QMessageBox::No )
        {
          whileBlocking( mUseAliasesForExportedName )->setCheckState( Qt::PartiallyChecked );
          return;
        }
      }

      for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
      {
        mUseAliasesForExportedName->setTristate( false );
        mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportName ) )->setText( mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->text() );
      }
    }
    break;
    case Qt::Checked:
    {
      // Check for modified entries
      bool modifiedEntries = false;
      for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
      {
        if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportName ) )->text()
             != mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->text() )
          modifiedEntries = true;
      }

      if ( modifiedEntries )
      {
        if ( QMessageBox::question( this, tr( "Modified names" ), tr( "Some names were modified and will be overridden. Do you want to continue?" ) )
             == QMessageBox::No )
        {
          whileBlocking( mUseAliasesForExportedName )->setCheckState( Qt::PartiallyChecked );
          return;
        }
      }

      for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
      {
        mUseAliasesForExportedName->setTristate( false );
        const QString alias = mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportName ) )->data( Qt::UserRole ).toString();
        mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportName ) )->setText( alias );
      }
    }
    break;
    case Qt::PartiallyChecked:
      // Do nothing
      break;
  }
}

void QgsVectorLayerSaveAsDialog::mReplaceRawFieldValues_stateChanged( int )
{
  if ( mAttributeTable->isColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) )
    return;

  const QSignalBlocker signalBlockerAttributeTable( mAttributeTable );
  const QSignalBlocker signalBlockerReplaceRawFieldValues( mReplaceRawFieldValues );

  if ( mReplaceRawFieldValues->checkState() != Qt::PartiallyChecked )
  {
    for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    {
      if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->checkState() == Qt::Checked && mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) && mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->flags() & Qt::ItemIsEnabled )
      {
        mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->setCheckState( mReplaceRawFieldValues->checkState() );
      }
    }
  }
  mReplaceRawFieldValues->setTristate( false );
}

void QgsVectorLayerSaveAsDialog::mAttributeTable_itemChanged( QTableWidgetItem *item )
{
  const QSignalBlocker signalBlockerAttributeTable( mAttributeTable );
  const QSignalBlocker signalBlockerReplaceRawFieldValues( mReplaceRawFieldValues );

  int row = item->row();
  int column = item->column();

  switch ( static_cast<ColumnIndex>( column ) )
  {
    case ColumnIndex::Name:
    {
      if ( mAttributeTable->isColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) || !mAttributeTable->item( row, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) || !( mAttributeTable->item( row, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->flags() & Qt::ItemIsUserCheckable ) )
        return;

      if ( mAttributeTable->item( row, column )->checkState() == Qt::Unchecked )
      {
        mAttributeTable->item( row, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->setCheckState( Qt::Unchecked );
        mAttributeTable->item( row, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->setFlags( Qt::ItemIsUserCheckable );
        bool checkBoxEnabled = false;
        for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
        {
          if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) && mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->flags() & Qt::ItemIsEnabled )
          {
            checkBoxEnabled = true;
            break;
          }
        }
        mReplaceRawFieldValues->setEnabled( checkBoxEnabled );
        if ( !checkBoxEnabled )
          mReplaceRawFieldValues->setCheckState( Qt::Unchecked );
      }
      else if ( mAttributeTable->item( row, column )->checkState() == Qt::Checked )
      {
        mAttributeTable->item( row, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        mReplaceRawFieldValues->setEnabled( true );
      }
    }
    break;
    case ColumnIndex::ExportName:
    {
      // Check empty export name
      if ( item->text().isEmpty() )
      {
        QMessageBox::warning( this, tr( "Empty export name" ), tr( "Empty export name are not allowed." ) );
        return;
      }

      // Rename eventually duplicated names
      QStringList names = attributesExportNames();
      while ( names.count( item->text() ) > 1 )
        item->setText( QString( "%1_2" ).arg( item->text() ) );

      mUseAliasesForExportedName->setCheckState( Qt::PartiallyChecked );
    }
    break;
    case ColumnIndex::Type:
      // Nothing to do
      break;
    case ColumnIndex::ExportAsDisplayedValue:
    {
      if ( mAttributeTable->item( row, column )->flags() & Qt::ItemIsUserCheckable )
      {
        bool allChecked = true;
        bool allUnchecked = true;
        for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
        {
          if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) && mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->flags() & Qt::ItemIsEnabled )
          {
            if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->checkState() == Qt::Unchecked )
              allChecked = false;
            else
              allUnchecked = false;
          }
        }
        mReplaceRawFieldValues->setCheckState( ( !allChecked && !allUnchecked ) ? Qt::PartiallyChecked : ( allChecked ) ? Qt::Checked
                                                                                                                        : Qt::Unchecked );
      }
    }
    break;
  }
}

void QgsVectorLayerSaveAsDialog::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mSelectedCrs = crs;
  mExtentGroupBox->setOutputCrs( mSelectedCrs );
}

QString QgsVectorLayerSaveAsDialog::fileName() const
{
  return mFilename->filePath();
}

QString QgsVectorLayerSaveAsDialog::layerName() const
{
  return leLayername->text();
}

QString QgsVectorLayerSaveAsDialog::encoding() const
{
  return mEncodingComboBox->currentText();
}

QString QgsVectorLayerSaveAsDialog::format() const
{
  return mFormatComboBox->currentData().toString();
}

QgsCoordinateReferenceSystem QgsVectorLayerSaveAsDialog::crs() const
{
  return mSelectedCrs;
}

QStringList QgsVectorLayerSaveAsDialog::datasourceOptions() const
{
  QStringList options;

  QgsVectorFileWriter::MetaData driverMetaData;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    QMap<QString, QgsVectorFileWriter::Option *>::ConstIterator it;

    for ( it = driverMetaData.driverOptions.constBegin(); it != driverMetaData.driverOptions.constEnd(); ++it )
    {
      switch ( it.value()->type )
      {
        case QgsVectorFileWriter::Int:
        {
          QgsVectorFileWriter::IntOption *opt = dynamic_cast<QgsVectorFileWriter::IntOption *>( *it );
          QSpinBox *sb = mDatasourceOptionsGroupBox->findChild<QSpinBox *>( it.key() );
          if ( opt && sb && sb->value() != opt->defaultValue )
            options << QStringLiteral( "%1=%2" ).arg( it.key() ).arg( sb->value() );
          break;
        }

        case QgsVectorFileWriter::Set:
        {
          QgsVectorFileWriter::SetOption *opt = dynamic_cast<QgsVectorFileWriter::SetOption *>( *it );
          QComboBox *cb = mDatasourceOptionsGroupBox->findChild<QComboBox *>( it.key() );
          if ( opt && cb && cb->itemData( cb->currentIndex() ) != opt->defaultValue )
            options << QStringLiteral( "%1=%2" ).arg( it.key(), cb->currentText() );
          break;
        }

        case QgsVectorFileWriter::String:
        {
          QgsVectorFileWriter::StringOption *opt = dynamic_cast<QgsVectorFileWriter::StringOption *>( *it );
          QLineEdit *le = mDatasourceOptionsGroupBox->findChild<QLineEdit *>( it.key() );
          if ( opt && le && le->text() != opt->defaultValue )
            options << QStringLiteral( "%1=%2" ).arg( it.key(), le->text() );
          break;
        }

        case QgsVectorFileWriter::Hidden:
        {
          QgsVectorFileWriter::HiddenOption *opt = dynamic_cast<QgsVectorFileWriter::HiddenOption *>( it.value() );
          if ( !opt->mValue.isEmpty() )
            options << QStringLiteral( "%1=%2" ).arg( it.key(), opt->mValue );
          break;
        }
      }
    }
  }

  QString plainText = mOgrDatasourceOptions->toPlainText().trimmed();
  if ( !plainText.isEmpty() )
    options += plainText.split( '\n' );

  return options;
}

QStringList QgsVectorLayerSaveAsDialog::layerOptions() const
{
  QStringList options;

  QgsVectorFileWriter::MetaData driverMetaData;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    QMap<QString, QgsVectorFileWriter::Option *>::ConstIterator it;

    for ( it = driverMetaData.layerOptions.constBegin(); it != driverMetaData.layerOptions.constEnd(); ++it )
    {
      switch ( it.value()->type )
      {
        case QgsVectorFileWriter::Int:
        {
          QgsVectorFileWriter::IntOption *opt = dynamic_cast<QgsVectorFileWriter::IntOption *>( *it );
          QSpinBox *sb = mLayerOptionsGroupBox->findChild<QSpinBox *>( it.key() );
          if ( opt && sb && sb->value() != opt->defaultValue )
            options << QStringLiteral( "%1=%2" ).arg( it.key() ).arg( sb->value() );
          break;
        }

        case QgsVectorFileWriter::Set:
        {
          QgsVectorFileWriter::SetOption *opt = dynamic_cast<QgsVectorFileWriter::SetOption *>( *it );
          QComboBox *cb = mLayerOptionsGroupBox->findChild<QComboBox *>( it.key() );
          if ( opt && cb && cb->itemData( cb->currentIndex() ) != opt->defaultValue )
            options << QStringLiteral( "%1=%2" ).arg( it.key(), cb->currentText() );
          break;
        }

        case QgsVectorFileWriter::String:
        {
          QgsVectorFileWriter::StringOption *opt = dynamic_cast<QgsVectorFileWriter::StringOption *>( *it );
          QLineEdit *le = mLayerOptionsGroupBox->findChild<QLineEdit *>( it.key() );
          if ( opt && le && le->text() != opt->defaultValue )
            options << QStringLiteral( "%1=%2" ).arg( it.key(), le->text() );
          break;
        }

        case QgsVectorFileWriter::Hidden:
        {
          QgsVectorFileWriter::HiddenOption *opt = dynamic_cast<QgsVectorFileWriter::HiddenOption *>( it.value() );
          if ( !opt->mValue.isEmpty() )
            options << QStringLiteral( "%1=%2" ).arg( it.key(), opt->mValue );
          break;
        }
      }
    }
  }

  QString plainText = mOgrLayerOptions->toPlainText().trimmed();
  if ( !plainText.isEmpty() )
    options += plainText.split( '\n' );

  return options;
}

QgsAttributeList QgsVectorLayerSaveAsDialog::selectedAttributes() const
{
  QgsAttributeList attributes;

  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->checkState() == Qt::Checked )
    {
      attributes.append( i );
    }
  }

  return attributes;
}

QgsAttributeList QgsVectorLayerSaveAsDialog::attributesAsDisplayedValues() const
{
  QgsAttributeList attributes;

  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->checkState() == Qt::Checked && !mAttributeTable->isColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) && mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->checkState() == Qt::Checked )
    {
      attributes.append( i );
    }
  }

  return attributes;
}

QStringList QgsVectorLayerSaveAsDialog::attributesExportNames() const
{
  QStringList exportNames;
  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    exportNames.append( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportName ) )->text() );

  return exportNames;
}

bool QgsVectorLayerSaveAsDialog::addToCanvas() const
{
  return mAddToCanvas->isChecked();
}

void QgsVectorLayerSaveAsDialog::setAddToCanvas( bool enabled )
{
  mAddToCanvasStateOnOpenCompatibleDriver = enabled;
  if ( mAddToCanvas->isEnabled() )
    mAddToCanvas->setChecked( enabled );
}

Qgis::FeatureSymbologyExport QgsVectorLayerSaveAsDialog::symbologyExport() const
{
  return mSymbologyExportComboBox->currentData().value<Qgis::FeatureSymbologyExport>();
}

double QgsVectorLayerSaveAsDialog::scale() const
{
  return mScaleWidget->scale();
}

void QgsVectorLayerSaveAsDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  mScaleWidget->setMapCanvas( canvas );
  mScaleWidget->setShowCurrentScaleButton( true );
  mExtentGroupBox->setCurrentExtent( canvas->mapSettings().visibleExtent(), canvas->mapSettings().destinationCrs() );
}

bool QgsVectorLayerSaveAsDialog::hasFilterExtent() const
{
  return mExtentGroupBox->isChecked();
}

QgsRectangle QgsVectorLayerSaveAsDialog::filterExtent() const
{
  return mExtentGroupBox->outputExtent();
}

void QgsVectorLayerSaveAsDialog::setOnlySelected( bool onlySelected )
{
  mSelectedOnly->setChecked( onlySelected );
}

bool QgsVectorLayerSaveAsDialog::onlySelected() const
{
  return mSelectedOnly->isChecked();
}

bool QgsVectorLayerSaveAsDialog::persistMetadata() const
{
  return mCheckPersistMetadata->isChecked();
}

Qgis::WkbType QgsVectorLayerSaveAsDialog::geometryType() const
{
  int currentIndexData = mGeometryTypeComboBox->currentData().toInt();
  if ( currentIndexData == -1 )
  {
    //automatic
    return Qgis::WkbType::Unknown;
  }

  return static_cast<Qgis::WkbType>( currentIndexData );
}

bool QgsVectorLayerSaveAsDialog::automaticGeometryType() const
{
  int currentIndexData = mGeometryTypeComboBox->currentData().toInt();
  return currentIndexData == -1;
}

bool QgsVectorLayerSaveAsDialog::forceMulti() const
{
  return mForceMultiCheckBox->isChecked();
}

void QgsVectorLayerSaveAsDialog::setForceMulti( bool checked )
{
  mForceMultiCheckBox->setChecked( checked );
}

bool QgsVectorLayerSaveAsDialog::includeZ() const
{
  return mIncludeZCheckBox->isChecked();
}

QgsVectorFileWriter::ActionOnExistingFile QgsVectorLayerSaveAsDialog::creationActionOnExistingFile() const
{
  return mActionOnExistingFile;
}

void QgsVectorLayerSaveAsDialog::setIncludeZ( bool checked )
{
  mIncludeZCheckBox->setChecked( checked );
}

void QgsVectorLayerSaveAsDialog::mSymbologyExportComboBox_currentIndexChanged( const QString &text )
{
  bool scaleEnabled = true;
  if ( text == tr( "No symbology" ) )
  {
    scaleEnabled = false;
  }
  mScaleWidget->setEnabled( scaleEnabled );
  mScaleLabel->setEnabled( scaleEnabled );
}

void QgsVectorLayerSaveAsDialog::mGeometryTypeComboBox_currentIndexChanged( int )
{
  Qgis::WkbType currentIndexData = static_cast<Qgis::WkbType>( mGeometryTypeComboBox->currentData().toInt() );

  if ( mGeometryTypeComboBox->currentIndex() != -1 && currentIndexData != Qgis::WkbType::NoGeometry )
  {
    mForceMultiCheckBox->setEnabled( true );
    mIncludeZCheckBox->setEnabled( true );
  }
  else
  {
    mForceMultiCheckBox->setEnabled( false );
    mForceMultiCheckBox->setChecked( false );
    mIncludeZCheckBox->setEnabled( false );
    mIncludeZCheckBox->setChecked( false );
  }
}

void QgsVectorLayerSaveAsDialog::mSelectAllAttributes_clicked()
{
  const QSignalBlocker signalBlockerAttributeTable( mAttributeTable );
  const QSignalBlocker signalBlockerReplaceRawFieldValues( mReplaceRawFieldValues );

  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    if ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->flags() & Qt::ItemIsEnabled )
    {
      if ( !mAttributeTable->isColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) && ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->flags() & Qt::ItemIsUserCheckable ) )
      {
        mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
      }
      mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->setCheckState( Qt::Checked );
    }
  }
  if ( !mAttributeTable->isColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) )
  {
    mReplaceRawFieldValues->setEnabled( true );
  }
}

void QgsVectorLayerSaveAsDialog::mDeselectAllAttributes_clicked()
{
  const QSignalBlocker signalBlockerAttributeTable( mAttributeTable );
  const QSignalBlocker signalBlockerReplaceRawFieldValues( mReplaceRawFieldValues );

  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    mAttributeTable->item( i, static_cast<int>( ColumnIndex::Name ) )->setCheckState( Qt::Unchecked );
    if ( !mAttributeTable->isColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) && ( mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->flags() & Qt::ItemIsUserCheckable ) )
    {
      mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->setFlags( Qt::ItemIsUserCheckable );
      mAttributeTable->item( i, static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) )->setCheckState( Qt::Unchecked );
    }
  }
  if ( !mAttributeTable->isColumnHidden( static_cast<int>( ColumnIndex::ExportAsDisplayedValue ) ) )
  {
    mReplaceRawFieldValues->setCheckState( Qt::Unchecked );
    mReplaceRawFieldValues->setEnabled( false );
  }
}

void QgsVectorLayerSaveAsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-new-layers-from-an-existing-layer" ) );
}
