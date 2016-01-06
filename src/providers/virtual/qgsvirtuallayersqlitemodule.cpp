/***************************************************************************
             qgsvirtuallayersqlitemodule.cpp : SQLite module for QGIS virtual layers
begin                : Nov 2015
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

#include <string.h>
#include <iostream>
#include <stdint.h>
#include <stdexcept>

#include <QCoreApplication>

#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmaplayerregistry.h>
#include <qgsproviderregistry.h>

#include <sqlite3.h>
#include <spatialite.h>
#include <stdio.h>
#include "qgsvirtuallayersqlitemodule.h"
#include "qgsvirtuallayerblob.h"
#include "qgsslottofunction.h"

/**
 * Structure created in SQLITE module creation and passed to xCreate/xConnect
 */
struct ModuleContext
{
  // private pointer needed for spatialite_init_ex;
  // allows to know whether the database has been initialied (null or not)
  bool init;
  ModuleContext() : init( false ) {}
};

/**
 * Create metadata tables if needed
 */
void initVirtualLayerMetadata( sqlite3* db )
{
  bool create_meta = false;

  sqlite3_stmt *stmt;
  int r;
  r = sqlite3_prepare_v2( db, "SELECT name FROM sqlite_master WHERE name='_meta'", -1, &stmt, nullptr );
  if ( r )
  {
    throw std::runtime_error( sqlite3_errmsg( db ) );
  }
  create_meta = sqlite3_step( stmt ) != SQLITE_ROW;
  sqlite3_finalize( stmt );

  char *errMsg;
  if ( create_meta )
  {
    r = sqlite3_exec( db, QString( "CREATE TABLE _meta (version INT, url TEXT); INSERT INTO _meta (version) VALUES(%1);" ).arg( VIRTUAL_LAYER_VERSION ).toLocal8Bit().constData(), nullptr, nullptr, &errMsg );
    if ( r )
    {
      throw std::runtime_error( errMsg );
    }
  }
}

void deleteGeometryBlob( void * p )
{
  delete[]( reinterpret_cast< unsigned char* >( p ) );
}

//-----------------------------------------------------------------------
//
//  Functions and structures used by the SQLite virtual table module
//
//-----------------------------------------------------------------------

// function called when a lived layer is deleted
void invalidateTable( void* b );

struct VTable
{
  // minimal set of members (see sqlite3.h)
  const sqlite3_module *pModule;  /* The module for this virtual table */
  int nRef;                       /* NO LONGER USED */
  char *zErrMsg;                  /* Error message from sqlite3_mprintf() */

  VTable( sqlite3* db, QgsVectorLayer* layer )
      : pModule( nullptr )
      , nRef( 0 )
      , zErrMsg( nullptr )
      , mSql( db )
      , mProvider( nullptr )
      , mLayer( layer )
      , mSlotToFunction( invalidateTable, this )
      , mName( layer->name() )
      , mPkColumn( -1 )
      , mValid( true )
  {
    if ( mLayer )
    {
      QObject::connect( layer, SIGNAL( layerDeleted() ), &mSlotToFunction, SLOT( onSignal() ) );
      init_();
    }
  }

  VTable( sqlite3* db, const QString& provider, const QString& source, const QString& name, const QString& encoding )
      : pModule( nullptr )
      , nRef( 0 )
      , zErrMsg( nullptr )
      , mSql( db )
      , mLayer( nullptr )
      , mName( name )
      , mEncoding( encoding )
      , mPkColumn( -1 )
      , mValid( true )
  {
    mProvider = static_cast<QgsVectorDataProvider*>( QgsProviderRegistry::instance()->provider( provider, source ) );
    if ( !mProvider )
    {
      throw std::runtime_error( "Invalid provider" );
    }
    else if ( mProvider && !mProvider->isValid() )
    {
      throw std::runtime_error(( "Provider error:" + mProvider->error().message() ).toUtf8().constData() );
    }
    if ( mProvider->capabilities() & QgsVectorDataProvider::SelectEncoding )
    {
      mProvider->setEncoding( mEncoding );
    }
    init_();
  }

  ~VTable()
  {
    if ( mProvider )
    {
      delete mProvider;
    }
  }

  QgsVectorDataProvider* provider() { return mProvider; }

  QgsVectorLayer* layer() { return mLayer; }

  QString name() const { return mName; }

