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
#include <QBuffer>

#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmaplayerregistry.h>
#include <qgsproviderregistry.h>
#include "qgsinterval.h"
#include <sqlite3.h>
#include <spatialite.h>
#include <stdio.h>
#include "qgsvirtuallayersqlitemodule.h"
#include "qgsvirtuallayerblob.h"
#include "qgsslottofunction.h"

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
      , mCrs( -1 )
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
      , mCrs( -1 )
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

  QgsFields fields() const { return mFields; }

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

  QgsFields mFields;

  void init_()
  {
    mFields = mLayer ? mLayer->fields() : mProvider->fields();
    QStringList sqlFields;

    // add a hidden field for rtree filtering
    sqlFields << "_search_frame_ HIDDEN BLOB";

    Q_FOREACH ( const QgsField& field, mFields )
    {
      QString typeName = "text";
      switch ( field.type() )
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
      sqlFields << field.name() + " " + typeName;
    }

    QgsVectorDataProvider* provider = mLayer ? mLayer->dataProvider() : mProvider;
    if ( provider->geometryType() != QGis::WKBNoGeometry )
    {
      // we have here a convenient hack
      // the type of a column can be declared with two numeric arguments, usually for setting numeric precision
      // we are using them to set the geometry type and srid
      // these will be reused by the provider when it will introspect the query to detect types
      sqlFields << QString( "geometry geometry(%1,%2)" ).arg( provider->geometryType() ).arg( provider->crs().postgisSrid() );
    }

    QgsAttributeList pkAttributeIndexes = provider->pkAttributeIndexes();
    if ( pkAttributeIndexes.size() == 1 )
    {
      mPkColumn = pkAttributeIndexes.at( 0 ) + 1;
    }

    mCreationStr = "CREATE TABLE vtable (" + sqlFields.join( "," ) + ")";

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

  explicit VTableCursor( VTable *vtab )
      : mVtab( vtab )
      , mEof( true )
  {}

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

int vtableCreateConnect( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **outVtab, char** outErr, bool isCreated )
{
  Q_UNUSED( aux );
  Q_UNUSED( isCreated );

#define RETURN_CSTR_ERROR(err) if (outErr) {size_t s = strlen(err); *outErr=reinterpret_cast<char*>(sqlite3_malloc( static_cast<int>( s ) +1)); strncpy(*outErr, err, s);}
#define RETURN_CPPSTR_ERROR(err) if (outErr) {*outErr=reinterpret_cast<char*>(sqlite3_malloc( static_cast<int>( err.size() )+1)); strncpy(*outErr, err.c_str(), err.size());}

  if ( argc < 4 )
  {
    std::string err( "Missing arguments: layer_id | provider, source" );
    RETURN_CPPSTR_ERROR( err );
    return SQLITE_ERROR;
  }

  QScopedPointer<VTable> newVtab;

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
      if ( outErr )
      {
        std::string err( "Cannot find layer " );
        err += argv[3];
        RETURN_CPPSTR_ERROR( err );
      }
      return SQLITE_ERROR;
    }
    newVtab.reset( new VTable( sql, static_cast<QgsVectorLayer*>( l ) ) );

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
      newVtab.reset( new VTable( sql, provider, source, argv[2], encoding ) );
    }
    catch ( std::runtime_error& e )
    {
      std::string err( e.what() );
      RETURN_CPPSTR_ERROR( err );
      return SQLITE_ERROR;
    }
  }

  r = sqlite3_declare_vtab( sql, newVtab->creationString().toLocal8Bit().constData() );
  if ( r )
  {
    RETURN_CSTR_ERROR( sqlite3_errmsg( sql ) );
    return r;
  }

  *outVtab = reinterpret_cast< sqlite3_vtab* >( newVtab.take() );
  return SQLITE_OK;
#undef RETURN_CSTR_ERROR
#undef RETURN_CPPSTR_ERROR
}

