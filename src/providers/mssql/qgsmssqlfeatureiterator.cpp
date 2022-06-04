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
#include "qgsmssqltransaction.h"
#include "qgslogger.h"
#include "qgsdbquerylog.h"
#include "qgssettings.h"
#include "qgsexception.h"
#include "qgsmssqldatabase.h"
#include "qgsgeometryengine.h"

#include <QObject>
#include <QTextStream>
#include <QSqlRecord>


QgsMssqlFeatureIterator::QgsMssqlFeatureIterator( QgsMssqlFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsMssqlFeatureSource>( source, ownSource, request )
  , mDisableInvalidGeometryHandling( source->mDisableInvalidGeometryHandling )
{
  mClosed = false;

  mParser.mIsGeography = mSource->mIsGeography;

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
  // prepare spatial filter geometries for optimal speed
  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
    case Qgis::SpatialFilterType::BoundingBox:
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      if ( !mRequest.referenceGeometry().isEmpty() )
      {
        mDistanceWithinGeom = mRequest.referenceGeometry();
        mDistanceWithinEngine.reset( QgsGeometry::createGeometryEngine( mDistanceWithinGeom.constGet() ) );
        mDistanceWithinEngine->prepareGeometry();
      }
      break;
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

double QgsMssqlFeatureIterator::validLat( const double latitude ) const
{
  if ( latitude < -90.0 )
    return -90.0;
  if ( latitude > 90.0 )
    return 90.0;
  return latitude;
}

double QgsMssqlFeatureIterator::validLon( const double longitude ) const
{
  if ( longitude < -15069.0 )
    return -15069.0;
  if ( longitude > 15069.0 )
    return 15069.0;
  return longitude;
}

QString QgsMssqlFeatureIterator::whereClauseFid( QgsFeatureId featureId )
{
  QString whereClause;

  switch ( mSource->mPrimaryKeyType )
  {
    case PktInt:
      Q_ASSERT( mSource->mPrimaryKeyAttrs.size() == 1 );
      whereClause = QStringLiteral( "[%1]=%2" ).arg( mSource->mFields.at( mSource->mPrimaryKeyAttrs[0] ).name(), FID_TO_STRING( featureId ) );
      break;

    case PktFidMap:
    {
      const QVariantList &pkVals = mSource->mShared->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        Q_ASSERT( pkVals.size() == mSource->mPrimaryKeyAttrs.size() );

        whereClause = QStringLiteral( "(" );

        QString delim;
        for ( int i = 0; i < mSource->mPrimaryKeyAttrs.size(); ++i )
        {
          const QgsField &fld = mSource->mFields.at( mSource->mPrimaryKeyAttrs[i] );
          whereClause += QStringLiteral( "%1[%2]=%3" ).arg( delim, fld.name(), QgsMssqlProvider::quotedValue( pkVals[i] ) );
          delim = QStringLiteral( " AND " );
        }

        whereClause += QLatin1Char( ')' );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
        whereClause = QStringLiteral( "NULL IS NOT NULL" );
      }
    }
    break;

    default:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = QStringLiteral( "NULL IS NOT NULL" );
      break;
  }

  return whereClause;
}

void QgsMssqlFeatureIterator::BuildStatement( const QgsFeatureRequest &request )
{
  mFallbackStatement.clear();
  mStatement.clear();

  bool limitAtProvider = mRequest.limit() >= 0 && mRequest.spatialFilterType() != Qgis::SpatialFilterType::DistanceWithin;

  // build sql statement

  // note: 'SELECT ' is added later, to account for 'SELECT TOP...' type queries
  QString delim;
  for ( auto idx : mSource->mPrimaryKeyAttrs )
  {
    mStatement += QStringLiteral( "%1[%2]" ).arg( delim, mSource->mFields.at( idx ).name() );
    delim = ',';
  }

  mAttributesToFetch << mSource->mPrimaryKeyAttrs;

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  QgsAttributeList attrs = subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();

  if ( subsetOfAttributes )
  {
    // ensure that all attributes required for expression filter are being fetched
    if ( request.filterType() == QgsFeatureRequest::FilterExpression )
    {
      //ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += qgis::listToSet( attrs );
      attrs = qgis::setToList( attributeIndexes );
    }

    // ensure that all attributes required for order by are fetched
    const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
    for ( int attrIndex : usedAttributeIndices )
    {
      if ( !attrs.contains( attrIndex ) )
        attrs << attrIndex;
    }
  }

  for ( int i : std::as_const( attrs ) )
  {
    if ( mSource->mPrimaryKeyAttrs.contains( i ) )
      continue;

    mStatement += QStringLiteral( ",[%1]" ).arg( mSource->mFields.at( i ).name() );

    mAttributesToFetch.append( i );
  }

  // get geometry col
  if ( ( !( request.flags() & QgsFeatureRequest::NoGeometry )
         || ( request.spatialFilterType() == Qgis::SpatialFilterType::DistanceWithin )
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
    QTextStream stream( &r );

    stream.setRealNumberPrecision( 8 );
    stream.setRealNumberNotation( QTextStream::FixedNotation );

    if ( mSource->mGeometryColType == QLatin1String( "geometry" ) )
    {
      stream << qgsDoubleToString( mFilterRect.xMinimum() ) << ' ' << qgsDoubleToString( mFilterRect.yMinimum() ) << ", "
             << qgsDoubleToString( mFilterRect.xMaximum() ) << ' ' << qgsDoubleToString( mFilterRect.yMinimum() ) << ", "
             << qgsDoubleToString( mFilterRect.xMaximum() ) << ' ' << qgsDoubleToString( mFilterRect.yMaximum() ) << ", "
             << qgsDoubleToString( mFilterRect.xMinimum() ) << ' ' << qgsDoubleToString( mFilterRect.yMaximum() ) << ", "
             << qgsDoubleToString( mFilterRect.xMinimum() ) << ' ' << qgsDoubleToString( mFilterRect.yMinimum() );
    }
    else
    {
      stream << qgsDoubleToString( validLon( mFilterRect.xMinimum() ) ) << ' ' << qgsDoubleToString( validLat( mFilterRect.yMinimum() ) ) << ", "
             << qgsDoubleToString( validLon( mFilterRect.xMaximum() ) ) << ' ' << qgsDoubleToString( validLat( mFilterRect.yMinimum() ) ) << ", "
             << qgsDoubleToString( validLon( mFilterRect.xMaximum() ) ) << ' ' << qgsDoubleToString( validLat( mFilterRect.yMaximum() ) ) << ", "
             << qgsDoubleToString( validLon( mFilterRect.xMinimum() ) ) << ' ' << qgsDoubleToString( validLat( mFilterRect.yMaximum() ) ) << ", "
             << qgsDoubleToString( validLon( mFilterRect.xMinimum() ) ) << ' ' << qgsDoubleToString( validLat( mFilterRect.yMinimum() ) );
    }

    mStatement += QLatin1String( " WHERE " );
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
  if ( request.filterType() == QgsFeatureRequest::FilterFid && !mSource->mPrimaryKeyAttrs.isEmpty() )
  {
    if ( !filterAdded )
      mStatement += QLatin1String( " WHERE " );
    else
      mStatement += QLatin1String( " AND " );

    if ( mSource->mPrimaryKeyType == PktInt )
    {
      mStatement += QStringLiteral( "[%1]=%2" ).arg( mSource->mFields[mSource->mPrimaryKeyAttrs[0]].name(), FID_TO_STRING( request.filterFid() ) );
    }
    else if ( mSource->mPrimaryKeyType == PktFidMap )
    {
      QVariantList key = mSource->mShared->lookupKey( request.filterFid() );
      if ( !key.isEmpty() )
      {
        mStatement += "(";

        QString delim;
        for ( int i = 0; i < mSource->mPrimaryKeyAttrs.size(); i++ )
        {
          QString colName = mSource->mFields[mSource->mPrimaryKeyAttrs[i]].name();
          QString expr;
          if ( key[i].isNull() )
            expr = QString( "[%1] IS NULL" ).arg( colName );
          else
            expr = QString( "[%1]=%2" ).arg( colName, QgsMssqlProvider::quotedValue( key[i] ) );

          mStatement += QStringLiteral( "%1%2" ).arg( delim, expr );
          delim = " AND ";
        }

        mStatement += ")";
      }
    }

    filterAdded = true;
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFids && !mSource->mPrimaryKeyAttrs.isEmpty()
            && !mRequest.filterFids().isEmpty() )
  {
    if ( !filterAdded )
      mStatement += QLatin1String( " WHERE " );
    else
      mStatement += QLatin1String( " AND " );

    if ( mSource->mPrimaryKeyType == PktInt )
    {
      QString delim;
      QString colName = mSource->mFields[mSource->mPrimaryKeyAttrs[0]].name();
      QString inClause = QStringLiteral( "[%1] IN (" ).arg( colName );
      const auto constFilterFids = mRequest.filterFids();
      for ( QgsFeatureId featureId : constFilterFids )
      {
        inClause += delim + FID_TO_STRING( featureId );
        delim = ',';
      }
      inClause.append( ')' );

      mStatement += inClause;
    }
    else
    {
      const auto constFilterFids = mRequest.filterFids();

      if ( !constFilterFids.isEmpty() )
      {
        QString delim( "(" );
        for ( QgsFeatureId featureId : constFilterFids )
        {
          mStatement += delim + whereClauseFid( featureId ) + ")";
          delim = " OR (";
        }

        mStatement += ")";
      }
    }

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
    QgsMssqlExpressionCompiler compiler = QgsMssqlExpressionCompiler( mSource, request.flags() & QgsFeatureRequest::IgnoreStaticNodesDuringExpressionCompilation );
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

  QStringList orderByParts;
  mOrderByCompiled = true;

  const auto constOrderBy = request.orderBy();
  for ( const QgsFeatureRequest::OrderByClause &clause : constOrderBy )
  {
    if ( ( clause.ascending() && !clause.nullsFirst() ) || ( !clause.ascending() && clause.nullsFirst() ) )
    {
      //not supported by SQL Server
      mOrderByCompiled = false;
      break;
    }

    QgsMssqlExpressionCompiler compiler = QgsMssqlExpressionCompiler( mSource, request.flags() & QgsFeatureRequest::IgnoreStaticNodesDuringExpressionCompilation );
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
    mOrderByClause = QStringLiteral( " ORDER BY %1" ).arg( orderByParts.join( QLatin1Char( ',' ) ) );
  }

  QgsDebugMsgLevel( mStatement + " " + mOrderByClause, 2 );
#if 0
  if ( fieldCount == 0 )
  {
    QgsDebugMsg( QStringLiteral( "QgsMssqlProvider::select no fields have been requested" ) );
    mStatement.clear();
  }
#endif
}

bool QgsMssqlFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( !mDatabase )
  {
    if ( mSource->mTransactionConn )
    {
      // Using shared connection for the transaction, but that's fine because we use
      // a mutex to prevent concurrent access to it from multiple threads.
      mDatabase = mSource->mTransactionConn;
    }
    else
    {
      // No existing connection, so set it up now. It's safe to do here as we're now in
      // the thread were iteration is actually occurring.
      mDatabase = QgsMssqlDatabase::connectDb( mSource->mService, mSource->mHost, mSource->mDatabaseName, mSource->mUserName, mSource->mPassword );
    }

    if ( !mDatabase->isValid() )
    {
      QgsDebugMsg( QStringLiteral( "Failed to open database" ) );
      QgsDebugMsg( mDatabase->errorText() );
      return false;
    }

    // create sql query
    mQuery.reset( new QgsMssqlQuery( mDatabase ) );

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

  while ( mQuery->next() )
  {
    feature.initAttributes( mSource->mFields.count() );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups

    for ( int i = 0; i < mAttributesToFetch.count(); i++ )
    {
      const QVariant originalValue = mQuery->value( i );
      QgsField fld = mSource->mFields.at( mAttributesToFetch.at( i ) );
      QVariant v = originalValue;
      if ( fld.type() == QVariant::Time )
        v = QgsMssqlProvider::convertTimeValue( v );
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

    QgsFeatureId fid = 0;

    switch ( mSource->mPrimaryKeyType )
    {
      case PktInt:
        // get 64bit integer from result
        fid = mQuery->record().value( mSource->mFields.at( mSource->mPrimaryKeyAttrs.value( 0 ) ).name() ).toLongLong();
        if ( mAttributesToFetch.contains( mSource->mPrimaryKeyAttrs.value( 0 ) ) )
          feature.setAttribute( mSource->mPrimaryKeyAttrs.value( 0 ), fid );
        break;

      case PktFidMap:
      {
        QVariantList primaryKeyVals;
        for ( int idx : std::as_const( mSource->mPrimaryKeyAttrs ) )
        {
          QgsField fld = mSource->mFields.at( idx );

          QVariant v = mQuery->record().value( fld.name() );
          if ( fld.type() == QVariant::Time )
            v = QgsMssqlProvider::convertTimeValue( v );
          if ( v.type() != fld.type() )
            v = QgsVectorDataProvider::convertValue( fld.type(), v.toString() );
          primaryKeyVals << v;

          if ( mAttributesToFetch.contains( idx ) )
            feature.setAttribute( idx, v );
        }

        fid = mSource->mShared->lookupFid( primaryKeyVals );
      }
      break;

      case PktUnknown:
        Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
        return false;
    }

    feature.setId( fid );

    feature.clearGeometry();
    if ( mSource->isSpatial() )
    {
      QByteArray ar = mQuery->record().value( mSource->mGeometryColName ).toByteArray();
      if ( !ar.isEmpty() )
      {
        std::unique_ptr<QgsAbstractGeometry> geom = mParser.parseSqlGeometry( reinterpret_cast< unsigned char * >( ar.data() ), ar.size() );
        if ( geom )
          feature.setGeometry( QgsGeometry( std::move( geom ) ) );
      }
    }

    geometryToDestinationCrs( feature, mTransform );
    if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
    {
      continue;
    }

    feature.setValid( true );
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

  QString sql { mOrderByClause.isEmpty() ? mStatement : mStatement + mOrderByClause };
  std::unique_ptr<QgsDatabaseQueryLogWrapper> logWrapper = std::make_unique<QgsDatabaseQueryLogWrapper>( sql, mSource->connInfo(), QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlFeatureIterator" ), QGS_QUERY_LOG_ORIGIN );

  bool result = mQuery->exec( sql );
  if ( !result )
  {
    logWrapper->setError( mQuery->lastError().text() );
    if ( !mFallbackStatement.isEmpty() )
    {
      //try with fallback statement
      sql = mOrderByClause.isEmpty() ? mFallbackStatement : mFallbackStatement + mOrderByClause;
      logWrapper.reset( new QgsDatabaseQueryLogWrapper( sql, mSource->connInfo(), QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlFeatureIterator" ), QGS_QUERY_LOG_ORIGIN ) );
      result = mQuery->exec( sql );
      if ( result )
      {
        mExpressionCompiled = false;
        mCompileStatus = NoCompilation;
      }
      else
      {
        logWrapper->setError( mQuery->lastError().text() );
      }
    }
  }

  if ( !result && !mOrderByClause.isEmpty() )
  {
    //try without order by clause
    logWrapper.reset( new QgsDatabaseQueryLogWrapper( mStatement, mSource->connInfo(), QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlFeatureIterator" ), QGS_QUERY_LOG_ORIGIN ) );
    result = mQuery->exec( mStatement );
    if ( result )
    {
      mOrderByCompiled = false;
    }
    else
    {
      logWrapper->setError( mQuery->lastError().text() );
    }
  }

  if ( !result && !mFallbackStatement.isEmpty() && !mOrderByClause.isEmpty() )
  {
    //try with fallback statement and without order by clause
    logWrapper.reset( new QgsDatabaseQueryLogWrapper( mFallbackStatement, mSource->connInfo(), QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlFeatureIterator" ), QGS_QUERY_LOG_ORIGIN ) );
    result = mQuery->exec( mFallbackStatement );
    if ( result )
    {
      mExpressionCompiled = false;
      mOrderByCompiled = false;
      mCompileStatus = NoCompilation;
    }
    else
    {
      logWrapper->setError( mQuery->lastError().text() );
    }
  }

  if ( !result )
  {
    QgsDebugMsg( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( mQuery->lastQuery(), mQuery->lastError().text() ) );
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

  iteratorClosed();

  mClosed = true;
  return true;
}

///////////////

QgsMssqlFeatureSource::QgsMssqlFeatureSource( const QgsMssqlProvider *p )
  : mFields( p->mAttributeFields )
  , mPrimaryKeyType( p->mPrimaryKeyType )
  , mPrimaryKeyAttrs( p->mPrimaryKeyAttrs )
  , mShared( p->mShared )
  , mSRId( p->mSRId )
  , mIsGeography( p->mParser.mIsGeography )
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
  , mTransactionConn( p->transaction() ? static_cast<QgsMssqlTransaction *>( p->transaction() )->conn() : std::shared_ptr<QgsMssqlDatabase>() )
  , mConnInfo( p->uri().uri( ) )
{}

QgsFeatureIterator QgsMssqlFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsMssqlFeatureIterator( this, false, request ) );
}

const QString &QgsMssqlFeatureSource::connInfo() const
{
  return mConnInfo;
}
