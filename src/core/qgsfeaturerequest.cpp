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

#include <QStringList>

//constants
const QString QgsFeatureRequest::ALL_ATTRIBUTES = QStringLiteral( "#!allattributes!#" );

QgsFeatureRequest::QgsFeatureRequest()
  : mFlags( nullptr )
{
}

QgsFeatureRequest::QgsFeatureRequest( QgsFeatureId fid )
  : mFilter( FilterFid )
  , mFilterFid( fid )
  , mFlags( nullptr )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsFeatureIds &fids )
  : mFilter( FilterFids )
  , mFilterFids( fids )
  , mFlags( nullptr )
{

}

QgsFeatureRequest::QgsFeatureRequest( const QgsRectangle &rect )
  : mFilterRect( rect )
  , mFlags( nullptr )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsExpression &expr, const QgsExpressionContext &context )
  : mFilter( FilterExpression )
  , mFilterExpression( new QgsExpression( expr ) )
  , mExpressionContext( context )
  , mFlags( nullptr )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsFeatureRequest &rh )
{
  operator=( rh );
}

QgsFeatureRequest &QgsFeatureRequest::operator=( const QgsFeatureRequest &rh )
{
  mFlags = rh.mFlags;
  mFilter = rh.mFilter;
  mFilterRect = rh.mFilterRect;
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
  mTransformErrorCallback = rh.mTransformErrorCallback;
  mConnectionTimeout = rh.mConnectionTimeout;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setFilterRect( const QgsRectangle &rect )
{
  mFilterRect = rect;
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

QgsFeatureRequest &QgsFeatureRequest::setLimit( long limit )
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

QgsFeatureRequest &QgsFeatureRequest::setSubsetOfAttributes( const QStringList &attrNames, const QgsFields &fields )
{
  if ( attrNames.contains( QgsFeatureRequest::ALL_ATTRIBUTES ) )
  {
    //attribute string list contains the all attributes flag, so we must fetch all attributes
    return *this;
  }

  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  Q_FOREACH ( const QString &attrName, attrNames )
  {
    int attrNum = fields.lookupField( attrName );
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

  Q_FOREACH ( const QString &attrName, attrNames )
  {
    int attrNum = fields.lookupField( attrName );
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

QgsFeatureRequest &QgsFeatureRequest::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setTransformErrorCallback( const std::function<void ( const QgsFeature & )> &callback )
{
  mTransformErrorCallback = callback;
  return *this;
}

bool QgsFeatureRequest::acceptFeature( const QgsFeature &feature )
{
  if ( !mFilterRect.isNull() )
  {
    if ( !feature.hasGeometry() || !feature.geometry().intersects( mFilterRect ) )
      return false;
  }

  switch ( mFilter )
  {
    case QgsFeatureRequest::FilterNone:
      return true;

    case QgsFeatureRequest::FilterFid:
      return ( feature.id() == mFilterFid );

    case QgsFeatureRequest::FilterExpression:
      mExpressionContext.setFeature( feature );
      return ( mFilterExpression->evaluate( &mExpressionContext ).toBool() );

    case QgsFeatureRequest::FilterFids:
      return ( mFilterFids.contains( feature.id() ) );
  }

  return true;
}

int QgsFeatureRequest::connectionTimeout() const
{
  return mConnectionTimeout;
}

void QgsFeatureRequest::setConnectionTimeout( int connectionTimeout )
{
  mConnectionTimeout = connectionTimeout;
}


#include "qgsfeatureiterator.h"
#include "qgslogger.h"

QgsAbstractFeatureSource::~QgsAbstractFeatureSource()
{
  while ( !mActiveIterators.empty() )
  {
    QgsAbstractFeatureIterator *it = *mActiveIterators.begin();
    QgsDebugMsg( "closing active iterator" );
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

QgsFeatureRequest::OrderBy::OrderBy( const QList<QgsFeatureRequest::OrderByClause> &other )
{
  Q_FOREACH ( const QgsFeatureRequest::OrderByClause &clause, other )
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

  QDomNodeList clauses = elem.childNodes();

  for ( int i = 0; i < clauses.size(); ++i )
  {
    QDomElement clauseElem = clauses.at( i ).toElement();
    QString expression = clauseElem.text();
    bool asc = clauseElem.attribute( QStringLiteral( "asc" ) ).toInt() != 0;
    bool nullsFirst  = clauseElem.attribute( QStringLiteral( "nullsFirst" ) ).toInt() != 0;

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

QString QgsFeatureRequest::OrderBy::dump() const
{
  QStringList results;

  QList<OrderByClause>::ConstIterator it;
  for ( it = constBegin(); it != constEnd(); ++it )
  {
    const OrderByClause &clause = *it;

    results << clause.dump();
  }

  return results.join( QStringLiteral( ", " ) );
}
