/***************************************************************************
  qgstransactiongroup.cpp - QgsTransactionGroup

 ---------------------
 begin                : 15.1.2016
 copyright            : (C) 2016 by mku
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstransactiongroup.h"

#include "qgstransaction.h"
#include "qgsvectorlayer.h"
#include "qgsdatasourceuri.h"
#include "qgsvectordataprovider.h"

#include <QTimer>

QgsTransactionGroup::QgsTransactionGroup( QObject *parent )
    : QObject( parent )
    , mEditingStarting( false )
    , mEditingStopping( false )
    , mTransaction( nullptr )
{

}

QgsTransactionGroup::~QgsTransactionGroup()
{
  delete mTransaction;
}

bool QgsTransactionGroup::addLayer( QgsVectorLayer* layer )
{
  if ( !QgsTransaction::supportsTransaction( layer ) )
    return false;

  QString connString = QgsDataSourceURI( layer->source() ).connectionInfo();

  if ( mConnString.isEmpty() )
  {
    mConnString = connString;
    mProviderKey = layer->providerType();
  }
  else if ( mConnString != connString || mProviderKey != layer->providerType() )
  {
    return false;
  }

  mLayers.insert( layer );

  connect( layer, SIGNAL( beforeEditingStarted() ), this, SLOT( onEditingStarted() ) );
  connect( layer, SIGNAL( layerDeleted() ), this, SLOT( onLayerDeleted() ) );

  return true;
}

void QgsTransactionGroup::onEditingStarted()
{
  if ( mTransaction )
    return;

  mTransaction = QgsTransaction::create( mConnString, mProviderKey );

  QString errorMsg;
  mTransaction->begin( errorMsg );

  Q_FOREACH ( QgsVectorLayer* layer, mLayers )
  {
    mTransaction->addLayer( layer );
    layer->startEditing();
    connect( layer, SIGNAL( beforeCommitChanges() ), this, SLOT( onCommitChanges() ) );
    connect( layer, SIGNAL( beforeRollBack() ), this, SLOT( onRollback() ) );
  }
}

void QgsTransactionGroup::onLayerDeleted()
{
  mLayers.remove( qobject_cast<QgsVectorLayer*>( sender() ) );
}

void QgsTransactionGroup::onCommitChanges()
{
  if ( mEditingStopping )
    return;

  mEditingStopping = true;

  QgsVectorLayer* triggeringLayer = qobject_cast<QgsVectorLayer*>( sender() );

  QString errMsg;
  if ( mTransaction->commit( errMsg ) )
  {
    Q_FOREACH ( QgsVectorLayer* layer, mLayers )
    {
      if ( layer != sender() )
        layer->commitChanges();
    }
  }
  else
  {
    emit commitError( errMsg );
    // Restart editing the calling layer in the next event loop cycle
    QTimer::singleShot( 0, triggeringLayer, SLOT( startEditing() ) );
  }
  mEditingStopping = false;
}

void QgsTransactionGroup::onRollback()
{
  if ( mEditingStopping )
    return;

  mEditingStopping = true;

  QgsVectorLayer* triggeringLayer = qobject_cast<QgsVectorLayer*>( sender() );

  QString errMsg;
  if ( mTransaction->rollback( errMsg ) )
  {
    Q_FOREACH ( QgsVectorLayer* layer, mLayers )
    {
      if ( layer != triggeringLayer )
        layer->rollBack();
    }
  }
  else
  {
    // Restart editing the calling layer in the next event loop cycle
    QTimer::singleShot( 0, triggeringLayer, SLOT( startEditing() ) );
  }
  mEditingStopping = false;
}

QString QgsTransactionGroup::providerKey() const
{
  return mProviderKey;
}

bool QgsTransactionGroup::isEmpty() const
{
  return mLayers.isEmpty();
}

QString QgsTransactionGroup::connString() const
{
  return mConnString;
}
