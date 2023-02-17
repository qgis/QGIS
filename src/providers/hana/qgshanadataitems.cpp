/***************************************************************************
   qgshanadataitems.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgshanaconnection.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaexception.h"
#include "qgshanadataitems.h"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"
#include "qgsfieldsitem.h"

#include <QMessageBox>
#include <climits>

QgsHanaConnectionItem::QgsHanaConnectionItem(
  QgsDataItem *parent,
  const QString &name,
  const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "SAP HANA" ) )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;

  updateToolTip( QString( ), QString( ) );
}

QVector<QgsDataItem *> QgsHanaConnectionItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsHanaConnectionRef conn( mName );
  if ( conn.isNull() )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    return items;
  }

  QgsHanaSettings settings( mName, true );

  try
  {
    QString userName = conn->getUserName();
    if ( userName.isEmpty() )
      userName = settings.userName();

    updateToolTip( userName, conn->getDatabaseVersion() );

    const QVector<QgsHanaSchemaProperty> schemas =
      conn->getSchemas( settings.userTablesOnly() ? userName : QString() );

    if ( schemas.isEmpty() )
    {
      items.append( new QgsErrorItem( this, tr( "No schemas found" ), mPath + "/error" ) );
    }
    else
    {
      for ( const QgsHanaSchemaProperty &schema : schemas )
      {
        QgsHanaSchemaItem *schemaItem = new QgsHanaSchemaItem( this, mName, schema.name,
            mPath + '/' + schema.name );
        items.append( schemaItem );
      }
    }
  }
  catch ( const QgsHanaException &ex )
  {
    QgsErrorItem *itemError = new QgsErrorItem( this, tr( "Server error occurred" ), mPath + "/error" );
    itemError->setToolTip( ex.what() );
    items.append( itemError );
  }

  return items;
}

bool QgsHanaConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
    return false;

  const QgsHanaConnectionItem *o = qobject_cast<const QgsHanaConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

void QgsHanaConnectionItem::refreshSchema( const QString &schema )
{
  for ( QgsDataItem *child : std::as_const( mChildren ) )
  {
    if ( child->name() == schema || schema.isEmpty() )
      child->refresh();
  }
}

void QgsHanaConnectionItem::updateToolTip( const QString &userName, const QString &dbmsVersion )
{
  QgsHanaSettings settings( mName, true );
  QString tip;
  if ( settings.connectionType() == QgsHanaConnectionType::HostPort )
  {
    if ( !settings.database().isEmpty() )
      tip = tr( "Database: " ) + settings.database();
    if ( !tip.isEmpty() )
      tip += '\n';
    tip += tr( "Host: " ) + settings.host() + QStringLiteral( " " );
    if ( QgsHanaIdentifierType::fromInt( settings.identifierType() ) == QgsHanaIdentifierType::InstanceNumber )
      tip += settings.identifier();
    else
      tip += settings.port();
    tip += '\n';
  }
  if ( !dbmsVersion.isEmpty() )
    tip += tr( "DB Version: " ) + dbmsVersion + '\n';
  tip += tr( "User: " ) + userName + '\n';
  tip += tr( "Encrypted: " ) + QString( settings.enableSsl() ? tr( "yes" ) : tr( "no" ) );
  setToolTip( tip );
}

bool QgsHanaConnectionItem::handleDrop( const QMimeData *data, const QString &toSchema )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  QStringList importResults;
  bool hasError = false;

  QgsDataSourceUri uri = QgsHanaSettings( mName, true ).toDataSourceUri();
  QgsHanaConnectionRef conn( uri );

  if ( !conn.isNull() )
  {
    const QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
    for ( const QgsMimeDataUtils::Uri &u : lst )
    {
      if ( u.layerType != QLatin1String( "vector" ) )
      {
        importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
        hasError = true; // only vectors can be imported
        continue;
      }

      // open the source layer
      bool owner;
      QString error;
      QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
      if ( !srcLayer )
      {
        importResults.append( QStringLiteral( "%1: %2" ).arg( u.name, error ) );
        hasError = true;
        continue;
      }

      if ( srcLayer->isValid() )
      {
        QgsDataSourceUri dsUri( u. uri );
        QString geomColumn = dsUri.geometryColumn();
        if ( geomColumn.isEmpty() )
        {
          bool fieldsInUpperCase = QgsHanaUtils::countFieldsWithFirstLetterInUppercase( srcLayer->fields() ) > srcLayer->fields().size() / 2;
          geomColumn = ( srcLayer->geometryType() != QgsWkbTypes::NullGeometry ) ? ( fieldsInUpperCase ? QStringLiteral( "GEOM" ) : QStringLiteral( "geom" ) ) : nullptr;
        }

        uri.setDataSource( toSchema, u.name, geomColumn, QString(), dsUri.keyColumn() );
        uri.setWkbType( srcLayer->wkbType() );

        std::unique_ptr< QgsVectorLayerExporterTask > exportTask(
          new QgsVectorLayerExporterTask( srcLayer, uri.uri( false ),
                                          QStringLiteral( "hana" ), srcLayer->crs(), QVariantMap(), owner ) );

        // when export is successful:
        connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this,
                 [ = ]()
        {
          QMessageBox::information( nullptr, tr( "Import to SAP HANA database" ), tr( "Import was successful." ) );
          refreshSchema( toSchema );
        } );

        // when an error occurs:
        connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this,
                 [ = ]( Qgis::VectorExportResult error, const QString & errorMessage )
        {
          if ( error != Qgis::VectorExportResult::UserCanceled )
          {
            QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
            output->setTitle( tr( "Import to SAP HANA database" ) );
            output->setMessage( tr( "Failed to import some layers!\n\n" ) +
                                errorMessage, QgsMessageOutput::MessageText );
            output->showMessage();
          }
          refreshSchema( toSchema );
        } );

        QgsApplication::taskManager()->addTask( exportTask.release() );
      }
      else
      {
        importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
        hasError = true;
      }
    }
  }
  else
  {
    importResults.append( tr( "Connection failed" ) );
    hasError = true;
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to SAP HANA database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) +
                        importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsHanaLayerItem::QgsHanaLayerItem(
  QgsDataItem *parent,
  const QString &name,
  const QString &path,
  Qgis::BrowserLayerType layerType,
  const QgsHanaLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, QStringLiteral( "hana" ) )
  , mLayerProperty( layerProperty )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete | Qgis::BrowserItemCapability::Fertile;
  mUri = createUri();
  setState( Qgis::BrowserItemState::NotPopulated );
}

QVector<QgsDataItem *> QgsHanaLayerItem::createChildren()
{
  QVector<QgsDataItem *> items;
  items.push_back( new QgsFieldsItem( this, uri() + QStringLiteral( "/columns/ " ), createUri(), providerKey(), mLayerProperty.schemaName, mLayerProperty.tableName ) );
  return items;
}

QString QgsHanaLayerItem::createUri() const
{
  QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( parent() ?
                                    parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugMsg( "Connection item not found." );
    return QString();
  }

  QgsHanaSettings settings( connItem->name(), true );

  QStringList pkColumns;
  if ( !mLayerProperty.pkCols.isEmpty() )
  {
    const QStringList pkColumnsStored = settings.keyColumns( mLayerProperty.schemaName, mLayerProperty.tableName );
    if ( !pkColumnsStored.empty() )
    {
      // We check whether the primary key columns still exist.
      auto intersection = qgis::listToSet( pkColumnsStored ).intersect( qgis::listToSet( mLayerProperty.pkCols ) );
      if ( intersection.size() == pkColumnsStored.size() )
      {
        for ( const auto &column : pkColumnsStored )
          pkColumns << QgsHanaUtils::quotedIdentifier( column );
      }
    }
  }

  QgsDataSourceUri uri = settings.toDataSourceUri();
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName,
                     mLayerProperty.geometryColName, mLayerProperty.sql, pkColumns.join( ',' ) );
  uri.setWkbType( mLayerProperty.type );
  if ( uri.wkbType() != QgsWkbTypes::NoGeometry )
    uri.setSrid( QString::number( mLayerProperty.srid ) );
  QgsDebugMsgLevel( QStringLiteral( "layer uri: %1" ).arg( uri.uri( false ) ), 4 );
  return uri.uri( false );
}

QString QgsHanaLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

// ---------------------------------------------------------------------------
QgsHanaSchemaItem::QgsHanaSchemaItem(
  QgsDataItem *parent,
  const QString &connectionName,
  const QString &name,
  const QString &path )
  : QgsDatabaseSchemaItem( parent, name, path, QStringLiteral( "SAP HANA" ) )
  , mConnectionName( connectionName )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mSchemaName = name;
}

QVector<QgsDataItem *> QgsHanaSchemaItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsHanaConnectionRef conn( mConnectionName );
  if ( conn.isNull() )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    return items;
  }

  try
  {
    QgsHanaSettings settings( mConnectionName, true );
    const QVector<QgsHanaLayerProperty> layers = conn->getLayersFull( mSchemaName,
        settings.allowGeometrylessTables(), settings.userTablesOnly() );

    items.reserve( layers.size() );
    for ( const QgsHanaLayerProperty &layerInfo : layers )
    {
      if ( layerInfo.isValid )
        items.append( createLayer( layerInfo ) );
      else
      {
        QgsErrorItem *itemInvalidLayer = new QgsErrorItem( this, layerInfo.defaultName(), mPath + "/error" );
        itemInvalidLayer->setToolTip( layerInfo.errorMessage );
        items.append( itemInvalidLayer );
      }
    }
  }
  catch ( const QgsHanaException &ex )
  {
    QgsErrorItem *itemError = new QgsErrorItem( this, tr( "Server error occurred" ), mPath + "/error" );
    itemError->setToolTip( ex.what() );
    items.append( itemError );
  }

  setName( mSchemaName );

  return items;
}

QgsHanaLayerItem *QgsHanaSchemaItem::createLayer( const QgsHanaLayerProperty &layerProperty )
{
  QString tip = layerProperty.isView ? QStringLiteral( "View" ) : QStringLiteral( "Table" );

  Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::TableLayer;
  if ( !layerProperty.geometryColName.isEmpty() && layerProperty.isGeometryValid() )
  {
    tip += tr( "\n%1 as %2" ).arg( layerProperty.geometryColName,
                                   QgsWkbTypes::displayString( layerProperty.type ) );

    if ( layerProperty.srid >= 0 )
      tip += tr( " (srid %1)" ).arg( layerProperty.srid );
    else
      tip += tr( " (unknown srid)" );

    if ( !layerProperty.tableComment.isEmpty() )
      tip = layerProperty.tableComment + '\n' + tip;

    QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( layerProperty.type );
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        layerType = Qgis::BrowserLayerType::Point;
        break;
      case QgsWkbTypes::LineGeometry:
        layerType = Qgis::BrowserLayerType::Line;
        break;
      case QgsWkbTypes::PolygonGeometry:
        layerType = Qgis::BrowserLayerType::Polygon;
        break;
      default:
        break;
    }
  }
  else
  {
    tip = tr( "as geometryless table" );
  }

  QgsHanaLayerItem *layerItem = new QgsHanaLayerItem( this, layerProperty.defaultName(),
      mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

QgsHanaRootItem::QgsHanaRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "SAP HANA" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconHana.svg" );
  populate();
}

QVector<QgsDataItem *> QgsHanaRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList connectionNames = QgsHanaSettings::getConnectionNames();
  connections.reserve( connectionNames.size() );
  for ( const QString &connName : connectionNames )
  {
    connections << new QgsHanaConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

void QgsHanaRootItem::onConnectionsChanged()
{
  refresh();
}

QgsDataItem *QgsHanaDataItemProvider::createDataItem(
  const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsHanaRootItem( parentItem, QStringLiteral( "SAP HANA" ), QStringLiteral( "hana:" ) );
}
