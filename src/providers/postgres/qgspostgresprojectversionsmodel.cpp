/***************************************************************************
    qgspostgresprojectversionsmodel.cpp
    ---------------------
    begin                : October 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresprojectversionsmodel.h"
#include "moc_qgspostgresprojectversionsmodel.cpp"

#include "qgsguiutils.h"
#include "qgspostgresutils.h"

QgsPostgresProjectVersionsModel::QgsPostgresProjectVersionsModel( const QString &connectionName, QObject *parent )
  : QAbstractTableModel( parent )
  , mConnectionName( connectionName )
{
  if ( !mConnectionName.isEmpty() )
  {
    mConn = QgsPostgresConn::connectDb( QgsPostgresConn::connUri( connectionName ), true );
  }
}

QgsPostgresProjectVersionsModel::~QgsPostgresProjectVersionsModel()
{
  if ( mConn )
  {
    mConn->unref();
  }
}

void QgsPostgresProjectVersionsModel::setConnection( const QString &connectionName )
{
  if ( mConn )
  {
    mConn->unref();
    mConn = nullptr;
  }

  mConnectionName = connectionName;

  if ( !connectionName.isEmpty() )
  {
    mConn = QgsPostgresConn::connectDb( QgsPostgresConn::connUri( connectionName ), true );
  }
}

int QgsPostgresProjectVersionsModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mVersions.size();
}

int QgsPostgresProjectVersionsModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return 3; // ModifiedTime, ModifiedUser, Comment
}

QVariant QgsPostgresProjectVersionsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= mVersions.size() )
    return QVariant();

  const PgProjectVersionData &version = mVersions.at( index.row() );

  if ( role == Qt::DisplayRole )
  {
    switch ( index.column() )
    {
      case ModifiedTime:
        return version.modifiedTime;
      case ModifiedUser:
        return version.modifiedUser;
      case Comment:
        return version.comment;
      default:
        return QVariant();
    }
  }
  else if ( role == Qt::UserRole && index.column() == ModifiedTime )
  {
    // Store date_saved in UserRole for version identification
    return version.dateSaved;
  }

  return QVariant();
}

QVariant QgsPostgresProjectVersionsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case ModifiedTime:
        return tr( "Modified Time" );
      case ModifiedUser:
        return tr( "Modified User" );
      case Comment:
        return tr( "Comment" );
      default:
        return QVariant();
    }
  }

  return QAbstractTableModel::headerData( section, orientation, role );
}

void QgsPostgresProjectVersionsModel::populateVersions( const QString &schema, const QString &project )
{
  beginResetModel();

  mVersions.clear();
  mSchema = schema;
  mProject = project;

  if ( !mConn )
  {
    endResetModel();
    return;
  }

  QgsTemporaryCursorOverride override( Qt::WaitCursor );

  bool versioningActive = QgsPostgresUtils::qgisProjectVersioningActive( mConn, schema );

  if ( versioningActive )
  {
    const QString sqlVersions = QStringLiteral( "SELECT '', CONCAT((metadata->>'last_modified_time')::TIMESTAMP(0)::TEXT, ' (latest version)'), (metadata->>'last_modified_user'), comment FROM  %1.qgis_projects WHERE name = %2 "
                                                "UNION ALL "
                                                "SELECT * FROM ( SELECT date_saved, (metadata->>'last_modified_time')::TIMESTAMP(0)::TEXT, (metadata->>'last_modified_user'), comment  FROM  %1.qgis_projects_versions WHERE name = %2 ORDER BY (metadata->>'last_modified_time')::TIMESTAMP DESC)" )
                                  .arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedValue( project ) );

    QgsPostgresResult resultVersions( mConn->PQexec( sqlVersions ) );

    mVersions.reserve( resultVersions.PQntuples() );

    for ( int i = 0; i < resultVersions.PQntuples(); ++i )
    {
      PgProjectVersionData version;
      version.dateSaved = resultVersions.PQgetvalue( i, 0 );    // date_saved (empty for latest)
      version.modifiedTime = resultVersions.PQgetvalue( i, 1 ); // metadata->>'last_modified_time'
      version.modifiedUser = resultVersions.PQgetvalue( i, 2 ); // metadata->>'last_modified_user'
      version.comment = resultVersions.PQgetvalue( i, 3 );      // comment

      mVersions.append( version );
    }
  }

  endResetModel();
}

QgsPostgresProjectUri QgsPostgresProjectVersionsModel::projectUriForRow( int row ) const
{
  QgsPostgresProjectUri postUri;
  postUri.connInfo = QgsPostgresConn::connUri( mConnectionName );
  postUri.schemaName = mSchema;
  postUri.projectName = mProject;

  if ( row >= 0 && row < mVersions.size() )
  {
    const PgProjectVersionData &version = mVersions.at( row );
    if ( !version.dateSaved.isEmpty() )
    {
      postUri.isVersion = true;
      postUri.dateSaved = version.dateSaved;
    }
  }

  return postUri;
}

void QgsPostgresProjectVersionsModel::clear()
{
  beginResetModel();
  mVersions.clear();
  mSchema.clear();
  mProject.clear();
  endResetModel();
}
