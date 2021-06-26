/***************************************************************************
   qgsproviderguimetadata.cpp
   --------------------------
    begin                : June 4th 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderguimetadata.h"
#include "qgsdataitemguiprovider.h"
#include "qgsprojectstorageguiprovider.h"
#include "qgssourceselectprovider.h"

QgsProviderGuiMetadata::QgsProviderGuiMetadata( const QString &key )
  : mKey( key )
{
}

QgsProviderGuiMetadata::~QgsProviderGuiMetadata() = default;

QList<QgsDataItemGuiProvider *> QgsProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>();
}

QList<QgsProjectStorageGuiProvider *> QgsProviderGuiMetadata::projectStorageGuiProviders()
{
  return QList<QgsProjectStorageGuiProvider *>();
}

QList<QgsSourceSelectProvider *> QgsProviderGuiMetadata::sourceSelectProviders()
{
  return QList<QgsSourceSelectProvider *>();
}

QList<QgsSubsetStringEditorProvider *> QgsProviderGuiMetadata::subsetStringEditorProviders()
{
  return QList<QgsSubsetStringEditorProvider *>();
}

QString QgsProviderGuiMetadata::key() const
{
  return mKey;
}

void QgsProviderGuiMetadata::registerGui( QMainWindow * )
{
}

