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

#include <QSettings>

QgsWFSConnection::QgsWFSConnection( const QString & theConnName )
    : QgsOWSConnection( "WFS", theConnName )
{
  const QString& key = QgsWFSConstants::CONNECTIONS_WFS + mConnName;

  QSettings settings;

  const QString& version = settings.value( key + "/" + QgsWFSConstants::SETTINGS_VERSION ).toString();
  if ( !version.isEmpty() )
  {
    mUri.setParam( QgsWFSConstants::URI_PARAM_VERSION, version );
  }

  const QString& maxnumfeatures = settings.value( key + "/" + QgsWFSConstants::SETTINGS_MAXNUMFEATURES ).toString();
  if ( !maxnumfeatures.isEmpty() )
  {
    mUri.setParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES, maxnumfeatures );
  }

  QgsDebugMsg( QString( "WFS full uri: '%1'." ).arg( QString( mUri.uri() ) ) );
}

QStringList QgsWFSConnection::connectionList()
{
  return QgsOWSConnection::connectionList( "WFS" );
}

void QgsWFSConnection::deleteConnection( const QString & name )
{
  QgsOWSConnection::deleteConnection( "WFS", name );
}

QString QgsWFSConnection::selectedConnection()
{
  return QgsOWSConnection::selectedConnection( "WFS" );
}

void QgsWFSConnection::setSelectedConnection( const QString & name )
{
  QgsOWSConnection::setSelectedConnection( "WFS", name );
}
