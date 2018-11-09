/***************************************************************************
    qgswfsconnection.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsconnection.h"
#include "qgswfsconstants.h"
#include "qgslogger.h"
#include "qgssettings.h"

QgsWfsConnection::QgsWfsConnection( const QString &connName )
  : QgsOwsConnection( QStringLiteral( "WFS" ), connName )
{
  const QString &key = QgsWFSConstants::CONNECTIONS_WFS + connectionName();

  QgsSettings settings;

  const QString &version = settings.value( key + "/" + QgsWFSConstants::SETTINGS_VERSION ).toString();
  if ( !version.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_VERSION ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_VERSION, version );
  }

  const QString &maxnumfeatures = settings.value( key + "/" + QgsWFSConstants::SETTINGS_MAXNUMFEATURES ).toString();
  if ( !maxnumfeatures.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES, maxnumfeatures );
  }

  const QString &pagesize = settings.value( key + "/" + QgsWFSConstants::SETTINGS_PAGE_SIZE ).toString();
  if ( !pagesize.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_PAGE_SIZE ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_PAGE_SIZE, pagesize );
  }

  if ( settings.contains( key + "/" + QgsWFSConstants::SETTINGS_PAGING_ENABLED ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_PAGING_ENABLED ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_PAGING_ENABLED,
                   settings.value( key + "/" + QgsWFSConstants::SETTINGS_PAGING_ENABLED, true ).toBool() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  }

  QgsDebugMsg( QStringLiteral( "WFS full uri: '%1'." ).arg( QString( mUri.uri() ) ) );
}

QStringList QgsWfsConnection::connectionList()
{
  return QgsOwsConnection::connectionList( QStringLiteral( "WFS" ) );
}

void QgsWfsConnection::deleteConnection( const QString &name )
{
  QgsOwsConnection::deleteConnection( QStringLiteral( "WFS" ), name );
}

QString QgsWfsConnection::selectedConnection()
{
  return QgsOwsConnection::selectedConnection( QStringLiteral( "WFS" ) );
}

void QgsWfsConnection::setSelectedConnection( const QString &name )
{
  QgsOwsConnection::setSelectedConnection( QStringLiteral( "WFS" ), name );
}
