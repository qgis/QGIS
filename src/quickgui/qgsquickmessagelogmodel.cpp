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
  , mMessageLog( QgsApplication::messageLog() )
{
  connect( mMessageLog, static_cast<void ( QgsMessageLog::* )( const QString &message, const QString &tag, Qgis::MessageLevel  level )>( &QgsMessageLog::messageReceived ), this, &QgsQuickMessageLogModel::onMessageReceived );
}

QHash<int, QByteArray> QgsQuickMessageLogModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[MessageRole]  = "Message";
  roles[MessageTagRole] = "MessageTag";
  roles[MessageLevelRole] = "MessageLevel";

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
  mMessages.prepend( LogMessage( tag, message, level ) );
  QgsDebugMsg( QStringLiteral( "Next message %1 : %2" ).arg( tag, message ) );
  endInsertRows();
}
