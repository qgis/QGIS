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
#include "moc_qgsdbimportvectorlayerdialog.cpp"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsgui.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"

QgsDbImportVectorLayerDialog::QgsDbImportVectorLayerDialog( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent )
  : QDialog( parent )
  , mConnection( connection )
{
  setupUi( this );
  setObjectName( "QgsDbImportVectorLayerDialog" );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsDbImportVectorLayerDialog::doImport );

  Q_ASSERT( connection );

  mEditSchema->setReadOnly( true );

  const bool supportsSchemas = mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Schemas );
  if ( !supportsSchemas )
  {
    delete mLabelSchemas;
    mLabelSchemas = nullptr;
    delete mEditSchema;
    mEditSchema = nullptr;
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
}

QgsDbImportVectorLayerDialog::~QgsDbImportVectorLayerDialog() = default;

void QgsDbImportVectorLayerDialog::setDestinationSchema( const QString &schema )
{
  if ( mEditSchema )
    mEditSchema->setText( schema );
}

void QgsDbImportVectorLayerDialog::setSourceUri( const QgsMimeDataUtils::Uri &uri )
{
  mEditTable->setText( uri.name );

  mSourceLayer.reset();
  bool owner = false;
  QString error;
  QgsVectorLayer *vl = uri.vectorLayer( owner, error );
  if ( owner )
    mSourceLayer.reset( vl );
  else if ( vl )
    mSourceLayer.reset( vl->clone() );

  if ( !mSourceLayer || !mSourceLayer->dataProvider() )
    return;

  if ( !mSourceLayer->isSpatial() )
  {
    delete mLabelGeometryColumn;
    mLabelGeometryColumn = nullptr;
    delete mEditGeometryColumnName;
    mEditGeometryColumnName = nullptr;
    delete mLabelDestinationCrs;
    mLabelDestinationCrs = nullptr;
    delete mCrsSelector;
    mCrsSelector = nullptr;
  }

  if ( mEditPrimaryKey )
  {
    // set initial geometry column name. We use the source layer's primary key column name if available,
    // else fallback to a default value given by the connection
    const QgsAttributeList pkAttributes = mSourceLayer->dataProvider()->pkAttributeIndexes();
    QString primaryKey = !pkAttributes.isEmpty() ? mSourceLayer->dataProvider()->fields().at( pkAttributes.at( 0 ) ).name() : QString();
    if ( primaryKey.isEmpty() )
    {
      QgsDataSourceUri dsUri( uri.uri );
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
      QgsDataSourceUri dsUri( uri.uri );
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
}

QString QgsDbImportVectorLayerDialog::schema() const
{
  return mEditSchema ? mEditSchema->text() : QString();
}

QString QgsDbImportVectorLayerDialog::tableName() const
{
  return mEditTable->text();
}

QString QgsDbImportVectorLayerDialog::tableComment() const
{
  return mEditComment ? mEditComment->toPlainText() : QString();
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
  if ( mEditSchema )
    exporterOptions.schema = mEditSchema->text();
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

  QgsCoordinateReferenceSystem destinationCrs;
  if ( mCrsSelector )
  {
    destinationCrs = mCrsSelector->crs();
  }

  return std::make_unique<QgsVectorLayerExporterTask>( mSourceLayer->clone(), destinationUri, mConnection->providerKey(), destinationCrs, allProviderOptions, true );
}
