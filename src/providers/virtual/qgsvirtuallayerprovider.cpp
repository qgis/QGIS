/***************************************************************************
           qgsvirtuallayerprovider.cpp Virtual layer data provider
begin                : Jan, 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
#include <sqlite3.h>
#include <spatialite.h>
}

#include <stdexcept>

#include "qgsvirtuallayerprovider.h"
#include "qgsvirtuallayerdefinition.h"
#include "qgsvirtuallayerfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgslogger.h"

#include "qgsvirtuallayerprovider.h"
#include "qgsvirtuallayersqlitemodule.h"
#include "qgsvirtuallayerqueryparser.h"

#include <QUrl>

const QString QgsVirtualLayerProvider::VIRTUAL_LAYER_KEY = QStringLiteral( "virtual" );
const QString QgsVirtualLayerProvider::VIRTUAL_LAYER_DESCRIPTION = QStringLiteral( "Virtual layer data provider" );
const QString QgsVirtualLayerProvider::VIRTUAL_LAYER_QUERY_VIEW = QStringLiteral( "_query" );

static QString quotedColumn( QString name )
{
  return "\"" + name.replace( QLatin1String( "\"" ), QLatin1String( "\"\"" ) ) + "\"";
}

#define PROVIDER_ERROR( msg ) do { mError = QgsError( msg, QgsVirtualLayerProvider::VIRTUAL_LAYER_KEY ); QgsDebugMsg( msg ); } while(0)


QgsVirtualLayerProvider::QgsVirtualLayerProvider( QString const &uri,
    const QgsDataProvider::ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  mError.clear();

  const QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  if ( !url.isValid() )
  {
    mValid = false;
    PROVIDER_ERROR( "Malformed URL" );
    return;
  }

  // xxxxx = open a virtual layer
  // xxxxx?key=value&key=value = create a virtual layer
  // ?key=value = create a temporary virtual layer

  // read url
  try
  {
    mDefinition = QgsVirtualLayerDefinition::fromUrl( url );

    mSubset = mDefinition.subsetString();

    if ( !mDefinition.isLazy() )
    {
      reloadData();
    }
  }
  catch ( std::runtime_error &e )
  {
    mValid = false;
    PROVIDER_ERROR( e.what() );
    return;
  }

  if ( mDefinition.geometrySrid() != -1 )
  {
    Q_NOWARN_DEPRECATED_PUSH
    mCrs = QgsCoordinateReferenceSystem( mDefinition.geometrySrid() );
    Q_NOWARN_DEPRECATED_POP
  }
}

void QgsVirtualLayerProvider::reloadProviderData()
{
  if ( mDefinition.sourceLayers().empty() && !mDefinition.filePath().isEmpty() && mDefinition.query().isEmpty() )
  {
    // open the file
    mValid = openIt();
  }
  else
  {
    // create the file
    mValid = createIt();
  }
}

bool QgsVirtualLayerProvider::loadSourceLayers()
{
  const auto constSourceLayers = mDefinition.sourceLayers();
  for ( const QgsVirtualLayerDefinition::SourceLayer &layer : constSourceLayers )
  {
    if ( layer.isReferenced() )
    {
      QgsMapLayer *l = QgsProject::instance()->mapLayer( layer.reference() );
      if ( !l )
      {
        PROVIDER_ERROR( QString( "Cannot find layer %1" ).arg( layer.reference() ) );
        return false;
      }
      if ( l->type() != QgsMapLayerType::VectorLayer )
      {
        PROVIDER_ERROR( QString( "Layer %1 is not a vector layer" ).arg( layer.reference() ) );
        return false;
      }
      // add the layer to the list
      QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( l );
      mLayers << SourceLayer( vl, layer.name() );
      // connect to modification signals to invalidate statistics
      connect( vl, &QgsVectorLayer::featureAdded, this, &QgsVirtualLayerProvider::invalidateStatistics );
      connect( vl, &QgsVectorLayer::featureDeleted, this, &QgsVirtualLayerProvider::invalidateStatistics );
      connect( vl, &QgsVectorLayer::geometryChanged, this, &QgsVirtualLayerProvider::invalidateStatistics );
      connect( vl, &QgsVectorLayer::updatedFields, this, [ = ] { createVirtualTable( vl, layer.name() ); } );
    }
    else
    {
      mLayers << SourceLayer( layer.provider(), layer.source(), layer.name(), layer.encoding() );
    }
  }
  return true;
}

bool QgsVirtualLayerProvider::openIt()
{
  spatialite_init( 0 );

  mPath = mDefinition.filePath();

  try
  {
    QgsScopedSqlite p( mPath );
    mSqlite = p;
  }
  catch ( std::runtime_error &e )
  {
    PROVIDER_ERROR( e.what() );
    return false;
  }

  {
    Sqlite::Query q( mSqlite.get(), QStringLiteral( "SELECT name FROM sqlite_master WHERE name='_meta'" ) );
    if ( q.step() != SQLITE_ROW )
    {
      PROVIDER_ERROR( "No metadata tables!" );
      return false;
    }
  }
  // look for the correct version and the stored url
  {
    Sqlite::Query q( mSqlite.get(), QStringLiteral( "SELECT version, url FROM _meta" ) );
    int version = 0;
    if ( q.step() == SQLITE_ROW )
    {
      version = q.columnInt( 0 );
      if ( version != VIRTUAL_LAYER_VERSION )
      {
        PROVIDER_ERROR( "Wrong virtual layer version!" );
        return false;
      }
      mDefinition = QgsVirtualLayerDefinition::fromUrl( QUrl( q.columnText( 1 ) ) );
    }
  }
  // overwrite the uri part of the definition
  mDefinition.setFilePath( mPath );


  // load source layers
  if ( !loadSourceLayers() )
  {
    return false;
  }

  /* only one table */
  if ( mDefinition.query().isEmpty() )
  {
    mTableName = mLayers[0].name;
  }
  else
  {
    mTableName = VIRTUAL_LAYER_QUERY_VIEW;
  }

  mSubset = mDefinition.subsetString();

  return true;
}

