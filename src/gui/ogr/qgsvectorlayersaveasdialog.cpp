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

static const int COLUMN_IDX_NAME = 0;
static const int COLUMN_IDX_TYPE = 1;
static const int COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE = 2;

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( long srsid, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mSelectedCrs( QgsCoordinateReferenceSystem::fromSrsId( srsid ) )
  , mAttributeTableItemChangedSlotEnabled( true )
  , mReplaceRawFieldValuesStateChangedSlotEnabled( true )
  , mActionOnExistingFile( QgsVectorFileWriter::CreateOrOverwriteFile )
{
  setup();
}

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( QgsVectorLayer *layer, Options options, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mLayer( layer )
  , mAttributeTableItemChangedSlotEnabled( true )
  , mReplaceRawFieldValuesStateChangedSlotEnabled( true )
  , mActionOnExistingFile( QgsVectorFileWriter::CreateOrOverwriteFile )
  , mOptions( options )
{
  if ( layer )
  {
    mSelectedCrs = layer->crs();
    mLayerExtent = layer->extent();
  }
  setup();

  if ( !( mOptions & Symbology ) )
  {
    mSymbologyExportLabel->hide();
    mSymbologyExportComboBox->hide();
    mScaleLabel->hide();
    mScaleWidget->hide();
  }

  if ( !( mOptions & DestinationCrs ) )
  {
    mCrsLabel->hide();
    mCrsSelector->hide();
  }
  if ( !( mOptions & Fields ) )
    mAttributesSelection->hide();

  if ( !( mOptions & SelectedOnly ) )
    mSelectedOnly->hide();

  if ( !( mOptions & AddToCanvas ) )
    mAddToCanvas->hide();

  if ( !( mOptions & GeometryType ) )
    mGeometryGroupBox->hide();

  if ( !( mOptions & Extent ) )
    mExtentGroupBox->hide();

  if ( !( mOptions & Metadata ) )
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

  const QList< QgsVectorFileWriter::DriverDetails > drivers = QgsVectorFileWriter::ogrDriverList();
  mFormatComboBox->blockSignals( true );
  for ( const QgsVectorFileWriter::DriverDetails &driver : drivers )
  {
    mFormatComboBox->addItem( driver.longName, driver.driverName );
  }

  QgsSettings settings;
  QString format = settings.value( QStringLiteral( "UI/lastVectorFormat" ), "GPKG" ).toString();
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( format ) );
  mFormatComboBox->blockSignals( false );

  const auto addGeomItem = [this]( QgsWkbTypes::Type type )
  {
    mGeometryTypeComboBox->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), type );
  };

  //add geometry types to combobox
  mGeometryTypeComboBox->addItem( tr( "Automatic" ), -1 );
  addGeomItem( QgsWkbTypes::Point );
  addGeomItem( QgsWkbTypes::LineString );
  addGeomItem( QgsWkbTypes::Polygon );
  mGeometryTypeComboBox->addItem( QgsWkbTypes::translatedDisplayString( QgsWkbTypes::GeometryCollection ), QgsWkbTypes::GeometryCollection );
  addGeomItem( QgsWkbTypes::NoGeometry );
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
  mSymbologyExportComboBox->addItem( tr( "No Symbology" ), QgsVectorFileWriter::NoSymbology );
  mSymbologyExportComboBox->addItem( tr( "Feature Symbology" ), QgsVectorFileWriter::FeatureSymbology );
  mSymbologyExportComboBox->addItem( tr( "Symbol Layer Symbology" ), QgsVectorFileWriter::SymbolLayerSymbology );
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

