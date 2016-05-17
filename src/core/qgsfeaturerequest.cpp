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

#include "qgsfield.h"
#include "qgsgeometry.h"

#include <QStringList>

//constants
const QString QgsFeatureRequest::AllAttributes = QString( "#!allattributes!#" );

QgsFeatureRequest::QgsFeatureRequest()
    : mFilter( FilterNone )
    , mFilterFid( -1 )
    , mFilterExpression( nullptr )
    , mFlags( nullptr )
    , mLimit( -1 )
{
}

QgsFeatureRequest::QgsFeatureRequest( QgsFeatureId fid )
    : mFilter( FilterFid )
    , mFilterFid( fid )
    , mFilterExpression( nullptr )
    , mFlags( nullptr )
    , mLimit( -1 )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsRectangle& rect )
    : mFilter( FilterRect )
    , mFilterRect( rect )
    , mFilterFid( -1 )
    , mFilterExpression( nullptr )
    , mFlags( nullptr )
    , mLimit( -1 )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsExpression& expr, const QgsExpressionContext &context )
    : mFilter( FilterExpression )
    , mFilterFid( -1 )
    , mFilterExpression( new QgsExpression( expr ) )
    , mExpressionContext( context )
    , mFlags( nullptr )
    , mLimit( -1 )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsFeatureRequest &rh )
{
  operator=( rh );
}

QgsFeatureRequest& QgsFeatureRequest::operator=( const QgsFeatureRequest & rh )
{
  mFlags = rh.mFlags;
  mFilter = rh.mFilter;
  mFilterRect = rh.mFilterRect;
  mFilterFid = rh.mFilterFid;
  mFilterFids = rh.mFilterFids;
  if ( rh.mFilterExpression )
  {
    mFilterExpression = new QgsExpression( *rh.mFilterExpression );
  }
  else
  {
    mFilterExpression = nullptr;
  }
  mExpressionContext = rh.mExpressionContext;
  mAttrs = rh.mAttrs;
  mSimplifyMethod = rh.mSimplifyMethod;
  mLimit = rh.mLimit;
  mOrderBy = rh.mOrderBy;
  return *this;
}

