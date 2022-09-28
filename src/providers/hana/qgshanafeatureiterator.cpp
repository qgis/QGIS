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
#include "qgshanaconnection.h"
#include "qgshanaexception.h"
#include "qgshanaexpressioncompiler.h"
#include "qgshanafeatureiterator.h"
#include "qgshanaprimarykeys.h"
#include "qgshanaprovider.h"
#include "qgshanacrsutils.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsgeometryengine.h"

namespace
{
  QgsRectangle clampBBOX( const QgsRectangle &bbox, const QgsCoordinateReferenceSystem &crs, double allowedExcessFactor )
  {
    // In geographic CRS', HANA will reject any points outside the "normalized"
    // range, which is (in radian) [-PI;PI] for longitude and [-PI/2;PI/2] for
    // latitude. As QGIS seems to expect that larger bounding boxes should
    // be allowed and should not wrap-around, we clamp the bounding boxes for
    // geographic CRS' here.
    if ( !crs.isGeographic() )
      return bbox;

    double factor = QgsHanaCrsUtils::getAngularUnits( crs );

    double minx = -M_PI / factor;
    double maxx = M_PI / factor;
    double spanx = maxx - minx;
    minx -= allowedExcessFactor * spanx;
    maxx += allowedExcessFactor * spanx;

    double miny = -0.5 * M_PI / factor;
    double maxy = 0.5 * M_PI / factor;
    double spany = maxy - miny;
    miny -= allowedExcessFactor * spany;
    maxy += allowedExcessFactor * spany;

    return bbox.intersect( QgsRectangle( minx, miny, maxx, maxy ) );
  }

  QString fieldExpression( const QgsField &field )
  {
    QString typeName = field.typeName();
    QString fieldName = QgsHanaUtils::quotedIdentifier( field.name() );
    if ( field.type() == QVariant::String &&
         ( typeName == QLatin1String( "ST_GEOMETRY" ) || typeName == QLatin1String( "ST_POINT" ) ) )
      return QStringLiteral( "%1.ST_ASWKT()" ).arg( fieldName );
    return fieldName;
  }
}

QgsHanaFeatureIterator::QgsHanaFeatureIterator(
  QgsHanaFeatureSource *source,
  bool ownSource,
  const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsHanaFeatureSource>( source, ownSource, request )
  , mDatabaseVersion( source->mDatabaseVersion )
  , mConnection( source->mUri )
{
  if ( mConnection.isNull() )
  {
    mClosed = true;
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

  try
  {
    mSqlQuery = buildSqlQuery( request );
    mSqlQueryParams = buildSqlQueryParameters();

    rewind();
  }
  catch ( const QgsHanaException & )
  {
    close();
  }
}

QgsHanaFeatureIterator::~QgsHanaFeatureIterator()
{
  close();
}

bool QgsHanaFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  mResultSet.reset();
  mResultSet = mConnection->executeQuery( mSqlQuery, mSqlQueryParams );

  return true;
}

bool QgsHanaFeatureIterator::close()
{
  if ( mClosed )
    return false;

  if ( mResultSet )
  {
    mResultSet->close();
    mResultSet.reset();
  }

  iteratorClosed();
  mClosed = true;

  return true;
}

bool QgsHanaFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed || !mResultSet )
    return false;

  while ( mResultSet->next() )
  {
    feature.initAttributes( mSource->mFields.count() );
    unsigned short paramIndex = 1;

    // Read feature id
    QgsFeatureId fid = FID_NULL;
    bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
    QgsAttributeList fetchAttributes = mRequest.subsetOfAttributes();

    if ( !mSource->mPrimaryKeyAttrs.isEmpty() )
    {
      switch ( mSource->mPrimaryKeyType )
      {
        case QgsHanaPrimaryKeyType::PktInt:
        {
          int idx = mSource->mPrimaryKeyAttrs.at( 0 );
          QVariant v = mResultSet->getValue( paramIndex );
          if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
            feature.setAttribute( idx, v );
          fid = QgsHanaPrimaryKeyUtils::intToFid( v.toInt() );
          ++paramIndex;
        }
        break;
        case QgsHanaPrimaryKeyType::PktInt64:
        {
          int idx = mSource->mPrimaryKeyAttrs.at( 0 );
          QVariant v = mResultSet->getValue( paramIndex );
          if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
            feature.setAttribute( idx, v );
          fid = mSource->mPrimaryKeyCntx->lookupFid( QVariantList( { v} ) );
          ++paramIndex;
        }
        break;
        case QgsHanaPrimaryKeyType::PktFidMap:
        {
          QVariantList pkValues;
          pkValues.reserve( mSource->mPrimaryKeyAttrs.size() );
          for ( int idx : std::as_const( mSource->mPrimaryKeyAttrs ) )
          {
            QVariant v = mResultSet->getValue( paramIndex );
            pkValues << v;
            if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
              feature.setAttribute( idx, v );
            paramIndex++;
          }
          fid = mSource->mPrimaryKeyCntx->lookupFid( pkValues );
        }
        break;
        case QgsHanaPrimaryKeyType::PktUnknown:
          break;
      }
    }

    feature.setId( fid );

    // Read attributes
    if ( mHasAttributes )
    {
      for ( int idx : std::as_const( mAttributesToFetch ) )
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

    geometryToDestinationCrs( feature, mTransform );
    if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
    {
      continue;
    }

    feature.setValid( true );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    return true;
  }
  return false;
}

