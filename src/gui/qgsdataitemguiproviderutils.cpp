/***************************************************************************
  qgsdataitemguiproviderutils.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemguiproviderutils.h"
#include "qgsdataitem.h"
#include "qgsdataitemguiprovider.h"

#include <QPointer>
#include <QMessageBox>

void QgsDataItemGuiProviderUtils::deleteConnectionsPrivate( const QStringList &connectionNames, const std::function<void( const QString & )> &deleteConnection, QPointer<QgsDataItem> firstParent )
{
  if ( connectionNames.size() > 1 )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connections" ), QObject::tr( "Are you sure you want to remove all %1 selected connections?" ).arg( connectionNames.size() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }
  else
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connection" ), QObject::tr( "Are you sure you want to remove the connection to “%1”?" ).arg( connectionNames.at( 0 ) ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  for ( const QString &connectionName : std::as_const( connectionNames ) )
  {
    deleteConnection( connectionName );
  }

  if ( firstParent )
    firstParent->refreshConnections();
}

const QString QgsDataItemGuiProviderUtils::uniqueName( const QString &name, const QStringList &connectionNames )
{
  int i = 0;
  QString newConnectionName( name );
  while ( connectionNames.contains( newConnectionName ) )
  {
    ++i;
    newConnectionName = QObject::tr( "%1 (copy %2)" ).arg( name ).arg( i );
  }

  return newConnectionName;
}
