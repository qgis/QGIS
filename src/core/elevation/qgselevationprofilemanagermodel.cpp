/***************************************************************************
    qgselevationprofilemanagermodel.cpp
    --------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationprofilemanagermodel.h"

#include "qgselevationprofile.h"
#include "qgselevationprofilemanager.h"
#include "qgsproject.h"

#include "moc_qgselevationprofilemanagermodel.cpp"

//
// QgsElevationProfileManagerModel
//

QgsElevationProfileManagerModel::QgsElevationProfileManagerModel( QgsElevationProfileManager *manager, QObject *parent )
  : QgsProjectStoredObjectManagerModel( manager, parent )
{
  connect( manager, &QgsElevationProfileManager::profileRenamed, this, &QgsElevationProfileManagerModel::objectRenamedInternal );
}

QgsElevationProfile *QgsElevationProfileManagerModel::profileFromIndex( const QModelIndex &index ) const
{
  return objectFromIndex( index );
}

QModelIndex QgsElevationProfileManagerModel::indexFromProfile( QgsElevationProfile *layout ) const
{
  return indexFromObject( layout );
}


//
// QgsElevationProfileManagerProxyModel
//

QgsElevationProfileManagerProxyModel::QgsElevationProfileManagerProxyModel( QObject *parent )
  : QgsProjectStoredObjectManagerProxyModel( parent )
{
}
