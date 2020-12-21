/***************************************************************************
    qgsarcgisportalutils.h
    --------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsarcgisportalutils.h"
#include "qgsarcgisrestquery.h"
#include <QUrl>
#include <QUrlQuery>

QVariantMap QgsArcGisPortalUtils::retrieveUserInfo( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle, QString &errorText, const QMap<QString, QString> &requestHeaders, QgsFeedback *feedback )
{
  QString endPoint = communityUrl;
  if ( endPoint.endsWith( '/' ) )
    endPoint.chop( 1 );

  if ( user.isEmpty() )
    endPoint += QStringLiteral( "/self" );
  else
    endPoint += QStringLiteral( "/users/" ) + user;

  QUrl queryUrl( endPoint );
  QUrlQuery query( queryUrl );
  query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  queryUrl.setQuery( query );

  return QgsArcGisRestQueryUtils::queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, feedback );
}

QVariantList QgsArcGisPortalUtils::retrieveUserGroups( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle, QString &errorText, const QMap<QString, QString> &requestHeaders, QgsFeedback *feedback )
{
  const QVariantMap info = retrieveUserInfo( communityUrl, user, authcfg, errorTitle, errorText, requestHeaders, feedback );
  return info.value( QStringLiteral( "groups" ) ).toList();
}
