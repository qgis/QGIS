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
}

void QgsDbImportVectorLayerDialog::doImport()
{
  // TODO -- validate

  accept();
}

std::unique_ptr<QgsVectorLayerExporterTask> QgsDbImportVectorLayerDialog::createExporterTask()
{
  if ( !mSourceLayer )
    return nullptr;

  QString destinationUri;
  const Qgis::WkbType destinationWkbType = mSourceLayer->wkbType();
  try
  {
    destinationUri = mConnection->createVectorLayerExporterDestinationUri( mEditSchema ? mEditSchema->text() : QString(), mEditTable->text(), destinationWkbType );
  }
  catch ( QgsProviderConnectionException &e )
  {
    return nullptr;
  }

  return std::make_unique<QgsVectorLayerExporterTask>( mSourceLayer->clone(), destinationUri, mConnection->providerKey(), mSourceLayer->crs(), QVariantMap(), true );
}
