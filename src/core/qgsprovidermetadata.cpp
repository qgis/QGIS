/***************************************************************************
                    qgsprovidermetadata.cpp  -  Metadata class for
                    describing a data provider.
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprovidermetadata.h"
#include "qgsdataprovider.h"
#include "qgsmaplayer.h"
#include "qgsexception.h"

QgsProviderMetadata::QgsProviderMetadata( QString const &key,
    QString const &description,
    QString const &library )
  : mKey( key )
  , mDescription( description )
  , mLibrary( library )
{}

QgsProviderMetadata::QgsProviderMetadata( const QString &key, const QString &description, const CreateDataProviderFunction &createFunc )
  : mKey( key )
  , mDescription( description )
  , mCreateFunction( createFunc )
{}

QgsProviderMetadata::~QgsProviderMetadata()
{
  qDeleteAll( mProviderConnections );
}

QString QgsProviderMetadata::key() const
{
  return mKey;
}

QString QgsProviderMetadata::description() const
{
  return mDescription;
}

QString QgsProviderMetadata::library() const
{
  return mLibrary;
}

QgsProviderMetadata::CreateDataProviderFunction QgsProviderMetadata::createFunction() const
{
  return mCreateFunction;
}

void QgsProviderMetadata::initProvider()
{

}

void QgsProviderMetadata::cleanupProvider()
{

}

QString QgsProviderMetadata::filters( FilterType )
{
  return QString();
}

QgsDataProvider *QgsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
{
  if ( mCreateFunction )
  {
    return mCreateFunction( uri, options );
  }
  return nullptr;
}

QVariantMap QgsProviderMetadata::decodeUri( const QString & )
{
  return QVariantMap();
}

QgsVectorLayerExporter::ExportError QgsProviderMetadata::createEmptyLayer(
  const QString &, const QgsFields &,
  QgsWkbTypes::Type, const QgsCoordinateReferenceSystem &,
  bool, QMap<int, int> &,
  QString &errorMessage, const QMap<QString, QVariant> * )
{
  errorMessage = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "createEmptyLayer" ) );
  return QgsVectorLayerExporter::ExportError::ErrProviderUnsupportedFeature;
}

QgsRasterDataProvider *QgsProviderMetadata::createRasterDataProvider(
  const QString &, const QString &,
  int, Qgis::DataType, int,
  int, double *,
  const QgsCoordinateReferenceSystem &,
  const QStringList & )
{
  return nullptr;
}

QList<QPair<QString, QString> > QgsProviderMetadata::pyramidResamplingMethods()
{
  return QList<QPair<QString, QString> >();
}

QList<QgsDataItemProvider *> QgsProviderMetadata::dataItemProviders() const
{
  return QList<QgsDataItemProvider *>();
}

int QgsProviderMetadata::listStyles( const QString &, QStringList &, QStringList &,
                                     QStringList &, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "listStyles" ) );
  return -1;
}

QString QgsProviderMetadata::getStyleById( const QString &, QString, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "getStyleById" ) );
  return QString();
}

bool QgsProviderMetadata::deleteStyleById( const QString &, QString, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "deleteStyleById" ) );
  return false;
}

bool QgsProviderMetadata::saveStyle( const QString &, const QString &, const QString &, const QString &,
                                     const QString &, const QString &, bool, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "saveStyle" ) );
  return false;
}

QString QgsProviderMetadata::loadStyle( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "loadStyle" ) );
  return QString();
}

bool QgsProviderMetadata::createDb( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "createDb" ) );
  return false;
}

QgsTransaction *QgsProviderMetadata::createTransaction( const QString & )
{
  return nullptr;
}

QMap<QString, QgsAbstractProviderConnection *> QgsProviderMetadata::connections( bool cached )
{
  Q_UNUSED( cached );
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "connections" ) ) );
}

QMap<QString, QgsAbstractDatabaseProviderConnection *> QgsProviderMetadata::dbConnections( bool cached )
{
  return connections<QgsAbstractDatabaseProviderConnection>( cached ) ;
}

QgsAbstractProviderConnection *QgsProviderMetadata::findConnection( const QString &name, bool cached )
{
  const QMap<QString, QgsAbstractProviderConnection *> constConns { connections( cached ) };
  const QStringList constKeys { constConns.keys( ) };
  for ( const QString &key : constKeys )
  {
    if ( key == name )
    {
      return constConns.value( key );
    }
  }
  return nullptr;
}

QgsAbstractProviderConnection *QgsProviderMetadata::createConnection( const QString &name )
{
  Q_UNUSED( name );
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "connection" ) ) );
}


QgsAbstractProviderConnection *QgsProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  Q_UNUSED( configuration );
  Q_UNUSED( uri );
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "connection" ) ) );
}

void QgsProviderMetadata::deleteConnection( const QString &name )
{
  Q_UNUSED( name );
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "deleteConnection" ) ) );
}

void QgsProviderMetadata::saveConnection( const QgsAbstractProviderConnection *connection, const QString &name )
{
  Q_UNUSED( connection );
  Q_UNUSED( name );
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "saveConnection" ) ) );
}

///@cond PRIVATE
void QgsProviderMetadata::saveConnectionProtected( const QgsAbstractProviderConnection *conn, const QString &name )
{
  conn->store( name );
  mProviderConnections.clear();
}
///@endcond

template<typename T>
QMap<QString, T *> QgsProviderMetadata::connections( bool cached )
{
  QMap<QString, T *> result;
  const auto constConns { connections( cached ) };
  const QStringList constConnKeys { constConns.keys() };
  for ( const auto &c : constConnKeys )
  {
    T *casted { static_cast<T *>( constConns.value( c ) ) };
    if ( casted )
    {
      result.insert( c, casted );
    }
  }
  return result;
}




