/***************************************************************************
  qgsquickmessagelogmodel.cpp
  --------------------------------------
  date                 : 13.7.2016
  copyright            : (C) 2016 by Matthias Kuhn
  email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"

#include "qgsquickmessagelogmodel.h"

QgsQuickMessageLogModel::QgsQuickMessageLogModel( QObject *parent )
  : QAbstractListModel( parent )
{
  connect( QgsApplication::messageLog(), static_cast<void ( QgsMessageLog::* )( const QString &message, const QString &tag, Qgis::MessageLevel  level )>( &QgsMessageLog::messageReceived ), this, &QgsQuickMessageLogModel::onMessageReceived );
}

QgsQuickMessageLogModel::LogMessage QgsQuickMessageLogModel::logMessage( const QString &tag, const QString &message, Qgis::MessageLevel level )
{
  LogMessage msg;
  msg.tag = tag;
  msg.message = message;
  msg.level = level;

  return msg;
}

QHash<int, QByteArray> QgsQuickMessageLogModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[MessageRole]  = QByteArrayLiteral( "Message" );
  roles[MessageTagRole] = QByteArrayLiteral( "MessageTag" );
  roles[MessageLevelRole] = QByteArrayLiteral( "MessageLevel" );

  return roles;
}

int QgsQuickMessageLogModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mMessages.size();
}

QVariant QgsQuickMessageLogModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() >= mMessages.size() )
    return QVariant();

  if ( role == MessageRole )
    return mMessages.at( index.row() ).message;
  else if ( role == MessageTagRole )
    return mMessages.at( index.row() ).tag;
  else if ( role == MessageLevelRole )
    return mMessages.at( index.row() ).level;

  return QVariant();
}

void QgsQuickMessageLogModel::onMessageReceived( const QString &message, const QString &tag, Qgis::MessageLevel level )
{
  beginInsertRows( QModelIndex(), 0, 0 );
  mMessages.prepend( logMessage( tag, message, level ) );
  QgsDebugMsg( QStringLiteral( "Next message %1 : %2" ).arg( tag, message ) );
  endInsertRows();
}
