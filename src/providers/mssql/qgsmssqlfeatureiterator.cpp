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
#include "qgssettings.h"
#include "qgsexception.h"
#include "qgsmssqlconnection.h"

#include <QObject>
#include <QTextStream>
#include <QSqlRecord>


QgsMssqlFeatureIterator::QgsMssqlFeatureIterator( QgsMssqlFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsMssqlFeatureSource>( source, ownSource, request )
  , mDisableInvalidGeometryHandling( source->mDisableInvalidGeometryHandling )
{
  mClosed = false;

  mParser.IsGeography = mSource->mIsGeography;

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }

  BuildStatement( request );

  // WARNING - we can't obtain the database connection now, as this method should be
  // run from the main thread, yet iteration can be done in a different thread.
  // This would result in failure, because QSqlDatabase instances cannot be used
  // from a different thread where they were created. Instead, we defer creation
  // of the database until the first feature is fetched.
}


QgsMssqlFeatureIterator::~QgsMssqlFeatureIterator()
{
  close();
}

void QgsMssqlFeatureIterator::BuildStatement( const QgsFeatureRequest &request )
{
  mFallbackStatement.clear();
  mStatement.clear();

  bool limitAtProvider = ( mRequest.limit() >= 0 );

  // build sql statement

  // note: 'SELECT ' is added later, to account for 'SELECT TOP...' type queries
  mStatement += QStringLiteral( "[%1]" ).arg( mSource->mFidColName );
  mFidCol = mSource->mFields.indexFromName( mSource->mFidColName );
  mAttributesToFetch.append( mFidCol );

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  QgsAttributeList attrs = subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();

  if ( subsetOfAttributes )
  {
    // ensure that all attributes required for expression filter are being fetched
    if ( request.filterType() == QgsFeatureRequest::FilterExpression )
    {
      //ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += attrs.toSet();
      attrs = attributeIndexes.toList();
    }

    // ensure that all attributes required for order by are fetched
    const QSet< QString > orderByAttributes = mRequest.orderBy().usedAttributes();
    for ( const QString &attr : orderByAttributes )
    {
      int attrIndex = mSource->mFields.lookupField( attr );
      if ( !attrs.contains( attrIndex ) )
        attrs << attrIndex;
    }
  }

  for ( int i : qgis::as_const( attrs ) )
  {
    QString fieldname = mSource->mFields.at( i ).name();
    if ( mSource->mFidColName == fieldname )
      continue;

    mStatement += QStringLiteral( ",[%1]" ).arg( fieldname );

    mAttributesToFetch.append( i );
  }

  // get geometry col
  if ( ( !( request.flags() & QgsFeatureRequest::NoGeometry )
         || ( request.filterType() == QgsFeatureRequest::FilterExpression && request.filterExpression()->needsGeometry() )
       )
       && mSource->isSpatial() )
  {
    mStatement += QStringLiteral( ",[%1]" ).arg( mSource->mGeometryColName );
  }

  mStatement += QStringLiteral( " FROM [%1].[%2]" ).arg( mSource->mSchemaName, mSource->mTableName );

  bool filterAdded = false;
  // set spatial filter
  if ( !mFilterRect.isNull() && mSource->isSpatial() && !mFilterRect.isEmpty() )
  {
    // polygons should be CCW for SqlGeography
    QString r;
    QTextStream foo( &r );

    foo.setRealNumberPrecision( 8 );
    foo.setRealNumberNotation( QTextStream::FixedNotation );
    foo <<  qgsDoubleToString( mFilterRect.xMinimum() ) << ' ' <<  qgsDoubleToString( mFilterRect.yMinimum() ) << ", "
        <<  qgsDoubleToString( mFilterRect.xMaximum() ) << ' ' << qgsDoubleToString( mFilterRect.yMinimum() ) << ", "
        <<  qgsDoubleToString( mFilterRect.xMaximum() ) << ' ' <<  qgsDoubleToString( mFilterRect.yMaximum() ) << ", "
        <<  qgsDoubleToString( mFilterRect.xMinimum() ) << ' ' <<  qgsDoubleToString( mFilterRect.yMaximum() ) << ", "
        <<  qgsDoubleToString( mFilterRect.xMinimum() ) << ' ' <<  qgsDoubleToString( mFilterRect.yMinimum() );

    mStatement += QStringLiteral( " WHERE " );
    if ( !mDisableInvalidGeometryHandling )
      mStatement += QStringLiteral( "[%1].STIsValid() = 1 AND " ).arg( mSource->mGeometryColName );

    // use the faster filter method only when we don't need an exact intersect test -- filter doesn't give exact
    // results when the layer has a spatial index
    QString test = mRequest.flags() & QgsFeatureRequest::ExactIntersect ? QStringLiteral( "STIntersects" ) : QStringLiteral( "Filter" );
    mStatement += QStringLiteral( "[%1].%2([%3]::STGeomFromText('POLYGON((%4))',%5)) = 1" ).arg(
                    mSource->mGeometryColName, test, mSource->mGeometryColType, r, QString::number( mSource->mSRId ) );
    filterAdded = true;
  }

  // set fid filter
  if ( request.filterType() == QgsFeatureRequest::FilterFid && !mSource->mFidColName.isEmpty() )
  {
    QString fidfilter = QStringLiteral( " [%1] = %2" ).arg( mSource->mFidColName, FID_TO_STRING( request.filterFid() ) );
    // set attribute filter
    if ( !filterAdded )
      mStatement += QLatin1String( " WHERE " );
    else
      mStatement += QLatin1String( " AND " );

    mStatement += fidfilter;
    filterAdded = true;
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFids && !mSource->mFidColName.isEmpty()
            && !mRequest.filterFids().isEmpty() )
  {
    QString delim;
    QString inClause = QStringLiteral( "%1 IN (" ).arg( mSource->mFidColName );
    Q_FOREACH ( QgsFeatureId featureId, mRequest.filterFids() )
    {
      inClause += delim + FID_TO_STRING( featureId );
      delim = ',';
    }
    inClause.append( ')' );

    if ( !filterAdded )
      mStatement += QLatin1String( " WHERE " );
    else
      mStatement += QLatin1String( " AND " );

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
    if ( QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
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

  if ( QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
  {
    Q_FOREACH ( const QgsFeatureRequest::OrderByClause &clause, request.orderBy() )
    {
      if ( ( clause.ascending() && !clause.nullsFirst() ) || ( !clause.ascending() && clause.nullsFirst() ) )
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
        part += clause.ascending() ? QStringLiteral( " ASC" ) : QStringLiteral( " DESC" );
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

  if ( !mOrderByCompiled && !request.orderBy().isEmpty() )
    limitAtProvider = false;

  if ( request.limit() >= 0 && limitAtProvider )
  {
    mStatement.prepend( QStringLiteral( "SELECT TOP %1 " ).arg( mRequest.limit() ) );
    if ( !mFallbackStatement.isEmpty() )
      mFallbackStatement.prepend( QStringLiteral( "SELECT TOP %1 " ).arg( mRequest.limit() ) );
  }
  else
  {
    mStatement.prepend( "SELECT " );
    if ( !mFallbackStatement.isEmpty() )
      mFallbackStatement.prepend( "SELECT " );
  }

  if ( !orderByParts.isEmpty() )
  {
    mOrderByClause = QStringLiteral( " ORDER BY %1" ).arg( orderByParts.join( QStringLiteral( "," ) ) );
  }

  QgsDebugMsg( mStatement + " " + mOrderByClause );
#if 0
  if ( fieldCount == 0 )
  {
    QgsDebugMsg( "QgsMssqlProvider::select no fields have been requested" );
    mStatement.clear();
  }
#endif
}


bool QgsMssqlFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( !mDatabase.isValid() )
  {
    // No existing connection, so set it up now. It's safe to do here as we're now in
    // the thread were iteration is actually occurring.
    mDatabase = QgsMssqlConnection::getDatabase( mSource->mService, mSource->mHost, mSource->mDatabaseName, mSource->mUserName, mSource->mPassword );

    if ( !mDatabase.open() )
    {
      QgsDebugMsg( QStringLiteral( "Failed to open database" ) );
      QgsDebugMsg( mDatabase.lastError().text() );
      return false;
    }

    // create sql query
    mQuery.reset( new QSqlQuery( mDatabase ) );

    // start selection
    if ( !rewind() )
      return false;
  }

  if ( !mQuery )
    return false;

  if ( !mQuery->isActive() )
  {
    QgsDebugMsg( QStringLiteral( "Read attempt on inactive query" ) );
    return false;
  }

  if ( mQuery->next() )
  {
    feature.initAttributes( mSource->mFields.count() );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups

    for ( int i = 0; i < mAttributesToFetch.count(); i++ )
    {
      const QVariant originalValue = mQuery->value( i );
      QgsField fld = mSource->mFields.at( mAttributesToFetch.at( i ) );
      QVariant v = originalValue;
      if ( v.type() != fld.type() )
        v = QgsVectorDataProvider::convertValue( fld.type(), originalValue.toString() );

      // second chance for time fields -- time fields are not correctly handled by sql server driver on linux (maybe win too?)
      if ( v.isNull() && fld.type() == QVariant::Time && originalValue.isValid() && originalValue.type() == QVariant::ByteArray )
      {
        // time fields can be returned as byte arrays... woot
        const QByteArray ba = originalValue.toByteArray();
        if ( ba.length() >= 5 )
        {
          const int hours = ba.at( 0 );
          const int mins = ba.at( 2 );
          const int seconds = ba.at( 4 );
          v = QTime( hours, mins, seconds );
          if ( !v.isValid() ) // can't handle it
            v = QVariant( QVariant::Time );
        }
      }

      feature.setAttribute( mAttributesToFetch.at( i ), v );
    }

    feature.setId( mQuery->record().value( mSource->mFidColName ).toLongLong() );

    feature.clearGeometry();
    if ( mSource->isSpatial() )
    {
      QByteArray ar = mQuery->record().value( mSource->mGeometryColName ).toByteArray();
      if ( !ar.isEmpty() )
      {
        if ( unsigned char *wkb = mParser.ParseSqlGeometry( reinterpret_cast< unsigned char * >( ar.data() ), ar.size() ) )
        {
          QgsGeometry g;
          g.fromWkb( wkb, mParser.GetWkbLen() );
          feature.setGeometry( g );
        }
      }
    }

    feature.setValid( true );
    geometryToDestinationCrs( feature, mTransform );
    return true;
  }
  return false;
}

bool QgsMssqlFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsMssqlFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
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
    QgsDebugMsg( QStringLiteral( "QgsMssqlFeatureIterator::rewind on empty statement" ) );
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
    close();
    return false;
  }

  return true;
}

bool QgsMssqlFeatureIterator::close()
{
  if ( mClosed )
    return false;

  if ( mQuery && mQuery->isActive() )
  {
    mQuery->finish();
  }

  mQuery.reset();

  if ( mDatabase.isOpen() )
    mDatabase.close();

  iteratorClosed();

  mClosed = true;
  return true;
}

///////////////

QgsMssqlFeatureSource::QgsMssqlFeatureSource( const QgsMssqlProvider *p )
  : mFields( p->mAttributeFields )
  , mFidColName( p->mFidColName )
  , mSRId( p->mSRId )
  , mIsGeography( p->mParser.IsGeography )
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
  , mDisableInvalidGeometryHandling( p->mDisableInvalidGeometryHandling )
  , mCrs( p->crs() )
{}

QgsFeatureIterator QgsMssqlFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsMssqlFeatureIterator( this, false, request ) );
}
