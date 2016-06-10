/***************************************************************************
    qgsspatialitefeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsspatialitefeatureiterator.h"

#include "qgsspatialiteconnection.h"
#include "qgsspatialiteconnpool.h"
#include "qgsspatialiteprovider.h"
#include "qgssqliteexpressioncompiler.h"

#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include <QSettings>

QgsSpatiaLiteFeatureIterator::QgsSpatiaLiteFeatureIterator( QgsSpatiaLiteFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsSpatiaLiteFeatureSource>( source, ownSource, request )
    , sqliteStatement( nullptr )
    , mExpressionCompiled( false )
{

  mHandle = QgsSpatiaLiteConnPool::instance()->acquireConnection( mSource->mSqlitePath );

  mFetchGeometry = !mSource->mGeometryColumn.isNull() && !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  mHasPrimaryKey = !mSource->mPrimaryKey.isEmpty();
  mRowNumber = 0;

  QStringList whereClauses;
  bool useFallbackWhereClause = false;
  QString fallbackWhereClause;
  QString whereClause;

  //beware - limitAtProvider needs to be set to false if the request cannot be completely handled
  //by the provider (eg utilising QGIS expression filters)
  bool limitAtProvider = ( mRequest.limit() >= 0 );

  if ( !request.filterRect().isNull() && !mSource->mGeometryColumn.isNull() )
  {
    // some kind of MBR spatial filtering is required
    whereClause = whereClauseRect();
    if ( ! whereClause.isEmpty() )
    {
      whereClauses.append( whereClause );
    }
  }

  if ( !mSource->mSubsetString.isEmpty() )
  {
    whereClause = "( " + mSource->mSubsetString + ')';
    if ( ! whereClause.isEmpty() )
    {
      whereClauses.append( whereClause );
    }
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    whereClause = whereClauseFid();
    if ( ! whereClause.isEmpty() )
    {
      whereClauses.append( whereClause );
    }
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFids )
  {
    whereClause = whereClauseFids();
    if ( ! whereClause.isEmpty() )
    {
      whereClauses.append( whereClause );
    }
  }
  //IMPORTANT - this MUST be the last clause added!
  else if ( request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    // ensure that all attributes required for expression filter are being fetched
    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && request.filterType() == QgsFeatureRequest::FilterExpression )
    {
      QgsAttributeList attrs = request.subsetOfAttributes();
      Q_FOREACH ( const QString& field, request.filterExpression()->referencedColumns() )
      {
        int attrIdx = mSource->mFields.fieldNameIndex( field );
        if ( !attrs.contains( attrIdx ) )
          attrs << attrIdx;
      }
      mRequest.setSubsetOfAttributes( attrs );
    }
    if ( request.filterExpression()->needsGeometry() )
    {
      mFetchGeometry = true;
    }

    if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
    {
      QgsSQLiteExpressionCompiler compiler = QgsSQLiteExpressionCompiler( source->mFields );

      QgsSqlExpressionCompiler::Result result = compiler.compile( request.filterExpression() );

      if ( result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial )
      {
        whereClause = compiler.result();
        if ( !whereClause.isEmpty() )
        {
          useFallbackWhereClause = true;
          fallbackWhereClause = whereClauses.join( " AND " );
          whereClauses.append( whereClause );
          //if only partial success when compiling expression, we need to double-check results using QGIS' expressions
          mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
          mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
        }
      }
      if ( result != QgsSqlExpressionCompiler::Complete )
      {
        //can't apply limit at provider side as we need to check all results using QGIS expressions
        limitAtProvider = false;
      }
    }
    else
    {
      limitAtProvider = false;
    }
  }


  whereClause = whereClauses.join( " AND " );

  // Setup the order by
  QStringList orderByParts;

  mOrderByCompiled = true;

  if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
  {
    Q_FOREACH ( const QgsFeatureRequest::OrderByClause& clause, request.orderBy() )
    {
      QgsSQLiteExpressionCompiler compiler = QgsSQLiteExpressionCompiler( source->mFields );
      QgsExpression expression = clause.expression();
      if ( compiler.compile( &expression ) == QgsSqlExpressionCompiler::Complete )
      {
        QString part;
        part = compiler.result();

        if ( clause.nullsFirst() )
          orderByParts << QString( "%1 IS NOT NULL" ).arg( part );
        else
          orderByParts << QString( "%1 IS NULL" ).arg( part );

        part += clause.ascending() ? " COLLATE NOCASE ASC" : " COLLATE NOCASE DESC";
        orderByParts << part;
      }
      else
      {
        // Bail out on first non-complete compilation.
        // Most important clauses at the beginning of the list
        // will still be sent and used to pre-sort so the local
        // CPU can use its cycles for fine-tuning.
        mOrderByCompiled = false;
        break;
      }
    }
  }
  else
  {
    mOrderByCompiled = false;
  }

  if ( !mOrderByCompiled )
    limitAtProvider = false;

  // preparing the SQL statement
  bool success = prepareStatement( whereClause, limitAtProvider ? mRequest.limit() : -1, orderByParts.join( "," ) );
  if ( !success && useFallbackWhereClause )
  {
    //try with the fallback where clause, eg for cases when using compiled expression failed to prepare
    mExpressionCompiled = false;
    success = prepareStatement( fallbackWhereClause, -1, orderByParts.join( "," ) );
  }

  if ( !success )
  {
    // some error occurred
    sqliteStatement = nullptr;
    close();
  }
}

QgsSpatiaLiteFeatureIterator::~QgsSpatiaLiteFeatureIterator()
{
  close();
}


bool QgsSpatiaLiteFeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( !sqliteStatement )
  {
    QgsDebugMsg( "Invalid current SQLite statement" );
    close();
    return false;
  }

  if ( !getFeature( sqliteStatement, feature ) )
  {
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = nullptr;
    close();
    return false;
  }

  feature.setValid( true );
  return true;
}

bool QgsSpatiaLiteFeatureIterator::nextFeatureFilterExpression( QgsFeature& f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}


bool QgsSpatiaLiteFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( sqlite3_reset( sqliteStatement ) == SQLITE_OK )
  {
    mRowNumber = 0;
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsSpatiaLiteFeatureIterator::close()
{
  if ( mClosed )
    return false;

  iteratorClosed();

  if ( !mHandle )
  {
    mClosed = true;
    return false;
  }

  if ( sqliteStatement )
  {
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = nullptr;
  }

  QgsSpatiaLiteConnPool::instance()->releaseConnection( mHandle );
  mHandle = nullptr;

  mClosed = true;
  return true;
}

////


bool QgsSpatiaLiteFeatureIterator::prepareStatement( const QString& whereClause, long limit, const QString& orderBy )
{
  if ( !mHandle )
    return false;

  try
  {
    QString sql = QString( "SELECT %1" ).arg( mHasPrimaryKey ? quotedPrimaryKey() : "0" );
    int colIdx = 1; // column 0 is primary key

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      QgsAttributeList fetchAttributes = mRequest.subsetOfAttributes();
      for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it )
      {
        sql += ',' + fieldName( mSource->mFields.field( *it ) );
        colIdx++;
      }
    }
    else
    {
      // fetch all attributes
      for ( int idx = 0; idx < mSource->mFields.count(); ++idx )
      {
        sql += ',' + fieldName( mSource->mFields.at( idx ) );
        colIdx++;
      }
    }

    if ( mFetchGeometry )
    {
      sql += QString( ", AsBinary(%1)" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ) );
      mGeomColIdx = colIdx;
    }
    sql += QString( " FROM %1" ).arg( mSource->mQuery );

    if ( !whereClause.isEmpty() )
      sql += QString( " WHERE %1" ).arg( whereClause );

    if ( !orderBy.isEmpty() )
      sql += QString( " ORDER BY %1" ).arg( orderBy );

    if ( limit >= 0 )
      sql += QString( " LIMIT %1" ).arg( limit );

    if ( sqlite3_prepare_v2( mHandle->handle(), sql.toUtf8().constData(), -1, &sqliteStatement, nullptr ) != SQLITE_OK )
    {
      // some error occurred
      QgsMessageLog::logMessage( QObject::tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mHandle->handle() ) ), QObject::tr( "SpatiaLite" ) );
      return false;
    }
  }
  catch ( QgsSpatiaLiteProvider::SLFieldNotFound )
  {
    rewind();
    return false;
  }

  return true;
}

QString QgsSpatiaLiteFeatureIterator::quotedPrimaryKey()
{
  return mSource->mPrimaryKey.isEmpty() ? "ROWID" : QgsSpatiaLiteProvider::quotedIdentifier( mSource->mPrimaryKey );
}

QString QgsSpatiaLiteFeatureIterator::whereClauseFid()
{
  return QString( "%1=%2" ).arg( quotedPrimaryKey() ).arg( mRequest.filterFid() );
}

QString QgsSpatiaLiteFeatureIterator::whereClauseFids()
{
  if ( mRequest.filterFids().isEmpty() )
    return "";

  QString expr = QString( "%1 IN (" ).arg( quotedPrimaryKey() ), delim;
  Q_FOREACH ( const QgsFeatureId featureId, mRequest.filterFids() )
  {
    expr += delim + QString::number( featureId );
    delim = ',';
  }
  expr += ')';
  return expr;
}

QString QgsSpatiaLiteFeatureIterator::whereClauseRect()
{
  QgsRectangle rect = mRequest.filterRect();
  QString whereClause;

  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    // we are requested to evaluate a true INTERSECT relationship
    whereClause += QString( "Intersects(%1, BuildMbr(%2)) AND " ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ), mbr( rect ) );
  }
  if ( mSource->mVShapeBased )
  {
    // handling a VirtualShape layer
    whereClause += QString( "MbrIntersects(%1, BuildMbr(%2))" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ), mbr( rect ) );
  }
  else if ( rect.isFinite() )
  {
    if ( mSource->mSpatialIndexRTree )
    {
      // using the RTree spatial index
      QString mbrFilter = QString( "xmin <= %1 AND " ).arg( qgsDoubleToString( rect.xMaximum() ) );
      mbrFilter += QString( "xmax >= %1 AND " ).arg( qgsDoubleToString( rect.xMinimum() ) );
      mbrFilter += QString( "ymin <= %1 AND " ).arg( qgsDoubleToString( rect.yMaximum() ) );
      mbrFilter += QString( "ymax >= %1" ).arg( qgsDoubleToString( rect.yMinimum() ) );
      QString idxName = QString( "idx_%1_%2" ).arg( mSource->mIndexTable, mSource->mIndexGeometry );
      whereClause += QString( "%1 IN (SELECT pkid FROM %2 WHERE %3)" )
                     .arg( quotedPrimaryKey(),
                           QgsSpatiaLiteProvider::quotedIdentifier( idxName ),
                           mbrFilter );
    }
    else if ( mSource->mSpatialIndexMbrCache )
    {
      // using the MbrCache spatial index
      QString idxName = QString( "cache_%1_%2" ).arg( mSource->mIndexTable, mSource->mIndexGeometry );
      whereClause += QString( "%1 IN (SELECT rowid FROM %2 WHERE mbr = FilterMbrIntersects(%3))" )
                     .arg( quotedPrimaryKey(),
                           QgsSpatiaLiteProvider::quotedIdentifier( idxName ),
                           mbr( rect ) );
    }
    else
    {
      // using simple MBR filtering
      whereClause += QString( "MbrIntersects(%1, BuildMbr(%2))" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ), mbr( rect ) );
    }
  }
  else
  {
    whereClause = '1';
  }
  return whereClause;
}


QString QgsSpatiaLiteFeatureIterator::mbr( const QgsRectangle& rect )
{
  return QString( "%1, %2, %3, %4" )
         .arg( qgsDoubleToString( rect.xMinimum() ),
               qgsDoubleToString( rect.yMinimum() ),
               qgsDoubleToString( rect.xMaximum() ),
               qgsDoubleToString( rect.yMaximum() ) );
}


QString QgsSpatiaLiteFeatureIterator::fieldName( const QgsField& fld )
{
  QString fieldname = QgsSpatiaLiteProvider::quotedIdentifier( fld.name() );
  const QString type = fld.typeName().toLower();
  if ( type.contains( "geometry" ) || type.contains( "point" ) ||
       type.contains( "line" ) || type.contains( "polygon" ) )
  {
    fieldname = QString( "AsText(%1)" ).arg( fieldname );
  }
  return fieldname;
}


bool QgsSpatiaLiteFeatureIterator::getFeature( sqlite3_stmt *stmt, QgsFeature &feature )
{
  bool subsetAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;

  int ret = sqlite3_step( stmt );
  if ( ret == SQLITE_DONE )
  {
    // there are no more rows to fetch
    return false;
  }
  if ( ret != SQLITE_ROW )
  {
    // some unexpected error occurred
    QgsMessageLog::logMessage( QObject::tr( "SQLite error getting feature: %1" ).arg( QString::fromUtf8( sqlite3_errmsg( mHandle->handle() ) ) ), QObject::tr( "SpatiaLite" ) );
    return false;
  }

  // one valid row has been fetched from the result set
  if ( !mFetchGeometry )
  {
    // no geometry was required
    feature.setGeometry( nullptr );
  }

  feature.initAttributes( mSource->mFields.count() );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  int ic;
  int n_columns = sqlite3_column_count( stmt );
  for ( ic = 0; ic < n_columns; ic++ )
  {
    if ( ic == 0 )
    {
      if ( mHasPrimaryKey )
      {
        // first column always contains the ROWID (or the primary key)
        QgsFeatureId fid = sqlite3_column_int64( stmt, ic );
        QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 3 );
        feature.setFeatureId( fid );
      }
      else
      {
        // autoincrement a row number
        mRowNumber++;
        feature.setFeatureId( mRowNumber );
      }
    }
    else if ( mFetchGeometry && ic == mGeomColIdx )
    {
      getFeatureGeometry( stmt, ic, feature );
    }
    else
    {
      if ( subsetAttributes )
      {
        if ( ic <= mRequest.subsetOfAttributes().size() )
        {
          int attrIndex = mRequest.subsetOfAttributes()[ic-1];
          feature.setAttribute( attrIndex, getFeatureAttribute( stmt, ic, mSource->mFields.at( attrIndex ).type() ) );
        }
      }
      else
      {
        int attrIndex = ic - 1;
        feature.setAttribute( attrIndex, getFeatureAttribute( stmt, ic, mSource->mFields.at( attrIndex ).type() ) );
      }
    }
  }

  return true;
}

QVariant QgsSpatiaLiteFeatureIterator::getFeatureAttribute( sqlite3_stmt* stmt, int ic, QVariant::Type type )
{
  if ( sqlite3_column_type( stmt, ic ) == SQLITE_INTEGER )
  {
    if ( type == QVariant::Int )
    {
      // INTEGER value
      return sqlite3_column_int( stmt, ic );
    }
    else
    {
      // INTEGER value
      return ( qint64 ) sqlite3_column_int64( stmt, ic );
    }
  }

  if ( sqlite3_column_type( stmt, ic ) == SQLITE_FLOAT )
  {
    // DOUBLE value
    return sqlite3_column_double( stmt, ic );
  }

  if ( sqlite3_column_type( stmt, ic ) == SQLITE_TEXT )
  {
    // TEXT value
    const char *txt = ( const char * ) sqlite3_column_text( stmt, ic );
    return QString::fromUtf8( txt );
  }

  // assuming NULL
  return QVariant( type );
}

void QgsSpatiaLiteFeatureIterator::getFeatureGeometry( sqlite3_stmt* stmt, int ic, QgsFeature& feature )
{
  if ( sqlite3_column_type( stmt, ic ) == SQLITE_BLOB )
  {
    unsigned char *featureGeom = nullptr;
    int geom_size = 0;
    const void *blob = sqlite3_column_blob( stmt, ic );
    int blob_size = sqlite3_column_bytes( stmt, ic );
    QgsSpatiaLiteProvider::convertToGeosWKB(( const unsigned char * )blob, blob_size, &featureGeom, &geom_size );
    if ( featureGeom )
    {
      QgsGeometry *g = new QgsGeometry();
      g->fromWkb( featureGeom, geom_size );
      feature.setGeometry( g );
    }
    else
      feature.setGeometry( nullptr );
  }
  else
  {
    // NULL geometry
    feature.setGeometry( nullptr );
  }
}

bool QgsSpatiaLiteFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}


QgsSpatiaLiteFeatureSource::QgsSpatiaLiteFeatureSource( const QgsSpatiaLiteProvider* p )
    : mGeometryColumn( p->mGeometryColumn )
    , mSubsetString( p->mSubsetString )
    , mFields( p->mAttributeFields )
    , mQuery( p->mQuery )
    , mIsQuery( p->mIsQuery )
    , mViewBased( p->mViewBased )
    , mVShapeBased( p->mVShapeBased )
    , mIndexTable( p->mIndexTable )
    , mIndexGeometry( p->mIndexGeometry )
    , mPrimaryKey( p->mPrimaryKey )
    , mSpatialIndexRTree( p->mSpatialIndexRTree )
    , mSpatialIndexMbrCache( p->mSpatialIndexMbrCache )
    , mSqlitePath( p->mSqlitePath )
{
}

QgsSpatiaLiteFeatureSource::~QgsSpatiaLiteFeatureSource()
{
}

QgsFeatureIterator QgsSpatiaLiteFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsSpatiaLiteFeatureIterator( this, false, request ) );
}
