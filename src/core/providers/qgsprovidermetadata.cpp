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
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidersublayerdetails.h"

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

QIcon QgsProviderMetadata::icon() const
{
  return QIcon();
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsProviderMetadata::capabilities() const
{
  return QgsProviderMetadata::ProviderMetadataCapabilities();
}

QgsProviderMetadata::ProviderCapabilities QgsProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapabilities();
}

QList<QgsMapLayerType> QgsProviderMetadata::supportedLayerTypes() const
{
  return {};
}

QString QgsProviderMetadata::library() const
{
  return mLibrary;
}

QString QgsProviderMetadata::suggestGroupNameForUri( const QString & /*uri*/ ) const
{
  return QString();
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

QList<QgsMeshDriverMetadata> QgsProviderMetadata::meshDriversMetadata()
{
  return QList<QgsMeshDriverMetadata>();
}

int QgsProviderMetadata::priorityForUri( const QString & ) const
{
  return 0;
}

QList<QgsMapLayerType> QgsProviderMetadata::validLayerTypesForUri( const QString & ) const
{
  return QList<QgsMapLayerType>();
}

bool QgsProviderMetadata::uriIsBlocklisted( const QString & ) const
{
  return false;
}

QStringList QgsProviderMetadata::sidecarFilesForUri( const QString & ) const
{
  return QStringList();
}

QList<QgsProviderSublayerDetails> QgsProviderMetadata::querySublayers( const QString &, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  return QList<QgsProviderSublayerDetails>();
}

QgsDataProvider *QgsProviderMetadata::createProvider( const QString &uri,
    const QgsDataProvider::ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
{
  if ( mCreateFunction )
  {
    return mCreateFunction( uri, options, flags );
  }
  return nullptr;
}

void QgsProviderMetadata::setBoolParameter( QVariantMap &uri, const QString &parameter, const QVariant &value )
{
  if ( value.toString().compare( QStringLiteral( "yes" ), Qt::CaseInsensitive ) == 0 ||
       value.toString().compare( QStringLiteral( "1" ), Qt::CaseInsensitive ) == 0 ||
       value.toString().compare( QStringLiteral( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    uri[ parameter ] = true;
  }
  else if ( value.toString().compare( QStringLiteral( "no" ), Qt::CaseInsensitive ) == 0 ||
            value.toString().compare( QStringLiteral( "0" ), Qt::CaseInsensitive ) == 0 ||
            value.toString().compare( QStringLiteral( "false" ), Qt::CaseInsensitive ) == 0 )
  {
    uri[ parameter ] = false;
  }
}

bool QgsProviderMetadata::boolParameter( const QVariantMap &uri, const QString &parameter, bool defaultValue )
{
  if ( uri.value( parameter, QString() ).toString().compare( QStringLiteral( "yes" ), Qt::CaseInsensitive ) == 0 ||
       uri.value( parameter, QString() ).toString().compare( QStringLiteral( "1" ), Qt::CaseInsensitive ) == 0 ||
       uri.value( parameter, QString() ).toString().compare( QStringLiteral( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    return true;
  }
  else if ( uri.value( parameter, QString() ).toString().compare( QStringLiteral( "no" ), Qt::CaseInsensitive ) == 0 ||
            uri.value( parameter, QString() ).toString().compare( QStringLiteral( "0" ), Qt::CaseInsensitive ) == 0 ||
            uri.value( parameter, QString() ).toString().compare( QStringLiteral( "false" ), Qt::CaseInsensitive ) == 0 )
  {
    return false;
  }

  return defaultValue;
}

QVariantMap QgsProviderMetadata::decodeUri( const QString & ) const
{
  return QVariantMap();
}

QString QgsProviderMetadata::encodeUri( const QVariantMap & ) const
{
  return QString();
}

Qgis::VectorExportResult QgsProviderMetadata::createEmptyLayer(
  const QString &, const QgsFields &,
  QgsWkbTypes::Type, const QgsCoordinateReferenceSystem &,
  bool, QMap<int, int> &,
  QString &errorMessage, const QMap<QString, QVariant> * )
{
  errorMessage = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "createEmptyLayer" ) );
  return Qgis::VectorExportResult::ErrorProviderUnsupportedFeature;
}

bool QgsProviderMetadata::createDatabase( const QString &, QString &errorMessage )
{
  errorMessage = QObject::tr( "The %1 provider does not support database creation" ).arg( key() );
  return false;
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

bool QgsProviderMetadata::createMeshData( const QgsMesh &,
    const QString &,
    const QString &,
    const QgsCoordinateReferenceSystem & ) const
{
  return false;
}

bool QgsProviderMetadata::createMeshData( const QgsMesh &,
    const QString &,
    const QgsCoordinateReferenceSystem & ) const
{
  return false;
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


bool QgsProviderMetadata::styleExists( const QString &, const QString &, QString &errorCause )
{
  errorCause.clear();
  return false;
}

QString QgsProviderMetadata::getStyleById( const QString &, const QString &, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "getStyleById" ) );
  return QString();
}

bool QgsProviderMetadata::deleteStyleById( const QString &, const QString &, QString &errCause )
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

QString QgsProviderMetadata::loadStoredStyle( const QString &, QString &, QString &errCause )
{
  errCause = QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "loadStoredStyle" ) );
  return QString();
}

bool QgsProviderMetadata::saveLayerMetadata( const QString &, const QgsLayerMetadata &, QString & )
{
  throw QgsNotSupportedException( QObject::tr( "Provider %1 does not support writing layer metadata" ).arg( key() ) );
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
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "createConnection" ) ) );
}


