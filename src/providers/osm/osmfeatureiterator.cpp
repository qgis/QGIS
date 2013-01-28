/***************************************************************************
    osmfeatureiterator.cpp
    ---------------------
    begin                : Januar 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "osmfeatureiterator.h"

#include "osmprovider.h"

#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgslogger.h"

QgsOSMFeatureIterator::QgsOSMFeatureIterator( QgsOSMDataProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), P( p )
    , mRectGeom( 0 )
{
  // make sure that only one iterator is active
  if ( P->mActiveIterator )
    P->mActiveIterator->close();
  P->mActiveIterator = this;


  bool hasFilterFid = mRequest.filterType() == QgsFeatureRequest::FilterFid;
  bool hasFilterRect = mRequest.filterType() == QgsFeatureRequest::FilterRect;
  QgsRectangle rect = mRequest.filterRect();
  mRectGeom = QgsGeometry::fromRect( rect );


  if ( P->mFeatureType == QgsOSMDataProvider::PointType )
  {
    const char* sqlSelectNode = "SELECT id, lat, lon, timestamp, user FROM node WHERE id=? AND usage=0 AND status<>'R' AND u=1";
    const char* sqlSelectPoints = "SELECT id, lat, lon, timestamp, user FROM node WHERE usage=0 AND status<>'R' AND u=1";
    const char* sqlSelectPointsIn = "SELECT id, lat, lon, timestamp, user FROM node WHERE usage=0 AND status<>'R' AND u=1 "
                                    "AND lat>=? AND lat<=? AND lon>=? AND lon<=?";

    const char* sql = hasFilterFid ? sqlSelectNode : ( hasFilterRect ? sqlSelectPointsIn : sqlSelectPoints );

    if ( sqlite3_prepare_v2( P->mDatabase, sql, -1, &mSelectStmt, 0 ) != SQLITE_OK )
    {
      QgsDebugMsg( "sqlite3 statement for points retrieval - prepare failed." );
      return;
    }

    if ( hasFilterFid )
    {
      sqlite3_bind_int64( mSelectStmt, 1, mRequest.filterFid() );
    }
    else if ( hasFilterRect )
    {
      // binding variables (boundary) for points selection!
      sqlite3_bind_double( mSelectStmt, 1, rect.yMinimum() );
      sqlite3_bind_double( mSelectStmt, 2, rect.yMaximum() );
      sqlite3_bind_double( mSelectStmt, 3, rect.xMinimum() );
      sqlite3_bind_double( mSelectStmt, 4, rect.xMaximum() );
    }
  }
  else if ( P->mFeatureType == QgsOSMDataProvider::LineType )
  {
    const char* sqlSelectWay     = "SELECT id, wkb, timestamp, user FROM way WHERE id=? AND status<>'R' AND u=1";
    const char* sqlSelectLines   = "SELECT w.id, w.wkb, w.timestamp, w.user FROM way w WHERE w.closed=0 AND w.status<>'R' AND w.u=1";
    const char* sqlSelectLinesIn = "SELECT w.id, w.wkb, w.timestamp, w.user FROM way w WHERE w.closed=0 AND w.status<>'R' AND w.u=1 "
                                   "AND (((w.max_lat between ? AND ?) OR (w.min_lat between ? AND ?) OR (w.min_lat<? AND w.max_lat>?)) "
                                   "AND ((w.max_lon between ? AND ?) OR (w.min_lon between ? AND ?) OR (w.min_lon<? AND w.max_lon>?)))";
    const char* sql = hasFilterFid ? sqlSelectWay : ( hasFilterRect ? sqlSelectLinesIn : sqlSelectLines );

    if ( sqlite3_prepare_v2( P->mDatabase, sql, -1, &mSelectStmt, 0 ) != SQLITE_OK )
    {
      QgsDebugMsg( "sqlite3 statement for lines retrieval - prepare failed." );
      return;
    }

    if ( hasFilterFid )
    {
      sqlite3_bind_int64( mSelectStmt, 1, mRequest.filterFid() );
    }
    else if ( hasFilterRect )
    {
      // binding variables (boundary) for lines selection!
      sqlite3_bind_double( mSelectStmt, 1, rect.yMinimum() );
      sqlite3_bind_double( mSelectStmt, 2, rect.yMaximum() );
      sqlite3_bind_double( mSelectStmt, 3, rect.yMinimum() );
      sqlite3_bind_double( mSelectStmt, 4, rect.yMaximum() );
      sqlite3_bind_double( mSelectStmt, 5, rect.yMinimum() );
      sqlite3_bind_double( mSelectStmt, 6, rect.yMaximum() );

      sqlite3_bind_double( mSelectStmt, 7, rect.xMinimum() );
      sqlite3_bind_double( mSelectStmt, 8, rect.xMaximum() );
      sqlite3_bind_double( mSelectStmt, 9, rect.xMinimum() );
      sqlite3_bind_double( mSelectStmt, 10, rect.xMaximum() );
      sqlite3_bind_double( mSelectStmt, 11, rect.xMinimum() );
      sqlite3_bind_double( mSelectStmt, 12, rect.xMaximum() );
    }
  }
  else // mFeatureType == PolygonType
  {
    const char* sqlSelectWay     = "SELECT id, wkb, timestamp, user FROM way WHERE id=? AND status<>'R' AND u=1";
    const char* sqlSelectPolys = "SELECT w.id, w.wkb, w.timestamp, w.user FROM way w WHERE w.closed=1 AND w.status<>'R' AND w.u=1";
    const char* sqlSelectPolysIn = "SELECT w.id, w.wkb, w.timestamp, w.user FROM way w WHERE w.closed=1 AND w.status<>'R' AND w.u=1 "
                                   "AND (((w.max_lat between ? AND ?) OR (w.min_lat between ? AND ?) OR (w.min_lat<? AND w.max_lat>?)) "
                                   "AND ((w.max_lon between ? AND ?) OR (w.min_lon between ? AND ?) OR (w.min_lon<? AND w.max_lon>?)))";
    const char* sql = hasFilterFid ? sqlSelectWay : ( hasFilterRect ? sqlSelectPolysIn : sqlSelectPolys );

    if ( sqlite3_prepare_v2( P->mDatabase, sql, -1, &mSelectStmt, 0 ) != SQLITE_OK )
    {
      QgsDebugMsg( "sqlite3 statement for polygons retrieval - prepare failed." );
      return;
    }

    if ( hasFilterFid )
    {
      sqlite3_bind_int64( mSelectStmt, 1, mRequest.filterFid() );
    }
    else if ( hasFilterRect )
    {
      // binding variables (boundary) for polygons selection!
      sqlite3_bind_double( mSelectStmt, 1, rect.yMinimum() );
      sqlite3_bind_double( mSelectStmt, 2, rect.yMaximum() );
      sqlite3_bind_double( mSelectStmt, 3, rect.yMinimum() );
      sqlite3_bind_double( mSelectStmt, 4, rect.yMaximum() );
      sqlite3_bind_double( mSelectStmt, 5, rect.yMinimum() );
      sqlite3_bind_double( mSelectStmt, 6, rect.yMaximum() );

      sqlite3_bind_double( mSelectStmt, 7, rect.xMinimum() );
      sqlite3_bind_double( mSelectStmt, 8, rect.xMaximum() );
      sqlite3_bind_double( mSelectStmt, 9, rect.xMinimum() );
      sqlite3_bind_double( mSelectStmt, 10, rect.xMaximum() );
      sqlite3_bind_double( mSelectStmt, 11, rect.xMinimum() );
      sqlite3_bind_double( mSelectStmt, 12, rect.xMaximum() );
    }
  }


  // prepare statement for tag retrieval
  const char* sqlSelectTags = "SELECT key, val FROM tag WHERE object_id=? AND object_type=?";
  int rc = sqlite3_prepare_v2( P->mDatabase, sqlSelectTags, -1, &mTagsStmt, 0 );
  if ( rc != SQLITE_OK )
  {
    QgsDebugMsg( "sqlite3 statement for feature tags selection - prepare failed." );
    return;
  }

  const char* sqlSelectTagValue = "SELECT val FROM tag WHERE object_id=? AND object_type=? AND key=?";
  rc = sqlite3_prepare_v2( P->mDatabase, sqlSelectTagValue, -1, &mCustomTagsStmt, 0 );
  if ( rc != SQLITE_OK )
  {
    QgsDebugMsg( "sqlite3 statement for tag value selection - prepare failed." );
    return;
  }


}


QgsOSMFeatureIterator::~QgsOSMFeatureIterator()
{
  close();
}

bool QgsOSMFeatureIterator::nextFeature( QgsFeature& feature )
{
  if ( mClosed )
    return false;

  // load next requested feature from sqlite3 database
  switch ( sqlite3_step( mSelectStmt ) )
  {
    case SQLITE_DONE:  // no more features to return
      feature.setValid( false );
      close();
      return false;

    case SQLITE_ROW:  // another feature to return
      if ( P->mFeatureType == QgsOSMDataProvider::PointType )
        return fetchNode( feature );
      else if ( P->mFeatureType == QgsOSMDataProvider::LineType )
        return fetchWay( feature );
      else if ( P->mFeatureType == QgsOSMDataProvider::PolygonType )
        return fetchWay( feature );
  }

  if ( P->mFeatureType == QgsOSMDataProvider::PointType )
  {
    QgsDebugMsg( "Getting next feature of type <point> failed." );
  }
  else if ( P->mFeatureType == QgsOSMDataProvider::LineType )
  {
    QgsDebugMsg( "Getting next feature of type <line> failed." );
  }
  else if ( P->mFeatureType == QgsOSMDataProvider::PolygonType )
  {
    QgsDebugMsg( "Getting next feature of type <polygon> failed." );
  }
  feature.setValid( false );
  close();
  return false;
}


bool QgsOSMFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // we have to reset precompiled database statement; thanx to this action the first feature
  // (returned by the query) will be selected again with the next calling of sqlite3_step(mStmt)
  if ( mSelectStmt )
    sqlite3_reset( mSelectStmt );

  return false;
}

bool QgsOSMFeatureIterator::close()
{
  if ( mClosed )
    return false;

  // destruct selected geometry
  delete mRectGeom;
  mRectGeom = 0;

  sqlite3_finalize( mSelectStmt );

  // finalize all created sqlite3 statements
  sqlite3_finalize( mTagsStmt );
  sqlite3_finalize( mCustomTagsStmt );

  // tell provider that this iterator is not active anymore
  P->mActiveIterator = 0;

  mClosed = true;
  return true;
}



bool QgsOSMFeatureIterator::fetchNode( QgsFeature& feature )
{
  sqlite3_stmt* stmt = mSelectStmt;
  bool fetchGeometry = !( mRequest.flags() & QgsFeatureRequest::NoGeometry );

  int selId = sqlite3_column_int( stmt, 0 );
  double selLat = sqlite3_column_double( stmt, 1 );
  double selLon = sqlite3_column_double( stmt, 2 );

  feature.setFeatureId( selId );
  feature.setValid( true );
  feature.initAttributes( P->mAttributeFields.count() );
  feature.setFields( &P->mAttributeFields ); // allow name-based attribute lookups

  // fetch feature's geometry
  if ( fetchGeometry )
  {
    char* geo = new char[21];
    memset( geo, 0, 21 );
    geo[0] = QgsApplication::endian();
    geo[geo[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBPoint;
    memcpy( geo + 5, &selLon, sizeof( double ) );
    memcpy( geo + 13, &selLat, sizeof( double ) );
    feature.setGeometryAndOwnership(( unsigned char * )geo, 24 );    // 24 is size of wkb point structure!
  }

  // fetch attributes
  fetchAttributes( feature, true );

  return true;
}


bool QgsOSMFeatureIterator::fetchWay( QgsFeature& feature )
{
  sqlite3_stmt* stmt = mSelectStmt;
  bool fetchGeometry = !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  bool useIntersect = mRequest.flags() & QgsFeatureRequest::ExactIntersect;

  int selId;
  QgsGeometry *theGeometry = NULL;
  bool fetchMoreRows = true;
  int rc = -1;

  do
  {
    selId = sqlite3_column_int( stmt, 0 );
    unsigned char *pzBlob = 0;
    int pnBlob = 0;

    if ( fetchGeometry || useIntersect || mRequest.filterType() == QgsFeatureRequest::FilterRect )
    {
      pnBlob = sqlite3_column_bytes( stmt, 1 );
      pzBlob = new unsigned char[pnBlob];
      memcpy( pzBlob, sqlite3_column_blob( stmt, 1 ), pnBlob );

      // create geometry
      theGeometry = new QgsGeometry();
      theGeometry->fromWkb(( unsigned char * ) pzBlob, pnBlob );
    }

    if ( theGeometry && ( theGeometry->type() == 3 ) && ( selId != 0 ) )
    {
      // line/polygon geometry is not cached!
      char *geo;
      int geolen;
      P->updateWayWKB( selId, ( P->mFeatureType == QgsOSMDataProvider::LineType ) ? 0 : 1, &geo, &geolen );
      theGeometry->fromWkb(( unsigned char * ) geo, ( size_t ) geolen );
    }

    if ( mRequest.filterType() == QgsFeatureRequest::FilterRect )
    {
      if ( useIntersect )
      {
        // when using intersect, some features might be ignored if they don't intersect the selection rect
        // intersect is a costly operation, use rectangle converted to geos for less conversions
        // (this is usually used during identification of an object)
        if ( theGeometry->intersects( mRectGeom ) )
          fetchMoreRows = false;
      }
      else
      {
        // when using selection rectangle but without exact intersection, check only overlap of bounding box
        // (usually used when drawing)
        if ( mRequest.filterRect().intersects( theGeometry->boundingBox() ) )
          fetchMoreRows = false;
      }
    }
    else
    {
      // no filter => always accept the new feature
      // (used in attribute table)
      fetchMoreRows = false;
    }

    // delete the geometry (if any) in case we're not going to use it anyway
    if ( fetchMoreRows )
      delete theGeometry;
  }
  while ( fetchMoreRows && (( rc = sqlite3_step( stmt ) ) == SQLITE_ROW ) );

  // no more features to return
  if ( rc == SQLITE_DONE )
  {
    sqlite3_exec( P->mDatabase, "COMMIT;", 0, 0, 0 );
    feature.setValid( false );
    return false;
  }

  // fetch feature's geometry
  if ( fetchGeometry )
  {
    feature.setGeometry( theGeometry );
  }
  else
  {
    delete theGeometry; // make sure it's deleted
  }

  feature.setFeatureId( selId );
  feature.setValid( true );

  // fetch attributes
  fetchAttributes( feature, false );

  return true;
}



void QgsOSMFeatureIterator::fetchAttributes( QgsFeature& feature, bool isNode )
{
  feature.initAttributes( P->mAttributeFields.count() );
  feature.setFields( &P->mAttributeFields ); // allow name-based attribute lookups

  // node
  const char* selTimestamp = ( const char* ) sqlite3_column_text( mSelectStmt, isNode ? 3 : 2 );
  const char* selUser = ( const char* ) sqlite3_column_text( mSelectStmt, isNode ? 4 : 3 );
  int selId = sqlite3_column_int( mSelectStmt, 0 );

  // TODO[MD]: subset of attributes

  //QgsAttributeList::const_iterator iter;
  //for ( iter = fetchAttrs.begin(); iter != fetchAttrs.end(); ++iter )
  for ( int i = 0; i < P->mAttributeFields.count(); ++i )
  {
    switch ( i )
    {
      case QgsOSMDataProvider::TimestampAttr:
        feature.setAttribute( QgsOSMDataProvider::TimestampAttr, QString::fromUtf8( selTimestamp ) );
        break;
      case QgsOSMDataProvider::UserAttr:
        feature.setAttribute( QgsOSMDataProvider::UserAttr, QString::fromUtf8( selUser ) );
        break;
      case QgsOSMDataProvider::TagAttr:
        feature.setAttribute( QgsOSMDataProvider::TagAttr, tagsForObject( isNode, selId ) );
        break;
      default: // suppose it's a custom tag
        if ( i >= QgsOSMDataProvider::CustomTagAttr && i < QgsOSMDataProvider::CustomTagAttr + P->mCustomTagsList.count() )
        {
          feature.setAttribute( i, tagForObject( isNode, selId, P->mCustomTagsList[i - QgsOSMDataProvider::CustomTagAttr] ) );
        }
    }
  }
}



QString QgsOSMFeatureIterator::tagForObject( bool isNode, int id, QString tagKey )
{
  sqlite3_bind_int( mCustomTagsStmt, 1, id );
  sqlite3_bind_text( mCustomTagsStmt, 2, isNode ? "node" : "way", -1, 0 );
  QByteArray tag = tagKey.toUtf8(); // must keep the byte array until the query is run
  sqlite3_bind_text( mCustomTagsStmt, 3, tag.data(), -1, 0 );

  QString value;
  int rc;

  if (( rc = sqlite3_step( mCustomTagsStmt ) ) == SQLITE_ROW )
  {
    const char* tagVal = ( const char* ) sqlite3_column_text( mCustomTagsStmt, 0 );
    value = QString::fromUtf8( tagVal );
  }
  else
  {
    // tag wasn't found
    sqlite3_reset( mCustomTagsStmt ); // make ready for next retrieval
    return "";
  }

  sqlite3_reset( mCustomTagsStmt ); // make ready for next retrieval
  return value;
}



QString QgsOSMFeatureIterator::tagsForObject( bool isNode, int id )
{
  sqlite3_bind_int( mTagsStmt, 1, id );
  sqlite3_bind_text( mTagsStmt, 2, isNode ? "node" : "way", -1, 0 );

  QString tags;
  int rc;

  while (( rc = sqlite3_step( mTagsStmt ) ) == SQLITE_ROW )
  {
    const char* tagKey = ( const char* ) sqlite3_column_text( mTagsStmt, 0 );
    const char* tagVal = ( const char* ) sqlite3_column_text( mTagsStmt, 1 );
    QString key = QString::fromUtf8( tagKey );
    QString val = QString::fromUtf8( tagVal );

    // we concatenate tags this way: "key1"="val1","key2"="val2","key3"="val3"
    // -all ; in keyX and valX are replaced by ;;
    // -all , in keyX and valX are replaced by ;
    // -all - in keyX and valX are replaced by --
    // -all = in keyX and valX are replaced by -
    key = key.replace( ';', ";;" );
    val = val.replace( ';', ";;" );
    key = key.replace( ',', ";" );
    val = val.replace( ',', ";" );

    key = key.replace( '-', "--" );
    val = val.replace( '-', "--" );
    key = key.replace( '=', "-" );
    val = val.replace( '=', "-" );

    if ( tags.count() > 0 )
      tags += ",";

    tags += QString( "\"%1\"=\"%2\"" ).arg( key ).arg( val );
  }

  if ( rc != SQLITE_DONE )
  {
    // no tags for object
    //QgsDebugMsg(QString("tags for object failed: type %1 id %2").arg(type).arg(id));
  }

  sqlite3_reset( mTagsStmt ); // make ready for next retrieval
  return tags;
}
