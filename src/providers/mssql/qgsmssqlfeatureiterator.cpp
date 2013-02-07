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
#include "qgsmssqlprovider.h"
#include "qgslogger.h"

#include <QObject>
#include <QTextStream>


QgsMssqlFeatureIterator::QgsMssqlFeatureIterator( QgsMssqlProvider* provider, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), mProvider( provider )
{
  mIsOpen = false;
  BuildStatement( request );

  mQuery = NULL;

  if ( mProvider->mQuery.isActive() )
  {
    mUseProviderQuery = false;
    // create a separate database connection if the default query is active
    QgsDebugMsg( "Creating a separate database connection" );
    QString id;
    // QString::sprintf adds 0x prefix
    id.sprintf( "%08p", this );
    mDatabase = mProvider->mDatabase.cloneDatabase( mProvider->mDatabase, id );
    if ( !mDatabase.open() )
    {
      QgsDebugMsg( "Failed to open database" );
      QString msg = mDatabase.lastError().text();
      QgsDebugMsg( msg );
      return;
    }
    // create sql query
    mQuery = new QSqlQuery( mDatabase );
  }
  else
  {
    mUseProviderQuery = true;
    mQuery = &mProvider->mQuery;
  }
  // start selection
  rewind();
}


QgsMssqlFeatureIterator::~QgsMssqlFeatureIterator()
{
  if ( !mUseProviderQuery )
  {
    if ( mQuery )
      delete mQuery;
    mDatabase.close();
  }
  else if ( mIsOpen )
    close();
}

void QgsMssqlFeatureIterator::BuildStatement( const QgsFeatureRequest& request )
{
  // build sql statement
  mStatement = QString( "select " );
  int fieldCount = 0;
  mFidCol = -1;
  mGeometryCol = -1;

  if ( !request.subsetOfAttributes().empty() )
  {
    // subset of attributes has been specified
    for ( QgsAttributeList::const_iterator it = request.subsetOfAttributes().begin(); it != request.subsetOfAttributes().end(); ++it )
    {
      if ( fieldCount != 0 )
        mStatement += ",";
      mStatement += "[" + mProvider->mAttributeFields[*it].name() + "]";

      if ( !mProvider->mFidColName.isEmpty() && mProvider->mFidColName == mProvider->mAttributeFields[*it].name() )
        mFidCol = fieldCount;

      ++fieldCount;
      mAttributesToFetch.append( *it );
    }
  }
  else
  {
    // get all attributes
    for ( int i = 0; i < mProvider->mAttributeFields.count(); i++ )
    {
      if ( fieldCount != 0 )
        mStatement += ",";
      mStatement += "[" + mProvider->mAttributeFields[i].name() + "]";

      if ( !mProvider->mFidColName.isEmpty() && mProvider->mFidColName == mProvider->mAttributeFields[i].name() )
        mFidCol = fieldCount;

      ++fieldCount;
      mAttributesToFetch.append( i );
    }
  }
  // get fid col if not yet required
  if ( mFidCol == -1 && !mProvider->mFidColName.isEmpty() )
  {
    if ( fieldCount != 0 )
      mStatement += ",";
    mStatement += "[" + mProvider->mFidColName + "]";
    mFidCol = fieldCount;
    ++fieldCount;
  }
  // get geometry col
  if ( request.flags() != QgsFeatureRequest::NoGeometry && !mProvider->mGeometryColName.isEmpty() )
  {
    if ( fieldCount != 0 )
      mStatement += ",";
    mStatement += "[" + mProvider->mGeometryColName + "]";
    mGeometryCol = fieldCount;
    ++fieldCount;
  }

  mStatement += " from ";
  if ( !mProvider->mSchemaName.isEmpty() )
    mStatement += "[" + mProvider->mSchemaName + "].";

  mStatement += "[" + mProvider->mTableName + "]";

  bool filterAdded = false;
  // set spatial filter
  if ( request.filterType() & QgsFeatureRequest::FilterRect )
  {
    // polygons should be CCW for SqlGeography
    QString r;
    QTextStream foo( &r );

    foo.setRealNumberPrecision( 8 );
    foo.setRealNumberNotation( QTextStream::FixedNotation );
    foo <<  request.filterRect().xMinimum() << " " <<  request.filterRect().yMinimum() << ", "
    <<  request.filterRect().xMaximum() << " " <<  request.filterRect().yMinimum() << ", "
    <<  request.filterRect().xMaximum() << " " <<  request.filterRect().yMaximum() << ", "
    <<  request.filterRect().xMinimum() << " " <<  request.filterRect().yMaximum() << ", "
    <<  request.filterRect().xMinimum() << " " <<  request.filterRect().yMinimum();

    mStatement += QString( " where [%1].STIntersects([%2]::STGeomFromText('POLYGON((%3))',%4)) = 1" ).arg(
                    mProvider->mGeometryColName, mProvider->mGeometryColType, r, QString::number( mProvider->mSRId ) );
    filterAdded = true;
  }

  // set fid filter
  if (( request.filterType() & QgsFeatureRequest::FilterFid ) && !mProvider->mFidColName.isEmpty() )
  {
    // set attribute filter
    if ( !filterAdded )
      mStatement += QString( " where [%1] = %2" ).arg( mProvider->mFidColName, QString::number( request.filterFid() ) );
    else
      mStatement += QString( " and [%1] = %2" ).arg( mProvider->mFidColName, QString::number( request.filterFid() ) );
    filterAdded = true;
  }

  if ( !mProvider->mSqlWhereClause.isEmpty() )
  {
    if ( !filterAdded )
      mStatement += " where (" + mProvider->mSqlWhereClause + ")";
    else
      mStatement += " and (" + mProvider->mSqlWhereClause + ")";
    filterAdded = true;
  }

  if ( fieldCount == 0 )
  {
    QgsDebugMsg( "QgsMssqlProvider::select no fields have been requested" );
    mStatement.clear();
  }
}


