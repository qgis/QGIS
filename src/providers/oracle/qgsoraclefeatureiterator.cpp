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
#include "qgsoracleconnpool.h"
#include "qgsoracleexpressioncompiler.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsgeometry.h"

#include <QObject>
#include <QSettings>

QgsOracleFeatureIterator::QgsOracleFeatureIterator( QgsOracleFeatureSource* source, bool ownSource, const QgsFeatureRequest &request )
    : QgsAbstractFeatureIteratorFromSource<QgsOracleFeatureSource>( source, ownSource, request )
    , mRewind( false )
    , mExpressionCompiled( false )
    , mFetchGeometry( false )
{
  mConnection = QgsOracleConnPool::instance()->acquireConnection( QgsOracleConn::toPoolName( mSource->mUri ) );
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

    // ensure that all attributes required for expression filter are being fetched
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      Q_FOREACH ( const QString& field, mRequest.filterExpression()->referencedColumns() )
      {
        int attrIdx = mSource->mFields.fieldNameIndex( field );
        if ( !mAttributeList.contains( attrIdx ) )
          mAttributeList << attrIdx;
      }
    }

  }
  else
    mAttributeList = mSource->mFields.allAttributesList();

  bool limitAtProvider = ( mRequest.limit() >= 0 );
  QString whereClause;

  if ( !mSource->mGeometryColumn.isNull() )
  {
    // fetch geometry if requested
    mFetchGeometry = ( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0;
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression && mRequest.filterExpression()->needsGeometry() )
    {
      mFetchGeometry = true;
    }

    if ( !mRequest.filterRect().isNull() )
    {
      // sdo_filter requires spatial index
      if ( mSource->mHasSpatialIndex )
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

        if (( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) != 0 )
        {
          // sdo_relate requires Spatial
          if ( mConnection->hasSpatial() )
          {
            whereClause += QString( " AND sdo_relate(%1,%2,'mask=ANYINTERACT')='TRUE'" )
                           .arg( QgsOracleProvider::quotedIdentifier( mSource->mGeometryColumn ) )
                           .arg( bbox );
          }
          else
          {
            // request geometry to do exact intersect in fetchFeature
            mFetchGeometry = true;
          }
        }
      }
      else
      {
        // request geometry to do bbox intersect in fetchFeature
        mFetchGeometry = true;
      }
    }
  }
  else if ( !mRequest.filterRect().isNull() )
  {
    QgsDebugMsg( "filterRect without geometry ignored" );
  }

  switch ( mRequest.filterType() )
  {
    case QgsFeatureRequest::FilterFid:
    {
      QString fidWhereClause = QgsOracleUtils::whereClause( mRequest.filterFid(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );
      whereClause = QgsOracleUtils::andWhereClauses( whereClause, fidWhereClause );
    }
    break;

    case QgsFeatureRequest::FilterFids:
    {
      QString fidsWhereClause = QgsOracleUtils::whereClause( mRequest.filterFids(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );
      whereClause = QgsOracleUtils::andWhereClauses( whereClause, fidsWhereClause );
    }
    break;

    case QgsFeatureRequest::FilterNone:
      break;

    case QgsFeatureRequest::FilterExpression:
      //handled below
      break;

    case QgsFeatureRequest::FilterRect:
      // Handled in the if-statement above
      break;
  }

  if ( mSource->mRequestedGeomType != QGis::WKBUnknown && mSource->mRequestedGeomType != mSource->mDetectedGeomType )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += QgsOracleConn::databaseTypeFilter( "FEATUREREQUEST", mSource->mGeometryColumn, mSource->mRequestedGeomType );
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";
    whereClause += "(" + mSource->mSqlWhereClause + ")";
  }

  //NOTE - must be last added!
  mExpressionCompiled = false;
  mCompileStatus = NoCompilation;
  QString fallbackStatement;
  bool useFallback = false;
  if ( request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
    {
      QgsOracleExpressionCompiler compiler( mSource );
      QgsSqlExpressionCompiler::Result result = compiler.compile( mRequest.filterExpression() );
      if ( result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial )
      {
        fallbackStatement = whereClause;
        useFallback = true;
        whereClause = QgsOracleUtils::andWhereClauses( whereClause, compiler.result() );

        //if only partial success when compiling expression, we need to double-check results using QGIS' expressions
        mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
        mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
        limitAtProvider = mExpressionCompiled;
      }
      else
      {
        limitAtProvider = false;
      }
    }
    else
    {
      limitAtProvider = false;
    }
  }

  if ( !mRequest.orderBy().isEmpty() )
  {
    limitAtProvider = false;
  }

  if ( mRequest.limit() >= 0 && limitAtProvider )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += QString( "rownum<=%1" ).arg( mRequest.limit() );
    fallbackStatement += QString( "rownum<=%1" ).arg( mRequest.limit() );
  }

  bool result = openQuery( whereClause, !useFallback );
  if ( !result && useFallback )
  {
    result = openQuery( fallbackStatement );
    if ( result )
    {
      mExpressionCompiled = false;
      mCompileStatus = NoCompilation;
    }
  }
}