  QString creationString() const { return mCreationStr; }

  long crs() const { return mCrs; }

  sqlite3* sql() { return mSql; }

  int pkColumn() const { return mPkColumn; }

  void invalidate() { mValid = false; }

  bool valid() const { return mValid; }

private:
  // connection
  sqlite3* mSql;

  // pointer to the underlying vector provider
  QgsVectorDataProvider* mProvider;
  // pointer to the vector layer, for referenced layer
  QgsVectorLayer* mLayer;
  // the QObjet responsible of receiving the deletion signal
  QgsSlotToFunction mSlotToFunction;

  QString mName;

  QString mEncoding;

  // primary key column (default = -1: none)
  int mPkColumn;

  // CREATE TABLE string
  QString mCreationStr;

  long mCrs;

  bool mValid;

  void init_()
  {
    const QgsFields& fields = mLayer ? mLayer->fields() : mProvider->fields();
    QStringList sql_fields;

    // add a hidden field for rtree filtering
    sql_fields << "_search_frame_ HIDDEN BLOB";

    for ( int i = 0; i < fields.count(); i++ )
    {
      QString typeName = "text";
      switch ( fields.at( i ).type() )
      {
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Bool:
        case QVariant::LongLong:
          typeName = "int";
          break;
        case QVariant::Double:
          typeName = "real";
          break;
        case QVariant::String:
        default:
          typeName = "text";
          break;
      }
      sql_fields << fields.at( i ).name() + " " + typeName;
    }

    QgsVectorDataProvider* provider = mLayer ? mLayer->dataProvider() : mProvider;
    if ( provider->geometryType() != QGis::WKBNoGeometry )
    {
      // we have here a convenient hack
      // the type of a column can be declared with two numeric arguments, usually for setting numeric precision
      // we are using them to set the geometry type and srid
      // these will be reused by the provider when it will introspect the query to detect types
      sql_fields << QString( "geometry geometry(%1,%2)" ).arg( provider->geometryType() ).arg( provider->crs().postgisSrid() );
    }

    if ( provider->pkAttributeIndexes().size() == 1 )
    {
      mPkColumn = provider->pkAttributeIndexes()[0] + 1;
    }

    mCreationStr = "CREATE TABLE vtable (" + sql_fields.join( "," ) + ")";

    mCrs = provider->crs().postgisSrid();
  }
};

// function called when a lived layer is deleted
void invalidateTable( void* p )
{
  reinterpret_cast<VTable *>( p )->invalidate();
}

struct VTableCursor
{
  // minimal set of members (see sqlite3.h)
  VTable *mVtab;

  // specific members
  QgsFeature mCurrentFeature;
  QgsFeatureIterator mIterator;
  bool mEof;

  VTableCursor( VTable *vtab ) : mVtab( vtab ), mEof( true ) {}

  void filter( const QgsFeatureRequest& request )
  {
    if ( !mVtab->valid() )
    {
      mEof = true;
      return;
    }

    mIterator = mVtab->layer() ? mVtab->layer()->getFeatures( request ) : mVtab->provider()->getFeatures( request );
    // get on the first record
    mEof = false;
    next();
  }

  void next()
  {
    if ( !mEof )
    {
      mEof = !mIterator.nextFeature( mCurrentFeature );
    }
  }

  bool eof() const { return mEof; }

  int nColumns() const
  {
    if ( !mVtab->valid() )
      return 0;
    return mVtab->layer() ? mVtab->layer()->fields().count() : mVtab->provider()->fields().count();
  }

  sqlite3_int64 currentId() const { return mCurrentFeature.id(); }

  QVariant currentAttribute( int column ) const { return mCurrentFeature.attribute( column ); }

  QPair<char*, int> currentGeometry() const
  {
    int blob_len = 0;
    char* blob = nullptr;
    const QgsGeometry* g = mCurrentFeature.constGeometry();
    if ( g && ! g->isEmpty() )
    {
      qgsGeometryToSpatialiteBlob( *g, mVtab->crs(), blob, blob_len );
    }
    return qMakePair( blob, blob_len );
  }
};

