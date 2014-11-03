/***************************************************************************
    qgsoraclefeatureiterator.cpp - Oracle feature iterator
    ---------------------
    begin                : December 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoraclefeatureiterator.h"
#include "qgsoracleprovider.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsgeometry.h"

#include <QObject>

QgsOracleFeatureIterator::QgsOracleFeatureIterator( QgsOracleFeatureSource* source, bool ownSource, const QgsFeatureRequest &request )
    : QgsAbstractFeatureIteratorFromSource( source, ownSource, request )
    , mRewind( false )
{
  mConnection = QgsOracleConn::connectDb( mSource->mUri.connectionInfo() );
  if ( !mConnection )
  {
    close();
    return;
  }

  mQry = QSqlQuery( *mConnection );

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    mAttributeList = mRequest.subsetOfAttributes();
    if ( mAttributeList.isEmpty() )
      mAttributeList = mSource->mFields.allAttributesList();
  }
  else
    mAttributeList = mSource->mFields.allAttributesList();

  QString whereClause;

  switch ( request.filterType() )
  {
    case QgsFeatureRequest::FilterExpression:
      break;

    case QgsFeatureRequest::FilterRect:
      if ( !mSource->mGeometryColumn.isNull() && mSource->mHasSpatialIndex )
      {
        QgsRectangle rect( mRequest.filterRect() );
        QString bbox = QString( "mdsys.sdo_geometry(2003,%1,NULL,"
                                "mdsys.sdo_elem_info_array(1,1003,3),"
                                "mdsys.sdo_ordinate_array(%2,%3,%4,%5)"
                                ")" )
                       .arg( mSource->mSrid < 1 ? "NULL" : QString::number( mSource->mSrid ) )
                       .arg( qgsDoubleToString( rect.xMinimum() ) )
                       .arg( qgsDoubleToString( rect.yMinimum() ) )
                       .arg( qgsDoubleToString( rect.xMaximum() ) )
                       .arg( qgsDoubleToString( rect.yMaximum() ) );

        whereClause = QString( "sdo_filter(%1,%2)='TRUE'" ).arg( QgsOracleProvider::quotedIdentifier( mSource->mGeometryColumn ) ).arg( bbox );
#if 0
        if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
        {
          whereClause += QString( " AND sdo_relate(%1,%2,'mask=ANYINTERACT')='TRUE'" )
                         .arg( quotedIdentifier( P->mGeometryColumn ) )
                         .arg( bbox );
        }
#endif
      }
      break;

    case QgsFeatureRequest::FilterFid:
      whereClause = QgsOracleUtils::whereClause( request.filterFid(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );
      break;

    case QgsFeatureRequest::FilterFids:
      whereClause = QgsOracleUtils::whereClause( request.filterFids(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );
      break;

    case QgsFeatureRequest::FilterNone:
      break;
  }

  if ( mSource->mRequestedGeomType != QGis::WKBUnknown && mSource->mRequestedGeomType != mSource->mDetectedGeomType )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += QgsOracleConn::databaseTypeFilter( "featureRequest", mSource->mGeometryColumn, mSource->mRequestedGeomType );
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";
    whereClause += "(" + mSource->mSqlWhereClause + ")";
  }

  openQuery( whereClause );
}

QgsOracleFeatureIterator::~QgsOracleFeatureIterator()
{
  close();
}

bool QgsOracleFeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( !mQry.isActive() )
    return false;

  for ( ;; )
  {
    feature.initAttributes( mSource->mFields.count() );
    feature.setGeometry( 0 );

    if ( mRewind )
    {
      mRewind = false;
      if ( !mQry.first() )
        return true;
    }
    else if ( !mQry.next() )
    {
      return false;
    }

    int col = 0;

    if (( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0 ||
        (( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) != 0 && !mConnection->hasSpatial() ) )
    {
      QByteArray *ba = static_cast<QByteArray*>( mQry.value( col++ ).data() );
      unsigned char *copy = new unsigned char[ba->size()];
      memcpy( copy, ba->constData(), ba->size() );

      feature.setGeometryAndOwnership( copy, ba->size() );

      if ( !mConnection->hasSpatial() &&
           mRequest.filterType() == QgsFeatureRequest::FilterRect &&
           ( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) != 0 &&
           ( !feature.geometry() || !feature.geometry()->intersects( mRequest.filterRect() ) ) )
      {
        // skip feature that don't intersect with our rectangle
        QgsDebugMsg( "no intersect" );
        continue;
      }


      if (( mRequest.flags() & QgsFeatureRequest::NoGeometry ) != 0 )
      {
        feature.setGeometryAndOwnership( 0, 0 );
      }
    }

    QgsFeatureId fid = 0;

    switch ( mSource->mPrimaryKeyType )
    {
      case pktInt:
        // get 64bit integer from result
        fid = mQry.value( col++ ).toLongLong();
        if ( mAttributeList.contains( mSource->mPrimaryKeyAttrs[0] ) )
          feature.setAttribute( mSource->mPrimaryKeyAttrs[0], fid );
        break;

      case pktRowId:
      case pktFidMap:
      {
        QList<QVariant> primaryKeyVals;

        if ( mSource->mPrimaryKeyType == pktFidMap )
        {
          foreach ( int idx, mSource->mPrimaryKeyAttrs )
          {
            const QgsField &fld = mSource->mFields[idx];

            QVariant v = mQry.value( col );
            if ( v.type() != fld.type() )
              v = QgsVectorDataProvider::convertValue( fld.type(), v.toString() );
            primaryKeyVals << v;

            if ( mAttributeList.contains( idx ) )
              feature.setAttribute( idx, v );

            col++;
          }
        }
        else
        {
          primaryKeyVals << mQry.value( col++ );
        }

        fid = mSource->mShared->lookupFid( QVariant( primaryKeyVals ) );
      }
      break;

      case pktUnknown:
        Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
        return false;
    }

    feature.setFeatureId( fid );
    QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 5 );

    // iterate attributes
    foreach ( int idx, mAttributeList )
    {
      if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      const QgsField &fld = mSource->mFields[idx];

      QVariant v = mQry.value( col );
      if ( v.type() != fld.type() )
        v = QgsVectorDataProvider::convertValue( fld.type(), v.toString() );
      feature.setAttribute( idx, v );

      col++;
    }

    feature.setValid( true );
    feature.setFields( &mSource->mFields ); // allow name-based attribute lookups

    return true;
  }
}

bool QgsOracleFeatureIterator::rewind()
{
  if ( !mQry.isActive() )
    return false;

  // move cursor to first record
  mRewind = true;
  return true;
}

bool QgsOracleFeatureIterator::close()
{
  if ( mQry.isActive() )
    mQry.finish();

  if ( mConnection )
    mConnection->disconnect();
  mConnection = 0;

  iteratorClosed();

  return true;
}

bool QgsOracleFeatureIterator::openQuery( QString whereClause )
{
  if (( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0 && mSource->mGeometryColumn.isNull() )
  {
    return false;
  }

  try
  {
    QString query = "SELECT ", delim = "";

    if (( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0 )
    {
      query += QgsOracleProvider::quotedIdentifier( mSource->mGeometryColumn );
      delim = ",";
    }

    switch ( mSource->mPrimaryKeyType )
    {
      case pktRowId:
        query += delim + QgsOracleProvider::quotedIdentifier( "ROWID" );
        delim = ",";
        break;

      case pktInt:
        query += delim + QgsOracleProvider::quotedIdentifier( mSource->mFields[ mSource->mPrimaryKeyAttrs[0] ].name() );
        delim = ",";
        break;

      case pktFidMap:
        foreach ( int idx, mSource->mPrimaryKeyAttrs )
        {
          query += delim + mConnection->fieldExpression( mSource->mFields[idx] );
          delim = ",";
        }
        break;

      case pktUnknown:
        QgsDebugMsg( "Cannot query without primary key." );
        return false;
        break;
    }

    foreach ( int idx, mAttributeList )
    {
      if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      query += delim + mConnection->fieldExpression( mSource->mFields[idx] );
    }

    query += QString( " FROM %1 \"featureRequest\"" ).arg( mSource->mQuery );

    if ( !whereClause.isEmpty() )
      query += QString( " WHERE %1" ).arg( whereClause );

    QgsDebugMsg( QString( "Fetch features: %1" ).arg( query ) );
    if ( !QgsOracleProvider::exec( mQry, query ) )
    {
      QgsMessageLog::logMessage( QObject::tr( "Fetching features failed.\nSQL:%1\nError: %2" )
                                 .arg( mQry.lastQuery() )
                                 .arg( mQry.lastError().text() ),
                                 QObject::tr( "Oracle" ) );
      return false;
    }
  }
  catch ( QgsOracleProvider::OracleFieldNotFound )
  {
    return false;
  }

  return true;
}

// -----------

QgsOracleFeatureSource::QgsOracleFeatureSource( const QgsOracleProvider* p )
    : mUri( p->mUri )
    , mFields( p->mAttributeFields )
    , mGeometryColumn( p->mGeometryColumn )
    , mSrid( p->mSrid )
    , mHasSpatialIndex( p->mHasSpatialIndex )
    , mDetectedGeomType( p->mDetectedGeomType )
    , mRequestedGeomType( p->mRequestedGeomType )
    , mSqlWhereClause( p->mSqlWhereClause )
    , mPrimaryKeyType( p->mPrimaryKeyType )
    , mPrimaryKeyAttrs( p->mPrimaryKeyAttrs )
    , mQuery( p->mQuery )
    , mShared( p->mShared )
{
}

QgsFeatureIterator QgsOracleFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsOracleFeatureIterator( this, false, request ) );
}
