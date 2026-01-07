/***************************************************************************
  qgsdbimportvectorlayerdialog.cpp
  --------------------------------------
  Date                 : March 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdbimportvectorlayerdialog.h"

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsdatabaseschemacombobox.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"

#include <QItemSelectionModel>
#include <QMenu>
#include <QPushButton>

#include "moc_qgsdbimportvectorlayerdialog.cpp"

QgsDbImportVectorLayerDialog::QgsDbImportVectorLayerDialog( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent )
  : QDialog( parent )
  , mConnection( connection )
{
  setupUi( this );
  setObjectName( "QgsDbImportVectorLayerDialog" );
  QgsGui::enableAutoGeometryRestore( this );

  mSourceLayerComboBox->setFilters( Qgis::LayerFilter::VectorLayer );
  connect( mSourceLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsDbImportVectorLayerDialog::sourceLayerComboChanged );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, [this]( const QgsCoordinateReferenceSystem &crs ) {
    mExtentGroupBox->setOutputCrs( crs );
  } );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsDbImportVectorLayerDialog::doImport );

  Q_ASSERT( connection );

  mFieldsView->setDestinationEditable( true );
  try
  {
    mFieldsView->setNativeTypes( connection->nativeTypes() );
  }
  catch ( QgsProviderConnectionException &e )
  {
    QgsDebugError( u"Could not retrieve connection native types: %1"_s.arg( e.what() ) );
  }
  connect( mResetButton, &QPushButton::clicked, this, &QgsDbImportVectorLayerDialog::loadFieldsFromLayer );
  connect( mAddButton, &QPushButton::clicked, this, &QgsDbImportVectorLayerDialog::addField );
  connect( mDeleteButton, &QPushButton::clicked, mFieldsView, &QgsFieldMappingWidget::removeSelectedFields );
  connect( mUpButton, &QPushButton::clicked, mFieldsView, &QgsFieldMappingWidget::moveSelectedFieldsUp );
  connect( mDownButton, &QPushButton::clicked, mFieldsView, &QgsFieldMappingWidget::moveSelectedFieldsDown );

  mEditButton->setPopupMode( QToolButton::InstantPopup ); // Show menu on button click

  // Create a menu for the dropdown
  QMenu *menu = new QMenu( mEditButton );

  // Add menu items
  menu->addAction( tr( "Convert All Field Names to Lowercase" ), this, [this]() {
    QgsFieldMappingModel *model = mFieldsView->model();
    for ( int i = 0; i < model->rowCount(); i++ )
    {
      const QModelIndex index = model->index( i, static_cast<int>( QgsFieldMappingModel::ColumnDataIndex::DestinationName ) );
      const QString name = model->data( index, Qt::EditRole ).toString();
      model->setData( index, name.toLower(), Qt::EditRole );
    }
  } );

  menu->addAction( tr( "Convert All Field Names to Uppercase" ), this, [this]() {
    QgsFieldMappingModel *model = mFieldsView->model();
    for ( int i = 0; i < model->rowCount(); i++ )
    {
      const QModelIndex index = model->index( i, static_cast<int>( QgsFieldMappingModel::ColumnDataIndex::DestinationName ) );
      const QString name = model->data( index, Qt::EditRole ).toString();
      model->setData( index, name.toUpper(), Qt::EditRole );
    }
  } );

  mEditButton->setMenu( menu );

  const bool supportsSchemas = mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Schemas );
  if ( supportsSchemas )
  {
    std::unique_ptr<QgsAbstractDatabaseProviderConnection> schemeComboConn;
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( mConnection->providerKey() );
    mSchemaCombo = new QgsDatabaseSchemaComboBox( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mConnection->uri(), QVariantMap() ) ) );
    mLayoutSchemeCombo->addWidget( mSchemaCombo );
  }
  else
  {
    delete mLabelSchemas;
    mLabelSchemas = nullptr;
    delete mLayoutSchemeCombo;
    mLayoutSchemeCombo = nullptr;
  }

  const bool supportsPrimaryKeyName = mConnection->tableImportCapabilities().testFlag( Qgis::DatabaseProviderTableImportCapability::SetPrimaryKeyName );
  if ( !supportsPrimaryKeyName )
  {
    delete mLabelPrimaryKey;
    mLabelPrimaryKey = nullptr;
    delete mEditPrimaryKey;
    mEditPrimaryKey = nullptr;
  }

  const bool supportsGeomColumnName = mConnection->tableImportCapabilities().testFlag( Qgis::DatabaseProviderTableImportCapability::SetGeometryColumnName );
  if ( !supportsGeomColumnName )
  {
    delete mLabelGeometryColumn;
    mLabelGeometryColumn = nullptr;
    delete mEditGeometryColumnName;
    mEditGeometryColumnName = nullptr;
  }

  const bool supportsTableComments = mConnection->capabilities2().testFlag( Qgis::DatabaseProviderConnectionCapability2::SetTableComment );
  if ( !supportsTableComments )
  {
    delete mLabelComment;
    mLabelComment = nullptr;
    delete mEditComment;
    mEditComment = nullptr;
  }

  mExtentGroupBox->setTitleBase( tr( "Filter by Extent" ) );
  mExtentGroupBox->setCheckable( true );
  mExtentGroupBox->setChecked( false );
  mExtentGroupBox->setCollapsed( true );

  mFilterExpressionWidget->registerExpressionContextGenerator( this );

  // populate initial layer
  sourceLayerComboChanged();
}

QgsDbImportVectorLayerDialog::~QgsDbImportVectorLayerDialog()
{
  // these widgets all potentially access mOwnedSource, so we need to force
  // them to be deleted BEFORE the layer
  delete mSourceLayerComboBox;
  mSourceLayerComboBox = nullptr;
  delete mFilterExpressionWidget;
  mFilterExpressionWidget = nullptr;
  delete mFieldsView;
  mFieldsView = nullptr;
}

void QgsDbImportVectorLayerDialog::setDestinationSchema( const QString &schema )
{
  if ( mSchemaCombo )
    mSchemaCombo->setSchema( schema );
}

void QgsDbImportVectorLayerDialog::setSourceUri( const QgsMimeDataUtils::Uri &uri )
{
  mOwnedSource.reset();
  mSourceLayer = nullptr;

  bool owner = false;
  QString error;
  QgsVectorLayer *vl = uri.vectorLayer( owner, error );
  if ( owner )
  {
    mOwnedSource.reset( vl );
    mBlockSourceLayerChanges++;
    mSourceLayerComboBox->setAdditionalLayers( { vl } );
    mSourceLayerComboBox->setLayer( vl );
    mBlockSourceLayerChanges--;
    setSourceLayer( mOwnedSource.get() );
  }
  else if ( vl )
  {
    mBlockSourceLayerChanges++;
    mSourceLayerComboBox->setLayer( vl );
    mBlockSourceLayerChanges--;
    setSourceLayer( vl );
  }
}

void QgsDbImportVectorLayerDialog::setSourceLayer( QgsVectorLayer *layer )
{
  mSourceLayer = layer;
  if ( !mSourceLayer || !mSourceLayer->dataProvider() )
    return;

  mEditTable->setText( layer->name() );

  const bool isSpatial = mSourceLayer->isSpatial();
  if ( mEditGeometryColumnName )
    mEditGeometryColumnName->setEnabled( isSpatial );
  if ( mCrsSelector )
    mCrsSelector->setEnabled( isSpatial );

  mExtentGroupBox->setEnabled( isSpatial );
  if ( !isSpatial )
    mExtentGroupBox->setChecked( false );

  const bool extentFilterEnabled = mExtentGroupBox->isChecked();
  mExtentGroupBox->setOriginalExtent( mSourceLayer->extent(), mSourceLayer->crs() );
  mExtentGroupBox->setOutputExtentFromOriginal();
  mExtentGroupBox->setChecked( extentFilterEnabled );
  mExtentGroupBox->setCollapsed( !extentFilterEnabled );

  mFilterExpressionWidget->setLayer( mSourceLayer );

  if ( mEditPrimaryKey )
  {
    // set initial geometry column name. We use the source layer's primary key column name if available,
    // else fallback to a default value given by the connection
    const QgsAttributeList pkAttributes = mSourceLayer->dataProvider()->pkAttributeIndexes();
    QString primaryKey = !pkAttributes.isEmpty() ? mSourceLayer->dataProvider()->fields().at( pkAttributes.at( 0 ) ).name() : QString();
    if ( primaryKey.isEmpty() )
    {
      QgsDataSourceUri dsUri( mSourceLayer->source() );
      primaryKey = dsUri.keyColumn();
    }
    if ( primaryKey.isEmpty() )
    {
      primaryKey = mConnection->defaultPrimaryKeyColumnName();
    }

    mEditPrimaryKey->setText( primaryKey );
  }

  if ( mEditGeometryColumnName )
  {
    // set initial geometry column name. We use the source layer's geometry name if available,
    // else fallback to a default value given by the connection
    QString geomColumn = mSourceLayer->dataProvider()->geometryColumnName();
    if ( geomColumn.isEmpty() )
    {
      QgsDataSourceUri dsUri( mSourceLayer->source() );
      geomColumn = dsUri.geometryColumn();
    }
    if ( geomColumn.isEmpty() )
    {
      geomColumn = mConnection->defaultGeometryColumnName();
    }

    mEditGeometryColumnName->setText( geomColumn );
  }

  if ( mCrsSelector )
  {
    mCrsSelector->setCrs( mSourceLayer->crs() );
  }

  if ( mEditComment )
  {
    mEditComment->setPlainText( mSourceLayer->metadata().abstract() );
  }

  mFieldsView->setSourceLayer( mSourceLayer );
  mFieldsView->setSourceFields( mSourceLayer->fields() );
  mFieldsView->setDestinationFields( mSourceLayer->fields() );

  const bool selectedFeatures = mSourceLayer->selectedFeatureCount() > 0;
  mSourceLayerOnlySelected->setEnabled( selectedFeatures );
}

void QgsDbImportVectorLayerDialog::loadFieldsFromLayer()
{
  if ( mSourceLayer )
  {
    mFieldsView->setSourceFields( mSourceLayer->fields() );
    mFieldsView->setDestinationFields( mSourceLayer->fields() );
  }
}

void QgsDbImportVectorLayerDialog::addField()
{
  const int rowCount = mFieldsView->model()->rowCount();
  mFieldsView->appendField( QgsField( u"new_field"_s ), u"NULL"_s );
  const QModelIndex index = mFieldsView->model()->index( rowCount, 0 );
  mFieldsView->selectionModel()->select(
    index,
    QItemSelectionModel::SelectionFlags(
      QItemSelectionModel::Clear | QItemSelectionModel::Select | QItemSelectionModel::Current | QItemSelectionModel::Rows
    )
  );
  mFieldsView->scrollTo( index );
}

QString QgsDbImportVectorLayerDialog::schema() const
{
  return mSchemaCombo ? mSchemaCombo->currentSchema() : QString();
}

QString QgsDbImportVectorLayerDialog::tableName() const
{
  return mEditTable->text();
}

QString QgsDbImportVectorLayerDialog::tableComment() const
{
  return mEditComment ? mEditComment->toPlainText() : QString();
}

void QgsDbImportVectorLayerDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  if ( canvas )
  {
    mExtentGroupBox->setCurrentExtent( canvas->mapSettings().visibleExtent(), canvas->mapSettings().destinationCrs() );
    mExtentGroupBox->setMapCanvas( canvas, false );
  }
}

void QgsDbImportVectorLayerDialog::doImport()
{
  // TODO -- validate

  accept();
}

std::unique_ptr<QgsVectorLayerExporterTask> QgsDbImportVectorLayerDialog::createExporterTask( const QVariantMap &extraProviderOptions )
{
  if ( !mSourceLayer || !mSourceLayer->dataProvider() )
    return nullptr;

  QString destinationUri;
  QVariantMap providerOptions;

  QgsAbstractDatabaseProviderConnection::VectorLayerExporterOptions exporterOptions;
  exporterOptions.layerName = mEditTable->text();
  if ( mSchemaCombo )
    exporterOptions.schema = mSchemaCombo->currentSchema();
  exporterOptions.wkbType = mSourceLayer->wkbType();
  if ( mEditPrimaryKey && !mEditPrimaryKey->text().trimmed().isEmpty() )
    exporterOptions.primaryKeyColumns << mEditPrimaryKey->text();
  if ( mEditGeometryColumnName )
    exporterOptions.geometryColumn = mEditGeometryColumnName->text();

  try
  {
    destinationUri = mConnection->createVectorLayerExporterDestinationUri( exporterOptions, providerOptions );
  }
  catch ( QgsProviderConnectionException &e )
  {
    return nullptr;
  }

  // options given to us by createVectorLayerExporterDestinationUri above should overwrite generic ones passed to this method
  QVariantMap allProviderOptions = extraProviderOptions;
  for ( auto it = providerOptions.constBegin(); it != providerOptions.constEnd(); ++it )
  {
    allProviderOptions.insert( it.key(), it.value() );
  }

  // overwrite?
  if ( mChkDropTable->isChecked() )
  {
    allProviderOptions.insert( u"overwrite"_s, true );
  }

  // This flag tells to the provider that field types do not need conversion -- we have already
  // explicitly set all fields to provider-specific field types and we do not need to treat
  // them as generic/different provider fields
  allProviderOptions.insert( u"skipConvertFields"_s, true );

  QgsVectorLayerExporter::ExportOptions exportOptions;
  if ( mCrsSelector )
  {
    exportOptions.setDestinationCrs( mCrsSelector->crs() );
  }
  exportOptions.setTransformContext( mSourceLayer->transformContext() );
  if ( !mFilterExpressionWidget->expression().isEmpty() )
  {
    exportOptions.setFilterExpression( mFilterExpressionWidget->expression() );
    exportOptions.setExpressionContext( createExpressionContext() );
  }

  if ( mExtentGroupBox->isEnabled() && mExtentGroupBox->isChecked() )
  {
    exportOptions.setExtent( QgsReferencedRectangle( mExtentGroupBox->outputExtent(), mExtentGroupBox->outputCrs() ) );
  }

  if ( mSourceLayerOnlySelected->isEnabled() && mSourceLayerOnlySelected->isChecked() )
  {
    exportOptions.setSelectedOnly( true );
  }

  const QList<QgsFieldMappingModel::Field> fieldMapping = mFieldsView->mapping();
  QList<QgsVectorLayerExporter::OutputField> outputFields;
  outputFields.reserve( fieldMapping.size() );
  for ( const QgsFieldMappingModel::Field &field : fieldMapping )
  {
    outputFields.append( QgsVectorLayerExporter::OutputField( field.field, field.expression ) );
  }
  exportOptions.setOutputFields( outputFields );

  return std::make_unique<QgsVectorLayerExporterTask>( mSourceLayer->clone(), destinationUri, mConnection->providerKey(), exportOptions, allProviderOptions, true );
}

QgsExpressionContext QgsDbImportVectorLayerDialog::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mSourceLayer ) );
  return expContext;
}

void QgsDbImportVectorLayerDialog::sourceLayerComboChanged()
{
  if ( mBlockSourceLayerChanges )
    return;

  if ( mSourceLayerComboBox->currentLayer() == mSourceLayer )
    return;

  setSourceLayer( qobject_cast< QgsVectorLayer * >( mSourceLayerComboBox->currentLayer() ) );
}
