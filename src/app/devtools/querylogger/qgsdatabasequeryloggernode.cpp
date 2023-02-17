/***************************************************************************
    qgsdatabasequeryloggernode.cpp
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatabasequeryloggernode.h"
#include "qgis.h"
#include "qgsjsonutils.h"
#include <QUrlQuery>
#include <QColor>
#include <QBrush>
#include <QFont>
#include <QAction>
#include <QDesktopServices>
#include <QApplication>
#include <QClipboard>
#include <nlohmann/json.hpp>


//
// QgsDatabaseQueryLoggerRootNode
//

QgsDatabaseQueryLoggerRootNode::QgsDatabaseQueryLoggerRootNode()
  : QgsDevToolsModelGroup( QString() )
{

}

QVariant QgsDatabaseQueryLoggerRootNode::data( int ) const
{
  return QVariant();
}

void QgsDatabaseQueryLoggerRootNode::removeRow( int row )
{
  mChildren.erase( mChildren.begin() + row );
}

QVariant QgsDatabaseQueryLoggerRootNode::toVariant() const
{
  QVariantList res;
  for ( const std::unique_ptr< QgsDevToolsModelNode > &child : mChildren )
    res << child->toVariant();
  return res;
}


//
// QgsDatabaseQueryLoggerQueryGroup
//

QgsDatabaseQueryLoggerQueryGroup::QgsDatabaseQueryLoggerQueryGroup( const QgsDatabaseQueryLogEntry &query )
  : QgsDevToolsModelGroup( QString() )
  , mSql( query.query )
  , mQueryId( query.queryId )
{
#if 0
  std::unique_ptr< QgsNetworkLoggerRequestDetailsGroup > detailsGroup = std::make_unique< QgsNetworkLoggerRequestDetailsGroup >( request );
  mDetailsGroup = detailsGroup.get();
  addChild( std::move( detailsGroup ) );
#endif

  addKeyValueNode( QObject::tr( "Provider" ), query.provider );
  addKeyValueNode( QObject::tr( "URI" ), query.uri );
  addKeyValueNode( QObject::tr( "Started at" ), QDateTime::fromMSecsSinceEpoch( query.startedTime ).toString( Qt::ISODateWithMs ) );
#if 0
  addKeyValueNode( QObject::tr( "Thread" ), query.originatingThreadId() );
#endif
  addKeyValueNode( QObject::tr( "Initiator" ), query.initiatorClass.isEmpty() ? QObject::tr( "unknown" ) : query.initiatorClass );
  if ( !query.origin.isEmpty() )
    addKeyValueNode( QObject::tr( "Location" ), query.origin );

}

QVariant QgsDatabaseQueryLoggerQueryGroup::data( int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
      return QStringLiteral( "%1 %2" ).arg( QString::number( mQueryId ),
                                            mSql );

    case Qt::ToolTipRole:
    {
      // Show no more than 255 characters
      return mSql.length() > 255 ? mSql.mid( 0, 255 ).append( QStringLiteral( "…" ) ) : mSql;

#if 0
      QString bytes = QObject::tr( "unknown" );
      if ( mBytesTotal != 0 )
      {
        if ( mBytesReceived > 0 && mBytesReceived < mBytesTotal )
          bytes = QStringLiteral( "%1/%2" ).arg( QString::number( mBytesReceived ), QString::number( mBytesTotal ) );
        else if ( mBytesReceived > 0 && mBytesReceived == mBytesTotal )
          bytes = QString::number( mBytesTotal );
      }
      // ?? adding <br/> instead of \n after (very long) url seems to break url up
      // COMPLETE, Status: 200 - text/xml; charset=utf-8 - 2334 bytes - 657 milliseconds
      return QStringLiteral( "%1<br/>%2 - Status: %3 - %4 - %5 bytes - %6 msec - %7 replies" )
             .arg( mUrl.url(),
                   statusToString( mStatus ),
                   QString::number( mHttpStatus ),
                   mContentType,
                   bytes,
                   mStatus == Status::Pending ? QString::number( mTimer.elapsed() / 1000 ) : QString::number( mTotalTime ),
                   QString::number( mReplies ) );
#endif
    }

    case RoleStatus:
      return static_cast< int >( mStatus );

    case RoleId:
      return mQueryId;

    case Qt::ForegroundRole:
    {
      switch ( mStatus )
      {
        case QgsDatabaseQueryLoggerQueryGroup::Status::Pending:
        case QgsDatabaseQueryLoggerQueryGroup::Status::Canceled:
          return QBrush( QColor( 0, 0, 0, 100 ) );
        case QgsDatabaseQueryLoggerQueryGroup::Status::Error:
          return QBrush( QColor( 235, 10, 10 ) );
        case QgsDatabaseQueryLoggerQueryGroup::Status::TimeOut:
          return QBrush( QColor( 235, 10, 10 ) );
        case QgsDatabaseQueryLoggerQueryGroup::Status::Complete:
          break;
      }
      break;
    }

    case Qt::FontRole:
    {
      if ( mStatus == Status::Canceled )
      {
        QFont f;
        f.setStrikeOut( true );
        return f;
      }
      break;
    }

    default:
      break;
  }
  return QVariant( );
}

QList<QAction *> QgsDatabaseQueryLoggerQueryGroup::actions( QObject *parent )
{
  QList< QAction * > res;

  QAction *copyUrlAction = new QAction( QObject::tr( "Copy SQL" ), parent );
  QObject::connect( copyUrlAction, &QAction::triggered, copyUrlAction, [ = ]
  {
    QApplication::clipboard()->setText( mSql );
  } );
  res << copyUrlAction;

  QAction *copyJsonAction = new QAction( QObject::tr( "Copy as JSON" ), parent );
  QObject::connect( copyJsonAction, &QAction::triggered, copyJsonAction, [ = ]
  {
    const QVariant value = toVariant();
    const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( value ).dump( 2 ) );
    QApplication::clipboard()->setText( json );

  } );
  res << copyJsonAction;

  return res;
}

QVariant QgsDatabaseQueryLoggerQueryGroup::toVariant() const
{
  QVariantMap res;
  res.insert( QStringLiteral( "SQL" ), mSql );

  for ( const auto &child : std::as_const( mChildren ) )
  {
    if ( const QgsDevToolsModelValueNode *valueNode = dynamic_cast< const QgsDevToolsModelValueNode *>( child.get() ) )
    {
      res.insert( valueNode->key(), valueNode->value() );
    }
  }
  return res;
}

void QgsDatabaseQueryLoggerQueryGroup::setFinished( const QgsDatabaseQueryLogEntry &query )
{
  if ( query.error.isEmpty() )
  {
    mStatus = query.canceled ? Status::Canceled : Status::Complete;
    addKeyValueNode( QObject::tr( "Total time (ms)" ), QLocale().toString( query.finishedTime - query.startedTime ) );
    if ( query.fetchedRows != -1 )
    {
      addKeyValueNode( QObject::tr( "Row count" ), QLocale().toString( query.fetchedRows ) );
    }
  }
  else
  {
    mStatus = Status::Error;
    addKeyValueNode( QObject::tr( "Error" ), query.error );
  }
}

void QgsDatabaseQueryLoggerQueryGroup::setStatus( QgsDatabaseQueryLoggerQueryGroup::Status status )
{
  mStatus = status;
}

QString QgsDatabaseQueryLoggerQueryGroup::statusToString( QgsDatabaseQueryLoggerQueryGroup::Status status )
{
  switch ( status )
  {
    case QgsDatabaseQueryLoggerQueryGroup::Status::Pending:
      return QObject::tr( "Pending" );
    case QgsDatabaseQueryLoggerQueryGroup::Status::Complete:
      return QObject::tr( "Complete" );
    case QgsDatabaseQueryLoggerQueryGroup::Status::Error:
      return QObject::tr( "Error" );
    case QgsDatabaseQueryLoggerQueryGroup::Status::TimeOut:
      return QObject::tr( "Timeout" );
    case QgsDatabaseQueryLoggerQueryGroup::Status::Canceled:
      return QObject::tr( "Canceled" );
  }
  return QString();
}

void QgsDatabaseQueryLoggerQueryGroup::setSql( const QString &sql )
{
  mSql = sql;
}

const QString &QgsDatabaseQueryLoggerQueryGroup::sql() const
{
  return mSql;
}