bool QgsVirtualLayerProvider::createIt()
{
  using namespace QgsVirtualLayerQueryParser;

  // consistency check
  if ( mDefinition.sourceLayers().size() > 1 && mDefinition.query().isEmpty() )
  {
    PROVIDER_ERROR( QString( "Don't know how to join layers, please specify a query" ) );
    return false;
  }

  if ( mDefinition.sourceLayers().empty() && mDefinition.filePath().isEmpty() && mDefinition.query().isEmpty() )
  {
    PROVIDER_ERROR( QString( "Please specify at least one source layer or a query" ) );
    return false;
  }

  if ( !mDefinition.filePath().isEmpty() && mDefinition.hasReferencedLayers() )
  {
    PROVIDER_ERROR( QString( "Cannot store referenced layers" ) );
    return false;
  }


  QVector<ColumnDef> gFields;
  if ( !mDefinition.query().isEmpty() )
  {

    const QStringList tables = referencedTables( mDefinition.query() );
    const auto constTables = tables;
    for ( const QString &tname : constTables )
    {
      // is it in source layers ?
      if ( mDefinition.hasSourceLayer( tname ) )
      {
        continue;
      }
      // is it in loaded layers ?
      bool found = false;
      const auto constMapLayers = QgsProject::instance()->mapLayers();
      for ( const QgsMapLayer *l : constMapLayers )
      {
        if ( l->type() != QgsMapLayerType::VectorLayer )
          continue;

        const QgsVectorLayer *vl = static_cast<const QgsVectorLayer *>( l );
        if ( ( vl->name() == tname ) || ( vl->name().compare( tname.toLower(), Qt::CaseInsensitive ) == 0 ) || ( vl->id() == tname ) )
        {
          mDefinition.addSource( tname, vl->id() );
          found = true;
          break;
        }
      }
      if ( !found )
      {
        PROVIDER_ERROR( QString( "Referenced table %1 in query not found!" ).arg( tname ) );
        return false;
      }
    }
  }

  QString path;
  mPath = mDefinition.filePath();
  // use a temporary file if needed
  if ( mPath.isEmpty() )
    path = QStringLiteral( ":memory:" );
  else
    path = mPath;

  spatialite_init( 0 );

  try
  {
    QgsScopedSqlite sqlite( path );
    mSqlite = sqlite;
  }
  catch ( std::runtime_error &e )
  {
    PROVIDER_ERROR( e.what() );
    return false;
  }

  resetSqlite();
  initVirtualLayerMetadata( mSqlite.get() );

  const bool noGeometry = mDefinition.geometryWkbType() == QgsWkbTypes::NoGeometry;

  // load source layers (and populate mLayers)
  if ( !loadSourceLayers() )
  {
    return false;
  }

  // now create virtual tables based on layers
  for ( int i = 0; i < mLayers.size(); i++ )
  {
    QgsVectorLayer *vlayer = mLayers.at( i ).layer;
    const QString vname = mLayers.at( i ).name;
    if ( vlayer )
    {
      createVirtualTable( vlayer, vname );
    }
    else
    {
      QString provider = mLayers.at( i ).provider;
      // double each single quote
      provider.replace( QLatin1String( "'" ), QLatin1String( "''" ) );
      QString source = mLayers.at( i ).source;
      source.replace( QLatin1String( "'" ), QLatin1String( "''" ) );
      const QString encoding = mLayers.at( i ).encoding;
      const QString createStr = QStringLiteral( "DROP TABLE IF EXISTS \"%1\"; CREATE VIRTUAL TABLE \"%1\" USING QgsVLayer('%2','%4',%3)" )
                                .arg( vname,
                                      provider,
                                      encoding,
                                      source ); // source must be the last argument here, since it can contains '%x' strings that would be replaced
      Sqlite::Query::exec( mSqlite.get(), createStr );
    }
  }

  QgsFields tfields;
  if ( !mDefinition.query().isEmpty() )
  {
    // look for column types of the query
    TableDef columns = columnDefinitionsFromQuery( mSqlite.get(), mDefinition.query() );

    for ( int i = 0; i < columns.size(); i++ )
    {
      ColumnDef &c = columns[i];

      if ( c.name().isEmpty() )
      {
        PROVIDER_ERROR( QString( "Result column #%1 has no name!" ).arg( i + 1 ) );
        return false;
      }

      // then override types by the ones defined in the url
      if ( mDefinition.fields().indexFromName( c.name() ) != -1 )
      {
        c.setScalarType( mDefinition.fields().field( c.name() ).type() );
      }

      if ( c.isGeometry() )
      {
        gFields << c;
      }
      // if the geometry field is not detected as a geometry, move it to the geometry fields
      // with the provided type and srid
      else if ( mDefinition.hasDefinedGeometry() && c.name() == mDefinition.geometryField() )
      {
        ColumnDef g;
        g.setName( mDefinition.geometryField() );
        g.setGeometry( mDefinition.geometryWkbType() );
        g.setSrid( mDefinition.geometrySrid() );
        gFields << g;
      }
      // default type: string
      else if ( c.scalarType() == QVariant::Invalid )
      {
        c.setScalarType( QVariant::String );
      }
      else
      {
        tfields.append( QgsField( c.name(), c.scalarType() ) );
      }
    }

    // process geometry field
    if ( !noGeometry )
    {
      // no geometry field defined yet, take the first detected
      if ( mDefinition.geometryField().isEmpty() )
      {
        if ( gFields.count() > 0 )
        {
          mDefinition.setGeometryField( gFields[0].name() );
          mDefinition.setGeometryWkbType( gFields[0].wkbType() );
          mDefinition.setGeometrySrid( gFields[0].srid() );
        }
      }
      // a geometry field is named, but has no type yet
      // look for a detected type
      else if ( !mDefinition.hasDefinedGeometry() )
      {
        bool found = false;
        for ( int i = 0; i < gFields.size(); i++ )
        {
          if ( gFields[i].name() == mDefinition.geometryField() )
          {
            // override the geometry type
            mDefinition.setGeometryWkbType( gFields[i].wkbType() );
            mDefinition.setGeometrySrid( gFields[i].srid() );
            found = true;
            break;
          }
        }
        if ( !found )
        {
          PROVIDER_ERROR( "Cannot find the specified geometry field!" );
          return false;
        }
      }

      if ( !mDefinition.geometryField().isEmpty() && !mDefinition.hasDefinedGeometry() )
      {
        PROVIDER_ERROR( "Can't deduce the geometry type of the geometry field!" );
        return false;
      }
    }

    // save field definitions
    mDefinition.setFields( tfields );

    mTableName = VIRTUAL_LAYER_QUERY_VIEW;

    // create a view
    const QString viewStr = QStringLiteral( "DROP VIEW IF EXISTS %1; CREATE VIEW %1 AS %2" )
                            .arg( VIRTUAL_LAYER_QUERY_VIEW,
                                  mDefinition.query() );
    Sqlite::Query::exec( mSqlite.get(), viewStr );
  }
  else
  {
    // no query => implies we must only have one virtual table
    mTableName = mLayers[0].name;

    const TableDef td = tableDefinitionFromVirtualTable( mSqlite.get(), mTableName );
    const auto constTd = td;
    for ( const ColumnDef &c : constTd )
    {
      if ( !c.isGeometry() )
      {
        tfields.append( QgsField( c.name(), c.scalarType() ) );
      }
      else if ( !noGeometry )
      {
        mDefinition.setGeometryField( QStringLiteral( "geometry" ) );
        mDefinition.setGeometryWkbType( c.wkbType() );
        mDefinition.setGeometrySrid( c.srid() );
      }
    }
    mDefinition.setFields( tfields );
  }

  // Save the definition back to the sqlite file
  {
    Sqlite::Query q( mSqlite.get(), QStringLiteral( "UPDATE _meta SET url=?" ) );
    q.bind( mDefinition.toUrl().toString() );
    q.step();
  }

  return true;
}

