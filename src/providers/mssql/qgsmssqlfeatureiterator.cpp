/***************************************************************************
                    qgsmssqlfeatureiterator.cpp  -  description
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlfeatureiterator.h"
#include "qgsmssqlexpressioncompiler.h"
#include "qgsmssqlprovider.h"
#include "qgslogger.h"

#include <QObject>
#include <QTextStream>
#include <QSqlRecord>
#include <QSettings>


QgsMssqlFeatureIterator::QgsMssqlFeatureIterator( QgsMssqlFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsMssqlFeatureSource>( source, ownSource, request )
    , mExpressionCompiled( false )
    , mOrderByCompiled( false )
{
  mClosed = false;
  mQuery = nullptr;

  mParser.IsGeography = mSource->mIsGeography;

  BuildStatement( request );

  // connect to the database
  mDatabase = QgsMssqlProvider::GetDatabase( mSource->mService, mSource->mHost, mSource->mDatabaseName, mSource->mUserName, mSource->mPassword );

  if ( !mDatabase.open() )
  {
    QgsDebugMsg( "Failed to open database" );
    QgsDebugMsg( mDatabase.lastError().text() );
    return;
  }

  // create sql query
  mQuery = new QSqlQuery( mDatabase );

  // start selection
  rewind();
}


QgsMssqlFeatureIterator::~QgsMssqlFeatureIterator()
{
  close();
}

void QgsMssqlFeatureIterator::BuildStatement( const QgsFeatureRequest& request )
{
  mFallbackStatement.clear();
  mStatement.clear();

  bool limitAtProvider = ( mRequest.limit() >= 0 );

  // build sql statement

  // note: 'SELECT ' is added later, to account for 'SELECT TOP...' type queries
  mStatement += QString( "[%1]" ).arg( mSource->mFidColName );
  mFidCol = mSource->mFields.indexFromName( mSource->mFidColName );
  mAttributesToFetch.append( mFidCol );

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  QgsAttributeList attrs = subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();

  // ensure that all attributes required for expression filter are being fetched
  if ( subsetOfAttributes && request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    Q_FOREACH ( const QString& field, request.filterExpression()->referencedColumns() )
    {
      int attrIdx = mSource->mFields.fieldNameIndex( field );
      if ( !attrs.contains( attrIdx ) )
        attrs << attrIdx;
    }
  }

  Q_FOREACH ( int i, attrs )
  {
    QString fieldname = mSource->mFields.at( i ).name();
    if ( mSource->mFidColName == fieldname )
      continue;

    mStatement += QString( ",[%1]" ).arg( fieldname );

    mAttributesToFetch.append( i );
  }

  // get geometry col
  if (( !( request.flags() & QgsFeatureRequest::NoGeometry )
        || ( request.filterType() == QgsFeatureRequest::FilterExpression && request.filterExpression()->needsGeometry() )
      )
      && mSource->isSpatial() )
  {
    mStatement += QString( ",[%1]" ).arg( mSource->mGeometryColName );
  }

  mStatement += QString( "FROM [%1].[%2]" ).arg( mSource->mSchemaName, mSource->mTableName );

  bool filterAdded = false;
  // set spatial filter
  if ( !request.filterRect().isNull() && mSource->isSpatial() && !request.filterRect().isEmpty() )
  {
    // polygons should be CCW for SqlGeography
    QString r;
    QTextStream foo( &r );

    foo.setRealNumberPrecision( 8 );
    foo.setRealNumberNotation( QTextStream::FixedNotation );
    foo <<  qgsDoubleToString( request.filterRect().xMinimum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMinimum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMaximum() ) << ' ' << qgsDoubleToString( request.filterRect().yMinimum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMaximum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMaximum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMinimum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMaximum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMinimum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMinimum() );

    mStatement += QString( " where [%1].STIsValid() = 1 AND [%1].STIntersects([%2]::STGeomFromText('POLYGON((%3))',%4)) = 1" ).arg(
                    mSource->mGeometryColName, mSource->mGeometryColType, r, QString::number( mSource->mSRId ) );
    filterAdded = true;
  }

  // set fid filter
  if ( request.filterType() == QgsFeatureRequest::FilterFid && !mSource->mFidColName.isEmpty() )
  {
    QString fidfilter = QString( " [%1] = %2" ).arg( mSource->mFidColName, FID_TO_STRING( request.filterFid() ) );
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
      mStatement += " WHERE (" + mSource->mSqlWhereClause + ')';
    else
      mStatement += " AND (" + mSource->mSqlWhereClause + ')';
    filterAdded = true;
  }

  //NOTE - must be last added!
  mExpressionCompiled = false;
  mCompileStatus = NoCompilation;
  if ( request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
    {
      QgsMssqlExpressionCompiler compiler = QgsMssqlExpressionCompiler( mSource );
      QgsSqlExpressionCompiler::Result result = compiler.compile( request.filterExpression() );
      if ( result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial )
      {
        mFallbackStatement = mStatement;
        if ( !filterAdded )
          mStatement += " WHERE (" + compiler.result() + ')';
        else
          mStatement += " AND (" + compiler.result() + ')';

        //if only partial success when compiling expression, we need to double-check results using QGIS' expressions
        mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
        mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
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

  if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
  {
    Q_FOREACH ( const QgsFeatureRequest::OrderByClause& clause, request.orderBy() )
    {
      if (( clause.ascending() && !clause.nullsFirst() ) || ( !clause.ascending() && clause.nullsFirst() ) )
      {
        //not supported by SQL Server
        mOrderByCompiled = false;
        break;
      }

      QgsMssqlExpressionCompiler compiler = QgsMssqlExpressionCompiler( mSource );
      QgsExpression expression = clause.expression();
      if ( compiler.compile( &expression ) == QgsSqlExpressionCompiler::Complete )
      {
        QString part;
        part = compiler.result();
        part += clause.ascending() ? " ASC" : " DESC";
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

  if ( request.limit() >= 0 && limitAtProvider )
  {
    mStatement.prepend( QString( "SELECT TOP %1 " ).arg( mRequest.limit() ) );
    if ( !mFallbackStatement.isEmpty() )
      mFallbackStatement.prepend( QString( "SELECT TOP %1 " ).arg( mRequest.limit() ) );
  }
  else
  {
    mStatement.prepend( "SELECT " );
    if ( !mFallbackStatement.isEmpty() )
      mFallbackStatement.prepend( "SELECT " );
  }

  if ( !orderByParts.isEmpty() )
  {
    mOrderByClause = QString( " ORDER BY %1" ).arg( orderByParts.join( "," ) );
  }

  QgsDebugMsg( mStatement );
#if 0
  if ( fieldCount == 0 )
  {
    QgsDebugMsg( "QgsMssqlProvider::select no fields have been requested" );
    mStatement.clear();
  }
#endif
}


bool QgsMssqlFeatureIterator::fetchFeature( QgsFeature& feature )
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

    for ( int i = 0; i < mAttributesToFetch.count(); i++ )
    {
      QVariant v = mQuery->value( i );
      const QgsField &fld = mSource->mFields.at( mAttributesToFetch.at( i ) );
      if ( v.type() != fld.type() )
        v = QgsVectorDataProvider::convertValue( fld.type(), v.toString() );
      feature.setAttribute( mAttributesToFetch.at( i ), v );
    }

    feature.setFeatureId( mQuery->record().value( mSource->mFidColName ).toLongLong() );

    if ( mSource->isSpatial() )
    {
      QByteArray ar = mQuery->record().value( mSource->mGeometryColName ).toByteArray();
      unsigned char* wkb = mParser.ParseSqlGeometry(( unsigned char* )ar.data(), ar.size() );
      if ( wkb )
      {
        QgsGeometry *g = new QgsGeometry();
        g->fromWkb( wkb, mParser.GetWkbLen() );
        feature.setGeometry( g );
      }
      else
      {
        feature.setGeometry( nullptr );
      }
    }
    else
    {
      feature.setGeometry( nullptr );
    }

    feature.setValid( true );
    return true;
  }
  return false;
}

bool QgsMssqlFeatureIterator::nextFeatureFilterExpression( QgsFeature& f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsMssqlFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}

bool QgsMssqlFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( mStatement.isEmpty() )
  {
    QgsDebugMsg( "QgsMssqlFeatureIterator::rewind on empty statement" );
    return false;
  }

  if ( !mQuery )
    return false;

  mQuery->clear();
  mQuery->setForwardOnly( true );

  bool result = mQuery->exec( mOrderByClause.isEmpty() ? mStatement : mStatement + mOrderByClause );
  if ( !result && !mFallbackStatement.isEmpty() )
  {
    //try with fallback statement
    result = mQuery->exec( mOrderByClause.isEmpty() ? mFallbackStatement : mFallbackStatement + mOrderByClause );
    if ( result )
    {
      mExpressionCompiled = false;
      mCompileStatus = NoCompilation;
    }
  }

  if ( !result && !mOrderByClause.isEmpty() )
  {
    //try without order by clause
    result = mQuery->exec( mStatement );
    if ( result )
      mOrderByCompiled = false;
  }

  if ( !result && !mFallbackStatement.isEmpty() && !mOrderByClause.isEmpty() )
  {
    //try with fallback statement and without order by clause
    result = mQuery->exec( mFallbackStatement );
    if ( result )
    {
      mExpressionCompiled = false;
      mOrderByCompiled = false;
      mCompileStatus = NoCompilation;
    }
  }

  if ( !result )
  {
    QgsDebugMsg( mQuery->lastError().text() );
    delete mQuery;
    mQuery = nullptr;
    if ( mDatabase.isOpen() )
      mDatabase.close();

    iteratorClosed();

    mClosed = true;
    return false;
  }

  return true;
}

bool QgsMssqlFeatureIterator::close()
{
  if ( mClosed )
    return false;

  if ( mQuery )
  {
    if ( !mQuery->isActive() )
    {
      QgsDebugMsg( "QgsMssqlFeatureIterator::close on inactive query" );
      return false;
    }

    mQuery->finish();
  }

  if ( mQuery )
  {
    delete mQuery;
    mQuery = nullptr;
  }

  if ( mDatabase.isOpen() )
    mDatabase.close();

  iteratorClosed();

  mClosed = true;
  return true;
}

///////////////

QgsMssqlFeatureSource::QgsMssqlFeatureSource( const QgsMssqlProvider* p )
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
    , mHost( p->mHost )
    , mSqlWhereClause( p->mSqlWhereClause )
{
  mSRId = p->mSRId;
  mIsGeography = p->mParser.IsGeography;
}

QgsMssqlFeatureSource::~QgsMssqlFeatureSource()
{
}

QgsFeatureIterator QgsMssqlFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsMssqlFeatureIterator( this, false, request ) );
}
