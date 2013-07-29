/***************************************************************************
    qgspostgresfeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
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
#include "qgspostgresfeatureiterator.h"
#include "qgspostgresprovider.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QObject>

// provider:
// - mProviderId
// - mGeometryColumn
// - mSpatialColType
// - mRequestedSrid
// - mDetectedSrid
// - mRequestedGeomType
// - mDetectedGeomType
// - mSqlWhereClause
// - mPrimaryKeyType
// - mPrimaryKeyAttrs
// - mAttributeFields
// - mFeaturesCounted
// - field()
// - convertValue()
// - lookupFid()
// - quotedIdentifier()
// - endianString()


const int QgsPostgresFeatureIterator::sFeatureQueueSize = 2000;


QgsPostgresFeatureIterator::QgsPostgresFeatureIterator( QgsPostgresProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), P( p )
    , mFeatureQueueSize( sFeatureQueueSize )
{
  mCursorName = QString( "qgisf%1_%2" ).arg( P->mProviderId ).arg( P->mIteratorCounter++ );

  P->mActiveIterators << this;

  QString whereClause;

  if ( request.filterType() == QgsFeatureRequest::FilterRect && !P->mGeometryColumn.isNull() )
  {
    whereClause = whereClauseRect();
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    whereClause = P->whereClause( request.filterFid() );
  }

  if ( !P->mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += "(" + P->mSqlWhereClause + ")";
  }

  if ( !declareCursor( whereClause ) )
  {
    mClosed = true;
    return;
  }

  mFetched = 0;
}


QgsPostgresFeatureIterator::~QgsPostgresFeatureIterator()
{
  close();
}


bool QgsPostgresFeatureIterator::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( mFeatureQueue.empty() )
  {
    QString fetch = QString( "FETCH FORWARD %1 FROM %2" ).arg( mFeatureQueueSize ).arg( mCursorName );
    QgsDebugMsgLevel( QString( "fetching %1 features." ).arg( mFeatureQueueSize ), 4 );
    if ( P->mConnectionRO->PQsendQuery( fetch ) == 0 ) // fetch features asynchronously
    {
      QgsMessageLog::logMessage( QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( mCursorName ).arg( P->mConnectionRO->PQerrorMessage() ), QObject::tr( "PostGIS" ) );
    }

    QgsPostgresResult queryResult;
    for ( ;; )
    {
      queryResult = P->mConnectionRO->PQgetResult();
      if ( !queryResult.result() )
        break;

      if ( queryResult.PQresultStatus() != PGRES_TUPLES_OK )
      {
        QgsMessageLog::logMessage( QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( mCursorName ).arg( P->mConnectionRO->PQerrorMessage() ), QObject::tr( "PostGIS" ) );
        break;
      }

      int rows = queryResult.PQntuples();
      if ( rows == 0 )
        continue;

      for ( int row = 0; row < rows; row++ )
      {
        mFeatureQueue.enqueue( QgsFeature() );
        getFeature( queryResult, row, mFeatureQueue.back() );
      } // for each row in queue
    }
  }

  if ( mFeatureQueue.empty() )
  {
    QgsDebugMsg( QString( "Finished after %1 features" ).arg( mFetched ) );
    close();

    /* only updates the feature count if it was already once.
     * Otherwise, this would lead to false feature count if
     * an existing project is open at a restrictive extent.
     */
    if ( P->mFeaturesCounted > 0 && P->mFeaturesCounted < mFetched )
    {
      QgsDebugMsg( QString( "feature count adjusted from %1 to %2" ).arg( P->mFeaturesCounted ).arg( mFetched ) );
      P->mFeaturesCounted = mFetched;
    }
    return false;
  }

  // Now return the next feature from the queue
  if ( mRequest.flags() & QgsFeatureRequest::NoGeometry )
  {
    feature.setGeometryAndOwnership( 0, 0 );
  }
  else
  {
    QgsGeometry* featureGeom = mFeatureQueue.front().geometryAndOwnership();
    feature.setGeometry( featureGeom );
  }
  feature.setFeatureId( mFeatureQueue.front().id() );
  feature.setAttributes( mFeatureQueue.front().attributes() );

  mFeatureQueue.dequeue();
  mFetched++;

  feature.setValid( true );
  feature.setFields( &P->mAttributeFields ); // allow name-based attribute lookups
  return true;
}


