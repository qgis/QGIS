/***************************************************************************
  qgstransactiongroup.cpp - QgsTransactionGroup
  ---------------------------------------------

 begin                : 15.1.2016
 Copyright            : (C) 2016 Matthias Kuhn
 Email                : matthias at opengis dot ch
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
{

}

bool QgsTransactionGroup::addLayer( QgsVectorLayer *layer )
{
  if ( !QgsTransaction::supportsTransaction( layer ) )
    return false;

  QString connString = QgsTransaction::connectionString( layer->source() );
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

  connect( layer, &QgsVectorLayer::beforeEditingStarted, this, &QgsTransactionGroup::onEditingStarted );
  connect( layer, &QgsVectorLayer::destroyed, this, &QgsTransactionGroup::onLayerDeleted );

  return true;
}

QSet<QgsVectorLayer *> QgsTransactionGroup::layers() const
{
  return mLayers;
}

bool QgsTransactionGroup::modified() const
{
  const auto constMLayers = mLayers;
  for ( QgsVectorLayer *layer : constMLayers )
  {
    if ( layer->isModified() )
      return true;
  }
  return false;
}

void QgsTransactionGroup::onEditingStarted()
{
  if ( mTransaction )
    return;

  mTransaction.reset( QgsTransaction::create( mConnString, mProviderKey ) );
  if ( !mTransaction )
    return;

  QString errorMsg;
  mTransaction->begin( errorMsg );

  const auto constMLayers = mLayers;
  for ( QgsVectorLayer *layer : constMLayers )
  {
    mTransaction->addLayer( layer );
    layer->startEditing();
    connect( layer, &QgsVectorLayer::beforeCommitChanges, this, &QgsTransactionGroup::onCommitChanges );
    connect( layer, &QgsVectorLayer::beforeRollBack, this, &QgsTransactionGroup::onRollback );
  }
}

void QgsTransactionGroup::onLayerDeleted()
{
  mLayers.remove( static_cast<QgsVectorLayer *>( sender() ) );
}

void QgsTransactionGroup::onCommitChanges()
{
  if ( mEditingStopping )
    return;

  mEditingStopping = true;

  QgsVectorLayer *triggeringLayer = qobject_cast<QgsVectorLayer *>( sender() );

  QString errMsg;
  if ( mTransaction->commit( errMsg ) )
  {
    const auto constMLayers = mLayers;
    for ( QgsVectorLayer *layer : constMLayers )
    {
      if ( layer != sender() )
        layer->commitChanges();
    }

    disableTransaction();
  }
  else
  {
    emit commitError( errMsg );
    // Restart editing the calling layer in the next event loop cycle
    QTimer::singleShot( 0, triggeringLayer, &QgsVectorLayer::startEditing );
  }
  mEditingStopping = false;
}

void QgsTransactionGroup::onRollback()
{
  if ( mEditingStopping )
    return;

  mEditingStopping = true;

  QgsVectorLayer *triggeringLayer = qobject_cast<QgsVectorLayer *>( sender() );

  QString errMsg;
  if ( mTransaction->rollback( errMsg ) )
  {
    const auto constMLayers = mLayers;
    for ( QgsVectorLayer *layer : constMLayers )
    {
      if ( layer != triggeringLayer )
        layer->rollBack();
    }
    disableTransaction();
  }
  else
  {
    // Restart editing the calling layer in the next event loop cycle
    QTimer::singleShot( 0, triggeringLayer, &QgsVectorLayer::startEditing );
  }
  mEditingStopping = false;
}

void QgsTransactionGroup::disableTransaction()
{
  mTransaction.reset();

  const auto constMLayers = mLayers;
  for ( QgsVectorLayer *layer : constMLayers )
  {
    disconnect( layer, &QgsVectorLayer::beforeCommitChanges, this, &QgsTransactionGroup::onCommitChanges );
    disconnect( layer, &QgsVectorLayer::beforeRollBack, this, &QgsTransactionGroup::onRollback );
  }
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