void QgsVirtualLayerProvider::createVirtualTable( QgsVectorLayer *vlayer, const QString &vname )
{
  const QString createStr = QStringLiteral( "DROP TABLE IF EXISTS \"%1\"; CREATE VIRTUAL TABLE \"%1\" USING QgsVLayer('%2');" ).arg( vname, vlayer->id() );
  Sqlite::Query::exec( mSqlite.get(), createStr );
}

bool QgsVirtualLayerProvider::cancelReload()
{
  return mSqlite.interrupt();
}

void QgsVirtualLayerProvider::resetSqlite()
{
  bool hasSpatialrefsys = false;
  {
    Sqlite::Query q( mSqlite.get(), QStringLiteral( "SELECT name FROM sqlite_master WHERE name='spatial_ref_sys'" ) );
    hasSpatialrefsys = q.step() == SQLITE_ROW;
  }

  QString sql = QStringLiteral( "DROP TABLE IF EXISTS _meta;" );
  if ( !hasSpatialrefsys )
  {
    sql += QLatin1String( "SELECT InitSpatialMetadata(1);" );
  }
  Sqlite::Query::exec( mSqlite.get(), sql );
}

QgsAbstractFeatureSource *QgsVirtualLayerProvider::featureSource() const
{
  return new QgsVirtualLayerFeatureSource( this );
}

