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
#include <QSqlRecord>


QgsMssqlFeatureIterator::QgsMssqlFeatureIterator( QgsMssqlFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsMssqlFeatureSource>( source, ownSource, request )
{
  mClosed = false;
  mQuery = NULL;

  mParser.IsGeography = mSource->mIsGeography;

  BuildStatement( request );

  // connect to the database
  mDatabase = QgsMssqlProvider::GetDatabase( mSource->mService, mSource->mHost, mSource->mDatabaseName, mSource->mUserName, mSource->mPassword );

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


QgsMssqlFeatureIterator::~QgsMssqlFeatureIterator()
{
  close();
}

void QgsMssqlFeatureIterator::BuildStatement( const QgsFeatureRequest& request )
{
  // build sql statement
  mStatement = QString( "SELECT " );

  mStatement += QString( "[%1]" ).arg( mSource->mFidColName );
  mFidCol = mSource->mFields.indexFromName( mSource->mFidColName );
  mAttributesToFetch.append( mFidCol );

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  foreach ( int i, subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList() )
  {
    QString fieldname = mSource->mFields[i].name();
    if ( mSource->mFidColName == fieldname )
      continue;

    mStatement += QString( ",[%1]" ).arg( fieldname );

    mAttributesToFetch.append( i );
  }

  // get geometry col
  if ( !( request.flags() & QgsFeatureRequest::NoGeometry ) && mSource->isSpatial() )
  {
    mStatement += QString( ",[%1]" ).arg( mSource->mGeometryColName );
  }

  mStatement += QString( "FROM [%1].[%2]" ).arg( mSource->mSchemaName, mSource->mTableName );

  bool filterAdded = false;
  // set spatial filter
  if ( request.filterType() == QgsFeatureRequest::FilterRect && mSource->isSpatial() && !request.filterRect().isEmpty() )
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
                    mSource->mGeometryColName, mSource->mGeometryColType, r, QString::number( mSource->mSRId ) );
    filterAdded = true;
  }

  // set fid filter
  if ( request.filterType() == QgsFeatureRequest::FilterFid && !mSource->mFidColName.isEmpty() )
  {
    QString fidfilter = QString( " [%1] = %2" ).arg( mSource->mFidColName, QString::number( request.filterFid() ) );
    // set attribute filter
    if ( !filterAdded )
      mStatement += " WHERE ";
    else
      mStatement += " AND ";

    mStatement += fidfilter;
    filterAdded = true;
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    if ( !filterAdded )
      mStatement += " WHERE (" + mSource->mSqlWhereClause + ")";
    else
      mStatement += " AND (" + mSource->mSqlWhereClause + ")";
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
    feature.setFields( &mSource->mFields ); // allow name-based attribute lookups

    for ( int i = 0; i < mAttributesToFetch.count(); i++ )
    {
      QVariant v = mQuery->value( i );
      feature.setAttribute( mAttributesToFetch[i], mQuery->value( i ) );
    }

    feature.setFeatureId( mQuery->record().value( mSource->mFidColName ).toLongLong() );

    if ( mSource->isSpatial() )
    {
      QByteArray ar = mQuery->record().value( mSource->mGeometryColName ).toByteArray();
      unsigned char* wkb = mParser.ParseSqlGeometry(( unsigned char* )ar.data(), ar.size() );
      if ( wkb )
      {
        feature.setGeometryAndOwnership( wkb, mParser.GetWkbLen() );
      }
    }

    feature.setValid( true );
    return true;
  }
  return false;
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
  if ( !mQuery->exec( mStatement ) )
  {
    QString msg = mQuery->lastError().text();
    QgsDebugMsg( msg );
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
    delete mQuery;

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
