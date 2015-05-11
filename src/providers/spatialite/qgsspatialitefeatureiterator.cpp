/***************************************************************************
    qgsspatialitefeatureiterator.cpp
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
#include "qgsspatialitefeatureiterator.h"

#include "qgsspatialiteconnection.h"
#include "qgsspatialiteconnpool.h"
#include "qgsspatialiteprovider.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"



QgsSpatiaLiteFeatureIterator::QgsSpatiaLiteFeatureIterator( QgsSpatiaLiteFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsSpatiaLiteFeatureSource>( source, ownSource, request )
    , sqliteStatement( NULL )
{

  mHandle = QgsSpatiaLiteConnPool::instance()->acquireConnection( mSource->mSqlitePath );

  mFetchGeometry = !mSource->mGeometryColumn.isNull() && !( mRequest.flags() & QgsFeatureRequest::NoGeometry );

  QString whereClause;
  if ( request.filterType() == QgsFeatureRequest::FilterRect && !mSource->mGeometryColumn.isNull() )
  {
    // some kind of MBR spatial filtering is required
    whereClause += whereClauseRect();
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    whereClause += whereClauseFid();
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFids )
  {
    whereClause += whereClauseFids();
  }

  if ( !mSource->mSubsetString.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
    {
      whereClause += " AND ";
    }
    whereClause += "( " + mSource->mSubsetString + ")";
  }

  // preparing the SQL statement
  if ( !prepareStatement( whereClause ) )
  {
    // some error occurred
    sqliteStatement = NULL;
    close();
    return;
  }
}

QgsSpatiaLiteFeatureIterator::~QgsSpatiaLiteFeatureIterator()
{
  close();
}


bool QgsSpatiaLiteFeatureIterator::fetchFeature( QgsFeature& feature )
{
  if ( mClosed )
    return false;

  feature.setValid( false );

  if ( sqliteStatement == NULL )
  {
    QgsDebugMsg( "Invalid current SQLite statement" );
    close();
    return false;
  }

  if ( !getFeature( sqliteStatement, feature ) )
  {
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
    close();
    return false;
  }

  feature.setValid( true );
  return true;
}


bool QgsSpatiaLiteFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( sqlite3_reset( sqliteStatement ) == SQLITE_OK )
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsSpatiaLiteFeatureIterator::close()
{
  if ( mClosed )
    return false;

  iteratorClosed();

  if ( sqliteStatement )
  {
    sqlite3_finalize( sqliteStatement );
    sqliteStatement = NULL;
  }

  QgsSpatiaLiteConnPool::instance()->releaseConnection( mHandle );
  mHandle = 0;

  mClosed = true;
  return true;
}

////


bool QgsSpatiaLiteFeatureIterator::prepareStatement( QString whereClause )
{
  try
  {
    QString sql = QString( "SELECT %1" ).arg( quotedPrimaryKey() );
    int colIdx = 1; // column 0 is primary key

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      const QgsAttributeList& fetchAttributes = mRequest.subsetOfAttributes();
      for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it )
      {
        sql += "," + fieldName( mSource->mFields.field( *it ) );
        colIdx++;
      }
    }
    else
    {
      // fetch all attributes
      for ( int idx = 0; idx < mSource->mFields.count(); ++idx )
      {
        sql += "," + fieldName( mSource->mFields[idx] );
        colIdx++;
      }
    }

    if ( mFetchGeometry )
    {
      sql += QString( ", AsBinary(%1)" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ) );
      mGeomColIdx = colIdx;
    }
    sql += QString( " FROM %1" ).arg( mSource->mQuery );

    if ( !whereClause.isEmpty() )
      sql += QString( " WHERE %1" ).arg( whereClause );

    if ( sqlite3_prepare_v2( mHandle->handle(), sql.toUtf8().constData(), -1, &sqliteStatement, NULL ) != SQLITE_OK )
    {
      // some error occurred
      QgsMessageLog::logMessage( QObject::tr( "SQLite error: %2\nSQL: %1" ).arg( sql ).arg( sqlite3_errmsg( mHandle->handle() ) ), QObject::tr( "SpatiaLite" ) );
      return false;
    }
  }
  catch ( QgsSpatiaLiteProvider::SLFieldNotFound )
  {
    rewind();
    return false;
  }

  return true;
}

QString QgsSpatiaLiteFeatureIterator::quotedPrimaryKey()
{
  return !mSource->isQuery ? "ROWID" : QgsSpatiaLiteProvider::quotedIdentifier( mSource->mPrimaryKey );
}

QString QgsSpatiaLiteFeatureIterator::whereClauseFid()
{
  return QString( "%1=%2" ).arg( quotedPrimaryKey() ).arg( mRequest.filterFid() );
}

QString QgsSpatiaLiteFeatureIterator::whereClauseFids()
{
  QStringList whereClauses;
  foreach ( const QgsFeatureId featureId, mRequest.filterFids() )
  {
    whereClauses << QString( "%1=%2" ).arg( quotedPrimaryKey() ).arg( featureId );
  }
  return whereClauses.isEmpty() ? "" : whereClauses.join( " OR " ).prepend( "(" ).append( ")" );
}

QString QgsSpatiaLiteFeatureIterator::whereClauseRect()
{
  QgsRectangle rect = mRequest.filterRect();
  QString whereClause;

  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    // we are requested to evaluate a true INTERSECT relationship
    whereClause += QString( "Intersects(%1, BuildMbr(%2)) AND " ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ) ).arg( mbr( rect ) );
  }
  if ( mSource->mVShapeBased )
  {
    // handling a VirtualShape layer
    whereClause += QString( "MbrIntersects(%1, BuildMbr(%2))" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ) ).arg( mbr( rect ) );
  }
  else if ( rect.isFinite() )
  {
    if ( mSource->spatialIndexRTree )
    {
      // using the RTree spatial index
      QString mbrFilter = QString( "xmin <= %1 AND " ).arg( qgsDoubleToString( rect.xMaximum() ) );
      mbrFilter += QString( "xmax >= %1 AND " ).arg( qgsDoubleToString( rect.xMinimum() ) );
      mbrFilter += QString( "ymin <= %1 AND " ).arg( qgsDoubleToString( rect.yMaximum() ) );
      mbrFilter += QString( "ymax >= %1" ).arg( qgsDoubleToString( rect.yMinimum() ) );
      QString idxName = QString( "idx_%1_%2" ).arg( mSource->mIndexTable ).arg( mSource->mIndexGeometry );
      whereClause += QString( "%1 IN (SELECT pkid FROM %2 WHERE %3)" )
                     .arg( quotedPrimaryKey() )
                     .arg( QgsSpatiaLiteProvider::quotedIdentifier( idxName ) )
                     .arg( mbrFilter );
    }
    else if ( mSource->spatialIndexMbrCache )
    {
      // using the MbrCache spatial index
      QString idxName = QString( "cache_%1_%2" ).arg( mSource->mIndexTable ).arg( mSource->mIndexGeometry );
      whereClause += QString( "%1 IN (SELECT rowid FROM %2 WHERE mbr = FilterMbrIntersects(%3))" )
                     .arg( quotedPrimaryKey() )
                     .arg( QgsSpatiaLiteProvider::quotedIdentifier( idxName ) )
                     .arg( mbr( rect ) );
    }
    else
    {
      // using simple MBR filtering
      whereClause += QString( "MbrIntersects(%1, BuildMbr(%2))" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mSource->mGeometryColumn ) ).arg( mbr( rect ) );
    }
  }
  else
  {
    whereClause = "1";
  }
  return whereClause;
}


QString QgsSpatiaLiteFeatureIterator::mbr( const QgsRectangle& rect )
{
  return QString( "%1, %2, %3, %4" )
         .arg( qgsDoubleToString( rect.xMinimum() ) )
         .arg( qgsDoubleToString( rect.yMinimum() ) )
         .arg( qgsDoubleToString( rect.xMaximum() ) )
         .arg( qgsDoubleToString( rect.yMaximum() ) );
}


QString QgsSpatiaLiteFeatureIterator::fieldName( const QgsField& fld )
{
  QString fieldname = QgsSpatiaLiteProvider::quotedIdentifier( fld.name() );
  const QString type = fld.typeName().toLower();
  if ( type.contains( "geometry" ) || type.contains( "point" ) ||
       type.contains( "line" ) || type.contains( "polygon" ) )
  {
    fieldname = QString( "AsText(%1)" ).arg( fieldname );
  }
  return fieldname;
}


bool QgsSpatiaLiteFeatureIterator::getFeature( sqlite3_stmt *stmt, QgsFeature &feature )
{
  bool subsetAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;

  int ret = sqlite3_step( stmt );
  if ( ret == SQLITE_DONE )
  {
    // there are no more rows to fetch
    return false;
  }
  if ( ret != SQLITE_ROW )
  {
    // some unexpected error occurred
    QgsMessageLog::logMessage( QObject::tr( "SQLite error getting feature: %1" ).arg( QString::fromUtf8( sqlite3_errmsg( mHandle->handle() ) ) ), QObject::tr( "SpatiaLite" ) );
    return false;
  }

  // one valid row has been fetched from the result set
  if ( !mFetchGeometry )
  {
    // no geometry was required
    feature.setGeometryAndOwnership( 0, 0 );
  }

  feature.initAttributes( mSource->mFields.count() );
  feature.setFields( &mSource->mFields ); // allow name-based attribute lookups

  int ic;
  int n_columns = sqlite3_column_count( stmt );
  for ( ic = 0; ic < n_columns; ic++ )
  {
    if ( ic == 0 )
    {
      // first column always contains the ROWID (or the primary key)
      QgsFeatureId fid = sqlite3_column_int64( stmt, ic );
      QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 3 );
      feature.setFeatureId( fid );
    }
    else if ( mFetchGeometry && ic == mGeomColIdx )
    {
      getFeatureGeometry( stmt, ic, feature );
    }
    else
    {
      if ( subsetAttributes )
      {
        if ( ic <= mRequest.subsetOfAttributes().size() )
        {
          int attrIndex = mRequest.subsetOfAttributes()[ic-1];
          feature.setAttribute( attrIndex, getFeatureAttribute( stmt, ic, mSource->mFields.at( attrIndex ).type() ) );
        }
      }
      else
      {
        int attrIndex = ic - 1;
        feature.setAttribute( attrIndex, getFeatureAttribute( stmt, ic, mSource->mFields.at( attrIndex ).type() ) );
      }
    }
  }

  return true;
}

QVariant QgsSpatiaLiteFeatureIterator::getFeatureAttribute( sqlite3_stmt* stmt, int ic, const QVariant::Type& type )
{
  if ( sqlite3_column_type( stmt, ic ) == SQLITE_INTEGER )
  {
    // INTEGER value
    return sqlite3_column_int( stmt, ic );
  }

  if ( sqlite3_column_type( stmt, ic ) == SQLITE_FLOAT )
  {
    // DOUBLE value
    return sqlite3_column_double( stmt, ic );
  }

  if ( sqlite3_column_type( stmt, ic ) == SQLITE_TEXT )
  {
    // TEXT value
    const char *txt = ( const char * ) sqlite3_column_text( stmt, ic );
    return QString::fromUtf8( txt );
  }

  // assuming NULL
  return QVariant( type );
}

void QgsSpatiaLiteFeatureIterator::getFeatureGeometry( sqlite3_stmt* stmt, int ic, QgsFeature& feature )
{
  if ( sqlite3_column_type( stmt, ic ) == SQLITE_BLOB )
  {
    unsigned char *featureGeom = NULL;
    size_t geom_size = 0;
    const void *blob = sqlite3_column_blob( stmt, ic );
    size_t blob_size = sqlite3_column_bytes( stmt, ic );
    QgsSpatiaLiteProvider::convertToGeosWKB(( const unsigned char * )blob, blob_size,
                                            &featureGeom, &geom_size );
    if ( featureGeom )
      feature.setGeometryAndOwnership( featureGeom, geom_size );
    else
      feature.setGeometryAndOwnership( 0, 0 );
  }
  else
  {
    // NULL geometry
    feature.setGeometryAndOwnership( 0, 0 );
  }
}


QgsSpatiaLiteFeatureSource::QgsSpatiaLiteFeatureSource( const QgsSpatiaLiteProvider* p )
    : mGeometryColumn( p->mGeometryColumn )
    , mSubsetString( p->mSubsetString )
    , mFields( p->attributeFields )
    , mQuery( p->mQuery )
    , isQuery( p->isQuery )
    , mVShapeBased( p->mVShapeBased )
    , mIndexTable( p->mIndexTable )
    , mIndexGeometry( p->mIndexGeometry )
    , mPrimaryKey( p->mPrimaryKey )
    , spatialIndexRTree( p->spatialIndexRTree )
    , spatialIndexMbrCache( p->spatialIndexMbrCache )
    , mSqlitePath( p->mSqlitePath )
{
}

QgsSpatiaLiteFeatureSource::~QgsSpatiaLiteFeatureSource()
{
}

QgsFeatureIterator QgsSpatiaLiteFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsSpatiaLiteFeatureIterator( this, false, request ) );
}
