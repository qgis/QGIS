/***************************************************************************
  qgsdb2featureiterator.cpp - DB2 spatial feature processing
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2featureiterator.h"
#include "qgsdb2provider.h"
#include "qgsdb2expressioncompiler.h"
#include <qgslogger.h>
#include <qgsgeometry.h>

#include <QObject>
#include <QTextStream>
#include <QSqlRecord>


QgsDb2FeatureIterator::QgsDb2FeatureIterator( QgsDb2FeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsDb2FeatureSource>( source, ownSource, request )
{
  mClosed = false;
  mQuery = NULL;

  BuildStatement( request );

  // connect to the database
  bool convertIntOk;
  int portNum = mSource->mPort.toInt( &convertIntOk, 10 );
  mDatabase = QgsDb2Provider::GetDatabase( mSource->mService, mSource->mDriver, mSource->mHost, portNum, mSource->mDatabaseName, mSource->mUserName, mSource->mPassword );

  if ( !mDatabase.open() )
  {
    QgsDebugMsg( "Failed to open database" );
    QString msg = mDatabase.lastError().text();
    QgsDebugMsg( msg );
    return;
  }

  // create sql query
  mQuery = new QSqlQuery( mDatabase );

  // start selection
  rewind();
}


QgsDb2FeatureIterator::~QgsDb2FeatureIterator()
{
  close();
}

void QgsDb2FeatureIterator::BuildStatement( const QgsFeatureRequest& request )
{
  bool limitAtProvider = ( mRequest.limit() >= 0 );
// Note, schema, table and column names are not escaped
// Not sure if this is a problem with upper/lower case names
  // build sql statement
  mStatement = QString( "SELECT " );

  mStatement += QString( "%1" ).arg( mSource->mFidColName );
  mFidCol = mSource->mFields.indexFromName( mSource->mFidColName );
  mAttributesToFetch.append( mFidCol );

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  Q_FOREACH ( int i, subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList() )
  {
    QString fieldname = mSource->mFields.at( i ).name();
    if ( mSource->mFidColName == fieldname )
      continue;

    mStatement += QString( ",%1" ).arg( fieldname );

    mAttributesToFetch.append( i );
  }

  // get geometry col in WKB format
  if ( !( request.flags() & QgsFeatureRequest::NoGeometry ) && mSource->isSpatial() )
  {
    mStatement += QString( ",DB2GSE.ST_ASBINARY(%1) AS %1 " ).arg( mSource->mGeometryColName );

//  mStatement += QString( ",VARCHAR(DB2GSE.ST_ASTEXT(%1)) AS %1 " ).arg( mSource->mGeometryColName );
//    mStatement += QString( ",DB2GSE.ST_ASTEXT(%1) AS %1 " ).arg( mSource->mGeometryColName );

    mAttributesToFetch.append( 2 );
  }

  mStatement += QString( " FROM %1.%2" ).arg( mSource->mSchemaName, mSource->mTableName );

  bool filterAdded = false;
  // set spatial filter
  if ( !request.filterRect().isNull() && mSource->isSpatial() && !request.filterRect().isEmpty() )
  {
    mStatement += QString( " WHERE DB2GSE.ENVELOPESINTERSECT(%1, %2, %3, %4, %5, %6) = 1" ).arg(
                    mSource->mGeometryColName,
                    qgsDoubleToString( request.filterRect().xMinimum() ),
                    qgsDoubleToString( request.filterRect().yMinimum() ),
                    qgsDoubleToString( request.filterRect().xMaximum() ),
                    qgsDoubleToString( request.filterRect().yMaximum() ),
                    QString::number( mSource->mSRId ) );
    filterAdded = true;
  }

  // set fid filter
  if ( request.filterType() == QgsFeatureRequest::FilterFid && !mSource->mFidColName.isEmpty() )
  {
    QString fidfilter = QString( " %1 = %2" ).arg( mSource->mFidColName, QString::number( request.filterFid() ) );
    // set attribute filter
    if ( !filterAdded )
      mStatement += " WHERE ";
    else
      mStatement += " AND ";

    mStatement += fidfilter;
    filterAdded = true;
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFids && !mSource->mFidColName.isEmpty()
            && !mRequest.filterFids().isEmpty() )
  {
    QString delim;
    QString inClause = QString( "%1 IN (" ).arg( mSource->mFidColName );
    Q_FOREACH ( QgsFeatureId featureId, mRequest.filterFids() )
    {
      inClause += delim + FID_TO_STRING( featureId );
      delim = ',';
    }
    inClause.append( ')' );

    if ( !filterAdded )
      mStatement += " WHERE ";
    else
      mStatement += " AND ";

    mStatement += inClause;
    filterAdded = true;
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    if ( !filterAdded )
      mStatement += " WHERE (" + mSource->mSqlWhereClause + ")";
    else
      mStatement += " AND (" + mSource->mSqlWhereClause + ")";
  }

  mExpressionCompiled = false;
  if ( request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    QgsDebugMsg( QString( "compileExpressions: %1" ).arg( QSettings().value( "/qgis/compileExpressions", true ).toString() ) );
    if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
    {
      QgsDb2ExpressionCompiler compiler = QgsDb2ExpressionCompiler( mSource );
      QgsSqlExpressionCompiler::Result result = compiler.compile( request.filterExpression() );
      QgsDebugMsg( QString( "compiler result: %1" ).arg( result ) + "; query: " + compiler.result() );
      if ( result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial )
      {
        if ( !filterAdded )
          mStatement += " WHERE (" + compiler.result() + ')';
        else
          mStatement += " AND (" + compiler.result() + ')';
        filterAdded = true;

        //if only partial success when compiling expression, we need to double-check results using QGIS' expressions
        mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
        limitAtProvider = mExpressionCompiled;
      }
      else
      {
        limitAtProvider = false;
      }
    }
    else
    {
      limitAtProvider = false;
    }
  }

  QStringList orderByParts;
  mOrderByCompiled = true;
  QgsDebugMsg( QString( "compileExpressions: %1" ).arg( QSettings().value( "/qgis/compileExpressions", true ).toString() ) );
  if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
  {
    Q_FOREACH ( const QgsFeatureRequest::OrderByClause& clause, request.orderBy() )
    {
      QgsDebugMsg( QString( "processing a clause; ascending: %1; nullsFirst: %2" ).arg( clause.ascending() ).arg( clause.nullsFirst() ) );

      if (( clause.ascending() && clause.nullsFirst() ) || ( !clause.ascending() && !clause.nullsFirst() ) )
      {
        // Not supported by DB2
        // NULLs are last in ascending order
        mOrderByCompiled = false;
        QgsDebugMsg( "ascending with nullsFirst not supported" );
        break;
      }

      QgsDb2ExpressionCompiler compiler = QgsDb2ExpressionCompiler( mSource );
      QgsExpression expression = clause.expression();
      QgsDebugMsg( "expression: " + expression.dump() );
      if ( compiler.compile( &expression ) == QgsSqlExpressionCompiler::Complete )
      {
        QgsDebugMsg( "compile complete" );
        QString part;
        part = compiler.result();
        part += clause.ascending() ? " ASC" : " DESC";
        orderByParts << part;
      }
      else
      {
        QgsDebugMsg( "compile of '" + expression.dump() + "' failed" );
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

  if ( !orderByParts.isEmpty() )
  {
    mOrderByClause = QString( " ORDER BY %1" ).arg( orderByParts.join( "," ) );
    mStatement += mOrderByClause;
  }

  if ( request.limit() > 0 )
  {
    mStatement += QString( " FETCH FIRST %1 ROWS ONLY" ).arg( mRequest.limit() );
  }

  QgsDebugMsg( mStatement );
#if 0 // TODO
  if ( fieldCount == 0 )
  {
    QgsDebugMsg( "QgsDb2Provider::select no fields have been requested" );
    mStatement.clear();
  }
#endif
}

bool QgsDb2FeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys )
{
  Q_UNUSED( orderBys )
  QgsDebugMsg( QString( "mOrderByCompiled: %1" ).arg( mOrderByCompiled ) );
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}

bool QgsDb2FeatureIterator::nextFeatureFilterExpression( QgsFeature& f )
{
  QgsDebugMsg( QString( "mExpressionCompiled: %1" ).arg( mExpressionCompiled ) );
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsDb2FeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( !mQuery )
    return false;

  if ( !mQuery->isActive() )
  {
    QgsDebugMsg( "Read attempt on inactive query" );
    return false;
  }

  if ( mQuery->next() )
  {
    feature.initAttributes( mSource->mFields.count() );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    QSqlRecord record = mQuery->record();
    for ( int i = 0; i < mAttributesToFetch.count(); i++ )
    {
      QVariant v = mQuery->value( i );
      QString attrName = record.fieldName( i );
      if ( attrName == mSource->mGeometryColName )
      {
        QgsDebugMsg( QString( "Geom col: %1" ).arg( attrName ) ); // not sure why we set geometry as a field value
      }
      else
      {
        QgsDebugMsg( QString( "Field: %1; value: %2" ).arg( attrName ).arg( v.toString() ) );
        /**
         * CHAR and VARCHAR fields seem to get corrupted sometimes when directly
         * calling feature.setAttribute(..., v) with mQuery->value(i). Workaround
         * that seems to fix the problem is to call v = QVariant(v.toString()).
         */
        if ( v.type() == QVariant::String )
        {
          v = QVariant( v.toString() );
        }
        feature.setAttribute( mAttributesToFetch[i], v );
      }
    }
    QgsDebugMsg( QString( "Fid: %1; value: %2" ).arg( mSource->mFidColName ).arg( record.value( mSource->mFidColName ).toLongLong() ) );
    feature.setFeatureId( mQuery->record().value( mSource->mFidColName ).toLongLong() );

    // David Adler - assumes ST_AsBinary returns expected wkb
    // and setGeometry accepts this wkb
    if ( mSource->isSpatial() )
    {
#if 1
      QByteArray ar = record.value( mSource->mGeometryColName ).toByteArray();
//      QString wkb( ar.toHex() );
//      QgsDebugMsg( "wkb: " + wkb );
//      QgsDebugMsg( "wkb size: " + QString( "%1" ).arg( ar.size() ) );
      size_t wkb_size = ar.size();
      if ( 0 < wkb_size )
      {
        unsigned char* db2data = new unsigned char[wkb_size + 1]; // allocate persistent storage
        memcpy( db2data, ( unsigned char* )ar.data(), wkb_size + 1 );
        QgsGeometry *g = new QgsGeometry();
        g->fromWkb( db2data, wkb_size );
        feature.setGeometry( g );
//        QgsDebugMsg( QString( "geometry type: %1" ).arg( g->wkbType() ) );
//        QByteArray ar2(( const char * )g->asWkb(), wkb_size + 1 );
//        QString wkb2( ar2.toHex() );
//        QgsDebugMsg( "wkb2: " + wkb2 );
      }
      else
      {
        QgsDebugMsg( "Geometry is empty" );
      }
#else
      QByteArray ar = record.value( mSource->mGeometryColName ).toByteArray();
      size_t wkt_size = ar.size();
      char * wkt = new char[wkt_size + 1]; // allocate persistent storage
      strcpy( wkt, ar.data() );
      QgsDebugMsg( "wkt: " + QString( wkt ) );
      QString hex( ar.toHex() );
      QgsDebugMsg( "hex: " + hex );
      QgsDebugMsg( "wkt size: " + QString( "%1" ).arg( ar.size() ) );
      QgsGeometry *g = QgsGeometry::fromWkt( QString( wkt ) );
      feature.setGeometry( g );
      delete wkt;

#endif
    }

    feature.setValid( true );
    return true;
  }
  return false;
}


