/***************************************************************************
                             qgsrelationshipsitem.cpp
                             -------------------
    begin                : 2022-07-28
    copyright            : (C) 2022 Nyall Dawson
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

#include "qgsrelationshipsitem.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgsabstractdatabaseproviderconnection.h"

QgsRelationshipsItem::QgsRelationshipsItem( QgsDataItem *parent,
    const QString &path,
    const QString &connectionUri,
    const QString &providerKey, const QString &schema, const QString &tableName )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, tr( "Relationships" ), path, providerKey )
  , mConnectionUri( connectionUri )
  , mSchema( schema )
  , mTableName( tableName )
{
  mCapabilities |= ( Qgis::BrowserItemCapability::Fertile | Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed );
}

QgsRelationshipsItem::~QgsRelationshipsItem() = default;

QVector<QgsDataItem *> QgsRelationshipsItem::createChildren()
{
  QVector<QgsDataItem *> children;
  try
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey() ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mConnectionUri, {} ) ) };
      if ( conn && ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::RetrieveRelationships ) )
      {
        QString relationError;
        QList< QgsWeakRelation > relations;
        try
        {
          relations = conn->relationships( mSchema, mTableName );
        }
        catch ( QgsProviderConnectionException &ex )
        {
          relationError = ex.what();
        }

        for ( const QgsWeakRelation &relation : std::as_const( relations ) )
        {
          try
          {
            QgsRelationshipItem *relationshipItem = new QgsRelationshipItem( this, relation );
            children.push_back( relationshipItem );
          }
          catch ( QgsProviderConnectionException &ex )
          {
            QgsMessageLog::logMessage( ex.what() );
          }
        }

        if ( !relationError.isEmpty() )
        {
          children.push_back( new QgsErrorItem( this, relationError, path() + QStringLiteral( "/relationerror" ) ) );
        }
      }
    }
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    children.push_back( new QgsErrorItem( this, ex.what(), path() + QStringLiteral( "/error" ) ) );
  }
  return children;
}

QIcon QgsRelationshipsItem::icon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconBrowserRelations.svg" ) );
}

QString QgsRelationshipsItem::connectionUri() const
{
  return mConnectionUri;
}

//
// QgsRelationshipItem
//

QgsRelationshipItem::QgsRelationshipItem( QgsDataItem *parent, const QgsWeakRelation &relation )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, relation.name(), parent->path() + '/' + relation.name(), parent->providerKey() )
  , mRelation( relation )
{
  // Precondition
  Q_ASSERT( dynamic_cast<QgsRelationshipsItem *>( parent ) );
  setState( Qgis::BrowserItemState::Populated );
  setToolTip( mRelation.name() );
}

QIcon QgsRelationshipItem::icon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconBrowserRelations.svg" ) );
}

const QgsWeakRelation &QgsRelationshipItem::relation() const
{
  return mRelation;
}

QgsRelationshipItem::~QgsRelationshipItem() = default;

