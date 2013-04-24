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

QgsOracleFeatureIterator::QgsOracleFeatureIterator( QgsOracleProvider *p, const QgsFeatureRequest &request )
    : QgsAbstractFeatureIterator( request )
    , P( p )
    , mRewind( false )
{
  mQry = QSqlQuery( *P->mConnection );

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    mAttributeList = mRequest.subsetOfAttributes();
    if ( mAttributeList.isEmpty() )
      mAttributeList = P->attributeIndexes();
  }
  else
    mAttributeList = P->attributeIndexes();

  QString whereClause;

  switch ( request.filterType() )
  {
    case QgsFeatureRequest::FilterRect:
      if ( !P->mGeometryColumn.isNull() )
      {
        QgsRectangle rect( mRequest.filterRect() );
        QString bbox = QString( "mdsys.sdo_geometry(2003,%1,NULL,"
                                "mdsys.sdo_elem_info_array(1,1003,3),"
                                "mdsys.sdo_ordinate_array(%2,%3,%4,%5)"
                                ")" )
                       .arg( P->mSrid < 1 ? "NULL" : QString::number( P->mSrid ) )
                       .arg( rect.xMinimum(), 0, 'f', 16 )
                       .arg( rect.yMinimum(), 0, 'f', 16 )
                       .arg( rect.xMaximum(), 0, 'f', 16 )
                       .arg( rect.yMaximum(), 0, 'f', 16 );

        if ( !P->mSpatialIndex.isNull() )
        {
          whereClause = QString( "sdo_filter(%1,%2)='TRUE'" ).arg( P->quotedIdentifier( P->mGeometryColumn ) ).arg( bbox );
#if 0
          if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
          {
            whereClause += QString( " AND sdo_relate(%1,%2,'mask=ANYINTERACT')='TRUE'" )
                           .arg( quotedIdentifier( P->mGeometryColumn ) )
                           .arg( bbox );
          }
#endif
        }
      }
      break;

    case QgsFeatureRequest::FilterFid:
      whereClause = P->whereClause( request.filterFid() );
      break;

    case QgsFeatureRequest::FilterNone:
      break;
  }

  if ( P->mRequestedGeomType != QGis::WKBUnknown && P->mRequestedGeomType != P->mDetectedGeomType )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += QgsOracleConn::databaseTypeFilter( "featureRequest", P->mGeometryColumn, P->mRequestedGeomType );
  }

  if ( !P->mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";
    whereClause += "(" + P->mSqlWhereClause + ")";
  }

  if ( !openQuery( whereClause ) )
    return;
}

QgsOracleFeatureIterator::~QgsOracleFeatureIterator()
{
  close();
}

bool QgsOracleFeatureIterator::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( !mQry.isActive() )
    return false;

  for ( ;; )
  {
    feature.initAttributes( P->fields().count() );
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
        (( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) != 0 && !P->mConnection->hasSpatial() ) )
    {
      QByteArray *ba = static_cast<QByteArray*>( mQry.value( col++ ).data() );
      unsigned char *copy = new unsigned char[ba->size()];
      memcpy( copy, ba->constData(), ba->size() );

      feature.setGeometryAndOwnership( copy, ba->size() );

      if ( !P->mConnection->hasSpatial() &&
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

    switch ( P->mPrimaryKeyType )
    {
      case QgsOracleProvider::pktInt:
        // get 64bit integer from result
        fid = mQry.value( col++ ).toLongLong();
        if ( mAttributeList.contains( P->mPrimaryKeyAttrs[0] ) )
          feature.setAttribute( P->mPrimaryKeyAttrs[0], fid );
        break;

      case QgsOracleProvider::pktRowId:
      case QgsOracleProvider::pktFidMap:
      {
        QList<QVariant> primaryKeyVals;

        if ( P->mPrimaryKeyType == QgsOracleProvider::pktFidMap )
        {
          foreach ( int idx, P->mPrimaryKeyAttrs )
          {
            const QgsField &fld = P->field( idx );

            QVariant v = P->convertValue( fld.type(), mQry.value( col ).toString() );
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

        fid = P->lookupFid( QVariant( primaryKeyVals ) );
      }
      break;

      case QgsOracleProvider::pktUnknown:
        Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
        return false;
    }

    feature.setFeatureId( fid );
    QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 5 );

    // iterate attributes
    foreach ( int idx, mAttributeList )
    {
      if ( P->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      const QgsField &fld = P->field( idx );

      QVariant v = P->convertValue( fld.type(), mQry.value( col ).toString() );
      feature.setAttribute( idx, v );

      col++;
    }

    feature.setValid( true );
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
  if ( !mQry.isActive() )
    return false;

  mQry.finish();

  return true;
}

bool QgsOracleFeatureIterator::openQuery( QString whereClause )
{
  if (( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0 && P->mGeometryColumn.isNull() )
  {
    return false;
  }

  try
  {
    QString query = "SELECT ", delim = "";

    if (( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0 )
    {
      query += P->quotedIdentifier( P->mGeometryColumn );
      delim = ",";
    }

    switch ( P->mPrimaryKeyType )
    {
      case QgsOracleProvider::pktRowId:
        query += delim + P->quotedIdentifier( "ROWID" );
        delim = ",";
        break;

      case QgsOracleProvider::pktInt:
        query += delim + P->quotedIdentifier( P->field( P->mPrimaryKeyAttrs[0] ).name() );
        delim = ",";
        break;

      case QgsOracleProvider::pktFidMap:
        foreach ( int idx, P->mPrimaryKeyAttrs )
        {
          query += delim + P->mConnection->fieldExpression( P->field( idx ) );
          delim = ",";
        }
        break;

      case QgsOracleProvider::pktUnknown:
        QgsDebugMsg( "Cannot query without primary key." );
        return false;
        break;
    }

    foreach ( int idx, mAttributeList )
    {
      if ( P->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      query += delim + P->mConnection->fieldExpression( P->field( idx ) );
    }

    query += QString( " FROM %1 \"featureRequest\"" ).arg( P->mQuery );

    if ( !whereClause.isEmpty() )
      query += QString( " WHERE %1" ).arg( whereClause );

    QgsDebugMsg( QString( "Fetch features: %1" ).arg( query ) );
    if ( !P->exec( mQry, query ) )
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