bool QgsPostgresFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // move cursor to first record
  P->mConnectionRO->PQexecNR( QString( "move absolute 0 in %1" ).arg( mCursorName ) );
  mFeatureQueue.empty();
  mFetched = 0;

  return true;
}

bool QgsPostgresFeatureIterator::close()
{
  if ( mClosed )
    return false;

  P->mConnectionRO->closeCursor( mCursorName );

  while ( !mFeatureQueue.empty() )
  {
    mFeatureQueue.dequeue();
  }

  P->mActiveIterators.remove( this );

  mClosed = true;
  return true;
}

///////////////

QString QgsPostgresFeatureIterator::whereClauseRect()
{
  QgsRectangle rect = mRequest.filterRect();
  if ( P->mSpatialColType == sctGeography )
  {
    rect = QgsRectangle( -180.0, -90.0, 180.0, 90.0 ).intersect( &rect );
    if ( !rect.isFinite() )
      return "false";
  }

  QString qBox;
  if ( P->mConnectionRO->majorVersion() < 2 )
  {
    qBox = QString( "setsrid('BOX3D(%1)'::box3d,%2)" )
           .arg( rect.asWktCoordinates() )
           .arg( P->mRequestedSrid.isEmpty() ? P->mDetectedSrid : P->mRequestedSrid );
  }
  else
  {
    qBox = QString( "st_makeenvelope(%1,%2,%3,%4,%5)" )
           .arg( qgsDoubleToString( rect.xMinimum() ) )
           .arg( qgsDoubleToString( rect.yMinimum() ) )
           .arg( qgsDoubleToString( rect.xMaximum() ) )
           .arg( qgsDoubleToString( rect.yMaximum() ) )
           .arg( P->mRequestedSrid.isEmpty() ? P->mDetectedSrid : P->mRequestedSrid );
  }

  QString whereClause = QString( "%1 && %2" )
                        .arg( P->quotedIdentifier( P->mGeometryColumn ) )
                        .arg( qBox );
  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    whereClause += QString( " AND %1(%2%3,%4)" )
                   .arg( P->mConnectionRO->majorVersion() < 2 ? "intersects" : "st_intersects" )
                   .arg( P->quotedIdentifier( P->mGeometryColumn ) )
                   .arg( P->mSpatialColType == sctGeography ? "::geometry" : "" )
                   .arg( qBox );
  }

  if ( !P->mRequestedSrid.isEmpty() && ( P->mRequestedSrid != P->mDetectedSrid || P->mRequestedSrid.toInt() == 0 ) )
  {
    whereClause += QString( " AND %1(%2%3)=%4" )
                   .arg( P->mConnectionRO->majorVersion() < 2 ? "srid" : "st_srid" )
                   .arg( P->quotedIdentifier( P->mGeometryColumn ) )
                   .arg( P->mSpatialColType == sctGeography ? "::geography" : "" )
                   .arg( P->mRequestedSrid );
  }

  if ( P->mRequestedGeomType != QGis::WKBUnknown && P->mRequestedGeomType != P->mDetectedGeomType )
  {
    whereClause += QString( " AND %1" ).arg( QgsPostgresConn::postgisTypeFilter( P->mGeometryColumn, P->mRequestedGeomType, P->mSpatialColType == sctGeography ) );
  }

  return whereClause;
}