bool QgsHanaFeatureIterator::nextFeatureFilterExpression( QgsFeature &feature )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( feature );
  else
    return fetchFeature( feature );
}

QString QgsHanaFeatureIterator::getBBOXFilter( ) const
{
  if ( mDatabaseVersion.majorVersion() == 1 )
    return QStringLiteral( "%1.ST_SRID(%2).ST_IntersectsRect(ST_GeomFromText(?, ?), ST_GeomFromText(?, ?)) = 1" )
           .arg( QgsHanaUtils::quotedIdentifier( mSource->mGeometryColumn ), QString::number( mSource->mSrid ) );
  else
    return QStringLiteral( "%1.ST_IntersectsRectPlanar(ST_GeomFromText(?, ?), ST_GeomFromText(?, ?)) = 1" )
           .arg( QgsHanaUtils::quotedIdentifier( mSource->mGeometryColumn ) );
}

QgsRectangle QgsHanaFeatureIterator::getFilterRect() const
{
  const QgsCoordinateReferenceSystem &crs = mSource->mCrs;
  if ( !crs.isGeographic() )
    return mFilterRect;

  if ( mDatabaseVersion.majorVersion() > 1 )
    return clampBBOX( mFilterRect, crs, 0.0 );

  int srid = QgsHanaUtils::toPlanarSRID( mSource->mSrid );
  if ( srid == mSource->mSrid )
    return mFilterRect;
  return clampBBOX( mFilterRect, crs, 0.5 );
}

bool QgsHanaFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}

