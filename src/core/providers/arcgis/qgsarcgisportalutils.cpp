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
#include "qgsfeedback.h"

#include <QUrl>
#include <QUrlQuery>

QVariantMap QgsArcGisPortalUtils::retrieveUserInfo( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, QgsFeedback *feedback )
{
  QString endPoint = communityUrl;
  if ( endPoint.endsWith( '/' ) )
    endPoint.chop( 1 );

  if ( user.isEmpty() )
    endPoint += QLatin1String( "/self" );
  else
    endPoint += QStringLiteral( "/users/" ) + user;

  QUrl queryUrl( endPoint );
  QUrlQuery query( queryUrl );
  query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  queryUrl.setQuery( query );

  return QgsArcGisRestQueryUtils::queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, feedback );
}

QVariantMap QgsArcGisPortalUtils::retrieveUserInfo( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle, QString &errorText, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback )
{
  return QgsArcGisPortalUtils::retrieveUserInfo( communityUrl, user, authcfg, errorTitle, errorText, QgsHttpHeaders( requestHeaders ), feedback );
}

QVariantList QgsArcGisPortalUtils::retrieveUserGroups( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, QgsFeedback *feedback )
{
  const QVariantMap info = retrieveUserInfo( communityUrl, user, authcfg, errorTitle, errorText, requestHeaders, feedback );
  return info.value( QStringLiteral( "groups" ) ).toList();
}

QVariantList QgsArcGisPortalUtils::retrieveUserGroups( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle, QString &errorText, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback )
{
  return QgsArcGisPortalUtils::retrieveUserGroups( communityUrl, user, authcfg, errorTitle, errorText, QgsHttpHeaders( requestHeaders ), feedback );
}

QVariantList QgsArcGisPortalUtils::retrieveGroupContent( const QString &contentUrl, const QString &groupId, const QString &authcfg, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, QgsFeedback *feedback, int pageSize )
{
  QString endPoint = contentUrl;
  if ( endPoint.endsWith( '/' ) )
    endPoint.chop( 1 );

  endPoint += QStringLiteral( "/groups/" ) + groupId;

  int start = 1;

  QVariantList items;
  while ( true )
  {
    QUrl queryUrl( endPoint );
    QUrlQuery query( queryUrl );
    query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
    query.addQueryItem( QStringLiteral( "start" ), QString::number( start ) );
    query.addQueryItem( QStringLiteral( "num" ), QString::number( pageSize ) );
    queryUrl.setQuery( query );

    const QVariantMap response = QgsArcGisRestQueryUtils::queryServiceJSON( queryUrl, authcfg, errorTitle, errorText, requestHeaders, feedback );
    if ( !errorText.isEmpty() )
      return QVariantList();

    items.append( response.value( QStringLiteral( "items" ) ).toList() );

    if ( feedback && feedback->isCanceled() )
      return items;

    const int total = response.value( QStringLiteral( "total" ) ).toInt();
    start += pageSize;
    if ( total < start )
      break;
  }
  return items;
}

QVariantList QgsArcGisPortalUtils::retrieveGroupContent( const QString &contentUrl, const QString &groupId, const QString &authcfg, QString &errorTitle, QString &errorText, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback, int pageSize )
{
  return QgsArcGisPortalUtils::retrieveGroupContent( contentUrl, groupId, authcfg, errorTitle, errorText, QgsHttpHeaders( requestHeaders ), feedback, pageSize );
}

QVariantList QgsArcGisPortalUtils::retrieveGroupItemsOfType( const QString &contentUrl, const QString &groupId, const QString &authcfg, const QList<int> &itemTypes, QString &errorTitle, QString &errorText, const QgsHttpHeaders &requestHeaders, QgsFeedback *feedback, int pageSize )
{
  const QVariantList items = retrieveGroupContent( contentUrl, groupId, authcfg, errorTitle, errorText, requestHeaders, feedback, pageSize );

  // filter results to desired types
  QVariantList result;
  for ( const QVariant &item : items )
  {
    const QVariantMap itemDef = item.toMap();
    const QString itemType = itemDef.value( QStringLiteral( "type" ) ).toString();

    for ( const int filterType : itemTypes )
    {
      if ( typeToString( static_cast< ItemType >( filterType ) ).compare( itemType, Qt::CaseInsensitive ) == 0 )
      {
        result << item;
        break;
      }
    }
  }
  return result;
}

QVariantList QgsArcGisPortalUtils::retrieveGroupItemsOfType( const QString &contentUrl, const QString &groupId, const QString &authcfg, const QList<int> &itemTypes, QString &errorTitle, QString &errorText, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback, int pageSize )
{
  return QgsArcGisPortalUtils::retrieveGroupItemsOfType( contentUrl, groupId, authcfg, itemTypes, errorTitle, errorText, QgsHttpHeaders( requestHeaders ), feedback, pageSize );
}


QString QgsArcGisPortalUtils::typeToString( QgsArcGisPortalUtils::ItemType type )
{
  switch ( type )
  {
    case QgsArcGisPortalUtils::FeatureService:
      return QStringLiteral( "Feature Service" );
    case QgsArcGisPortalUtils::MapService:
      return QStringLiteral( "Map Service" );
    case QgsArcGisPortalUtils::ImageService:
      return QStringLiteral( "Image Service" );
  }
  return QString();
}