bool QgsMssqlFeatureIterator::nextFeature( QgsFeature& feature )
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
    feature.initAttributes( mProvider->mAttributeFields.count() );
    feature.setFields( &mProvider->mAttributeFields ); // allow name-based attribute lookups

    for ( int i = 0; i < mAttributesToFetch.count(); i++ )
    {
      QVariant v = mQuery->value( i );
      feature.setAttribute( mAttributesToFetch[i], mQuery->value( i ) );
    }

    if ( mFidCol >= 0 )
    {
      feature.setFeatureId( mQuery->value( mFidCol ).toLongLong() );
    }

    if ( mGeometryCol >= 0 )
    {
      QByteArray ar = mQuery->value( mGeometryCol ).toByteArray();
      unsigned char* wkb = mProvider->parser.ParseSqlGeometry(( unsigned char* )ar.data(), ar.size() );
      if ( wkb )
      {
        feature.setGeometryAndOwnership( wkb, mProvider->parser.GetWkbLen() );
      }
    }

    feature.setValid( true );
    return true;
  }
  return false;
}


bool QgsMssqlFeatureIterator::rewind()
{
  if ( mStatement.isEmpty() )
  {
    QgsDebugMsg( "QgsMssqlFeatureIterator::rewind on empty statement" );
    return false;
  }

  if ( !mQuery )
    return false;

  mQuery->clear();
  mQuery->setForwardOnly( true );
  if ( !mQuery->exec( mStatement ) )
  {
    QString msg = mQuery->lastError().text();
    QgsDebugMsg( msg );
  }
  else
    mIsOpen = true;

  return true;
}

bool QgsMssqlFeatureIterator::close()
{
  mIsOpen = false;
  if ( !mQuery )
    return false;

  if ( !mQuery->isActive() )
  {
    QgsDebugMsg( "QgsMssqlFeatureIterator::close on inactive query" );
    return false;
  }

  mQuery->finish();
  return true;
}

///////////////