void dbInit( sqlite3* db )
{
  // create metadata tables
  initVirtualLayerMetadata( db );
}

int vtableCreate( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **outVtab, char** outErr )
{
  try
  {
    dbInit( sql );
  }
  catch ( std::runtime_error& e )
  {
    if ( outErr )
    {
      *outErr = reinterpret_cast< char* >( sqlite3_malloc( static_cast< int >( strlen( e.what() ) ) + 1 ) );
      strcpy( *outErr, e.what() );
    }
    return SQLITE_ERROR;
  }

  return vtableCreateConnect( sql, aux, argc, argv, outVtab, outErr, /* is_created */ true );
}

int vtableConnect( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **outVtab, char** outErr )
{
  return vtableCreateConnect( sql, aux, argc, argv, outVtab, outErr, /* is_created */ false );
}

int vtableDestroy( sqlite3_vtab *vtab )
{
  if ( vtab )
  {
    delete reinterpret_cast<VTable*>( vtab );
  }
  return SQLITE_OK;
}

int vtableDisconnect( sqlite3_vtab *vtab )
{
  if ( vtab )
  {
    delete reinterpret_cast<VTable*>( vtab );
  }
  return SQLITE_OK;
}

int vtableRename( sqlite3_vtab *vtab, const char *newName )
{
  Q_UNUSED( vtab );
  Q_UNUSED( newName );

  return SQLITE_OK;
}

int vtableBestIndex( sqlite3_vtab *pvtab, sqlite3_index_info* indexInfo )
{
  VTable *vtab = reinterpret_cast< VTable* >( pvtab );
  for ( int i = 0; i < indexInfo->nConstraint; i++ )
  {
    // request for primary key filter with '='
    if (( indexInfo->aConstraint[i].usable ) &&
        ( vtab->pkColumn() == indexInfo->aConstraint[i].iColumn ) &&
        ( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_EQ ) )
    {
      indexInfo->aConstraintUsage[i].argvIndex = 1;
      indexInfo->aConstraintUsage[i].omit = 1;
      indexInfo->idxNum = 1; // PK filter
      indexInfo->estimatedCost = 1.0;
      indexInfo->idxStr = nullptr;
      indexInfo->needToFreeIdxStr = 0;
      return SQLITE_OK;
    }

    // request for filter with a comparison operator
    if (( indexInfo->aConstraint[i].usable ) &&
        ( indexInfo->aConstraint[i].iColumn > 0 ) &&
        ( indexInfo->aConstraint[i].iColumn <= vtab->fields().count() ) &&
        (( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_EQ ) || // if no PK
         ( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_GT ) ||
         ( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_LE ) ||
         ( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_LT ) ||
         ( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_GE )
#ifdef SQLITE_INDEX_CONSTRAINT_LIKE
         || ( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_LIKE )
#endif
        ) )
    {
      indexInfo->aConstraintUsage[i].argvIndex = 1;
      indexInfo->aConstraintUsage[i].omit = 1;
      indexInfo->idxNum = 3; // expression filter
      indexInfo->estimatedCost = 2.0; // probably better than no index

      QString expr = vtab->fields().at( indexInfo->aConstraint[i].iColumn - 1 ).name();
      switch ( indexInfo->aConstraint[i].op )
      {
        case SQLITE_INDEX_CONSTRAINT_EQ:
          expr += " = ";
          break;
        case SQLITE_INDEX_CONSTRAINT_GT:
          expr += " > ";
          break;
        case SQLITE_INDEX_CONSTRAINT_LE:
          expr += " <= ";
          break;
        case SQLITE_INDEX_CONSTRAINT_LT:
          expr += " < ";
          break;
        case SQLITE_INDEX_CONSTRAINT_GE:
          expr += " >= ";
          break;
#ifdef SQLITE_INDEX_CONSTRAINT_LIKE
        case SQLITE_INDEX_CONSTRAINT_LIKE:
          expr += " LIKE ";
          break;
#endif
        default:
          break;
      }

      QByteArray ba = expr.toUtf8();
      char* cp = ( char* )sqlite3_malloc( ba.size() + 1 );
      memcpy( cp, ba.constData(), ba.size() + 1 );

      indexInfo->idxStr = cp;
      indexInfo->needToFreeIdxStr = 1;
      return SQLITE_OK;
    }

    // request for rtree filtering
    if (( indexInfo->aConstraint[i].usable ) &&
        ( 0 == indexInfo->aConstraint[i].iColumn ) &&
        ( indexInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_EQ ) )
    {
      indexInfo->aConstraintUsage[i].argvIndex = 1;
      // do not test for equality, since it is used for filtering, not to return an actual value
      indexInfo->aConstraintUsage[i].omit = 1;
      indexInfo->idxNum = 2; // RTree filter
      indexInfo->estimatedCost = 1.0;
      indexInfo->idxStr = nullptr;
      indexInfo->needToFreeIdxStr = 0;
      return SQLITE_OK;
    }
  }
  indexInfo->idxNum = 0;
  indexInfo->estimatedCost = 10.0;
  indexInfo->idxStr = nullptr;
  indexInfo->needToFreeIdxStr = 0;
  return SQLITE_OK;
}