void getGeometryType( const QgsVectorDataProvider* provider, QString& geometryTypeStr, int& geometryDim, int& geometryWkbType, long& srid )
{
  srid = const_cast<QgsVectorDataProvider*>( provider )->crs().postgisSrid();
  QgsWKBTypes::Type t = QGis::fromOldWkbType( provider->geometryType() );
  geometryTypeStr = QgsWKBTypes::displayString( t );
  geometryDim = QgsWKBTypes::coordDimensions( t );
  if (( t != QgsWKBTypes::NoGeometry ) && ( t != QgsWKBTypes::Unknown ) )
    geometryWkbType = static_cast<int>( t );
  else
    geometryWkbType = 0;
}

int vtable_create_connect( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **out_vtab, char** out_err, bool is_created )
{
  Q_UNUSED( aux );
  Q_UNUSED( is_created );

#define RETURN_CSTR_ERROR(err) if (out_err) {size_t s = strlen(err); *out_err=reinterpret_cast<char*>(sqlite3_malloc( static_cast<int>( s ) +1)); strncpy(*out_err, err, s);}
#define RETURN_CPPSTR_ERROR(err) if (out_err) {*out_err=reinterpret_cast<char*>(sqlite3_malloc( static_cast<int>( err.size() )+1)); strncpy(*out_err, err.c_str(), err.size());}

  if ( argc < 4 )
  {
    std::string err( "Missing arguments: layer_id | provider, source" );
    RETURN_CPPSTR_ERROR( err );
    return SQLITE_ERROR;
  }

  QScopedPointer<VTable> new_vtab;
  QString vname( argv[2] );
  int r;
  if ( argc == 4 )
  {
    // CREATE VIRTUAL TABLE vtab USING QgsVLayer(layer_id)
    // vtab = argv[2]
    // layer_id = argv[3]
    QString layerid( argv[3] );
    if ( layerid.size() >= 1 && layerid[0] == '\'' )
    {
      layerid = layerid.mid( 1, layerid.size() - 2 );
    }
    QgsMapLayer *l = QgsMapLayerRegistry::instance()->mapLayer( layerid );
    if ( !l || l->type() != QgsMapLayer::VectorLayer )
    {
      if ( out_err )
      {
        std::string err( "Cannot find layer " );
        err += argv[3];
        RETURN_CPPSTR_ERROR( err );
      }
      return SQLITE_ERROR;
    }
    new_vtab.reset( new VTable( sql, static_cast<QgsVectorLayer*>( l ) ) );

  }
  else if ( argc == 5 || argc == 6 )
  {
    // CREATE VIRTUAL TABLE vtab USING QgsVLayer(provider,source[,encoding])
    // vtab = argv[2]
    // provider = argv[3]
    // source = argv[4]
    // encoding = argv[5]
    QString provider = argv[3];
    QString source = argv[4];
    QString encoding = "UTF-8";
    if ( argc == 6 )
    {
      encoding = argv[5];
    }
    if ( provider.size() >= 1 && provider[0] == '\'' )
    {
      // trim and undouble single quotes
      provider = provider.mid( 1, provider.size() - 2 ).replace( "''", "'" );
    }
    if ( source.size() >= 1 && source[0] == '\'' )
    {
      // trim and undouble single quotes
      source = source.mid( 1, source.size() - 2 ).replace( "''", "'" );
    }
    try
    {
      new_vtab.reset( new VTable( sql, provider, source, argv[2], encoding ) );
    }
    catch ( std::runtime_error& e )
    {
      std::string err( e.what() );
      RETURN_CPPSTR_ERROR( err );
      return SQLITE_ERROR;
    }
  }

  r = sqlite3_declare_vtab( sql, new_vtab->creationString().toLocal8Bit().constData() );
  if ( r )
  {
    RETURN_CSTR_ERROR( sqlite3_errmsg( sql ) );
    return r;
  }

  *out_vtab = reinterpret_cast< sqlite3_vtab* >( new_vtab.take() );
  return SQLITE_OK;
#undef RETURN_CSTR_ERROR
#undef RETURN_CPPSTR_ERROR
}

void db_init( sqlite3* db, ModuleContext* context )
{
  if ( context->init )
  {
    // db already initialized
    return;
  }

  // create metadata tables
  initVirtualLayerMetadata( db );
}

