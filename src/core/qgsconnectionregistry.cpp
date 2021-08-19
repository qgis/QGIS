/***************************************************************************
                         qgsconnectionregistry.cpp
                         --------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconnectionregistry.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include <QRegularExpression>

QgsConnectionRegistry::QgsConnectionRegistry( QObject *parent SIP_TRANSFERTHIS )
  : QObject( parent )
{
}

QgsAbstractProviderConnection *QgsConnectionRegistry::createConnection( const QString &id )
{
  const QRegularExpressionMatch m = QRegularExpression( QStringLiteral( "(.*?)\\://(.*)" ) ).match( id );
  if ( !m.hasMatch() )
    throw QgsProviderConnectionException( QObject::tr( "Invalid connection id" ) );

  const QString providerKey = m.captured( 1 );
  const QString name = m.captured( 2 );

  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( providerKey );

  if ( !md )
    throw QgsProviderConnectionException( QObject::tr( "Invalid provider key: %1" ).arg( providerKey ) );

  return md->createConnection( name );
}
