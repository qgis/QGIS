/***************************************************************************
    qgsfeaturerequest.cpp
    ---------------------
    begin                : Mai 2012
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
#include "qgsfeaturerequest.h"

#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"

#include <QStringList>

//constants
const QString QgsFeatureRequest::ALL_ATTRIBUTES = QStringLiteral( "#!allattributes!#" );

QgsFeatureRequest::QgsFeatureRequest()
{
}

QgsFeatureRequest::~QgsFeatureRequest() = default;

QgsFeatureRequest::QgsFeatureRequest( QgsFeatureId fid )
  : mFilter( FilterFid )
  , mFilterFid( fid )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsFeatureIds &fids )
  : mFilter( FilterFids )
  , mFilterFids( fids )
{

}

QgsFeatureRequest::QgsFeatureRequest( const QgsRectangle &rect )
  : mSpatialFilter( !rect.isNull() ? Qgis::SpatialFilterType::BoundingBox : Qgis::SpatialFilterType::NoFilter )
  , mFilterRect( rect )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsExpression &expr, const QgsExpressionContext &context )
  : mFilter( FilterExpression )
  , mFilterExpression( new QgsExpression( expr ) )
  , mExpressionContext( context )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsFeatureRequest &rh )
{
  operator=( rh );
}

QgsFeatureRequest &QgsFeatureRequest::operator=( const QgsFeatureRequest &rh )
{
  if ( &rh == this )
    return *this;

  mFlags = rh.mFlags;
  mFilter = rh.mFilter;
  mSpatialFilter = rh.mSpatialFilter;
  mFilterRect = rh.mFilterRect;
  mReferenceGeometry = rh.mReferenceGeometry;
  mReferenceGeometryEngine = rh.mReferenceGeometryEngine;
  mDistanceWithin = rh.mDistanceWithin;
  mFilterFid = rh.mFilterFid;
  mFilterFids = rh.mFilterFids;
  if ( rh.mFilterExpression )
  {
    mFilterExpression.reset( new QgsExpression( *rh.mFilterExpression ) );
  }
  else
  {
    mFilterExpression.reset( nullptr );
  }
  mInvalidGeometryFilter = rh.mInvalidGeometryFilter;
  mInvalidGeometryCallback = rh.mInvalidGeometryCallback;
  mExpressionContext = rh.mExpressionContext;
  mAttrs = rh.mAttrs;
  mSimplifyMethod = rh.mSimplifyMethod;
  mLimit = rh.mLimit;
  mOrderBy = rh.mOrderBy;
  mCrs = rh.mCrs;
  mTransformContext = rh.mTransformContext;
  mTransformErrorCallback = rh.mTransformErrorCallback;
  mTimeout = rh.mTimeout;
  mRequestMayBeNested = rh.mRequestMayBeNested;
  mFeedback = rh.mFeedback;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setFilterRect( const QgsRectangle &rect )
{
  mFilterRect = rect;
  mReferenceGeometry = QgsGeometry();
  mDistanceWithin = 0;
  if ( mFilterRect.isNull() )
  {
    mSpatialFilter = Qgis::SpatialFilterType::NoFilter;
  }
  else
  {
    mSpatialFilter = Qgis::SpatialFilterType::BoundingBox;
  }
  return *this;
}

QgsRectangle QgsFeatureRequest::filterRect() const
{
  return mFilterRect;
}

QgsFeatureRequest &QgsFeatureRequest::setDistanceWithin( const QgsGeometry &geometry, double distance )
{
  mReferenceGeometry = geometry;
  if ( !mReferenceGeometry.isEmpty() )
  {
    mReferenceGeometryEngine.reset( QgsGeometry::createGeometryEngine( mReferenceGeometry.constGet() ) );
    mReferenceGeometryEngine->prepareGeometry();
  }
  else
  {
    mReferenceGeometryEngine.reset();
  }
  mDistanceWithin = distance;
  mSpatialFilter = Qgis::SpatialFilterType::DistanceWithin;
  mFilterRect = mReferenceGeometry.boundingBox().buffered( mDistanceWithin );

  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setFilterFid( QgsFeatureId fid )
{
  mFilter = FilterFid;
  mFilterFid = fid;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setFilterFids( const QgsFeatureIds &fids )
{
  mFilter = FilterFids;
  mFilterFids = fids;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setInvalidGeometryCheck( QgsFeatureRequest::InvalidGeometryCheck check )
{
  mInvalidGeometryFilter = check;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setInvalidGeometryCallback( const std::function<void ( const QgsFeature & )> &callback )
{
  mInvalidGeometryCallback = callback;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setFilterExpression( const QString &expression )
{
  mFilter = FilterExpression;
  mFilterExpression.reset( new QgsExpression( expression ) );
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::combineFilterExpression( const QString &expression )
{
  if ( mFilterExpression )
  {
    setFilterExpression( QStringLiteral( "(%1) AND (%2)" ).arg( mFilterExpression->expression(), expression ) );
  }
  else
  {
    setFilterExpression( expression );
  }
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::addOrderBy( const QString &expression, bool ascending )
{
  mOrderBy.append( OrderByClause( expression, ascending ) );
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::addOrderBy( const QString &expression, bool ascending, bool nullsfirst )
{
  mOrderBy.append( OrderByClause( expression, ascending, nullsfirst ) );
  return *this;
}

QgsFeatureRequest::OrderBy QgsFeatureRequest::orderBy() const
{
  return mOrderBy;
}

QgsFeatureRequest &QgsFeatureRequest::setOrderBy( const QgsFeatureRequest::OrderBy &orderBy )
{
  mOrderBy = orderBy;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setLimit( long long limit )
{
  mLimit = limit;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setFlags( QgsFeatureRequest::Flags flags )
{
  mFlags = flags;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setSubsetOfAttributes( const QgsAttributeList &attrs )
{
  mFlags |= SubsetOfAttributes;
  mAttrs = attrs;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setNoAttributes()
{
  return setSubsetOfAttributes( QgsAttributeList() );
}

QgsFeatureRequest &QgsFeatureRequest::setSubsetOfAttributes( const QStringList &attrNames, const QgsFields &fields )
{
  if ( attrNames.contains( QgsFeatureRequest::ALL_ATTRIBUTES ) )
  {
    //attribute string list contains the all attributes flag, so we must fetch all attributes
    return *this;
  }

  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  const auto constAttrNames = attrNames;
  for ( const QString &attrName : constAttrNames )
  {
    const int attrNum = fields.lookupField( attrName );
    if ( attrNum != -1 && !mAttrs.contains( attrNum ) )
      mAttrs.append( attrNum );
  }

  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setSubsetOfAttributes( const QSet<QString> &attrNames, const QgsFields &fields )
{
  if ( attrNames.contains( QgsFeatureRequest::ALL_ATTRIBUTES ) )
  {
    //attribute string list contains the all attributes flag, so we must fetch all attributes
    return *this;
  }

  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  const auto constAttrNames = attrNames;
  for ( const QString &attrName : constAttrNames )
  {
    const int attrNum = fields.lookupField( attrName );
    if ( attrNum != -1 && !mAttrs.contains( attrNum ) )
      mAttrs.append( attrNum );
  }

  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setSimplifyMethod( const QgsSimplifyMethod &simplifyMethod )
{
  mSimplifyMethod = simplifyMethod;
  return *this;
}


QgsCoordinateReferenceSystem QgsFeatureRequest::destinationCrs() const
{
  return mCrs;
}

QgsCoordinateTransformContext QgsFeatureRequest::transformContext() const
{
  return mTransformContext;
}

QgsFeatureRequest &QgsFeatureRequest::setDestinationCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setTransformErrorCallback( const std::function<void ( const QgsFeature & )> &callback )
{
  mTransformErrorCallback = callback;
  return *this;
}

bool QgsFeatureRequest::acceptFeature( const QgsFeature &feature )
{
  // check the attribute/id filter first, it's more likely to be faster than
  // the spatial filter
  switch ( mFilter )
  {
    case QgsFeatureRequest::FilterNone:
      break;

    case QgsFeatureRequest::FilterFid:
      if ( feature.id() != mFilterFid )
        return false;
      break;

    case QgsFeatureRequest::FilterExpression:
      mExpressionContext.setFeature( feature );
      if ( !mFilterExpression->evaluate( &mExpressionContext ).toBool() )
        return false;
      break;

    case QgsFeatureRequest::FilterFids:
      if ( !mFilterFids.contains( feature.id() ) )
        return false;
      break;
  }

  switch ( mSpatialFilter )
  {
    case Qgis::SpatialFilterType::NoFilter:
      break;

    case Qgis::SpatialFilterType::BoundingBox:
      if ( !feature.hasGeometry() ||
           (
             ( mFlags & ExactIntersect && !feature.geometry().intersects( mFilterRect ) )
             ||
             ( !( mFlags & ExactIntersect ) && !feature.geometry().boundingBoxIntersects( mFilterRect ) )
           )
         )
        return false;
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      if ( !feature.hasGeometry()
           || !mReferenceGeometryEngine
           || !feature.geometry().boundingBoxIntersects( mFilterRect )
           || !mReferenceGeometryEngine->distanceWithin( feature.geometry().constGet(), mDistanceWithin )
         )
        return false;
      break;
  }

  return true;
}

int QgsFeatureRequest::connectionTimeout() const
{
  return mTimeout;
}

QgsFeatureRequest &QgsFeatureRequest::setConnectionTimeout( int connectionTimeout )
{
  mTimeout = connectionTimeout;
  return *this;
}

int QgsFeatureRequest::timeout() const
{
  return mTimeout;
}

QgsFeatureRequest &QgsFeatureRequest::setTimeout( int timeout )
{
  mTimeout = timeout;
  return *this;
}

bool QgsFeatureRequest::requestMayBeNested() const
{
  return mRequestMayBeNested;
}

QgsFeatureRequest &QgsFeatureRequest::setRequestMayBeNested( bool requestMayBeNested )
{
  mRequestMayBeNested = requestMayBeNested;
  return *this;
}

void QgsFeatureRequest::setFeedback( QgsFeedback *feedback )
{
  mFeedback = feedback;
}

QgsFeedback *QgsFeatureRequest::feedback() const
{
  return mFeedback;
}


#include "qgsfeatureiterator.h"
#include "qgslogger.h"

QgsAbstractFeatureSource::~QgsAbstractFeatureSource()
{
  while ( !mActiveIterators.empty() )
  {
    QgsAbstractFeatureIterator *it = *mActiveIterators.begin();
    QgsDebugMsgLevel( QStringLiteral( "closing active iterator" ), 2 );
    it->close();
  }
}

void QgsAbstractFeatureSource::iteratorOpened( QgsAbstractFeatureIterator *it )
{
  mActiveIterators.insert( it );
}

void QgsAbstractFeatureSource::iteratorClosed( QgsAbstractFeatureIterator *it )
{
  mActiveIterators.remove( it );
}



QgsFeatureRequest::OrderByClause::OrderByClause( const QString &expression, bool ascending )
  : mExpression( expression )
  , mAscending( ascending )
{
  // postgres behavior: default for ASC: NULLS LAST, default for DESC: NULLS FIRST
  mNullsFirst = !ascending;
}

QgsFeatureRequest::OrderByClause::OrderByClause( const QString &expression, bool ascending, bool nullsfirst )
  : mExpression( expression )
  , mAscending( ascending )
  , mNullsFirst( nullsfirst )
{
}

QgsFeatureRequest::OrderByClause::OrderByClause( const QgsExpression &expression, bool ascending )
  : mExpression( expression )
  , mAscending( ascending )
{
  // postgres behavior: default for ASC: NULLS LAST, default for DESC: NULLS FIRST
  mNullsFirst = !ascending;
}

QgsFeatureRequest::OrderByClause::OrderByClause( const QgsExpression &expression, bool ascending, bool nullsfirst )
  : mExpression( expression )
  , mAscending( ascending )
  , mNullsFirst( nullsfirst )
{

}

bool QgsFeatureRequest::OrderByClause::ascending() const
{
  return mAscending;
}

void QgsFeatureRequest::OrderByClause::setAscending( bool ascending )
{
  mAscending = ascending;
}

bool QgsFeatureRequest::OrderByClause::nullsFirst() const
{
  return mNullsFirst;
}

void QgsFeatureRequest::OrderByClause::setNullsFirst( bool nullsFirst )
{
  mNullsFirst = nullsFirst;
}

QString QgsFeatureRequest::OrderByClause::dump() const
{
  return QStringLiteral( "%1 %2 %3" )
         .arg( mExpression.expression(),
               mAscending ? "ASC" : "DESC",
               mNullsFirst ? "NULLS FIRST" : "NULLS LAST" );
}

QgsExpression QgsFeatureRequest::OrderByClause::expression() const
{
  return mExpression;
}

bool QgsFeatureRequest::OrderByClause::prepare( QgsExpressionContext *context )
{
  return mExpression.prepare( context );
}

QgsFeatureRequest::OrderBy::OrderBy() = default;

QgsFeatureRequest::OrderBy::OrderBy( const QList<QgsFeatureRequest::OrderByClause> &other )
{
  const auto constOther = other;
  for ( const QgsFeatureRequest::OrderByClause &clause : constOther )
  {
    append( clause );
  }
}

QList<QgsFeatureRequest::OrderByClause> QgsFeatureRequest::OrderBy::list() const
{
  return *this;
}

void QgsFeatureRequest::OrderBy::save( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();
  QList<OrderByClause>::ConstIterator it;
  for ( it = constBegin(); it != constEnd(); ++it )
  {
    const OrderByClause &clause = *it;
    QDomElement clauseElem = doc.createElement( QStringLiteral( "orderByClause" ) );
    clauseElem.setAttribute( QStringLiteral( "asc" ), clause.ascending() );
    clauseElem.setAttribute( QStringLiteral( "nullsFirst" ), clause.nullsFirst() );
    clauseElem.appendChild( doc.createTextNode( clause.expression().expression() ) );

    elem.appendChild( clauseElem );
  }
}

void QgsFeatureRequest::OrderBy::load( const QDomElement &elem )
{
  clear();

  const QDomNodeList clauses = elem.childNodes();

  for ( int i = 0; i < clauses.size(); ++i )
  {
    const QDomElement clauseElem = clauses.at( i ).toElement();
    const QString expression = clauseElem.text();
    const bool asc = clauseElem.attribute( QStringLiteral( "asc" ) ).toInt() != 0;
    const bool nullsFirst  = clauseElem.attribute( QStringLiteral( "nullsFirst" ) ).toInt() != 0;

    append( OrderByClause( expression, asc, nullsFirst ) );
  }
}

QSet<QString> QgsFeatureRequest::OrderBy::usedAttributes() const
{
  QSet<QString> usedAttributes;

  QList<OrderByClause>::ConstIterator it;
  for ( it = constBegin(); it != constEnd(); ++it )
  {
    const OrderByClause &clause = *it;

    usedAttributes.unite( clause.expression().referencedColumns() );
  }

  return usedAttributes;
}

QSet<int> QgsFeatureRequest::OrderBy::usedAttributeIndices( const QgsFields &fields ) const
{
  QSet<int> usedAttributeIdx;
  for ( const OrderByClause &clause : *this )
  {
    const auto referencedColumns = clause.expression().referencedColumns();
    for ( const QString &fieldName : referencedColumns )
    {
      const int idx = fields.lookupField( fieldName );
      if ( idx >= 0 )
      {
        usedAttributeIdx.insert( idx );
      }
    }
  }
  return usedAttributeIdx;
}

QString QgsFeatureRequest::OrderBy::dump() const
{
  QStringList results;

  QList<OrderByClause>::ConstIterator it;
  for ( it = constBegin(); it != constEnd(); ++it )
  {
    const OrderByClause &clause = *it;

    results << clause.dump();
  }

  return results.join( QLatin1String( ", " ) );
}