QString QgsVirtualLayerProvider::storageType() const
{
  return QStringLiteral( "No storage per se, view data from other data sources" );
}

QgsCoordinateReferenceSystem QgsVirtualLayerProvider::crs() const
{
  return mCrs;
}

QgsFeatureIterator QgsVirtualLayerProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsVirtualLayerFeatureIterator( new QgsVirtualLayerFeatureSource( this ), false, request ) );
}

QString QgsVirtualLayerProvider::subsetString() const
{
  return mSubset;
}

bool QgsVirtualLayerProvider::setSubsetString( const QString &subset, bool updateFeatureCount )
{
  if ( subset == mSubset )
    return true;

  mSubset = subset;
  clearMinMaxCache();
  if ( updateFeatureCount )
    updateStatistics();

  mDefinition.setSubsetString( subset );

  setDataSourceUri( mDefinition.toString() );

  emit dataChanged();

  return true;
}


QgsWkbTypes::Type QgsVirtualLayerProvider::wkbType() const
{
  return static_cast<QgsWkbTypes::Type>( mDefinition.geometryWkbType() );
}

long long QgsVirtualLayerProvider::featureCount() const
{
  if ( !mCachedStatistics )
  {
    updateStatistics();
  }
  return mFeatureCount;
}

QgsRectangle QgsVirtualLayerProvider::extent() const
{
  if ( !mCachedStatistics )
  {
    updateStatistics();
  }
  return mExtent;
}