int vtable_create( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **out_vtab, char** out_err )
{
  try
  {
    db_init( sql, reinterpret_cast<ModuleContext*>( aux ) );
  }
  catch ( std::runtime_error& e )
  {
    if ( out_err )
    {
      *out_err = reinterpret_cast< char* >( sqlite3_malloc( static_cast< int >( strlen( e.what() ) ) + 1 ) );
      strcpy( *out_err, e.what() );
    }
    return SQLITE_ERROR;
  }

  return vtable_create_connect( sql, aux, argc, argv, out_vtab, out_err, /* is_created */ true );
}

int vtable_connect( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **out_vtab, char** out_err )
{
  return vtable_create_connect( sql, aux, argc, argv, out_vtab, out_err, /* is_created */ false );
}

int vtable_destroy( sqlite3_vtab *vtab )
{
  if ( vtab )
  {
    delete reinterpret_cast<VTable*>( vtab );
  }
  return SQLITE_OK;
}

int vtable_disconnect( sqlite3_vtab *vtab )
{
  if ( vtab )
  {
    delete reinterpret_cast<VTable*>( vtab );
  }
  return SQLITE_OK;
}

int vtable_rename( sqlite3_vtab *vtab, const char *new_name )
{
  Q_UNUSED( vtab );
  Q_UNUSED( new_name );

  return SQLITE_OK;
}

int vtable_bestindex( sqlite3_vtab *pvtab, sqlite3_index_info* index_info )
{
  VTable *vtab = reinterpret_cast< VTable* >( pvtab );
  for ( int i = 0; i < index_info->nConstraint; i++ )
  {
    if (( index_info->aConstraint[i].usable ) &&
        ( vtab->pkColumn() == index_info->aConstraint[i].iColumn ) &&
        ( index_info->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_EQ ) )
    {
      // request for primary key filter
      index_info->aConstraintUsage[i].argvIndex = 1;
      index_info->aConstraintUsage[i].omit = 1;
      index_info->idxNum = 1; // PK filter
      index_info->estimatedCost = 1.0; // ??
      //index_info->estimatedRows = 1;
      index_info->idxStr = nullptr;
      index_info->needToFreeIdxStr = 0;
      return SQLITE_OK;
    }
    if (( index_info->aConstraint[i].usable ) &&
        ( 0 == index_info->aConstraint[i].iColumn ) &&
        ( index_info->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_EQ ) )
    {
      // request for rtree filtering
      index_info->aConstraintUsage[i].argvIndex = 1;
      // do not test for equality, since it is used for filtering, not to return an actual value
      index_info->aConstraintUsage[i].omit = 1;
      index_info->idxNum = 2; // RTree filter
      index_info->estimatedCost = 1.0; // ??
      //index_info->estimatedRows = 1;
      index_info->idxStr = nullptr;
      index_info->needToFreeIdxStr = 0;
      return SQLITE_OK;
    }
  }
  index_info->idxNum = 0;
  index_info->estimatedCost = 10.0;
  //index_info->estimatedRows = 10;
  index_info->idxStr = nullptr;
  index_info->needToFreeIdxStr = 0;
  return SQLITE_OK;
}

int vtable_open( sqlite3_vtab *vtab, sqlite3_vtab_cursor **out_cursor )
{
  VTableCursor *ncursor = new VTableCursor( reinterpret_cast< VTable* >( vtab ) );
  *out_cursor = reinterpret_cast< sqlite3_vtab_cursor* >( ncursor );
  return SQLITE_OK;
}

int vtable_close( sqlite3_vtab_cursor * cursor )
{
  if ( cursor )
  {
    delete reinterpret_cast<VTableCursor*>( cursor );
  }
  return SQLITE_OK;
}

int vtable_filter( sqlite3_vtab_cursor * cursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv )
{
  Q_UNUSED( argc );
  Q_UNUSED( idxStr );

  QgsFeatureRequest request;
  if ( idxNum == 1 )
  {
    // id filter
    request.setFilterFid( sqlite3_value_int( argv[0] ) );
  }
  else if ( idxNum == 2 )
  {
    // rtree filter
    const char* blob = reinterpret_cast< const char* >( sqlite3_value_blob( argv[0] ) );
    int bytes = sqlite3_value_bytes( argv[0] );
    QgsRectangle r( spatialiteBlobBbox( blob, bytes ) );
    request.setFilterRect( r );
  }
  VTableCursor *c = reinterpret_cast<VTableCursor*>( cursor );
  c->filter( request );
  return SQLITE_OK;
}