QgsOracleFeatureIterator::~QgsOracleFeatureIterator()
{
  close();
}

bool QgsOracleFeatureIterator::nextFeatureFilterExpression( QgsFeature& f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
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
      if ( !QgsOracleProvider::exec( mQry, mSql ) )
      {
        QgsMessageLog::logMessage( QObject::tr( "Fetching features failed.\nSQL:%1\nError: %2" )
                                   .arg( mQry.lastQuery() )
                                   .arg( mQry.lastError().text() ),
                                   QObject::tr( "Oracle" ) );
        return false;
      }
    }
    if ( !mQry.next() )
    {
      return false;
    }

    int col = 0;

    if ( mFetchGeometry )
    {
      QByteArray *ba = static_cast<QByteArray*>( mQry.value( col++ ).data() );
      if ( ba->size() > 0 )
      {
        unsigned char *copy = new unsigned char[ba->size()];
        memcpy( copy, ba->constData(), ba->size() );

        QgsGeometry *g = new QgsGeometry();
        g->fromWkb( copy, ba->size() );
        feature.setGeometry( g );
      }
      else
      {
        feature.setGeometry( 0 );
      }

      if ( !mRequest.filterRect().isNull() )
      {
        if ( !feature.geometry() )
        {
          QgsDebugMsg( "no geometry to intersect" );
          continue;
        }

        if (( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) == 0 )
        {
          // couldn't use sdo_filter earlier
          if ( !mSource->mHasSpatialIndex )
          {
            // only intersect with bbox
            if ( !feature.geometry()->boundingBox().intersects( mRequest.filterRect() ) )
            {
              // skip feature that don't intersect with our rectangle
              QgsDebugMsg( "no bbox intersect" );
              continue;
            }
          }
        }
        else if ( !mConnection->hasSpatial() || !mSource->mHasSpatialIndex )
        {
          // couldn't use sdo_relate earlier
          if ( !feature.geometry()->intersects( mRequest.filterRect() ) )
          {
            // skip feature that don't intersect with our rectangle
            QgsDebugMsg( "no exact intersect" );
            continue;
          }
        }
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
          Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
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
    Q_FOREACH ( int idx, mAttributeList )
    {
      if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      const QgsField &fld = mSource->mFields[idx];

      QVariant v = mQry.value( col );
      if ( fld.type() == QVariant::ByteArray && fld.typeName().endsWith( ".SDO_GEOMETRY" ) )
      {
        QByteArray *ba = static_cast<QByteArray*>( v.data() );
        if ( ba->size() > 0 )
        {
          unsigned char *copy = new unsigned char[ba->size()];
          memcpy( copy, ba->constData(), ba->size() );

          QgsGeometry *g = new QgsGeometry();
          g->fromWkb( copy, ba->size() );
          v = g->exportToWkt();
          delete g;
        }
        else
        {
          v = QVariant( QVariant::String );
        }
      }
      else if ( v.type() != fld.type() )
        v = QgsVectorDataProvider::convertValue( fld.type(), v.toString() );
      feature.setAttribute( idx, v );

      col++;
    }

    feature.setValid( true );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups

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
    QgsOracleConnPool::instance()->releaseConnection( mConnection );
  mConnection = 0;

  iteratorClosed();

  return true;
}

bool QgsOracleFeatureIterator::openQuery( QString whereClause, bool showLog )
{
  try
  {
    QString query = "SELECT ", delim = "";

    if ( mFetchGeometry )
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
        Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
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

    Q_FOREACH ( int idx, mAttributeList )
    {
      if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      query += delim + mConnection->fieldExpression( mSource->mFields[idx] );
    }

    query += QString( " FROM %1 \"FEATUREREQUEST\"" ).arg( mSource->mQuery );

    if ( !whereClause.isEmpty() )
      query += QString( " WHERE %1" ).arg( whereClause );

    QgsDebugMsg( QString( "Fetch features: %1" ).arg( query ) );
    mSql = query;
    if ( !QgsOracleProvider::exec( mQry, query ) )
    {
      if ( showLog )
      {
        QgsMessageLog::logMessage( QObject::tr( "Fetching features failed.\nSQL:%1\nError: %2" )
                                   .arg( mQry.lastQuery() )
                                   .arg( mQry.lastError().text() ),
                                   QObject::tr( "Oracle" ) );
      }
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
