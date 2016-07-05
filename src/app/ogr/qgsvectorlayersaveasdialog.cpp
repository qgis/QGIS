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
#include "qgscrscache.h"
#include "qgsvectorlayersaveasdialog.h"
#include "qgsgenericprojectionselector.h"
#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"

#include <QSettings>
#include <QFileDialog>
#include <QTextCodec>

static const int COLUMN_IDX_NAME = 0;
static const int COLUMN_IDX_TYPE = 1;
static const int COLUMN_IDX_EXPORT_AS_DISPLAYED_VALUE = 2;

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( long srsid, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mCRS( srsid )
    , mLayer( 0 )
    , mAttributeTableItemChangedSlotEnabled( true )
    , mReplaceRawFieldValuesStateChangedSlotEnabled( true )
{
  setup();
}

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( QgsVectorLayer *layer, int options, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mLayer( layer )
    , mAttributeTableItemChangedSlotEnabled( true )
    , mReplaceRawFieldValuesStateChangedSlotEnabled( true )
{
  if ( layer )
  {
    mCRS = layer->crs().srsid();
    mLayerExtent = layer->extent();
  }
  setup();
  if ( !( options & Symbology ) )
  {
    mSymbologyExportLabel->hide();
    mSymbologyExportComboBox->hide();
    mScaleLabel->hide();
    mScaleSpinBox->hide();
  }

  mSelectedOnly->setEnabled( layer && layer->selectedFeatureCount() != 0 );
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
}

void QgsVectorLayerSaveAsDialog::setup()
{
  setupUi( this );
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/VectorLayerSaveAs/geometry" ).toByteArray() );

  QMap<QString, QString> map = QgsVectorFileWriter::ogrDriverList();
  mFormatComboBox->blockSignals( true );
  for ( QMap< QString, QString>::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
  {
    mFormatComboBox->addItem( it.key(), it.value() );
  }

  QString format = settings.value( "/UI/lastVectorFormat", "ESRI Shapefile" ).toString();
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( format ) );
  mFormatComboBox->blockSignals( false );

  //add geometry types to combobox
  mGeometryTypeComboBox->addItem( tr( "Automatic" ), -1 );
  mGeometryTypeComboBox->addItem( QgsWKBTypes::displayString( QgsWKBTypes::Point ), QgsWKBTypes::Point );
  mGeometryTypeComboBox->addItem( QgsWKBTypes::displayString( QgsWKBTypes::LineString ), QgsWKBTypes::LineString );
  mGeometryTypeComboBox->addItem( QgsWKBTypes::displayString( QgsWKBTypes::Polygon ), QgsWKBTypes::Polygon );
  mGeometryTypeComboBox->addItem( QgsWKBTypes::displayString( QgsWKBTypes::GeometryCollection ), QgsWKBTypes::GeometryCollection );
  mGeometryTypeComboBox->addItem( tr( "No geometry" ), QgsWKBTypes::NoGeometry );
  mGeometryTypeComboBox->setCurrentIndex( mGeometryTypeComboBox->findData( -1 ) );

  mEncodingComboBox->addItems( QgsVectorDataProvider::availableEncodings() );

  QString enc = settings.value( "/UI/encoding", "System" ).toString();
  int idx = mEncodingComboBox->findText( enc );
  if ( idx < 0 )
  {
    mEncodingComboBox->insertItem( 0, enc );
    idx = 0;
  }

  QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsBySrsId( mCRS );
  mCrsSelector->setCrs( srs );
  mCrsSelector->setLayerCrs( srs );
  mCrsSelector->dialog()->setMessage( tr( "Select the coordinate reference system for the vector file. "
                                          "The data points will be transformed from the layer coordinate reference system." ) );

  mEncodingComboBox->setCurrentIndex( idx );
  on_mFormatComboBox_currentIndexChanged( mFormatComboBox->currentIndex() );

  //symbology export combo box
  mSymbologyExportComboBox->addItem( tr( "No symbology" ), QgsVectorFileWriter::NoSymbology );
  mSymbologyExportComboBox->addItem( tr( "Feature symbology" ), QgsVectorFileWriter::FeatureSymbology );
  mSymbologyExportComboBox->addItem( tr( "Symbol layer symbology" ), QgsVectorFileWriter::SymbolLayerSymbology );
  on_mSymbologyExportComboBox_currentIndexChanged( mSymbologyExportComboBox->currentText() );

  // extent group box
  mExtentGroupBox->setOutputCrs( srs );
  mExtentGroupBox->setOriginalExtent( mLayerExtent, srs );
  mExtentGroupBox->setOutputExtentFromOriginal();
  mExtentGroupBox->setCheckable( true );
  mExtentGroupBox->setChecked( false );
  mExtentGroupBox->setCollapsed( true );
}

QList<QPair<QLabel*, QWidget*> > QgsVectorLayerSaveAsDialog::createControls( const QMap<QString, QgsVectorFileWriter::Option*>& options )
{
  QList<QPair<QLabel*, QWidget*> > controls;
  QMap<QString, QgsVectorFileWriter::Option*>::ConstIterator it;

  for ( it = options.constBegin(); it != options.constEnd(); ++it )
  {
    QgsVectorFileWriter::Option *option = it.value();
    QLabel *label = new QLabel( it.key() );
    QWidget *control = nullptr;
    switch ( option->type )
    {
      case QgsVectorFileWriter::Int:
      {
        QgsVectorFileWriter::IntOption *opt = dynamic_cast<QgsVectorFileWriter::IntOption*>( option );
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
        QgsVectorFileWriter::SetOption *opt = dynamic_cast<QgsVectorFileWriter::SetOption*>( option );
        if ( opt )
        {
          QComboBox* cb = new QComboBox();
          cb->setObjectName( it.key() );
          Q_FOREACH ( const QString& val, opt->values )
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
        QgsVectorFileWriter::StringOption *opt = dynamic_cast<QgsVectorFileWriter::StringOption*>( option );
        if ( opt )
        {
          QLineEdit* le = new QLineEdit( opt->defaultValue );
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
      label->setToolTip( QString( "<p>%1</p>" ).arg( option->docString ) );
      control->setToolTip( QString( "<p>%1</p>" ).arg( option->docString ) );

      controls << QPair<QLabel*, QWidget*>( label, control );
    }
  }

  return controls;
}

QgsVectorLayerSaveAsDialog::~QgsVectorLayerSaveAsDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/VectorLayerSaveAs/geometry", saveGeometry() );
}

void QgsVectorLayerSaveAsDialog::accept()
{
  QSettings settings;
  settings.setValue( "/UI/lastVectorFileFilterDir", QFileInfo( filename() ).absolutePath() );
  settings.setValue( "/UI/lastVectorFormat", format() );
  settings.setValue( "/UI/encoding", encoding() );
  QDialog::accept();
}

void QgsVectorLayerSaveAsDialog::on_mFormatComboBox_currentIndexChanged( int idx )
{
  Q_UNUSED( idx );

  browseFilename->setEnabled( true );
  leFilename->setEnabled( true );
  bool selectAllFields = true;
  bool fieldsAsDisplayedValues = false;

  if ( format() == "KML" )
  {
    mAttributesSelection->setEnabled( true );
    selectAllFields = false;
  }
  else if ( format() == "DXF" )
  {
    mAttributesSelection->setEnabled( false );
    selectAllFields = false;
  }
  else
  {
    mAttributesSelection->setEnabled( true );
    fieldsAsDisplayedValues = ( format() == "CSV" || format() == "XLS" || format() == "XLSX" || format() == "ODS" );
  }

  if ( mLayer )
  {
    mAttributeTable->setRowCount( mLayer->fields().count() );

    bool foundFieldThatCanBeExportedAsDisplayedValue = false;
    for ( int i = 0; i < mLayer->fields().size(); ++i )
    {
      if ( mLayer->editFormConfig()->widgetType( i ) != "TextEdit" &&
           QgsEditorWidgetRegistry::instance()->factory( mLayer->editFormConfig()->widgetType( i ) ) )
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

    for ( int i = 0; i < mLayer->fields().size(); ++i )
    {
      const QgsField &fld = mLayer->fields().at( i );
      Qt::ItemFlags flags = mLayer->providerType() != "oracle" || !fld.typeName().contains( "SDO_GEOMETRY" ) ? Qt::ItemIsEnabled : Qt::NoItemFlags;
      QTableWidgetItem *item;
      item = new QTableWidgetItem( fld.name() );
      item->setFlags( flags | Qt::ItemIsUserCheckable );
      item->setCheckState(( selectAllFields ) ? Qt::Checked : Qt::Unchecked );
      mAttributeTable->setItem( i, COLUMN_IDX_NAME, item );

      item = new QTableWidgetItem( fld.typeName() );
      item->setFlags( flags );
      mAttributeTable->setItem( i, COLUMN_IDX_TYPE, item );

      if ( foundFieldThatCanBeExportedAsDisplayedValue )
      {
        QgsEditorWidgetFactory *factory;
        if ( flags == Qt::ItemIsEnabled &&
             mLayer->editFormConfig()->widgetType( i ) != "TextEdit" &&
             ( factory = QgsEditorWidgetRegistry::instance()->factory( mLayer->editFormConfig()->widgetType( i ) ) ) )
        {
          item = new QTableWidgetItem( tr( "Use %1" ).arg( factory->name() ) );
          item->setFlags(( selectAllFields ) ? ( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable ) : Qt::ItemIsUserCheckable );
          item->setCheckState(( selectAllFields && fieldsAsDisplayedValues ) ? Qt::Checked : Qt::Unchecked );
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
    mReplaceRawFieldValues->setChecked( selectAllFields && fieldsAsDisplayedValues );
    mReplaceRawFieldValuesStateChangedSlotEnabled = true;
    mReplaceRawFieldValues->setEnabled( selectAllFields );
    mReplaceRawFieldValues->setVisible( foundFieldThatCanBeExportedAsDisplayedValue );

    mAttributeTable->resizeColumnsToContents();
  }

  QgsVectorFileWriter::MetaData driverMetaData;

  while ( mDatasourceOptionsGroupBox->layout()->count() )
  {
    QLayoutItem* item = mDatasourceOptionsGroupBox->layout()->takeAt( 0 );
    delete item->widget();
    delete item;
  }

  while ( mLayerOptionsGroupBox->layout()->count() )
  {
    QLayoutItem* item = mLayerOptionsGroupBox->layout()->takeAt( 0 );
    delete item->widget();
    delete item;
  }

  // workaround so the Q_FOREACH macro does not get confused by the ','
  typedef QPair<QLabel*, QWidget*> LabelControlPair;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    if ( driverMetaData.driverOptions.size() != 0 )
    {
      mDatasourceOptionsGroupBox->setVisible( true );
      QList<QPair<QLabel*, QWidget*> > controls = createControls( driverMetaData.driverOptions );

      QFormLayout* datasourceLayout = dynamic_cast<QFormLayout*>( mDatasourceOptionsGroupBox->layout() );

      Q_FOREACH ( LabelControlPair control, controls )
      {
        datasourceLayout->addRow( control.first, control.second );
      }
    }
    else
    {
      mDatasourceOptionsGroupBox->setVisible( false );
    }

    if ( driverMetaData.layerOptions.size() != 0 )
    {
      mLayerOptionsGroupBox->setVisible( true );
      QList<QPair<QLabel*, QWidget*> > controls = createControls( driverMetaData.layerOptions );

      QFormLayout* layerOptionsLayout = dynamic_cast<QFormLayout*>( mLayerOptionsGroupBox->layout() );

      Q_FOREACH ( LabelControlPair control, controls )
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
}

void QgsVectorLayerSaveAsDialog::on_mReplaceRawFieldValues_stateChanged( int )
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

void QgsVectorLayerSaveAsDialog::on_mAttributeTable_itemChanged( QTableWidgetItem * item )
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
    mReplaceRawFieldValues->setCheckState(( !allChecked && !allUnchecked ) ? Qt::PartiallyChecked : ( allChecked ) ? Qt::Checked : Qt::Unchecked );
  }
  mAttributeTableItemChangedSlotEnabled = true;
  mReplaceRawFieldValuesStateChangedSlotEnabled = true;
}

void QgsVectorLayerSaveAsDialog::on_leFilename_textChanged( const QString& text )
{
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( QFileInfo( text ).absoluteDir().exists() );
}

void QgsVectorLayerSaveAsDialog::on_browseFilename_clicked()
{
  QSettings settings;
  QString dirName = leFilename->text().isEmpty() ? settings.value( "/UI/lastVectorFileFilterDir", QDir::homePath() ).toString() : leFilename->text();
  QString filterString = QgsVectorFileWriter::filterForDriver( format() );
  QString outputFile = QFileDialog::getSaveFileName( nullptr, tr( "Save layer as..." ), dirName, filterString );
  if ( !outputFile.isNull() )
  {
    leFilename->setText( outputFile );
  }
}

void QgsVectorLayerSaveAsDialog::on_mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem& crs )
{
  mCRS = crs.srsid();
  mExtentGroupBox->setOutputCrs( crs );
}

QString QgsVectorLayerSaveAsDialog::filename() const
{
  return leFilename->text();
}

QString QgsVectorLayerSaveAsDialog::encoding() const
{
  return mEncodingComboBox->currentText();
}

QString QgsVectorLayerSaveAsDialog::format() const
{
  return mFormatComboBox->itemData( mFormatComboBox->currentIndex() ).toString();
}

long QgsVectorLayerSaveAsDialog::crs() const
{
  return mCRS;
}

QStringList QgsVectorLayerSaveAsDialog::datasourceOptions() const
{
  QStringList options;

  QgsVectorFileWriter::MetaData driverMetaData;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    QMap<QString, QgsVectorFileWriter::Option*>::ConstIterator it;

    for ( it = driverMetaData.driverOptions.constBegin(); it != driverMetaData.driverOptions.constEnd(); ++it )
    {
      switch ( it.value()->type )
      {
        case QgsVectorFileWriter::Int:
        {
          QSpinBox* sb = mDatasourceOptionsGroupBox->findChild<QSpinBox*>( it.key() );
          if ( sb )
            options << QString( "%1=%2" ).arg( it.key() ).arg( sb->value() );
          break;
        }

        case QgsVectorFileWriter::Set:
        {
          QComboBox* cb = mDatasourceOptionsGroupBox->findChild<QComboBox*>( it.key() );
          if ( cb && !cb->itemData( cb->currentIndex() ).isNull() )
            options << QString( "%1=%2" ).arg( it.key(), cb->currentText() );
          break;
        }

        case QgsVectorFileWriter::String:
        {
          QLineEdit* le = mDatasourceOptionsGroupBox->findChild<QLineEdit*>( it.key() );
          if ( le )
            options << QString( "%1=%2" ).arg( it.key(), le->text() );
          break;
        }

        case QgsVectorFileWriter::Hidden:
        {
          QgsVectorFileWriter::HiddenOption *opt =
            dynamic_cast<QgsVectorFileWriter::HiddenOption*>( it.value() );
          options << QString( "%1=%2" ).arg( it.key(), opt->mValue );
          break;
        }
      }
    }
  }

  return options + mOgrDatasourceOptions->toPlainText().split( '\n' );
}

QStringList QgsVectorLayerSaveAsDialog::layerOptions() const
{
  QStringList options;

  QgsVectorFileWriter::MetaData driverMetaData;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    QMap<QString, QgsVectorFileWriter::Option*>::ConstIterator it;

    for ( it = driverMetaData.layerOptions.constBegin(); it != driverMetaData.layerOptions.constEnd(); ++it )
    {
      switch ( it.value()->type )
      {
        case QgsVectorFileWriter::Int:
        {
          QSpinBox* sb = mLayerOptionsGroupBox->findChild<QSpinBox*>( it.key() );
          if ( sb )
            options << QString( "%1=%2" ).arg( it.key() ).arg( sb->value() );
          break;
        }

        case QgsVectorFileWriter::Set:
        {
          QComboBox* cb = mLayerOptionsGroupBox->findChild<QComboBox*>( it.key() );
          if ( cb && !cb->itemData( cb->currentIndex() ).isNull() )
            options << QString( "%1=%2" ).arg( it.key(), cb->currentText() );
          break;
        }

        case QgsVectorFileWriter::String:
        {
          QLineEdit* le = mLayerOptionsGroupBox->findChild<QLineEdit*>( it.key() );
          if ( le )
            options << QString( "%1=%2" ).arg( it.key(), le->text() );
          break;
        }

        case QgsVectorFileWriter::Hidden:
        {
          QgsVectorFileWriter::HiddenOption *opt =
            dynamic_cast<QgsVectorFileWriter::HiddenOption*>( it.value() );
          options << QString( "%1=%2" ).arg( it.key(), opt->mValue );
          break;
        }
      }
    }
  }

  return options + mOgrLayerOptions->toPlainText().split( '\n' );
}

bool QgsVectorLayerSaveAsDialog::attributeSelection() const
{
  return true;
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
  return mAddToCanvas->isChecked();
}

int QgsVectorLayerSaveAsDialog::symbologyExport() const
{
  return mSymbologyExportComboBox->itemData( mSymbologyExportComboBox->currentIndex() ).toInt();
}

double QgsVectorLayerSaveAsDialog::scaleDenominator() const
{
  return mScaleSpinBox->value();
}

void QgsVectorLayerSaveAsDialog::setCanvasExtent( const QgsRectangle& canvasExtent, const QgsCoordinateReferenceSystem& canvasCrs )
{
  mExtentGroupBox->setCurrentExtent( canvasExtent, canvasCrs );
}

bool QgsVectorLayerSaveAsDialog::hasFilterExtent() const
{
  return mExtentGroupBox->isChecked();
}

QgsRectangle QgsVectorLayerSaveAsDialog::filterExtent() const
{
  return mExtentGroupBox->outputExtent();
}

bool QgsVectorLayerSaveAsDialog::onlySelected() const
{
  return mSelectedOnly->isChecked();
}

QgsWKBTypes::Type QgsVectorLayerSaveAsDialog::geometryType() const
{
  int currentIndexData = mGeometryTypeComboBox->itemData( mGeometryTypeComboBox->currentIndex() ).toInt();
  if ( currentIndexData == -1 )
  {
    //automatic
    return QgsWKBTypes::Unknown;
  }

  return ( QgsWKBTypes::Type )currentIndexData;
}

bool QgsVectorLayerSaveAsDialog::automaticGeometryType() const
{
  int currentIndexData = mGeometryTypeComboBox->itemData( mGeometryTypeComboBox->currentIndex() ).toInt();
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

void QgsVectorLayerSaveAsDialog::setIncludeZ( bool checked )
{
  mIncludeZCheckBox->setChecked( checked );
}

void QgsVectorLayerSaveAsDialog::on_mSymbologyExportComboBox_currentIndexChanged( const QString& text )
{
  bool scaleEnabled = true;
  if ( text == tr( "No symbology" ) )
  {
    scaleEnabled = false;
  }
  mScaleSpinBox->setEnabled( scaleEnabled );
  mScaleLabel->setEnabled( scaleEnabled );
}

void QgsVectorLayerSaveAsDialog::on_mGeometryTypeComboBox_currentIndexChanged( int index )
{
  int currentIndexData = mGeometryTypeComboBox->itemData( index ).toInt();

  mForceMultiCheckBox->setEnabled( currentIndexData != -1 );
  mIncludeZCheckBox->setEnabled( currentIndexData != -1 );
}

void QgsVectorLayerSaveAsDialog::on_mSelectAllAttributes_clicked()
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

void QgsVectorLayerSaveAsDialog::on_mDeselectAllAttributes_clicked()
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
