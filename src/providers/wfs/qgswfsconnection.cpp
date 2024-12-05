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
#include "moc_qgswfsconnection.cpp"
#include "qgswfsconstants.h"
#include "qgslogger.h"
#include "qgssettingsentryimpl.h"


static const QString SERVICE_WFS = QStringLiteral( "WFS" );


QgsWfsConnection::QgsWfsConnection( const QString &connName )
  : QgsOwsConnection( SERVICE_WFS, connName )
{
  const QStringList detailsParameters = { service().toLower(), connName };
  const QString version = settingsVersion->value( detailsParameters );
  if ( !version.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_VERSION ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_VERSION, version );
  }

  const QString maxnumfeatures = settingsMaxNumFeatures->value( detailsParameters );
  if ( !maxnumfeatures.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES, maxnumfeatures );
  }

  const QString pagesize = settingsPagesize->value( detailsParameters );
  if ( !pagesize.isEmpty() )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_PAGE_SIZE ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_PAGE_SIZE, pagesize );
  }

  if ( settingsPagingEnabled->exists( detailsParameters ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_PAGING_ENABLED ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_PAGING_ENABLED, settingsPagingEnabled->value( detailsParameters ) );
  }

  if ( settingsPreferCoordinatesForWfsT11->exists( detailsParameters ) )
  {
    mUri.removeParam( QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES ); // setParam allow for duplicates!
    mUri.setParam( QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES, settingsPreferCoordinatesForWfsT11->value( detailsParameters ) ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "WFS full uri: '%1'." ).arg( QString( mUri.uri() ) ), 4 );
}

QStringList QgsWfsConnection::connectionList()
{
  return QgsOwsConnection::connectionList( SERVICE_WFS );
}

void QgsWfsConnection::deleteConnection( const QString &name )
{
  QgsOwsConnection::deleteConnection( SERVICE_WFS, name );
}

QString QgsWfsConnection::selectedConnection()
{
  return QgsOwsConnection::selectedConnection( SERVICE_WFS );
}

void QgsWfsConnection::setSelectedConnection( const QString &name )
{
  QgsOwsConnection::setSelectedConnection( SERVICE_WFS, name );
}