QgsAbstractProviderConnection *QgsProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  Q_UNUSED( configuration );
  Q_UNUSED( uri );
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "createConnection" ) ) );
}

void QgsProviderMetadata::deleteConnection( const QString &name )
{
  Q_UNUSED( name );
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "deleteConnection" ) ) );
}

void QgsProviderMetadata::saveConnection( const QgsAbstractProviderConnection *connection, const QString &name )
{
  Q_UNUSED( connection )
  Q_UNUSED( name )
  throw QgsProviderConnectionException( QObject::tr( "Provider %1 has no %2 method" ).arg( key(), QStringLiteral( "saveConnection" ) ) );
}

///@cond PRIVATE
void QgsProviderMetadata::saveConnectionProtected( const QgsAbstractProviderConnection *conn, const QString &name )
{
  const bool isNewConnection = !connections().contains( name );
  conn->store( name );
  mProviderConnections.clear();

  if ( !isNewConnection )
    emit connectionChanged( name );
  else
    emit connectionCreated( name );
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

QgsMeshDriverMetadata::QgsMeshDriverMetadata() = default;

QgsMeshDriverMetadata::QgsMeshDriverMetadata( const QString &name,
    const QString &description,
    const MeshDriverCapabilities &capabilities,
    const QString &writeDatasetOnfileSuffix )
  : mName( name )
  , mDescription( description )
  , mCapabilities( capabilities )
  , mWriteDatasetOnFileSuffix( writeDatasetOnfileSuffix )
{
}

QgsMeshDriverMetadata::QgsMeshDriverMetadata( const QString &name,
    const QString &description,
    const MeshDriverCapabilities &capabilities,
    const QString &writeDatasetOnfileSuffix,
    const QString &writeMeshFrameOnFileSuffix,
    int maxVerticesPerface )
  : mName( name )
  , mDescription( description )
  , mCapabilities( capabilities )
  , mWriteDatasetOnFileSuffix( writeDatasetOnfileSuffix )
  , mWriteMeshFrameOnFileSuffix( ( writeMeshFrameOnFileSuffix ) )
  , mMaxVerticesPerFace( maxVerticesPerface )
{
}

QgsMeshDriverMetadata::MeshDriverCapabilities QgsMeshDriverMetadata::capabilities() const
{
  return mCapabilities;
}

QString QgsMeshDriverMetadata::name() const
{
  return mName;
}

QString QgsMeshDriverMetadata::description() const
{
  return mDescription;
}

QString QgsMeshDriverMetadata::writeDatasetOnFileSuffix() const
{
  return mWriteDatasetOnFileSuffix;
}

QString QgsMeshDriverMetadata::writeMeshFrameOnFileSuffix() const
{
  return mWriteMeshFrameOnFileSuffix;
}

int QgsMeshDriverMetadata::maximumVerticesCountPerFace() const
{
  return mMaxVerticesPerFace;
}
