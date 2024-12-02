/***************************************************************************
                             qgsdatacollectionitem.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatacollectionitem.h"
#include "moc_qgsdatacollectionitem.cpp"
#include "qgsapplication.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgslogger.h"

#include <QRegularExpression>

QgsDataCollectionItem::QgsDataCollectionItem( QgsDataItem *parent,
    const QString &name,
    const QString &path,
    const QString &providerKey )
  : QgsDataItem( Qgis::BrowserItemType::Collection, parent, name, path, providerKey )
{
  mCapabilities = Qgis::BrowserItemCapability::Fertile;
  mIconName = QStringLiteral( "/mIconDbSchema.svg" );
}

QgsDataCollectionItem::~QgsDataCollectionItem()
{
  QgsDebugMsgLevel( "mName = " + mName + " mPath = " + mPath, 2 );

// Do not delete children, children are deleted by QObject parent
#if 0
  const auto constMChildren = mChildren;
  for ( QgsDataItem *i : constMChildren )
  {
    QgsDebugMsgLevel( QStringLiteral( "delete child = 0x%0" ).arg( static_cast<qlonglong>( i ), 8, 16, QLatin1Char( '0' ) ), 2 );
    delete i;
  }
#endif
}

QIcon QgsDataCollectionItem::iconDataCollection()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconDbSchema.svg" ) );
}

QIcon QgsDataCollectionItem::openDirIcon( const QColor &fillColor, const QColor &strokeColor )
{
  return fillColor.isValid() || strokeColor.isValid()
         ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderOpenParams.svg" ), fillColor, strokeColor )
         : QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderOpen.svg" ) );
}

QIcon QgsDataCollectionItem::homeDirIcon( const QColor &fillColor, const QColor &strokeColor )
{
  return fillColor.isValid() || strokeColor.isValid()
         ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderHomeParams.svg" ), fillColor, strokeColor )
         : QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderHome.svg" ) );
}

QgsAbstractDatabaseProviderConnection *QgsDataCollectionItem::databaseConnection() const
{
  const QString dataProviderKey { QgsApplication::dataItemProviderRegistry()->dataProviderKey( providerKey() ) };
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( dataProviderKey ) };

  if ( ! md )
  {
    return nullptr;
  }

  const QString connectionName { name() };

  try
  {
    // First try to retrieve the connection by name if this is a stored connection
    if ( md->findConnection( connectionName ) )
    {
      return static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) );
    }

    // If that fails, try to create a connection from the path, in case this is a
    // filesystem-based DB (gpkg or spatialite)
    // The name is useless, we need to get the file path from the data item path
    const QString databaseFilePath { path().remove( QRegularExpression( R"re([\aZ]{2,}://)re" ) ) };

    if ( QFile::exists( databaseFilePath ) )
    {
      return static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( databaseFilePath, {} ) );
    }
  }
  catch ( QgsProviderConnectionException & )
  {
    // This is expected and it is not an error in case the provider does not implement
    // the connections API
  }
  return nullptr;
}

QIcon QgsDataCollectionItem::iconDir( const QColor &fillColor, const QColor &strokeColor )
{
  return fillColor.isValid() || strokeColor.isValid()
         ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderParams.svg" ), fillColor, strokeColor )
         : QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolder.svg" ) );
}