QList<QPair<QLabel *, QWidget *> > QgsVectorLayerSaveAsDialog::createControls( const QMap<QString, QgsVectorFileWriter::Option *> &options )
{
  QList<QPair<QLabel *, QWidget *> > controls;
  QMap<QString, QgsVectorFileWriter::Option *>::ConstIterator it;

  for ( it = options.constBegin(); it != options.constEnd(); ++it )
  {
    QgsVectorFileWriter::Option *option = it.value();
    QLabel *label = new QLabel( it.key() );
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
            cb->addItem( tr( "<Default>" ), QVariant( QVariant::String ) );
          int idx = cb->findText( opt->defaultValue );
          if ( idx == -1 )
            idx = cb->findData( QVariant( QVariant::String ) );
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
    if ( QgsVectorFileWriter::areThereNewFieldsToCreate( filename(), layername(), mLayer, selectedAttributes() ) )
    {
      if ( QMessageBox::question( this,
                                  tr( "Save Vector Layer As" ),
                                  tr( "The existing layer has additional fields. Do you want to add the missing fields to the layer?" ) ) == QMessageBox::Yes )
      {
        mActionOnExistingFile = QgsVectorFileWriter::AppendToLayerAddFields;
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
  settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), QFileInfo( filename() ).absolutePath() );
  settings.setValue( QStringLiteral( "UI/lastVectorFormat" ), format() );
  settings.setValue( QStringLiteral( "UI/encoding" ), encoding() );
  QDialog::accept();
}

void QgsVectorLayerSaveAsDialog::mFormatComboBox_currentIndexChanged( int idx )
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
      mFilename->setFilePath( QStringLiteral( "%1/%2.%3" ).arg( fi.path() ).arg( fi.baseName() ).arg( ext ) );
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
    if ( mOptions & Fields )
    {
      mAttributesSelection->setVisible( true );
      isFormatForFieldsAsDisplayedValues = ( sFormat == QLatin1String( "CSV" ) ||
                                             sFormat == QLatin1String( "XLS" ) ||
                                             sFormat == QLatin1String( "XLSX" ) ||
                                             sFormat == QLatin1String( "ODS" ) );
    }
  }

  // Show symbology options only for some formats
  if ( QgsVectorFileWriter::supportsFeatureStyles( sFormat ) && ( mOptions & Symbology ) )
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

  leLayername->setEnabled( sFormat == QLatin1String( "KML" ) ||
                           sFormat == QLatin1String( "GPKG" ) ||
                           sFormat == QLatin1String( "XLSX" ) ||
                           sFormat == QLatin1String( "ODS" ) ||
                           sFormat == QLatin1String( "FileGDB" ) ||
                           sFormat == QLatin1String( "SQLite" ) ||
                           sFormat == QLatin1String( "SpatiaLite" ) );

  if ( sFormat == QLatin1String( "XLSX" ) )
    leLayername->setMaxLength( 31 );
  else if ( leLayername->isEnabled() )
    leLayername->setMaxLength( 32767 ); // default length

  if ( !leLayername->isEnabled() )
    leLayername->setText( QString() );
  else if ( leLayername->text().isEmpty() &&
            !mFilename->filePath().isEmpty() )
  {
    QString layerName = QFileInfo( mFilename->filePath() ).baseName();
    leLayername->setText( layerName );
  }

  if ( mLayer )
  {
    mAttributeTable->setRowCount( mLayer->fields().count() );

    bool foundFieldThatCanBeExportedAsDisplayedValue = false;
    for ( int i = 0; i < mLayer->fields().size(); ++i )
    {
      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, mLayer->fields()[i].name() );
      if ( setup.type() != QLatin1String( "TextEdit" ) &&
           QgsGui::editorWidgetRegistry()->factory( setup.type() ) )
      {
        foundFieldThatCanBeExportedAsDisplayedValue = true;
        break;
      }
    }
    if ( foundFieldThatCanBeExportedAsDisplayedValue )
    {
      mAttributeTable->setColumnCount( 3 );
      mAttributeTable->setHorizontalHeaderLabels( QStringList() << tr( "Name" ) << tr( "Type" ) << tr( "Replace with displayed values" ) );
    }
    else
    {
      mAttributeTable->setColumnCount( 2 );
      mAttributeTable->setHorizontalHeaderLabels( QStringList() << tr( "Name" ) << tr( "Type" ) );
    }

    mAttributeTableItemChangedSlotEnabled = false;

    bool checkReplaceRawFieldValues = selectAllFields && isFormatForFieldsAsDisplayedValues;
    for ( int i = 0; i < mLayer->fields().size(); ++i )
    {
      QgsField fld = mLayer->fields().at( i );
      Qt::ItemFlags flags = mLayer->providerType() != QLatin1String( "oracle" ) || !fld.typeName().contains( QLatin1String( "SDO_GEOMETRY" ) ) ? Qt::ItemIsEnabled : Qt::NoItemFlags;
      QTableWidgetItem *item = nullptr;
      item = new QTableWidgetItem( fld.name() );
      item->setFlags( flags | Qt::ItemIsUserCheckable );
      item->setCheckState( ( selectAllFields ) ? Qt::Checked : Qt::Unchecked );
      mAttributeTable->setItem( i, COLUMN_IDX_NAME, item );

      item = new QTableWidgetItem( fld.typeName() );
      item->setFlags( flags );
      mAttributeTable->setItem( i, COLUMN_IDX_TYPE, item );

      if ( foundFieldThatCanBeExportedAsDisplayedValue )
      {
        const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, mLayer->fields()[i].name() );
        QgsEditorWidgetFactory *factory = nullptr;
        const QString widgetId( setup.type() );
        if ( flags == Qt::ItemIsEnabled &&
             widgetId != QLatin1String( "TextEdit" ) &&
             ( factory = QgsGui::editorWidgetRegistry()->factory( widgetId ) ) )
        {
          item = new QTableWidgetItem( tr( "Use %1" ).arg( factory->name() ) );
          item->setFlags( ( selectAllFields ) ? ( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable ) : Qt::ItemIsUserCheckable );
          const bool checkItem = ( selectAllFields && isFormatForFieldsAsDisplayedValues &&
                                   ( widgetId == QLatin1String( "ValueMap" ) ||
                                     widgetId == QLatin1String( "ValueRelation" ) ||
                                     widgetId == QLatin1String( "CheckBox" ) ||
                                     widgetId == QLatin1String( "RelationReference" ) ) );
          checkReplaceRawFieldValues &= checkItem;
          item->setCheckState( checkItem ?
                               Qt::Checked : Qt::Unchecked );
          mAttributeTable->setItem( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE, item );
        }
        else
        {
          item = new QTableWidgetItem();
          item->setFlags( Qt::NoItemFlags );
          mAttributeTable->setItem( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE, item );
        }
      }
    }

    mAttributeTableItemChangedSlotEnabled = true;

    mReplaceRawFieldValuesStateChangedSlotEnabled = false;
    mReplaceRawFieldValues->setChecked( checkReplaceRawFieldValues );
    mReplaceRawFieldValuesStateChangedSlotEnabled = true;
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
      QList<QPair<QLabel *, QWidget *> > controls = createControls( driverMetaData.driverOptions );

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
      QList<QPair<QLabel *, QWidget *> > controls = createControls( driverMetaData.layerOptions );

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
    mAddToCanvas->setEnabled( GDALGetMetadataItem( hDriver, GDAL_DCAP_OPEN, nullptr ) != nullptr );
  }
}

