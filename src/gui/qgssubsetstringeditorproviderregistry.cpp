/***************************************************************************
    qgssubsetstringeditorproviderregistry.cpp
     ----------------------------------------
    Date                 : 15-Nov-2020
    Copyright            : (C) 2020 by Even Rouault
    Email                : even.rouault at spatials.com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssubsetstringeditorprovider.h"
#include "qgssubsetstringeditorproviderregistry.h"
#include "qgsproviderguiregistry.h"
#include "qgsquerybuilder.h"

#include <memory>

QgsSubsetStringEditorProviderRegistry::QgsSubsetStringEditorProviderRegistry() = default;

QgsSubsetStringEditorProviderRegistry::~QgsSubsetStringEditorProviderRegistry()
{
  qDeleteAll( mProviders );
}

QList<QgsSubsetStringEditorProvider *> QgsSubsetStringEditorProviderRegistry::providers()
{
  return mProviders;
}

void QgsSubsetStringEditorProviderRegistry::addProvider( QgsSubsetStringEditorProvider *provider )
{
  mProviders.append( provider );
}

bool QgsSubsetStringEditorProviderRegistry::removeProvider( QgsSubsetStringEditorProvider *provider )
{
  const int index = mProviders.indexOf( provider );
  if ( index >= 0 )
  {
    delete mProviders.takeAt( index );
    return true;
  }
  return false;
}


void QgsSubsetStringEditorProviderRegistry::initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry )
{
  if ( !providerGuiRegistry )
    return;

  const QStringList providersList = providerGuiRegistry->providerList();
  for ( const QString &key : providersList )
  {
    const QList<QgsSubsetStringEditorProvider *> providerList = providerGuiRegistry->subsetStringEditorProviders( key );
    // the function is a factory - we keep ownership of the returned providers
    for ( auto provider : providerList )
    {
      addProvider( provider );
    }
  }
}

QgsSubsetStringEditorProvider *QgsSubsetStringEditorProviderRegistry::providerByName( const QString &name )
{
  const QList<QgsSubsetStringEditorProvider *> providerList = providers();
  for ( const auto provider :  providerList )
  {
    if ( provider->name() == name )
    {
      return provider;
    }
  }
  return nullptr;
}

QList<QgsSubsetStringEditorProvider *> QgsSubsetStringEditorProviderRegistry::providersByKey( const QString &providerKey )
{
  QList<QgsSubsetStringEditorProvider *> result;
  const QList<QgsSubsetStringEditorProvider *> providerList = providers();
  for ( const auto provider : providerList )
  {
    if ( provider->providerKey() == providerKey )
    {
      result << provider;
    }
  }
  return result;
}

QgsSubsetStringEditorInterface *QgsSubsetStringEditorProviderRegistry::createDialog( QgsVectorLayer *layer, QWidget *parent, Qt::WindowFlags fl )
{
  const QList<QgsSubsetStringEditorProvider *> providerList = providers();
  QgsSubsetStringEditorProvider *bestProviderCandidate = nullptr;
  // Loop over providers to find one that can handle the layer.
  // And prefer one that will also indicate to handle the storage type.
  for ( const auto provider : providerList )
  {
    if ( provider->canHandleLayer( layer ) )
    {
      if ( provider->canHandleLayerStorageType( layer ) )
      {
        return provider->createDialog( layer, parent, fl );
      }
      bestProviderCandidate = provider;
    }
  }
  if ( bestProviderCandidate )
  {
    return bestProviderCandidate->createDialog( layer, parent, fl );
  }

  return new QgsQueryBuilder( layer, parent, fl );
}
