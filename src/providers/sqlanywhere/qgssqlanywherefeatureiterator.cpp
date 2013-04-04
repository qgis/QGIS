/***************************************************************************
    qgssqlanywherefeatureiterator.cpp - QGIS feature iterator for SQL Anywhere DBMS
    --------------------------
    begin                : Jan 2013
    copyright            : (C) 2013 by SAP AG or an SAP affiliate company.
    author               : David DeHaan, Mary Steele
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssqlanywherefeatureiterator.h"
#include "qgssqlanywhereprovider.h"

#include <qgsapplication.h>
#include <qgslogger.h>
#include <qgsmessagelog.h>

#include <QObject>

// provider:
// - mGeometryColumn
// - mAttributeFields
// - mConnRO
// - mQuotedTableName
// - mKeyColumn
// - mSrid
// - mSrsExtent
// - mSubsetString
// - geomColIdent()
// - isValid()
// - fields()
// - field()
// - ensureConnRO()
// - quotedIdentifier()
// - reportError()
// - tr()

QgsSqlAnywhereFeatureIterator::QgsSqlAnywhereFeatureIterator( QgsSqlAnywhereProvider* p, const QgsFeatureRequest & request )
    : QgsAbstractFeatureIterator( request ), P( p )
    , mStmt( NULL ), mStmtRect()
{

  mClosed = false;
  QString whereClause = P->getWhereClause();

  if ( request.filterType() == QgsFeatureRequest::FilterRect && !P->mGeometryColumn.isNull() )
  {
    mStmtRect = mRequest.filterRect();
    SaDebugMsg( "initial rectangle =" + mStmtRect.toString() );
    mStmtRect = mStmtRect.intersect( &P->mSrsExtent );
    SaDebugMsg( "rectangle clipped to SRS extent =" + mStmtRect.toString() );

    whereClause += " AND " + whereClauseRect();
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    whereClause += " AND " + whereClauseFid();
  }

  if ( !prepareStatement( whereClause ) )
  {
    mStmt = NULL;
    mClosed = true;
    return;
  }
}


QgsSqlAnywhereFeatureIterator::~QgsSqlAnywhereFeatureIterator()
{
  close();
}

bool
QgsSqlAnywhereFeatureIterator::nextFeature( QgsFeature& feature )
{
  if ( mClosed )
    return false;

  feature.setValid( false );

  if ( !P->isValid() )
  {
    SaDebugMsg( "Read attempt on an invalid SQL Anywhere data source" );
    return false;
  }

  if ( mStmt == NULL || !mStmt->isValid() )
  {
    SaDebugMsg( "nextFeature() called with invalid cursor()" );
    return false;
  }

  return nextFeature( feature, mStmt );
}


bool QgsSqlAnywhereFeatureIterator::nextFeature( QgsFeature& feature, SqlAnyStatement *stmt )
{
  feature.setValid( false );

  bool fetchGeometry = !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  bool subsetAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;

  if ( mClosed )
    return false;

  if ( !P->mConnRO || !P->mConnRO->isAlive() )
  {
    SaDebugMsg( "No database connection." );
    return false;
  }

  bool    ok;
  int     id;
  a_sqlany_data_value geom;
  unsigned char *geomBuf = NULL;


  ok = ( stmt != NULL && stmt->fetchNext() );

  // if no more rows...
  if ( !ok )
    return false;

  if ( !fetchGeometry )
    feature.setGeometryAndOwnership( 0, 0 );

  int numAttributes = P->fields().count(); // also used later for sanity check

  feature.initAttributes( numAttributes );
  feature.setFields( &P->mAttributeFields ); // allow name-based attribute lookups

  int i = 0;
  int numcols = stmt->numCols();
  int colidx = 0; // Refers to which column we're on in "feature" (the row)
  for ( i = 0; i < numcols; i++ )
  {
    if ( i == 0 )
    {
      // first column always contains primary key
      ok = stmt->getInt( i, id );
      if ( !ok ) break;
      QgsDebugMsgLevel( QString( "pid=%1" ).arg( id ), 3 );
      feature.setFeatureId( id );
    }
    else if ( i == 1 && fetchGeometry )
    {
      // second column contains QKB geometry value
      ok = stmt->getColumn( i, &geom );
      if ( !ok ) break;
      QgsDebugMsgLevel( QString( "retrieved geometry column" ), 3 );
      geomBuf = new unsigned char[ *geom.length + 1 ];
      memset( geomBuf, '\0', *geom.length );
      memcpy( geomBuf, geom.buffer, *geom.length );
      feature.setGeometryAndOwnership( geomBuf, *geom.length + 1 );
    }
    else
    {
      if ( i == 1 )
      {
        feature.setGeometryAndOwnership( 0, 0 ); // no geometry to fetch
      }
      int attrIndex = subsetAttributes ? mRequest.subsetOfAttributes()[colidx++] : colidx++;
      QVariant val;
      ok = stmt->getQVariant( i, val ); // ok may be false if value was NULL, but this is a valid return

      // Sanity check before setting the attribute value
      if ( colidx - 1 == i  // First column is always pk, so colidx should be at least 1 behind
           || ( colidx - 1 == i - 1 && fetchGeometry ) // if fetchGeometry is true, colidx should be 2 behind
           || attrIndex >= numAttributes ) // index should always be less than the count
      {
        SaDebugMsg( QString( "Error retrieving feature column %1 with attribute index %2" ).arg( i ).arg( attrIndex ) );
        return false;
      }
      // So now this should not crash.
      feature.setAttribute( attrIndex, val );
    }

  }

  feature.setValid( true );
  return true;
}


bool QgsSqlAnywhereFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  mStmt->reset();
  mStmt->execute();
  return true;
}

bool QgsSqlAnywhereFeatureIterator::close()
{
  if ( mClosed )
    return false;

  if ( mStmt )
  {
    delete mStmt;
    mStmt = NULL;
  }

  mClosed = true;
  return true;
}

bool
QgsSqlAnywhereFeatureIterator::prepareStatement( QString whereClause )
// adapted from QgsSpatialLiteFeatureIterator::prepareStatement
{
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && P->mGeometryColumn.isNull() )
    return false;

  if ( !P->ensureConnRO() )
  {
    SaDebugMsg( "No read-only database connection." );
    return false;
  }

  QString sql = QString( "SELECT %1" ).arg( quotedPrimaryKey() ); // Column 0 is primary key

  // Column 1 is geometry, if applicable
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    sql += QString( ", %1 .ST_AsBinary('WKB(Version=1.1;endian=%2)') " )
           .arg( P->mGeometryColumn )
           .arg( QgsApplication::endian() == QgsApplication::XDR ? "xdr" : "ndr" );
  }

  // Add the rest of the columns
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    const QgsAttributeList& fetchAttributes = mRequest.subsetOfAttributes();
    for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it )
    {
      QString name = P->field( *it ).name();
      if ( !name.isEmpty() /*&& name != P->mKeyColumn*/ )
      {
        sql += "," + P->quotedIdentifier( name );
      }
    }
  }
  else
  {
    // fetch all attributes
    for ( int idx = 0; idx < P->mAttributeFields.count(); ++idx )
    {
      QString name = P->mAttributeFields[idx].name();
      if ( !name.isEmpty() /*&& name != P->mKeyColumn*/ )
      {
        sql += "," + P->quotedIdentifier( name );
      }
    }
  }

  sql += QString( " FROM %1 " ).arg( P->mQuotedTableName );

  if ( !whereClause.isEmpty() )
    sql += QString( " WHERE %1 OPTIONS(FORCE OPTIMIZATION)" ).arg( whereClause );


  if ( mStmt )
    delete mStmt;

  mStmt = P->mConnRO->prepare( sql );
  if ( !mStmt->isValid() )
  {
    // (Error msg will be printed using SaDebugMsg in prepare())
    rewind();
    return false;
  }

  // bind parameters if necessary
  if ( !mStmtRect.isEmpty() )
  {
    double xmin = mStmtRect.xMinimum();
    double ymin = mStmtRect.yMinimum();
    double xmax = mStmtRect.xMaximum();
    double ymax = mStmtRect.yMaximum();

    a_sqlany_bind_param xminParam;
    a_sqlany_bind_param yminParam;
    a_sqlany_bind_param xmaxParam;
    a_sqlany_bind_param ymaxParam;

    size_t xminLen = sizeof( double );
    size_t yminLen = sizeof( double );
    size_t xmaxLen = sizeof( double );
    size_t ymaxLen = sizeof( double );

    if ( !mStmt->describe_bind_param( 0, xminParam )
         || !mStmt->describe_bind_param( 1, yminParam )
         || !mStmt->describe_bind_param( 2, xmaxParam )
         || !mStmt->describe_bind_param( 3, ymaxParam ) )
    {
      P->reportError( QObject::tr( "Error describing bind parameters" ), mStmt );
      return false;
    }

    xminParam.value.buffer = ( char * ) & xmin;
    yminParam.value.buffer = ( char * ) & ymin;
    xmaxParam.value.buffer = ( char * ) & xmax;
    ymaxParam.value.buffer = ( char * ) & ymax;

    xminParam.value.length = &xminLen;
    yminParam.value.length = &yminLen;
    xmaxParam.value.length = &xmaxLen;
    ymaxParam.value.length = &ymaxLen;

    xminParam.value.type = A_DOUBLE;
    yminParam.value.type = A_DOUBLE;
    xmaxParam.value.type = A_DOUBLE;
    ymaxParam.value.type = A_DOUBLE;

    if ( !mStmt->bind_param( 0, xminParam )
         || !mStmt->bind_param( 1, yminParam )
         || !mStmt->bind_param( 2, xmaxParam )
         || !mStmt->bind_param( 3, ymaxParam ) )
    {
      P->reportError( QObject::tr( "Error binding parameters" ), mStmt );
      return false;
    }
  }

  // Execute the statement
  if ( !mStmt->execute() )
  {
    // (Error msg will be printed using SaDebugMsg in execute())
    rewind();
    return false;
  }

  return true;
}

QString
QgsSqlAnywhereFeatureIterator::whereClauseRect() const
{
  bool exactIntersect = ( mRequest.flags() & QgsFeatureRequest::ExactIntersect );

  QString whereClause;
  whereClause += QString( "%1 .%2 ( new ST_Polygon( "
                          "new ST_Point( ?, ?, %3), "
                          "new ST_Point( ?, ?, %3 ) ) ) = 1 " )
                 .arg( P->geomColIdent() )
                 .arg(( exactIntersect ? "ST_Intersects" : "ST_IntersectsFilter" ) )
                 .arg( P->mSrid );
  // Note: IntersectsFilter is the less expensive estimate

  return whereClause;
}

QString QgsSqlAnywhereFeatureIterator::quotedPrimaryKey() const
{
  return P->quotedIdentifier( P->mKeyColumn );
}

QString QgsSqlAnywhereFeatureIterator::whereClauseFid() const
{
  return QString( "%1=%2" ).arg( quotedPrimaryKey() ).arg( mRequest.filterFid() );
}