void QgsVectorLayerSaveAsDialog::mReplaceRawFieldValues_stateChanged( int )
{
  if ( !mReplaceRawFieldValuesStateChangedSlotEnabled )
    return;
  if ( mAttributeTable->columnCount() != 3 )
    return;
  mReplaceRawFieldValuesStateChangedSlotEnabled = false;
  mAttributeTableItemChangedSlotEnabled = false;
  if ( mReplaceRawFieldValues->checkState() != Qt::PartiallyChecked )
  {
    for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    {
      if ( mAttributeTable->item( i, COLUMN_IDX_NAME )->checkState() == Qt::Checked &&
           mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE ) &&
           mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->flags() & Qt::ItemIsEnabled )
      {
        mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->setCheckState( mReplaceRawFieldValues->checkState() );
      }
    }
  }
  mReplaceRawFieldValues->setTristate( false );
  mAttributeTableItemChangedSlotEnabled = true;
  mReplaceRawFieldValuesStateChangedSlotEnabled = true;
}

void QgsVectorLayerSaveAsDialog::mAttributeTable_itemChanged( QTableWidgetItem *item )
{
  if ( !mAttributeTableItemChangedSlotEnabled )
    return;
  mReplaceRawFieldValuesStateChangedSlotEnabled = false;
  mAttributeTableItemChangedSlotEnabled = false;
  int row = item->row();
  int column = item->column();
  if ( column == COLUMN_IDX_NAME &&
       mAttributeTable->item( row, column )->checkState() == Qt::Unchecked &&
       mAttributeTable->columnCount() == 3 &&
       mAttributeTable->item( row, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE ) &&
       ( mAttributeTable->item( row, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->flags() & Qt::ItemIsUserCheckable ) )
  {
    mAttributeTable->item( row, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->setCheckState( Qt::Unchecked );
    mAttributeTable->item( row, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->setFlags( Qt::ItemIsUserCheckable );
    bool checkBoxEnabled = false;
    for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    {
      if ( mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE ) &&
           mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->flags() & Qt::ItemIsEnabled )
      {
        checkBoxEnabled = true;
        break;
      }
    }
    mReplaceRawFieldValues->setEnabled( checkBoxEnabled );
    if ( !checkBoxEnabled )
      mReplaceRawFieldValues->setCheckState( Qt::Unchecked );
  }
  else if ( column == COLUMN_IDX_NAME &&
            mAttributeTable->item( row, column )->checkState() == Qt::Checked &&
            mAttributeTable->columnCount() == 3 &&
            mAttributeTable->item( row, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE ) &&
            ( mAttributeTable->item( row, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->flags() & Qt::ItemIsUserCheckable ) )
  {
    mAttributeTable->item( row, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
    mReplaceRawFieldValues->setEnabled( true );
  }
  else if ( column == COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE &&
            ( mAttributeTable->item( row, column )->flags() & Qt::ItemIsUserCheckable ) )
  {
    bool allChecked = true;
    bool allUnchecked = true;
    for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    {
      if ( mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE ) &&
           mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->flags() & Qt::ItemIsEnabled )
      {
        if ( mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->checkState() == Qt::Unchecked )
          allChecked = false;
        else
          allUnchecked = false;
      }
    }
    mReplaceRawFieldValues->setCheckState( ( !allChecked && !allUnchecked ) ? Qt::PartiallyChecked : ( allChecked ) ? Qt::Checked : Qt::Unchecked );
  }
  mAttributeTableItemChangedSlotEnabled = true;
  mReplaceRawFieldValuesStateChangedSlotEnabled = true;
}

void QgsVectorLayerSaveAsDialog::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mSelectedCrs = crs;
  mExtentGroupBox->setOutputCrs( mSelectedCrs );
}

QString QgsVectorLayerSaveAsDialog::filename() const
{
  return mFilename->filePath();
}

QString QgsVectorLayerSaveAsDialog::layername() const
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

long QgsVectorLayerSaveAsDialog::crs() const
{
  return mSelectedCrs.srsid();
}

QgsCoordinateReferenceSystem QgsVectorLayerSaveAsDialog::crsObject() const
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
          QgsVectorFileWriter::HiddenOption *opt =
            dynamic_cast<QgsVectorFileWriter::HiddenOption *>( it.value() );
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
          QgsVectorFileWriter::HiddenOption *opt =
            dynamic_cast<QgsVectorFileWriter::HiddenOption *>( it.value() );
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
    if ( mAttributeTable->item( i, COLUMN_IDX_NAME )->checkState() == Qt::Checked )
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
    if ( mAttributeTable->item( i, COLUMN_IDX_NAME )->checkState() == Qt::Checked &&
         mAttributeTable->columnCount() == 3 &&
         mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->checkState() == Qt::Checked )
    {
      attributes.append( i );
    }
  }

  return attributes;
}