int vtableOpen( sqlite3_vtab *vtab, sqlite3_vtab_cursor **outCursor )
{
  VTableCursor *ncursor = new VTableCursor( reinterpret_cast< VTable* >( vtab ) );
  *outCursor = reinterpret_cast< sqlite3_vtab_cursor* >( ncursor );
  return SQLITE_OK;
}

int vtableClose( sqlite3_vtab_cursor * cursor )
{
  if ( cursor )
  {
    delete reinterpret_cast<VTableCursor*>( cursor );
  }
  return SQLITE_OK;
}

int vtableFilter( sqlite3_vtab_cursor * cursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv )
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
  else if ( idxNum == 3 )
  {
    // comparison operator filter
    // build an expression filter and rely on expression compiler if available
    QString expr = idxStr;
    switch ( sqlite3_value_type( argv[0] ) )
    {
      case SQLITE_INTEGER:
        expr += QString::number( sqlite3_value_int64( argv[0] ) );
        break;
      case SQLITE_FLOAT:
        expr += QString::number( sqlite3_value_double( argv[0] ) );
        break;
      case SQLITE_TEXT:
      {
        int n = sqlite3_value_bytes( argv[0] );
        const char* t = reinterpret_cast<const char*>( sqlite3_value_text( argv[0] ) );
        QString str = QString::fromUtf8( t, n );
        expr += "'" + str.replace( "'", "''" ) + "'";
        break;
      }
      default:
        expr = "";
    }
    if ( !expr.isEmpty() )
    {
      request.setFilterExpression( expr );
    }
  }
  VTableCursor *c = reinterpret_cast<VTableCursor*>( cursor );
  c->filter( request );
  return SQLITE_OK;
}

int vtableNext( sqlite3_vtab_cursor *cursor )
{
  VTableCursor* c = reinterpret_cast<VTableCursor*>( cursor );
  c->next();
  return SQLITE_OK;
}

int vtableEof( sqlite3_vtab_cursor *cursor )
{
  VTableCursor* c = reinterpret_cast<VTableCursor*>( cursor );
  return c->eof();
}

int vtableRowId( sqlite3_vtab_cursor *cursor, sqlite3_int64 *outRowid )
{
  VTableCursor* c = reinterpret_cast<VTableCursor*>( cursor );
  *outRowid = c->currentId();

  return SQLITE_OK;
}

int vtableColumn( sqlite3_vtab_cursor *cursor, sqlite3_context* ctxt, int idx )
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


static QCoreApplication* coreApp = nullptr;

void moduleDestroy( void* )
{
  if ( coreApp )
  {
    delete coreApp;
  }
}