void QgsVirtualLayerProvider::updateStatistics() const
{
  const bool hasGeometry = mDefinition.geometryWkbType() != QgsWkbTypes::NoGeometry;
  const QString subset = mSubset.isEmpty() ? QString() : " WHERE " + mSubset;
  const QString sql = QStringLiteral( "SELECT Count(*)%1 FROM %2%3" )
                      .arg( hasGeometry ? QStringLiteral( ",Min(MbrMinX(%1)),Min(MbrMinY(%1)),Max(MbrMaxX(%1)),Max(MbrMaxY(%1))" ).arg( quotedColumn( mDefinition.geometryField() ) ) : QString(),
                            mTableName,
                            subset );

  try
  {
    Sqlite::Query q( mSqlite.get(), sql );
    if ( q.step() == SQLITE_ROW )
    {
      mFeatureCount = q.columnInt64( 0 );
      if ( hasGeometry )
      {
        double x1, y1, x2, y2;
        x1 = q.columnDouble( 1 );
        y1 = q.columnDouble( 2 );
        x2 = q.columnDouble( 3 );
        y2 = q.columnDouble( 4 );
        mExtent = QgsRectangle( x1, y1, x2, y2 );
      }
      mCachedStatistics = true;
    }
  }
  catch ( std::runtime_error &e )
  {
    pushError( tr( "Error while executing feature count request : %1" ).arg( e.what() ) );
    mFeatureCount = 0;
    return;
  }
}

void QgsVirtualLayerProvider::invalidateStatistics()
{
  mCachedStatistics = false;
}

QgsFields QgsVirtualLayerProvider::fields() const
{
  return mDefinition.fields();
}

bool QgsVirtualLayerProvider::isValid() const
{
  return mValid;
}

QgsVectorDataProvider::Capabilities QgsVirtualLayerProvider::capabilities() const
{
  QgsVectorDataProvider::Capabilities capabilities = CancelSupport | QgsVectorDataProvider::Capability::ReloadData;

  if ( !mDefinition.uid().isNull() )
  {
    capabilities |= SelectAtId;
  }

  return capabilities;
}

QString QgsVirtualLayerProvider::name() const
{
  return VIRTUAL_LAYER_KEY;
}

QString QgsVirtualLayerProvider::providerKey()
{
  return VIRTUAL_LAYER_KEY;
}

QString QgsVirtualLayerProvider::description() const
{
  return VIRTUAL_LAYER_DESCRIPTION;
}

QgsAttributeList QgsVirtualLayerProvider::pkAttributeIndexes() const
{
  if ( !mDefinition.uid().isNull() )
  {
    const QgsFields &fields = mDefinition.fields();
    for ( int i = 0; i < fields.size(); i++ )
    {
      if ( fields.at( i ).name().compare( mDefinition.uid(), Qt::CaseInsensitive ) == 0 )
      {
        QgsAttributeList l;
        l << i;
        return l;
      }
    }
  }
  return QgsAttributeList();
}

QSet<QgsMapLayerDependency> QgsVirtualLayerProvider::dependencies() const
{
  QSet<QgsMapLayerDependency> deps;
  const auto constSourceLayers = mDefinition.sourceLayers();
  for ( const QgsVirtualLayerDefinition::SourceLayer &l : constSourceLayers )
  {
    if ( l.isReferenced() )
      deps << QgsMapLayerDependency( l.reference(), QgsMapLayerDependency::PresenceDependency, QgsMapLayerDependency::FromProvider );
  }
  return deps;
}

QgsVirtualLayerProvider *QgsVirtualLayerProviderMetadata::createProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
{
  return new QgsVirtualLayerProvider( uri, options, flags );
}

QgsVirtualLayerProviderMetadata::QgsVirtualLayerProviderMetadata():
  QgsProviderMetadata( QgsVirtualLayerProvider::VIRTUAL_LAYER_KEY, QgsVirtualLayerProvider::VIRTUAL_LAYER_DESCRIPTION )
{
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsVirtualLayerProviderMetadata();
}
#endif