bool QgsDb2FeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( mStatement.isEmpty() )
  {
    QgsDebugMsg( "rewind on empty statement" );
    return false;
  }

  if ( !mQuery )
    return false;

  mQuery->clear();
  mQuery->setForwardOnly( true );
  QgsDebugMsg( "Execute mStatement: " + mStatement );
  if ( !mQuery->exec( mStatement ) )
  {
    QString msg = mQuery->lastError().text();
    QgsDebugMsg( msg );
    close();
    return false;
  }

  return true;
}

bool QgsDb2FeatureIterator::close()
{
  if ( mClosed )
    return false;

  if ( mQuery )
  {
    if ( !mQuery->isActive() )
    {
      QgsDebugMsg( "QgsDb2FeatureIterator::close on inactive query" );
      return false;
    }

    mQuery->finish();
  }

  if ( mQuery )
    delete mQuery;

  if ( mDatabase.isOpen() )
    mDatabase.close();

  iteratorClosed();

  mClosed = true;
  return true;
}

///////////////

QgsDb2FeatureSource::QgsDb2FeatureSource( const QgsDb2Provider* p )
    : mFields( p->mAttributeFields )
    , mFidColName( p->mFidColName )
    , mGeometryColName( p->mGeometryColName )
    , mGeometryColType( p->mGeometryColType )
    , mSchemaName( p->mSchemaName )
    , mTableName( p->mTableName )
    , mUserName( p->mUserName )
    , mPassword( p->mPassword )
    , mService( p->mService )
    , mDatabaseName( p->mDatabaseName )
    , mDriver( p->mDriver )
    , mHost( p->mHost )
    , mPort( p->mPort )
    , mSqlWhereClause( p->mSqlWhereClause )
{   //TODO set mEnvironment to LUW or ZOS? -David
  mSRId = p->mSRId;
}

QgsDb2FeatureSource::~QgsDb2FeatureSource()
{
}

QgsFeatureIterator QgsDb2FeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsDb2FeatureIterator( this, false, request ) );
}