bool QgsPostgresFeatureIterator::declareCursor( const QString& whereClause )
{
  bool fetchGeometry = !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  if ( fetchGeometry && P->mGeometryColumn.isNull() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Trying to fetch geometry on a layer without geometry." ), QObject::tr( "PostgreSQL" ) );
    return false;
  }

  try
  {
    QString query = "SELECT ", delim = "";

    if ( fetchGeometry )
    {
      query += QString( "%1(%2(%3%4),'%5')" )
               .arg( P->mConnectionRO->majorVersion() < 2 ? "asbinary" : "st_asbinary" )
               .arg( P->mConnectionRO->majorVersion() < 2 ? "force_2d"
                   : P->mConnectionRO->majorVersion() > 2 || P->mConnectionRO->minorVersion() > 0 ? "ST_Force2D"
                   : "st_force_2d" )
               .arg( P->quotedIdentifier( P->mGeometryColumn ) )
               .arg( P->mSpatialColType == sctGeography ? "::geometry" : "" )
               .arg( P->endianString() );
      delim = ",";
    }

    switch ( P->mPrimaryKeyType )
    {
      case QgsPostgresProvider::pktOid:
        query += delim + "oid";
        delim = ",";
        break;

      case QgsPostgresProvider::pktTid:
        query += delim + "ctid";
        delim = ",";
        break;

      case QgsPostgresProvider::pktInt:
        query += delim + P->quotedIdentifier( P->field( P->mPrimaryKeyAttrs[0] ).name() );
        delim = ",";
        break;

      case QgsPostgresProvider::pktFidMap:
        foreach ( int idx, P->mPrimaryKeyAttrs )
        {
          query += delim + P->mConnectionRO->fieldExpression( P->field( idx ) );
          delim = ",";
        }
        break;

      case QgsPostgresProvider::pktUnknown:
        QgsDebugMsg( "Cannot declare cursor without primary key." );
        return false;
        break;
    }

    bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
    foreach ( int idx, subsetOfAttributes ? mRequest.subsetOfAttributes() : P->attributeIndexes() )
    {
      if ( P->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      query += delim + P->mConnectionRO->fieldExpression( P->field( idx ) );
    }

    query += " FROM " + P->mQuery;

    if ( !whereClause.isEmpty() )
      query += QString( " WHERE %1" ).arg( whereClause );

    if ( !P->mConnectionRO->openCursor( mCursorName, query ) )
    {
      // reloading the fields might help next time around
      rewind();
      P->loadFields();
      return false;
    }
  }
  catch ( QgsPostgresProvider::PGFieldNotFound )
  {
    rewind();
    return false;
  }

  return true;
}


bool QgsPostgresFeatureIterator::getFeature( QgsPostgresResult &queryResult, int row, QgsFeature &feature )
{
  try
  {
    feature.initAttributes( P->fields().count() );

    int col = 0;

    if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
    {
      int returnedLength = ::PQgetlength( queryResult.result(), row, col );
      if ( returnedLength > 0 )
      {
        unsigned char *featureGeom = new unsigned char[returnedLength + 1];
        memset( featureGeom, 0, returnedLength + 1 );
        memcpy( featureGeom, PQgetvalue( queryResult.result(), row, col ), returnedLength );
        feature.setGeometryAndOwnership( featureGeom, returnedLength + 1 );
      }
      else
      {
        feature.setGeometryAndOwnership( 0, 0 );
      }

      col++;
    }

    QgsFeatureId fid = 0;

    bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
    const QgsAttributeList& fetchAttributes = mRequest.subsetOfAttributes();

    switch ( P->mPrimaryKeyType )
    {
      case QgsPostgresProvider::pktOid:
      case QgsPostgresProvider::pktTid:
      case QgsPostgresProvider::pktInt:
        fid = P->mConnectionRO->getBinaryInt( queryResult, row, col++ );
        if ( P->mPrimaryKeyType == QgsPostgresProvider::pktInt &&
             ( !subsetOfAttributes || fetchAttributes.contains( P->mPrimaryKeyAttrs[0] ) ) )
          feature.setAttribute( P->mPrimaryKeyAttrs[0], fid );
        break;

      case QgsPostgresProvider::pktFidMap:
      {
        QList<QVariant> primaryKeyVals;

        foreach ( int idx, P->mPrimaryKeyAttrs )
        {
          const QgsField &fld = P->field( idx );

          QVariant v = P->convertValue( fld.type(), queryResult.PQgetvalue( row, col ) );
          primaryKeyVals << v;

          if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
            feature.setAttribute( idx, v );

          col++;
        }

        fid = P->lookupFid( QVariant( primaryKeyVals ) );
      }
      break;

      case QgsPostgresProvider::pktUnknown:
        Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
        return false;
    }

    feature.setFeatureId( fid );
    QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 4 );

    // iterate attributes
    if ( subsetOfAttributes )
    {
      foreach ( int idx, fetchAttributes )
        getFeatureAttribute( idx, queryResult, row, col, feature );
    }
    else
    {
      for ( int idx = 0; idx < P->mAttributeFields.count(); ++idx )
        getFeatureAttribute( idx, queryResult, row, col, feature );
    }

    return true;
  }
  catch ( QgsPostgresProvider::PGFieldNotFound )
  {
    return false;
  }
}

void QgsPostgresFeatureIterator::getFeatureAttribute( int idx, QgsPostgresResult& queryResult, int row, int& col, QgsFeature& feature )
{
  if ( P->mPrimaryKeyAttrs.contains( idx ) )
    return;

  QVariant v = P->convertValue( P->mAttributeFields[idx].type(), queryResult.PQgetvalue( row, col ) );
  feature.setAttribute( idx, v );

  col++;
}