// the expression context used for calling qgis functions
QgsExpressionContext qgisFunctionExpressionContext;

void qgisFunctionWrapper( sqlite3_context* ctxt, int nArgs, sqlite3_value** args )
{
  // convert from sqlite3 value to QVariant and then call the qgis expression function
  // the 3 basic sqlite3 types (int, float, text) are converted to their QVariant equivalent
  // Expression::Interval is handled specifically
  // geometries are converted between spatialite and QgsGeometry
  // other data types (datetime mainly) are represented as BLOBs thanks to QVariant serializing functions

  QgsExpression::Function* foo = reinterpret_cast<QgsExpression::Function*>( sqlite3_user_data( ctxt ) );

  QVariantList variants;
  for ( int i = 0; i < nArgs; i++ )
  {
    int t = sqlite3_value_type( args[i] );
    switch ( t )
    {
      case SQLITE_INTEGER:
        variants << QVariant( sqlite3_value_int64( args[i] ) );
        break;
      case SQLITE_FLOAT:
        variants << QVariant( sqlite3_value_double( args[i] ) );
        break;
      case SQLITE_TEXT:
      {
        int n = sqlite3_value_bytes( args[i] );
        const char* t = reinterpret_cast<const char*>( sqlite3_value_text( args[i] ) );
        QString str = QString::fromUtf8( t, n );
        variants << QVariant( str );
        break;
      }
      case SQLITE_BLOB:
      {
        int n = sqlite3_value_bytes( args[i] );
        const char* blob = reinterpret_cast<const char*>( sqlite3_value_blob( args[i] ) );
        // spatialite blobs start with a 0 byte
        if ( n > 0 && blob[0] == 0 )
        {
          QgsGeometry geom = spatialiteBlobToQgsGeometry( blob, n );
          variants << QVariant::fromValue( geom );
        }
        else
        {
          // else it is another type
          QByteArray ba = QByteArray::fromRawData( blob + 1, n - 1 );
          QBuffer buffer( &ba );
          buffer.open( QIODevice::ReadOnly );
          QDataStream ds( &buffer );
          QVariant v;
          ds >> v;
          buffer.close();
          variants << v;
        }
        break;
      }
      default:
        variants << QVariant(); // null
        break;
    };
  }

  QgsExpression parentExpr( "" );
  QVariant ret = foo->func( variants, &qgisFunctionExpressionContext, &parentExpr );
  if ( parentExpr.hasEvalError() )
  {
    QByteArray ba = parentExpr.evalErrorString().toUtf8();
    sqlite3_result_error( ctxt, ba.constData(), ba.size() );
    return;
  }

  if ( ret.isNull() )
  {
    sqlite3_result_null( ctxt );
    return;
  }

  switch ( ret.type() )
  {
    case QVariant::Bool:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
      sqlite3_result_int64( ctxt, ret.toLongLong() );
      break;
    case QVariant::Double:
      sqlite3_result_double( ctxt, ret.toDouble() );
      break;
    case QVariant::String:
    {
      QByteArray ba( ret.toByteArray() );
      sqlite3_result_text( ctxt, ba.constData(), ba.size(), SQLITE_TRANSIENT );
      break;
    }
    case QVariant::UserType:
    {
      if ( ret.canConvert<QgsGeometry>() )
      {
        char* blob = nullptr;
        int size = 0;
        qgsGeometryToSpatialiteBlob( ret.value<QgsGeometry>(), /*srid*/0, blob, size );
        sqlite3_result_blob( ctxt, blob, size, deleteGeometryBlob );
      }
      else if ( ret.canConvert<QgsInterval>() )
      {
        sqlite3_result_double( ctxt, ret.value<QgsInterval>().seconds() );
      }
      break;
    }
    default:
    {
      QBuffer buffer;
      buffer.open( QBuffer::ReadWrite );
      QDataStream ds( &buffer );
      // something different from 0 (to distinguish from the first byte of a geometry blob)
      char type = 1;
      buffer.write( &type, 1 );
      // then the serialized version of the variant
      ds << ret;
      buffer.close();
      sqlite3_result_blob( ctxt, buffer.buffer().constData(), buffer.buffer().size(), SQLITE_TRANSIENT );
    }
  };
}

