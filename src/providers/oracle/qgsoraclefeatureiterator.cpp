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
#include "qgssettings.h"
#include "qgsexception.h"

#include <QObject>

QgsOracleFeatureIterator::QgsOracleFeatureIterator( QgsOracleFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
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

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    mClosed = true;
    return;
  }

  QVariantList args;
  mQry = QSqlQuery( *mConnection );

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    mAttributeList = mRequest.subsetOfAttributes();
    if ( mAttributeList.isEmpty() )
      mAttributeList = mSource->mFields.allAttributesList();

    // ensure that all attributes required for expression filter are being fetched
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      Q_FOREACH ( const QString &field, mRequest.filterExpression()->referencedColumns() )
      {
        int attrIdx = mSource->mFields.lookupField( field );
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

    if ( !mFilterRect.isNull() )
    {
      // sdo_filter requires spatial index
      if ( mSource->mHasSpatialIndex )
      {
        QString bbox = QStringLiteral( "mdsys.sdo_geometry(2003,?,NULL,"
                                       "mdsys.sdo_elem_info_array(1,1003,3),"
                                       "mdsys.sdo_ordinate_array(?,?,?,?)"
                                       ")" );

        whereClause = QStringLiteral( "sdo_filter(%1,%2)='TRUE'" )
                      .arg( QgsOracleProvider::quotedIdentifier( mSource->mGeometryColumn ) ).arg( bbox );

        args << ( mSource->mSrid < 1 ? QVariant( QVariant::Int ) : mSource->mSrid ) << mFilterRect.xMinimum() << mFilterRect.yMinimum() << mFilterRect.xMaximum() << mFilterRect.yMaximum();

        if ( ( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) != 0 )
        {
          // sdo_relate requires Spatial
          if ( mConnection->hasSpatial() )
          {
            whereClause += QString( " AND sdo_relate(%1,%2,'mask=ANYINTERACT')='TRUE'" )
                           .arg( QgsOracleProvider::quotedIdentifier( mSource->mGeometryColumn ) )
                           .arg( bbox );
            args << ( mSource->mSrid < 1 ? QVariant( QVariant::Int ) : mSource->mSrid ) << mFilterRect.xMinimum() << mFilterRect.yMinimum() << mFilterRect.xMaximum() << mFilterRect.yMaximum();
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
  else if ( !mFilterRect.isNull() )
  {
    QgsDebugMsg( "filterRect without geometry ignored" );
  }

  switch ( mRequest.filterType() )
  {
    case QgsFeatureRequest::FilterFid:
    {
      QString fidWhereClause = QgsOracleUtils::whereClause( mRequest.filterFid(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared, args );
      whereClause = QgsOracleUtils::andWhereClauses( whereClause, fidWhereClause );
    }
    break;

    case QgsFeatureRequest::FilterFids:
    {
      QString fidsWhereClause = QgsOracleUtils::whereClause( mRequest.filterFids(), mSource->mFields, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared, args );
      whereClause = QgsOracleUtils::andWhereClauses( whereClause, fidsWhereClause );
    }
    break;

    case QgsFeatureRequest::FilterNone:
      break;

    case QgsFeatureRequest::FilterExpression:
      //handled below
      break;

  }

  if ( mSource->mRequestedGeomType != QgsWkbTypes::Unknown && mSource->mRequestedGeomType != mSource->mDetectedGeomType )
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
    if ( QgsSettings().value( "qgis/compileExpressions", true ).toBool() )
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

    whereClause += QStringLiteral( "rownum<=?" );
    fallbackStatement += QStringLiteral( "rownum<=?" );
    args << QVariant::fromValue( mRequest.limit() );
  }

  bool result = openQuery( whereClause, args, !useFallback );
  if ( !result && useFallback )
  {
    result = openQuery( fallbackStatement, args );
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

bool QgsOracleFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsOracleFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( !mQry.isActive() )
    return false;

  for ( ;; )
  {
    feature.initAttributes( mSource->mFields.count() );
    feature.clearGeometry();

    if ( mRewind )
    {
      mRewind = false;
      if ( !QgsOracleProvider::exec( mQry, mSql, mArgs ) )
      {
        QgsMessageLog::logMessage( QObject::tr( "Fetching features failed.\nSQL: %1\nError: %2" )
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
      QByteArray ba( mQry.value( col++ ).toByteArray() );
      if ( ba.size() > 0 )
      {
        QgsGeometry g;
        g.fromWkb( ba );
        feature.setGeometry( g );
      }
      else
      {
        feature.clearGeometry();
      }

      if ( !mFilterRect.isNull() )
      {
        if ( !feature.hasGeometry() )
        {
          QgsDebugMsg( "no geometry to intersect" );
          continue;
        }

        if ( ( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) == 0 )
        {
          // couldn't use sdo_filter earlier
          if ( !mSource->mHasSpatialIndex )
          {
            // only intersect with bbox
            if ( !feature.geometry().boundingBox().intersects( mFilterRect ) )
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
          if ( !feature.geometry().intersects( mFilterRect ) )
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
      case PktInt:
        // get 64bit integer from result
        fid = mQry.value( col++ ).toLongLong();
        if ( mAttributeList.contains( mSource->mPrimaryKeyAttrs[0] ) )
          feature.setAttribute( mSource->mPrimaryKeyAttrs[0], fid );
        break;

      case PktRowId:
      case PktFidMap:
      {
        QList<QVariant> primaryKeyVals;

        if ( mSource->mPrimaryKeyType == PktFidMap )
        {
          Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
          {
            QgsField fld = mSource->mFields.at( idx );

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

      case PktUnknown:
        Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
        return false;
    }

    feature.setId( fid );
    QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 5 );

    // iterate attributes
    Q_FOREACH ( int idx, mAttributeList )
    {
      if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      QgsField fld = mSource->mFields.at( idx );

      QVariant v = mQry.value( col );
      if ( fld.type() == QVariant::ByteArray && fld.typeName().endsWith( ".SDO_GEOMETRY" ) )
      {
        QByteArray ba( v.toByteArray() );
        if ( ba.size() > 0 )
        {
          QgsGeometry g;
          g.fromWkb( ba );
          v = g.asWkt();
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

    geometryToDestinationCrs( feature, mTransform );

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
  mConnection = nullptr;

  iteratorClosed();

  return true;
}

bool QgsOracleFeatureIterator::openQuery( QString whereClause, QVariantList args, bool showLog )
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
      case PktRowId:
        query += delim + QgsOracleProvider::quotedIdentifier( "ROWID" );
        delim = ",";
        break;

      case PktInt:
        query += delim + QgsOracleProvider::quotedIdentifier( mSource->mFields.at( mSource->mPrimaryKeyAttrs[0] ).name() );
        delim = ",";
        break;

      case PktFidMap:
        Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
        {
          query += delim + mConnection->fieldExpression( mSource->mFields.at( idx ) );
          delim = ",";
        }
        break;

      case PktUnknown:
        QgsDebugMsg( "Cannot query without primary key." );
        return false;
        break;
    }

    Q_FOREACH ( int idx, mAttributeList )
    {
      if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
        continue;

      query += delim + mConnection->fieldExpression( mSource->mFields.at( idx ) );
    }

    query += QString( " FROM %1 \"FEATUREREQUEST\"" ).arg( mSource->mQuery );

    if ( !whereClause.isEmpty() )
      query += QString( " WHERE %1" ).arg( whereClause );

    QgsDebugMsg( QString( "Fetch features: %1" ).arg( query ) );
    mSql = query;
    mArgs = args;
    if ( !QgsOracleProvider::exec( mQry, query, args ) )
    {
      if ( showLog )
      {
        QgsMessageLog::logMessage( QObject::tr( "Fetching features failed.\nSQL: %1\nError: %2" )
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

QgsOracleFeatureSource::QgsOracleFeatureSource( const QgsOracleProvider *p )
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
  , mCrs( p->crs() )
  , mShared( p->mShared )
{
}

QgsFeatureIterator QgsOracleFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsOracleFeatureIterator( this, false, request ) );
}
