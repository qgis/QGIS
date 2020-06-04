/***************************************************************************
   qgshanafeatureiterator.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgsgeometryfactory.h"
#include "qgshanaexception.h"
#include "qgshanaexpressioncompiler.h"
#include "qgshanafeatureiterator.h"
#include "qgshanaprovider.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"

static QString andWhereClauses( const QString &c1, const QString &c2 )
{
  if ( c1.isEmpty() )
    return c2;
  if ( c2.isEmpty() )
    return c1;

  return QStringLiteral( "(%1) AND (%2)" ).arg( c1, c2 );
}

QgsHanaFeatureIterator::QgsHanaFeatureIterator(
  QgsHanaFeatureSource *source,
  bool ownSource,
  const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsHanaFeatureSource>( source, ownSource, request )
  , mConnection( source->mUri )
  , mSrsExtent( source->mSrsExtent )
  , mFidColumn( source->mFidColumn )
{
  mClosed = true;

  if ( mConnection.isNull() )
  {
    iteratorClosed();
    return;
  }

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );

  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    iteratorClosed();
    return;
  }

  try
  {
    mSqlQuery = buildSqlQuery( request );
    mClosed = false;

    rewind();
  }
  catch ( const QgsHanaException & )
  {
    iteratorClosed();
  }
}

QgsHanaFeatureIterator::~QgsHanaFeatureIterator()
{
  if ( !mClosed )
    close();
}

bool QgsHanaFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  mResultSet.reset();
  if ( !( mFilterRect.isNull() || mFilterRect.isEmpty() ) && mSource->isSpatial() && mHasGeometryColumn )
  {
    QString ll = QStringLiteral( "POINT(%1 %2)" ).arg( QString::number( mFilterRect.xMinimum() ),  QString::number( mFilterRect.yMinimum() ) );
    QString ur = QStringLiteral( "POINT(%1 %2)" ).arg( QString::number( mFilterRect.xMaximum() ),  QString::number( mFilterRect.yMaximum() ) );
    mResultSet = mConnection->executeQuery( mSqlQuery, { ll, mSource->mSrid, ur, mSource->mSrid } );
  }
  else
    mResultSet = mConnection->executeQuery( mSqlQuery );

  return true;
}

bool QgsHanaFeatureIterator::close()
{
  if ( mClosed )
    return false;

  mResultSet->close();
  iteratorClosed();
  mClosed = true;

  return true;
}

bool QgsHanaFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( !mResultSet->next() )
    return false;

  feature.initAttributes( mSource->mFields.count() );
  unsigned short paramIndex = 1;

  // Read feature id
  if ( !mFidColumn.isEmpty() )
  {
    QVariant id = mResultSet->getValue( paramIndex );
    feature.setId( id.toLongLong() );
    feature.setAttribute( 0, id );
    ++paramIndex;
  }
  else
  {
    feature.setId( 0u );
  }

  // Read attributes
  if ( mHasAttributes )
  {
    Q_FOREACH ( int idx, mAttributesToFetch )
    {
      feature.setAttribute( idx, mResultSet->getValue( paramIndex ) );
      ++paramIndex;
    }
  }

  // Read geometry
  if ( mHasGeometryColumn )
  {
    QgsGeometry geom = mResultSet->getGeometry( paramIndex );
    if ( !geom.isNull() )
      feature.setGeometry( geom );
    else
      feature.clearGeometry();
  }
  else
  {
    feature.clearGeometry();
  }

  feature.setValid( true );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups
  geometryToDestinationCrs( feature, mTransform );


  return true;
}

bool QgsHanaFeatureIterator::nextFeatureFilterExpression( QgsFeature &feature )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( feature );
  else
    return fetchFeature( feature );
}

QString QgsHanaFeatureIterator::getBBOXFilter( const QVersionNumber &dbVersion ) const
{
  if ( dbVersion.majorVersion() == 1 )
    return QStringLiteral( "%1.ST_SRID(%2).ST_IntersectsRect(ST_GeomFromText(?, ?), ST_GeomFromText(?, ?)) = 1" )
           .arg( QgsHanaUtils::quotedIdentifier( mSource->mGeometryColumn ), QString::number( mSource->mSrid ) );
  else
    return QStringLiteral( "%1.ST_IntersectsRectPlanar(ST_GeomFromText(?, ?), ST_GeomFromText(?, ?)) = 1" )
           .arg( QgsHanaUtils::quotedIdentifier( mSource->mGeometryColumn ) );
}

bool QgsHanaFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}

QString QgsHanaFeatureIterator::buildSqlQuery( const QgsFeatureRequest &request )
{
  bool limitAtProvider = ( mRequest.limit() >= 0 );
  QgsRectangle filterRect = mFilterRect;
  if ( !mSrsExtent.isEmpty() )
    filterRect = mSrsExtent.intersect( filterRect );

  if ( !filterRect.isFinite() )
    QgsMessageLog::logMessage( QObject::tr( "Infinite filter rectangle specified" ), QObject::tr( "HANA" ) );

  QStringList orderByParts;
#if 0
  mOrderByCompiled = true;

  if ( QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
  {
    const auto constOrderBy = request.orderBy();
    for ( const QgsFeatureRequest::OrderByClause &clause : constOrderBy )
    {
      QgsHanaExpressionCompiler compiler = QgsHanaExpressionCompiler( mSource );
      QgsExpression expression = clause.expression();
      if ( compiler.compile( &expression ) == QgsSqlExpressionCompiler::Complete )
      {
        QString part;
        part = compiler.result();
        part += clause.ascending() ? QStringLiteral( " ASC" ) : QStringLiteral( " DESC" );
        part += clause.nullsFirst() ? QStringLiteral( " NULLS FIRST" ) : QStringLiteral( " NULLS LAST" );
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
#endif

  if ( !mOrderByCompiled )
    limitAtProvider = false;

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  QgsAttributeList attrs = ( subsetOfAttributes ) ?
                           request.subsetOfAttributes() : mSource->mFields.allAttributesList();

  if ( subsetOfAttributes )
  {
    // Ensure that all attributes required for expression filter are fetched
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      //ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += attrs.toSet();
      attrs = attributeIndexes.toList();
    }

    if ( !mRequest.orderBy().isEmpty() )
    {
      // Ensure that all attributes required for order by are fetched
      const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
      for ( int attrIndex : usedAttributeIndices )
      {
        if ( !attrs.contains( attrIndex ) )
          attrs << attrIndex;
      }
    }
  }

  Q_FOREACH ( int i, attrs )
  {
    QString fieldname = mSource->mFields.at( i ).name();
    if ( mFidColumn == fieldname )
      continue;
    mAttributesToFetch.append( i );
  }

  QString sqlFields = "";

  // Add feature id column
  if ( !mFidColumn.isEmpty() )
    sqlFields += QgsHanaUtils::quotedIdentifier( mFidColumn ) + ",";

  Q_FOREACH ( int i, mAttributesToFetch )
  {
    QString fieldname = mSource->mFields.at( i ).name();
    sqlFields += QStringLiteral( "%1," ).arg( QgsHanaUtils::quotedIdentifier( fieldname ) );
  }
  mHasAttributes = !mAttributesToFetch.isEmpty();

  // Add geometry column
  if ( ( !( request.flags() & QgsFeatureRequest::NoGeometry )
         || ( request.filterType() == QgsFeatureRequest::FilterExpression &&
              request.filterExpression()->needsGeometry() ) ) && mSource->isSpatial() )
  {
    sqlFields += QStringLiteral( "%1" ).arg( QgsHanaUtils::quotedIdentifier( mSource->mGeometryColumn ) );
    mHasGeometryColumn = true;
  }

  if ( sqlFields.isEmpty() )
  {
    sqlFields = "*";
  }
  else
  {
    if ( sqlFields.endsWith( ',' ) )
      sqlFields.truncate( sqlFields.length() - 1 );
  }

  QString sql = QStringLiteral( "SELECT %1 FROM %2.%3" ).arg(
                  sqlFields,
                  QgsHanaUtils::quotedIdentifier( mSource->mSchemaName ),
                  QgsHanaUtils::quotedIdentifier( mSource->mTableName ) );

  QString sqlFilter;
  // Set spatial filter
  if ( !( filterRect.isNull() || filterRect.isEmpty() ) && mSource->isSpatial() && mHasGeometryColumn )
    sqlFilter = getBBOXFilter( QgsHanaUtils::toHANAVersion( mConnection->getDatabaseVersion() ) );

  if ( !mSource->mQueryWhereClause.isEmpty() )
    sqlFilter = andWhereClauses( sqlFilter, mSource->mQueryWhereClause );

  // Set fid filter
  if ( !mFidColumn.isEmpty() )
  {
    if ( request.filterType() == QgsFeatureRequest::FilterFid )
    {
      QString inClause = QStringLiteral( " %1 = %2" ).arg(
                           QgsHanaUtils::quotedIdentifier( mFidColumn ), FID_TO_STRING( request.filterFid() ) );
      sqlFilter = andWhereClauses( sqlFilter, inClause );
    }
    else if ( request.filterType() == QgsFeatureRequest::FilterFids && !mRequest.filterFids().isEmpty() )
    {
      QString delim;
      QString inClause = QStringLiteral( "%1 IN (" ).arg( QgsHanaUtils::quotedIdentifier( mFidColumn ) );
      Q_FOREACH ( QgsFeatureId featureId, mRequest.filterFids() )
      {
        inClause += delim + FID_TO_STRING( featureId );
        delim = ',';
      }
      inClause.append( ')' );

      sqlFilter = andWhereClauses( sqlFilter, inClause );
    }
  }

  //IMPORTANT - this MUST be the last clause added
  mExpressionCompiled = false;
  mCompileStatus = NoCompilation;
  if ( request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    if ( QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
    {
      QgsHanaExpressionCompiler compiler = QgsHanaExpressionCompiler( mSource );
      QgsSqlExpressionCompiler::Result result = compiler.compile( request.filterExpression() );
      switch ( result )
      {
        case QgsSqlExpressionCompiler::Result::Complete:
        case QgsSqlExpressionCompiler::Result::Partial:
        {
          QString filterExpr = compiler.result();
          if ( !filterExpr.isEmpty() )
          {
            sqlFilter = andWhereClauses( sqlFilter, filterExpr );
            //if only partial success when compiling expression, we need to double-check results
            //using QGIS' expressions
            mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Result::Complete );
            mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
          }
        }
        break;
        case QgsSqlExpressionCompiler::Result::Fail:
          QgsDebugMsg( QStringLiteral( "Unable to compile filter expression: '%1'" )
                       .arg( request.filterExpression()->expression() ).toStdString().c_str() );
          break;
        case QgsSqlExpressionCompiler::Result::None:
          break;
      }
      if ( result != QgsSqlExpressionCompiler::Result::Complete )
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

  if ( !sqlFilter.isEmpty() )
    sql += QStringLiteral( " WHERE " ) + sqlFilter;

  if ( !orderByParts.isEmpty() )
    sql += QStringLiteral( " ORDER BY %1 " ).arg( orderByParts.join( QStringLiteral( "," ) ) );

  if ( limitAtProvider )
    sql += QStringLiteral( " LIMIT %1" ).arg( mRequest.limit() );

  QgsDebugMsgLevel( "Query: " + sql, 4 );
  return sql;
}

QgsHanaFeatureSource::QgsHanaFeatureSource( const QgsHanaProvider *p )
  : mUri( p->mUri )
  , mSchemaName( p->mSchemaName )
  , mTableName( p->mTableName )
  , mFidColumn( p->mFidColumn )
  , mFields( p->mAttributeFields )
  , mFieldInfos( p->mFieldInfos )
  , mGeometryColumn( p->mGeometryColumn )
  , mGeometryType( p->wkbType() )
  , mSrid( p->mSrid )
  , mSrsExtent( p->mSrsExtent )
  , mCrs( p->crs() )
  , mQueryWhereClause( p->mQueryWhereClause )
{
  if ( p->mHasSrsPlanarEquivalent && p->mDatabaseVersion.majorVersion() <= 1 )
    mSrid = QgsHanaUtils::toPlanarSRID( p->mSrid );
}

QgsHanaFeatureSource::~QgsHanaFeatureSource()
{
}

QgsFeatureIterator QgsHanaFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsHanaFeatureIterator( this, false, request ) );
}
