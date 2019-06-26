/***************************************************************************
   qgsproviderguimetadata.cpp
   --------------------------
    begin                : June 4th 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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

QString QgsProviderGuiMetadata::key() const
{
  return mKey;
}

void QgsProviderGuiMetadata::registerGui( QMainWindow * )
{
}

