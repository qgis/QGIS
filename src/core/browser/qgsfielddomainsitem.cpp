/***************************************************************************
                             qgsfielddomainsitem.h
                             -------------------
    begin                : 2022-01-27
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

#include "qgsfielddomainsitem.h"

#include "qgsapplication.h"
#include "qgsfielddomain.h"
#include "qgsmessagelog.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

#include "moc_qgsfielddomainsitem.cpp"

QgsFieldDomainsItem::QgsFieldDomainsItem( QgsDataItem *parent,
    const QString &path,
    const QString &connectionUri,
    const QString &providerKey )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, tr( "Field Domains" ), path, providerKey )
  , mConnectionUri( connectionUri )
{
  mCapabilities |= ( Qgis::BrowserItemCapability::Fertile | Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed );
}

QgsFieldDomainsItem::~QgsFieldDomainsItem() = default;

QVector<QgsDataItem *> QgsFieldDomainsItem::createChildren()
{
  QVector<QgsDataItem *> children;
  try
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey() ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mConnectionUri, {} ) ) };
      if ( conn && ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::RetrieveFieldDomain ) )
      {
        QString domainError;
        QStringList fieldDomains;
        try
        {
          fieldDomains = conn->fieldDomainNames();
        }
        catch ( QgsProviderConnectionException &ex )
        {
          domainError = ex.what();
        }

        for ( const QString &name : std::as_const( fieldDomains ) )
        {
          try
          {
            std::unique_ptr< QgsFieldDomain > domain( conn->fieldDomain( name ) );
            QgsFieldDomainItem *fieldDomainItem { new QgsFieldDomainItem( this, domain.release() ) };
            children.push_back( fieldDomainItem );
          }
          catch ( QgsProviderConnectionException &ex )
          {
            QgsMessageLog::logMessage( ex.what() );
          }
        }

        if ( !domainError.isEmpty() )
        {
          children.push_back( new QgsErrorItem( this, domainError, path() + u"/domainerror"_s ) );
        }
      }
    }
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    children.push_back( new QgsErrorItem( this, ex.what(), path() + u"/error"_s ) );
  }
  return children;
}

QIcon QgsFieldDomainsItem::icon()
{
  return QgsApplication::getThemeIcon( u"mSourceFields.svg"_s );
}

QString QgsFieldDomainsItem::connectionUri() const
{
  return mConnectionUri;
}

//
// QgsFieldDomainItem
//

QgsFieldDomainItem::QgsFieldDomainItem( QgsDataItem *parent, QgsFieldDomain *domain )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, domain->name(), parent->path() + '/' + domain->name(), parent->providerKey() )
  , mDomain( domain )
{
  // Precondition
  Q_ASSERT( dynamic_cast<QgsFieldDomainsItem *>( parent ) );
  setState( Qgis::BrowserItemState::Populated );
  setToolTip( domain->description().isEmpty() ? domain->name() : domain->description() );
  QgsFieldDomainsItem *domainsItem = qobject_cast<QgsFieldDomainsItem *>( parent );
  mConnectionUri = domainsItem->connectionUri();
}

QIcon QgsFieldDomainItem::icon()
{
  switch ( mDomain->type() )
  {
    case Qgis::FieldDomainType::Coded:
      return QgsApplication::getThemeIcon( u"/mIconFieldText.svg"_s );
    case Qgis::FieldDomainType::Range:
      return QgsApplication::getThemeIcon( u"/mIconFieldInteger.svg"_s );
    case Qgis::FieldDomainType::Glob:
      return QgsApplication::getThemeIcon( u"/mIconFieldText.svg"_s );
  }
  BUILTIN_UNREACHABLE
}

const QgsFieldDomain *QgsFieldDomainItem::fieldDomain()
{
  return mDomain.get();
}

QgsFieldDomainItem::~QgsFieldDomainItem() = default;

QString QgsFieldDomainItem::connectionUri() const
{
  return mConnectionUri;
}