void registerQgisFunctions( sqlite3* db )
{
  QStringList excludedFunctions;
  excludedFunctions << "min" << "max" << "coalesce" << "get_feature" << "getFeature" << "attribute";
  QStringList reservedFunctions;
  reservedFunctions << "left" << "right" << "union";
  // register QGIS expression functions
  Q_FOREACH ( QgsExpression::Function* foo, QgsExpression::Functions() )
  {
    if ( foo->usesgeometry() || foo->lazyEval() )
    {
      // there is no "current" feature here, so calling functions that access "the" geometry does not make sense
      // also, we can't pass Node values for lazy evaluations
      continue;
    }
    if ( excludedFunctions.contains( foo->name() ) )
      continue;

    QStringList names;
    names << foo->name();
    names << foo->aliases();

    Q_FOREACH ( QString name, names ) // for each alias
    {
      if ( reservedFunctions.contains( name ) ) // reserved keyword
        name = "_" + name;
      if ( name.startsWith( "$" ) )
        continue;

      // register the function and pass the pointer to the Function* as user data
      int r = sqlite3_create_function( db, name.toUtf8().constData(), foo->params(), SQLITE_UTF8, foo, qgisFunctionWrapper, nullptr, nullptr );
      if ( r != SQLITE_OK )
      {
        // is it because a function of the same name already exist (in Spatialite for instance ?)
        // we then try to recreate it with a prefix
        name = "qgis_" + name;
        sqlite3_create_function( db, name.toUtf8().constData(), foo->params(), SQLITE_UTF8, foo, qgisFunctionWrapper, nullptr, nullptr );
      }
    }
  }

  // initialize the expression context
  qgisFunctionExpressionContext << QgsExpressionContextUtils::globalScope();
  qgisFunctionExpressionContext << QgsExpressionContextUtils::projectScope();
}

int qgsvlayerModuleInit( sqlite3 *db, char **pzErrMsg, void * unused /*const sqlite3_api_routines *pApi*/ )
{
  Q_UNUSED( pzErrMsg );
  Q_UNUSED( unused );

  int rc = SQLITE_OK;

  // check if qgis providers are loaded
  if ( !QCoreApplication::instance() )
  {
    // if run standalone
    static int moduleArgc = 1;
    static char moduleName[] = "qgsvlayer_module";
    static char* moduleArgv[] = { moduleName };
    coreApp = new QCoreApplication( moduleArgc, moduleArgv );
    QgsApplication::init();
    QgsApplication::initQgis();
  }

  static sqlite3_module module;
  module.xCreate = vtableCreate;
  module.xConnect = vtableConnect;
  module.xBestIndex = vtableBestIndex;
  module.xDisconnect = vtableDisconnect;
  module.xDestroy = vtableDestroy;
  module.xOpen = vtableOpen;
  module.xClose = vtableClose;
  module.xFilter = vtableFilter;
  module.xNext = vtableNext;
  module.xEof = vtableEof;
  module.xColumn = vtableColumn;
  module.xRowid = vtableRowId;
  module.xRename = vtableRename;

  module.xUpdate = nullptr;
  module.xBegin = nullptr;
  module.xSync = nullptr;
  module.xCommit = nullptr;
  module.xRollback = nullptr;
  module.xFindFunction = nullptr;
  module.xSavepoint = nullptr;
  module.xRelease = nullptr;
  module.xRollbackTo = nullptr;

  sqlite3_create_module_v2( db, "QgsVLayer", &module, nullptr, moduleDestroy );

  registerQgisFunctions( db );

  return rc;
}