QString QgsHanaFeatureIterator::buildSqlQuery( const QgsFeatureRequest &request )
{
  const bool geometryRequested = ( request.flags() & QgsFeatureRequest::NoGeometry ) == 0
                                 || !mFilterRect.isNull()
                                 || request.spatialFilterType() == Qgis::SpatialFilterType::DistanceWithin;
  bool limitAtProvider = ( request.limit() >= 0 ) && mRequest.spatialFilterType() != Qgis::SpatialFilterType::DistanceWithin;

  QgsRectangle filterRect = mFilterRect;
  if ( !mSource->mSrsExtent.isEmpty() )
    filterRect = mSource->mSrsExtent.intersect( filterRect );

  if ( !filterRect.isFinite() )
    QgsMessageLog::logMessage( QObject::tr( "Infinite filter rectangle specified" ), QObject::tr( "SAP HANA" ) );

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
  QgsAttributeIds attrIds = qgis::listToSet( subsetOfAttributes ?
                            request.subsetOfAttributes() : mSource->mFields.allAttributesList() );

  if ( subsetOfAttributes )
  {
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
      // Ensure that all attributes required for expression filter are fetched
      attrIds.unite( request.filterExpression()->referencedAttributeIndexes( mSource->mFields ) );

    if ( !mRequest.orderBy().isEmpty() )
      // Ensure that all attributes required for order by are fetched
      attrIds.unite( mRequest.orderBy().usedAttributeIndices( mSource->mFields ) );
  }

  QStringList sqlFields;
  // Add feature id column
  for ( int idx : std::as_const( mSource->mPrimaryKeyAttrs ) )
  {
    const QgsField &field = mSource->mFields.at( idx );
    sqlFields.push_back( fieldExpression( field ) );
  }

  for ( int idx : std::as_const( attrIds ) )
  {
    if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
      continue;

    mAttributesToFetch.append( idx );
    const QgsField &field = mSource->mFields.at( idx );
    sqlFields.push_back( fieldExpression( field ) );
  }

  mHasAttributes = !mAttributesToFetch.isEmpty();

  // Add geometry column
  if ( mSource->isSpatial() &&
       ( geometryRequested || ( request.filterType() == QgsFeatureRequest::FilterExpression &&
                                request.filterExpression()->needsGeometry() ) ) )
  {
    sqlFields += QgsHanaUtils::quotedIdentifier( mSource->mGeometryColumn );
    mHasGeometryColumn = true;
  }

  QStringList sqlFilter;
  // Set spatial filter
  if ( mSource->isSpatial() && mHasGeometryColumn && !( filterRect.isNull() || filterRect.isEmpty() ) )
    sqlFilter.push_back( getBBOXFilter() );

  if ( !mSource->mQueryWhereClause.isEmpty() )
    sqlFilter.push_back( mSource->mQueryWhereClause );

  // Set fid filter
  if ( !mSource->mPrimaryKeyAttrs.isEmpty() )
  {
    if ( request.filterType() == QgsFeatureRequest::FilterFid )
    {
      QString fidWhereClause = QgsHanaPrimaryKeyUtils::buildWhereClause( request.filterFid(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, *mSource->mPrimaryKeyCntx );
      if ( fidWhereClause.isEmpty() )
        throw QgsHanaException( QStringLiteral( "Key values for feature %1 not found." ).arg( request.filterFid() ) );
      sqlFilter.push_back( fidWhereClause );
    }
    else if ( request.filterType() == QgsFeatureRequest::FilterFids && !mRequest.filterFids().isEmpty() )
    {
      QString fidsWhereClause = QgsHanaPrimaryKeyUtils::buildWhereClause( request.filterFids(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, *mSource->mPrimaryKeyCntx );
      if ( fidsWhereClause.isEmpty() )
        throw QgsHanaException( QStringLiteral( "Key values for features not found." ) );
      sqlFilter.push_back( fidsWhereClause );
    }
  }

  //IMPORTANT - this MUST be the last clause added
  mExpressionCompiled = false;
  mCompileStatus = NoCompilation;
  if ( request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    if ( QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
    {
      QgsHanaExpressionCompiler compiler = QgsHanaExpressionCompiler( mSource, request.flags() & QgsFeatureRequest::IgnoreStaticNodesDuringExpressionCompilation );
      QgsSqlExpressionCompiler::Result result = compiler.compile( request.filterExpression() );
      switch ( result )
      {
        case QgsSqlExpressionCompiler::Result::Complete:
        case QgsSqlExpressionCompiler::Result::Partial:
        {
          QString filterExpr = compiler.result();
          if ( !filterExpr.isEmpty() )
          {
            sqlFilter.push_back( filterExpr );
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

  QString sql = QStringLiteral( "SELECT %1 FROM %2" ).arg(
                  sqlFields.isEmpty() ? QStringLiteral( "*" ) : sqlFields.join( ',' ),
                  mSource->mQuery );

  if ( !sqlFilter.isEmpty() )
    sql += QStringLiteral( " WHERE (%1)" ).arg( sqlFilter.join( QLatin1String( ") AND (" ) ) );

  if ( !orderByParts.isEmpty() )
    sql += QStringLiteral( " ORDER BY %1 " ).arg( orderByParts.join( ',' ) );

  if ( limitAtProvider )
    sql += QStringLiteral( " LIMIT %1" ).arg( mRequest.limit() );

  QgsDebugMsgLevel( "Query: " + sql, 4 );

  return sql;
}

QVariantList QgsHanaFeatureIterator::buildSqlQueryParameters( ) const
{
  if ( !( mFilterRect.isNull() || mFilterRect.isEmpty() ) && mSource->isSpatial() && mHasGeometryColumn )
  {
    QgsRectangle filterRect = getFilterRect();
    QString ll = QStringLiteral( "POINT(%1 %2)" ).arg( QString::number( filterRect.xMinimum() ),  QString::number( filterRect.yMinimum() ) );
    QString ur = QStringLiteral( "POINT(%1 %2)" ).arg( QString::number( filterRect.xMaximum() ),  QString::number( filterRect.yMaximum() ) );
    return { ll, mSource->mSrid, ur, mSource->mSrid };
  }
  return QVariantList();
}

QgsHanaFeatureSource::QgsHanaFeatureSource( const QgsHanaProvider *p )
  : mDatabaseVersion( p->mDatabaseVersion )
  , mUri( p->mUri )
  , mQuery( p->mQuerySource )
  , mQueryWhereClause( p->mQueryWhereClause )
  , mPrimaryKeyType( p->mPrimaryKeyType )
  , mPrimaryKeyAttrs( p->mPrimaryKeyAttrs )
  , mPrimaryKeyCntx( p->mPrimaryKeyCntx )
  , mFields( p->mFields )
  , mGeometryColumn( p->mGeometryColumn )
  , mGeometryType( p->wkbType() )
  , mSrid( p->mSrid )
  , mSrsExtent( p->mSrsExtent )
  , mCrs( p->crs() )
{
  if ( p->mHasSrsPlanarEquivalent && p->mDatabaseVersion.majorVersion() <= 1 )
    mSrid = QgsHanaUtils::toPlanarSRID( p->mSrid );
}

QgsHanaFeatureSource::~QgsHanaFeatureSource() = default;

QgsFeatureIterator QgsHanaFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsHanaFeatureIterator( this, false, request ) );
}
