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
  else if ( request.filterType() == QgsFeatureRequest::FilterFids )
  {
    whereClause = P->whereClause( request.filterFids() );
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


bool QgsPostgresFeatureIterator::fetchFeature( QgsFeature& feature )
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
  if ( !mFetchGeometry )
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

bool QgsPostgresFeatureIterator::prepareSimplification( const QgsSimplifyMethod& simplifyMethod )
{
  // setup simplification of geometries to fetch
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification && !simplifyMethod.forceLocalOptimization() )
  {
    QgsSimplifyMethod::MethodType methodType = simplifyMethod.methodType();

    if ( methodType == QgsSimplifyMethod::OptimizeForRendering || methodType == QgsSimplifyMethod::PreserveTopology )
    {
      return true;
    }
    else
    {
      QgsDebugMsg( QString( "Simplification method type (%1) is not recognised by PostgresFeatureIterator" ).arg( methodType ) );
    }
  }
  return QgsAbstractFeatureIterator::prepareSimplification( simplifyMethod );
}

bool QgsPostgresFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  return methodType == QgsSimplifyMethod::OptimizeForRendering || methodType == QgsSimplifyMethod::PreserveTopology;
}

bool QgsPostgresFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // move cursor to first record
  P->mConnectionRO->PQexecNR( QString( "move absolute 0 in %1" ).arg( mCursorName ) );
  mFeatureQueue.clear();
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
  }

  if ( !rect.isFinite() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Infinite filter rectangle specified" ), QObject::tr( "PostGIS" ) );
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
  mFetchGeometry = !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && !P->mGeometryColumn.isNull();

  try
  {
    const QgsSimplifyMethod& simplifyMethod = mRequest.simplifyMethod();

    QString query = "SELECT ", delim = "";

    if ( mFetchGeometry && !simplifyMethod.forceLocalOptimization() && simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification && QGis::flatType( QGis::singleType( P->geometryType() ) ) != QGis::WKBPoint )
    {
      QString simplifyFunctionName = simplifyMethod.methodType() == QgsSimplifyMethod::OptimizeForRendering
                                     ? ( P->mConnectionRO->majorVersion() < 2 ? "simplify" : "st_simplify" )
                                         : ( P->mConnectionRO->majorVersion() < 2 ? "simplifypreservetopology" : "st_simplifypreservetopology" );

      double tolerance = simplifyMethod.methodType() == QgsSimplifyMethod::OptimizeForRendering
                         ? simplifyMethod.toleranceForDouglasPeuckerAlgorithms()
                         : simplifyMethod.tolerance();

      query += QString( "%1(%5(%2%3,%6),'%4')" )
               .arg( P->mConnectionRO->majorVersion() < 2 ? "asbinary" : "st_asbinary" )
               .arg( P->quotedIdentifier( P->mGeometryColumn ) )
               .arg( P->mSpatialColType == sctGeography ? "::geometry" : "" )
               .arg( P->endianString() )
               .arg( simplifyFunctionName )
               .arg( tolerance );
      delim = ",";
    }
    else if ( mFetchGeometry )
    {
      query += QString( "%1(%2%3,'%4')" )
               .arg( P->mConnectionRO->majorVersion() < 2 ? "asbinary" : "st_asbinary" )
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

    if ( mFetchGeometry )
    {
      int returnedLength = ::PQgetlength( queryResult.result(), row, col );
      if ( returnedLength > 0 )
      {
        unsigned char *featureGeom = new unsigned char[returnedLength + 1];
        memcpy( featureGeom, PQgetvalue( queryResult.result(), row, col ), returnedLength );
        memset( featureGeom + returnedLength, 0, 1 );

        // modify 2.5D WKB types to make them compliant with OGR
        unsigned int wkbType;
        memcpy( &wkbType, featureGeom + 1, sizeof( wkbType ) );

        // convert unsupported types to supported ones
        switch ( wkbType )
        {
          case 15:
            // 2D polyhedral => multipolygon
            wkbType = 6;
            break;
          case 1015:
            // 3D polyhedral => multipolygon
            wkbType = 1006;
            break;
          case 17:
            // 2D triangle => polygon
            wkbType = 3;
            break;
          case 1017:
            // 3D triangle => polygon
            wkbType = 1003;
            break;
          case 16:
            // 2D TIN => multipolygon
            wkbType = 6;
            break;
          case 1016:
            // TIN => multipolygon
            wkbType = 1006;
            break;
        }
        // convert from postgis types to qgis types
        if ( wkbType >= 1000 )
        {
          wkbType = wkbType - 1000 + QGis::WKBPoint25D - 1;
        }
        memcpy( featureGeom + 1, &wkbType, sizeof( wkbType ) );

        // change wkb type of inner geometries
        if ( wkbType == QGis::WKBMultiPoint25D ||
             wkbType == QGis::WKBMultiLineString25D ||
             wkbType == QGis::WKBMultiPolygon25D )
        {
          unsigned int numGeoms = *(( int* )( featureGeom + 5 ) );
          unsigned char* wkb = featureGeom + 9;
          for ( unsigned int i = 0; i < numGeoms; ++i )
          {
            unsigned int localType;
            memcpy( &localType, wkb + 1, sizeof( localType ) );
            switch ( localType )
            {
              case 15:
                // 2D polyhedral => multipolygon
                localType = 6;
                break;
              case 1015:
                // 3D polyhedral => multipolygon
                localType = 1006;
                break;
              case 17:
                // 2D triangle => polygon
                localType = 3;
                break;
              case 1017:
                // 3D triangle => polygon
                localType = 1003;
                break;
              case 16:
                // 2D TIN => multipolygon
                localType = 6;
                break;
              case 1016:
                // TIN => multipolygon
                localType = 1006;
                break;
            }
            if ( localType >= 1000 )
            {
              localType = localType - 1000 + QGis::WKBPoint25D - 1;
            }
            memcpy( wkb + 1, &localType, sizeof( localType ) );

            // skip endian and type info
            wkb += sizeof( unsigned int ) + 1;

            // skip coordinates
            switch ( wkbType )
            {
              case QGis::WKBMultiPoint25D:
                wkb += sizeof( double ) * 3;
                break;
              case QGis::WKBMultiLineString25D:
              {
                unsigned int nPoints = *(( int* ) wkb );
                wkb += sizeof( nPoints );
                wkb += sizeof( double ) * 3 * nPoints;
              }
              break;
              default:
              case QGis::WKBMultiPolygon25D:
              {
                unsigned int nRings = *(( int* ) wkb );
                wkb += sizeof( nRings );
                for ( unsigned int j = 0; j < nRings; ++j )
                {
                  unsigned int nPoints = *(( int* ) wkb );
                  wkb += sizeof( nPoints );
                  wkb += sizeof( double ) * 3 * nPoints;
                }
              }
              break;
            }
          }
        }

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