int vtable_next( sqlite3_vtab_cursor *cursor )
{
  VTableCursor* c = reinterpret_cast<VTableCursor*>( cursor );
  c->next();
  return SQLITE_OK;
}

int vtable_eof( sqlite3_vtab_cursor *cursor )
{
  VTableCursor* c = reinterpret_cast<VTableCursor*>( cursor );
  return c->eof();
}

int vtable_rowid( sqlite3_vtab_cursor *cursor, sqlite3_int64 *out_rowid )
{
  VTableCursor* c = reinterpret_cast<VTableCursor*>( cursor );
  *out_rowid = c->currentId();

  return SQLITE_OK;
}

int vtable_column( sqlite3_vtab_cursor *cursor, sqlite3_context* ctxt, int idx )
{
  VTableCursor* c = reinterpret_cast<VTableCursor*>( cursor );
  if ( idx == 0 )
  {
    // _search_frame_, return null
    sqlite3_result_null( ctxt );
    return SQLITE_OK;
  }
  if ( idx == c->nColumns() + 1 )
  {
    QPair<char*, int> g = c->currentGeometry();
    if ( !g.first )
      sqlite3_result_null( ctxt );
    else
      sqlite3_result_blob( ctxt, g.first, g.second, deleteGeometryBlob );
    return SQLITE_OK;
  }
  QVariant v = c->currentAttribute( idx - 1 );
  if ( v.isNull() )
  {
    sqlite3_result_null( ctxt );
  }
  else
  {
    switch ( v.type() )
    {
      case QVariant::Int:
      case QVariant::UInt:
        sqlite3_result_int( ctxt, v.toInt() );
        break;
      case QVariant::LongLong:
        sqlite3_result_int64( ctxt, v.toLongLong() );
        break;
      case QVariant::Double:
        sqlite3_result_double( ctxt, v.toDouble() );
        break;
      default:
      {
        sqlite3_result_text( ctxt, v.toString().toUtf8(), -1, SQLITE_TRANSIENT );
      }
      break;
    }
  }
  return SQLITE_OK;
}

int vtable_findfunction( sqlite3_vtab *pVtab,
                         int nArg,
                         const char *zName,
                         void ( **pxFunc )( sqlite3_context*, int, sqlite3_value** ),
                         void **ppArg )
{
  Q_UNUSED( pVtab );
  Q_UNUSED( nArg );
  Q_UNUSED( zName );
  Q_UNUSED( pxFunc );
  Q_UNUSED( ppArg );
  return SQLITE_OK;
}



sqlite3_module module;

static QCoreApplication* core_app = nullptr;

static int module_argc = 1;
static char module_name[] = "qgsvlayer_module";
static char* module_argv[] = { module_name };

void module_destroy( void * d )
{
  delete reinterpret_cast<ModuleContext*>( d );

  if ( core_app )
  {
    delete core_app;
  }
}

int qgsvlayer_module_init( sqlite3 *db, char **pzErrMsg, void * unused /*const sqlite3_api_routines *pApi*/ )
{
  Q_UNUSED( pzErrMsg );
  Q_UNUSED( unused );

  int rc = SQLITE_OK;

  // check if qgis providers are loaded
  if ( !QCoreApplication::instance() )
  {
    // if run standalone
    core_app = new QCoreApplication( module_argc, module_argv );
    QgsApplication::init();
    QgsApplication::initQgis();
  }

  module.xCreate = vtable_create;
  module.xConnect = vtable_connect;
  module.xBestIndex = vtable_bestindex;
  module.xDisconnect = vtable_disconnect;
  module.xDestroy = vtable_destroy;
  module.xOpen = vtable_open;
  module.xClose = vtable_close;
  module.xFilter = vtable_filter;
  module.xNext = vtable_next;
  module.xEof = vtable_eof;
  module.xColumn = vtable_column;
  module.xRowid = vtable_rowid;
  module.xRename = vtable_rename;

  module.xUpdate = nullptr;
  module.xBegin = nullptr;
  module.xSync = nullptr;
  module.xCommit = nullptr;
  module.xRollback = nullptr;
  module.xFindFunction = nullptr;
  module.xSavepoint = nullptr;
  module.xRelease = nullptr;
  module.xRollbackTo = nullptr;

  ModuleContext* context = new ModuleContext;
  sqlite3_create_module_v2( db, "QgsVLayer", &module, context, module_destroy );

  return rc;
}