bool QgsVectorLayerSaveAsDialog::addToCanvas() const
{
  return mAddToCanvas->isChecked() && mAddToCanvas->isEnabled();
}

void QgsVectorLayerSaveAsDialog::setAddToCanvas( bool enabled )
{
  mAddToCanvas->setChecked( enabled );
}

int QgsVectorLayerSaveAsDialog::symbologyExport() const
{
  return mSymbologyExportComboBox->currentData().toInt();
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

QgsWkbTypes::Type QgsVectorLayerSaveAsDialog::geometryType() const
{
  int currentIndexData = mGeometryTypeComboBox->currentData().toInt();
  if ( currentIndexData == -1 )
  {
    //automatic
    return QgsWkbTypes::Unknown;
  }

  return static_cast< QgsWkbTypes::Type >( currentIndexData );
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

void QgsVectorLayerSaveAsDialog::mGeometryTypeComboBox_currentIndexChanged( int index )
{
  int currentIndexData = mGeometryTypeComboBox->itemData( index ).toInt();

  if ( currentIndexData != -1 && currentIndexData != QgsWkbTypes::NoGeometry )
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
  mAttributeTableItemChangedSlotEnabled = false;
  mReplaceRawFieldValuesStateChangedSlotEnabled = false;
  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    if ( mAttributeTable->item( i, COLUMN_IDX_NAME )->flags() & Qt::ItemIsEnabled )
    {
      if ( mAttributeTable->columnCount() == 3 &&
           ( mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->flags() & Qt::ItemIsUserCheckable ) )
      {
        mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
      }
      mAttributeTable->item( i, COLUMN_IDX_NAME )->setCheckState( Qt::Checked );
    }
  }
  if ( mAttributeTable->columnCount() == 3 )
  {
    mReplaceRawFieldValues->setEnabled( true );
  }
  mAttributeTableItemChangedSlotEnabled = true;
  mReplaceRawFieldValuesStateChangedSlotEnabled = true;
}

void QgsVectorLayerSaveAsDialog::mDeselectAllAttributes_clicked()
{
  mAttributeTableItemChangedSlotEnabled = false;
  mReplaceRawFieldValuesStateChangedSlotEnabled = false;
  for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
  {
    mAttributeTable->item( i, COLUMN_IDX_NAME )->setCheckState( Qt::Unchecked );
    if ( mAttributeTable->columnCount() == 3 &&
         ( mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->flags() & Qt::ItemIsUserCheckable ) )
    {
      mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->setFlags( Qt::ItemIsUserCheckable );
      mAttributeTable->item( i, COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE )->setCheckState( Qt::Unchecked );
    }
  }
  if ( mAttributeTable->columnCount() == 3 )
  {
    mReplaceRawFieldValues->setCheckState( Qt::Unchecked );
    mReplaceRawFieldValues->setEnabled( false );
  }
  mAttributeTableItemChangedSlotEnabled = true;
  mReplaceRawFieldValuesStateChangedSlotEnabled = true;
}

void QgsVectorLayerSaveAsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-new-layers-from-an-existing-layer" ) );
}