QgsFeatureRequest::~QgsFeatureRequest()
{
  delete mFilterExpression;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterRect( const QgsRectangle& rect )
{
  if ( mFilter == FilterNone )
    mFilter = FilterRect;
  mFilterRect = rect;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterFid( QgsFeatureId fid )
{
  mFilter = FilterFid;
  mFilterFid = fid;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterFids( const QgsFeatureIds& fids )
{
  mFilter = FilterFids;
  mFilterFids = fids;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterExpression( const QString& expression )
{
  mFilter = FilterExpression;
  delete mFilterExpression;
  mFilterExpression = new QgsExpression( expression );
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::combineFilterExpression( const QString& expression )
{
  if ( mFilterExpression )
  {
    setFilterExpression( QString( "(%1) AND (%2)" ).arg( mFilterExpression->expression(), expression ) );
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

QgsFeatureRequest& QgsFeatureRequest::addOrderBy( const QString& expression, bool ascending )
{
  mOrderBy.append( OrderByClause( expression, ascending ) );
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::addOrderBy( const QString& expression, bool ascending, bool nullsfirst )
{
  mOrderBy.append( OrderByClause( expression, ascending, nullsfirst ) );
  return *this;
}

QgsFeatureRequest::OrderBy QgsFeatureRequest::orderBy() const
{
  return mOrderBy;
}

QgsFeatureRequest& QgsFeatureRequest::setOrderBy( const QgsFeatureRequest::OrderBy& orderBy )
{
  mOrderBy = orderBy;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setLimit( long limit )
{
  mLimit = limit;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFlags( const QgsFeatureRequest::Flags& flags )
{
  mFlags = flags;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QgsAttributeList& attrs )
{
  mFlags |= SubsetOfAttributes;
  mAttrs = attrs;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields )
{
  if ( attrNames.contains( QgsFeatureRequest::AllAttributes ) )
  {
    //attribute string list contains the all attributes flag, so we must fetch all attributes
    return *this;
  }

  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  Q_FOREACH ( const QString& attrName, attrNames )
  {
    int attrNum = fields.fieldNameIndex( attrName );
    if ( attrNum != -1 && !mAttrs.contains( attrNum ) )
      mAttrs.append( attrNum );
  }

  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setSimplifyMethod( const QgsSimplifyMethod& simplifyMethod )
{
  mSimplifyMethod = simplifyMethod;
  return *this;
}

bool QgsFeatureRequest::acceptFeature( const QgsFeature& feature )
{
  switch ( mFilter )
  {
    case QgsFeatureRequest::FilterNone:
      return true;

    case QgsFeatureRequest::FilterRect:
      if ( feature.constGeometry() && feature.constGeometry()->intersects( mFilterRect ) )
        return true;
      else
        return false;

    case QgsFeatureRequest::FilterFid:
      if ( feature.id() == mFilterFid )
        return true;
      else
        return false;

    case QgsFeatureRequest::FilterExpression:
      mExpressionContext.setFeature( feature );
      if ( mFilterExpression->evaluate( &mExpressionContext ).toBool() )
        return true;
      else
        return false;

    case QgsFeatureRequest::FilterFids:
      if ( mFilterFids.contains( feature.id() ) )
        return true;
      else
        return false;
  }

  return true;
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

void QgsAbstractFeatureSource::iteratorOpened( QgsAbstractFeatureIterator* it )
{
  mActiveIterators.insert( it );
}

void QgsAbstractFeatureSource::iteratorClosed( QgsAbstractFeatureIterator* it )
{
  mActiveIterators.remove( it );
}



QgsFeatureRequest::OrderByClause::OrderByClause( const QString& expression, bool ascending )
    : mExpression( expression )
    , mAscending( ascending )
{
  // postgres behavior: default for ASC: NULLS LAST, default for DESC: NULLS FIRST
  mNullsFirst = !ascending;
}

QgsFeatureRequest::OrderByClause::OrderByClause( const QString& expression, bool ascending, bool nullsfirst )
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
  return QString( "%1 %2 %3" )
         .arg( mExpression.expression(),
               mAscending ? "ASC" : "DESC",
               mNullsFirst ? "NULLS FIRST" : "NULLS LAST" );
}

QgsExpression QgsFeatureRequest::OrderByClause::expression() const
{
  return mExpression;
}

QgsFeatureRequest::OrderBy::OrderBy( const QList<QgsFeatureRequest::OrderByClause>& other )
{
  Q_FOREACH ( const QgsFeatureRequest::OrderByClause& clause, other )
  {
    append( clause );
  }
}

QList<QgsFeatureRequest::OrderByClause> QgsFeatureRequest::OrderBy::list() const
{
  return *this;
}

void QgsFeatureRequest::OrderBy::save( QDomElement& elem ) const
{
  QDomDocument doc = elem.ownerDocument();
  QList<OrderByClause>::ConstIterator it;
  for ( it = constBegin(); it != constEnd(); ++it )
  {
    const OrderByClause& clause = *it;
    QDomElement clauseElem = doc.createElement( "orderByClause" );
    clauseElem.setAttribute( "asc", clause.ascending() );
    clauseElem.setAttribute( "nullsFirst", clause.nullsFirst() );
    clauseElem.appendChild( doc.createTextNode( clause.expression().expression() ) );

    elem.appendChild( clauseElem );
  }
}

void QgsFeatureRequest::OrderBy::load( const QDomElement& elem )
{
  clear();

  QDomNodeList clauses = elem.childNodes();

  for ( int i = 0; i < clauses.size(); ++i )
  {
    QDomElement clauseElem = clauses.at( i ).toElement();
    QString expression = clauseElem.text();
    bool asc = clauseElem.attribute( "asc" ).toInt() != 0;
    bool nullsFirst  = clauseElem.attribute( "nullsFirst" ).toInt() != 0;

    append( OrderByClause( expression, asc, nullsFirst ) );
  }
}

QSet<QString> QgsFeatureRequest::OrderBy::usedAttributes() const
{
  QSet<QString> usedAttributes;

  QList<OrderByClause>::ConstIterator it;
  for ( it = constBegin(); it != constEnd(); ++it )
  {
    const OrderByClause& clause = *it;

    usedAttributes.unite( clause.expression().referencedColumns().toSet() );
  }

  return usedAttributes;
}

QString QgsFeatureRequest::OrderBy::dump() const
{
  QStringList results;

  QList<OrderByClause>::ConstIterator it;
  for ( it = constBegin(); it != constEnd(); ++it )
  {
    const OrderByClause& clause = *it;

    results << clause.dump();
  }

  return results.join( ", " );
}
