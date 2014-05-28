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


QgsMssqlFeatureIterator::QgsMssqlFeatureIterator( QgsMssqlFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource( source, ownSource, request )
{
  mClosed = false;
  mQuery = NULL;

  mParser.IsGeography = mSource->mIsGeography;

  BuildStatement( request );

  // connect to the database
  mDatabase = GetDatabase( mSource->mDriver, mSource->mHost, mSource->mDatabaseName, mSource->mUserName, mSource->mPassword );

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
      mStatement += "[" + mSource->mFields[*it].name() + "]";

      if ( !mSource->mFidColName.isEmpty() && mSource->mFidColName == mSource->mFields[*it].name() )
        mFidCol = fieldCount;

      ++fieldCount;
      mAttributesToFetch.append( *it );
    }
  }
  else
  {
    // get all attributes
    for ( int i = 0; i < mSource->mFields.count(); i++ )
    {
      if ( fieldCount != 0 )
        mStatement += ",";
      mStatement += "[" + mSource->mFields[i].name() + "]";

      if ( !mSource->mFidColName.isEmpty() && mSource->mFidColName == mSource->mFields[i].name() )
        mFidCol = fieldCount;

      ++fieldCount;
      mAttributesToFetch.append( i );
    }
  }

  // get fid col if not yet required
  if ( mFidCol == -1 && !mSource->mFidColName.isEmpty() )
  {
    if ( fieldCount != 0 )
      mStatement += ",";
    mStatement += "[" + mSource->mFidColName + "]";
    mFidCol = fieldCount;
    ++fieldCount;
  }
  // get geometry col
  if ( !( request.flags() & QgsFeatureRequest::NoGeometry ) && mSource->isSpatial() )
  {
    if ( fieldCount != 0 )
      mStatement += ",";
    mStatement += "[" + mSource->mGeometryColName + "]";
    mGeometryCol = fieldCount;
    ++fieldCount;
  }

  mStatement += " from ";
  if ( !mSource->mSchemaName.isEmpty() )
    mStatement += "[" + mSource->mSchemaName + "].";

  mStatement += "[" + mSource->mTableName + "]";

  bool filterAdded = false;
  // set spatial filter
  if ( request.filterType() & QgsFeatureRequest::FilterRect && isSpatial() )
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
  if (( request.filterType() & QgsFeatureRequest::FilterFid ) && !mSource->mFidColName.isEmpty() )
  {
    // set attribute filter
    if ( !filterAdded )
      mStatement += QString( " where [%1] = %2" ).arg( mSource->mFidColName, QString::number( request.filterFid() ) );
    else
      mStatement += QString( " and [%1] = %2" ).arg( mSource->mFidColName, QString::number( request.filterFid() ) );
    filterAdded = true;
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    if ( !filterAdded )
      mStatement += " where (" + mSource->mSqlWhereClause + ")";
    else
      mStatement += " and (" + mSource->mSqlWhereClause + ")";
  }

//  QgsDebugMsg( mStatement );
  if ( fieldCount == 0 )
  {
    QgsDebugMsg( "QgsMssqlProvider::select no fields have been requested" );
    mStatement.clear();
  }
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

    if ( mFidCol >= 0 )
    {
      feature.setFeatureId( mQuery->value( mFidCol ).toLongLong() );
    }

    if ( mGeometryCol >= 0 )
    {
      QByteArray ar = mQuery->value( mGeometryCol ).toByteArray();
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

QSqlDatabase QgsMssqlFeatureIterator::GetDatabase( QString driver, QString host, QString database, QString username, QString password )
{
  QSqlDatabase db;
  QString connectionName;

  // create a separate database connection for each feature source
  QgsDebugMsg( "Creating a separate database connection" );
  QString id;

  // QString::sprintf adds 0x prefix
  id.sprintf( "%p", this );

  if ( driver.isEmpty() )
  {
    if ( host.isEmpty() )
    {
      QgsDebugMsg( "QgsMssqlProvider host name not specified" );
      return db;
    }

    if ( database.isEmpty() )
    {
      QgsDebugMsg( "QgsMssqlProvider database name not specified" );
      return db;
    }
    connectionName = host + "." + database + "." + id;
  }
  else
    connectionName = driver;

  if ( !QSqlDatabase::contains( connectionName ) )
    db = QSqlDatabase::addDatabase( "QODBC", connectionName );
  else
    db = QSqlDatabase::database( connectionName );

  db.setHostName( host );
  QString connectionString = "";
  if ( !driver.isEmpty() )
  {
    // driver was specified explicitly
    connectionString = driver;
  }
  else
  {
#ifdef WIN32
    connectionString = "driver={SQL Server}";
#else
    connectionString = "driver={FreeTDS};port=1433";
#endif
  }

  if ( !host.isEmpty() )
    connectionString += ";server=" + host;

  if ( !database.isEmpty() )
    connectionString += ";database=" + database;

  if ( password.isEmpty() )
    connectionString += ";trusted_connection=yes";
  else
    connectionString += ";uid=" + username + ";pwd=" + password;

  if ( !username.isEmpty() )
    db.setUserName( username );

  if ( !password.isEmpty() )
    db.setPassword( password );

  db.setDatabaseName( connectionString );
  return db;
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
