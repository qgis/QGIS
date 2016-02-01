#include "qgsdb2featureiterator.h"
#include "qgsdb2provider.h"
#include <qgslogger.h>
#include <qgsgeometry.h>

#include <QObject>
#include <QTextStream>
#include <QSqlRecord>


QgsDb2FeatureIterator::QgsDb2FeatureIterator( QgsDb2FeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsDb2FeatureSource>( source, ownSource, request )
{
  mClosed = false;
  mQuery = NULL;

  BuildStatement( request );

  // connect to the database
  bool convertIntOk;
  int portNum = mSource->mPort.toInt( &convertIntOk, 10 );
  mDatabase = QgsDb2Provider::GetDatabase( mSource->mService, mSource->mDriver, mSource->mHost, portNum, mSource->mDatabaseName, mSource->mUserName, mSource->mPassword );

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


QgsDb2FeatureIterator::~QgsDb2FeatureIterator()
{
  close();
}

void QgsDb2FeatureIterator::BuildStatement( const QgsFeatureRequest& request )
{
// Note, schema, table and column names are not escaped
// Not sure if this is a problem with upper/lower case names
  // build sql statement
  mStatement = QString( "SELECT " );

  mStatement += QString( "%1" ).arg( mSource->mFidColName );
  mFidCol = mSource->mFields.indexFromName( mSource->mFidColName );
  mAttributesToFetch.append( mFidCol );

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  Q_FOREACH ( int i, subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList() )
  {
    QString fieldname = mSource->mFields.at( i ).name();
    if ( mSource->mFidColName == fieldname )
      continue;

    mStatement += QString( ",%1" ).arg( fieldname );

    mAttributesToFetch.append( i );
  }

  // get geometry col in WKB format
  if ( !( request.flags() & QgsFeatureRequest::NoGeometry ) && mSource->isSpatial() )
  {
 mStatement += QString( ",DB2GSE.ST_ASBINARY(%1) AS %1 " ).arg( mSource->mGeometryColName );

//  mStatement += QString( ",VARCHAR(DB2GSE.ST_ASTEXT(%1)) AS %1 " ).arg( mSource->mGeometryColName );
//    mStatement += QString( ",DB2GSE.ST_ASTEXT(%1) AS %1 " ).arg( mSource->mGeometryColName );

    mAttributesToFetch.append( 2 );
  }

  mStatement += QString( " FROM %1.%2" ).arg( mSource->mSchemaName, mSource->mTableName );

  bool filterAdded = false;
  // set spatial filter
  if ( !request.filterRect().isNull() && mSource->isSpatial() && !request.filterRect().isEmpty() )
  {
    QString r;
    QTextStream foo( &r );

    /**
     * The first form (def USE_POLY) mirrors Microsoft SQL's implementation. //TODO test this form
     * The second form (not def USE_POLY) is more efficient for DB2 because it
     * compares envelopes of the geometries with the envelope of the viewing
     * rectangle instead of an edge-by-edge geometric comparison of the geometry
     * and the viewing rectangle. This has the drawback that it can sometimes
     * include geometries that are completely outside the viewing rectangle but
     * this is generally not an issue.
     */
#ifdef USE_POLY
    foo.setRealNumberPrecision( 8 );
    foo.setRealNumberNotation( QTextStream::FixedNotation );
    foo <<  qgsDoubleToString( request.filterRect().xMinimum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMinimum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMaximum() ) << ' ' << qgsDoubleToString( request.filterRect().yMinimum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMaximum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMaximum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMinimum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMaximum() ) << ", "
    <<  qgsDoubleToString( request.filterRect().xMinimum() ) << ' ' <<  qgsDoubleToString( request.filterRect().yMinimum() );

    mStatement += QString( " WHERE DB2GSE.ST_INTERSECTS(%1,DB2GSE.ST_POLYGON('POLYGON((%2))',%3)) = 1" ).arg(
                    mSource->mGeometryColName, r, QString::number( mSource->mSRId ) );
#else
    mStatement += QString( " WHERE DB2GSE.ENVELOPESINTERSECT(%1, %2, %3, %4, %5, %6) = 1" ).arg(
                    mSource->mGeometryColName,
                    qgsDoubleToString( request.filterRect().xMinimum() ),
                    qgsDoubleToString( request.filterRect().yMinimum() ),
                    qgsDoubleToString( request.filterRect().xMaximum() ),
                    qgsDoubleToString( request.filterRect().yMaximum() ),
                    QString::number( mSource->mSRId ) );

#endif
    filterAdded = true;
  }

  // set fid filter
  if ( request.filterType() == QgsFeatureRequest::FilterFid && !mSource->mFidColName.isEmpty() )
  {
    QString fidfilter = QString( " %1 = %2" ).arg( mSource->mFidColName, QString::number( request.filterFid() ) );
    // set attribute filter
    if ( !filterAdded )
      mStatement += " WHERE ";
    else
      mStatement += " AND ";

    mStatement += fidfilter;
    filterAdded = true;
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    if ( !filterAdded )
      mStatement += " WHERE (" + mSource->mSqlWhereClause + ")";
    else
      mStatement += " AND (" + mSource->mSqlWhereClause + ")";
  }

  if ( request.limit() >= 0 && request.filterType() != QgsFeatureRequest::FilterExpression )
    mStatement += QString( " FETCH FIRST %1 ROWS ONLY" ).arg( mRequest.limit() );

  QgsDebugMsg( mStatement );
#if 0 // TODO
  if ( fieldCount == 0 )
  {
    QgsDebugMsg( "QgsDb2Provider::select no fields have been requested" );
    mStatement.clear();
  }
#endif
}

bool QgsDb2FeatureIterator::fetchFeature( QgsFeature& feature )
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
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    QSqlRecord record = mQuery->record();
    for ( int i = 0; i < mAttributesToFetch.count(); i++ )
    {
      QVariant v = mQuery->value( i );
      QString attrName = record.fieldName( i );
      if ( attrName == mSource->mGeometryColName )
      {
        QgsDebugMsg( QString( "Geom col: %1" ).arg( attrName ) ); // not sure why we set geometry as a field value
 //       QgsDebugMsg( QString( "Field: %1; value: %2" ).arg( attrName ).arg( v.toString() ) );
        QgsDebugMsg( QString( "type: %1; typeName: %2" ).arg( v.type() ).arg( v.typeName() ) );
      }
      else
      {
        QgsDebugMsg( QString( "Field: %1; value: %2" ).arg( attrName ).arg( v.toString() ) );
        QgsDebugMsg( QString( "type: %1; typeName: %2" ).arg( v.type() ).arg( v.typeName() ) );
        /**
         * CHAR and VARCHAR fields seem to get corrupted sometimes when directly
         * calling feature.setAttribute(..., v) with mQuery->value(i). Workaround
         * that seems to fix the problem is to call v = QVariant(v.toString()).
         */
        if ( v.type() == QVariant::String )
        {
          QgsDebugMsg( "handle string type" );
          v = QVariant( v.toString() );
        }
        feature.setAttribute( mAttributesToFetch[i], v );
      }
    }
    QgsDebugMsg( QString( "Fid: %1; value: %2" ).arg( mSource->mFidColName ).arg( record.value( mSource->mFidColName ).toLongLong() ) );
    feature.setFeatureId( mQuery->record().value( mSource->mFidColName ).toLongLong() );

    // David Adler - assumes ST_AsBinary returns expected wkb
    // and setGeometry accepts this wkb
    if ( mSource->isSpatial() )
    {
#if 1
      QByteArray ar = record.value( mSource->mGeometryColName ).toByteArray();
      QString wkb( ar.toHex() );
      QgsDebugMsg( "wkb: " + wkb );
      QgsDebugMsg( "wkb size: " + QString( "%1" ).arg( ar.size() ) );
      size_t wkb_size = ar.size();
      if (0 < wkb_size) 
      {
      
      unsigned char* db2data = new unsigned char[wkb_size + 1]; // allocate persistent storage
      memcpy( db2data, ( unsigned char* )ar.data(), wkb_size + 1 );
      
      QgsGeometry *g = new QgsGeometry();
      g->fromWkb( db2data, wkb_size );
      feature.setGeometry( g );
//      QString wkt = ((QgsAbstractGeometryV2 *)g)->asWkt();
//      QString wkt = g->geometry()->asWkt();
      QgsDebugMsg( QString("geometry type: %1").arg(g->wkbType()) );
 
      QByteArray ar2((const char *)g->asWkb(), wkb_size + 1);
      QString wkb2(ar2.toHex());
      QgsDebugMsg("wkb2: " + wkb2); 
//      QgsDebugMsg("geometry WKT: " + wkt);
      } else {
        QgsDebugMsg("Geometry is empty");
      }
#else
      QByteArray ar = record.value( mSource->mGeometryColName ).toByteArray();
      size_t wkt_size = ar.size();
      char * wkt = new char[wkt_size + 1]; // allocate persistent storage
      strcpy( wkt, ar.data() );
      QgsDebugMsg( "wkt: " + QString( wkt ) );
      QString hex( ar.toHex() );
      QgsDebugMsg( "hex: " + hex );
      QgsDebugMsg( "wkt size: " + QString( "%1" ).arg( ar.size() ) );
      QgsGeometry *g = QgsGeometry::fromWkt( QString( wkt ) );
      feature.setGeometry( g );
      delete wkt;

#endif
    }

    feature.setValid( true );
    return true;
  }
  return false;
}


bool QgsDb2FeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( mStatement.isEmpty() )
  {
    QgsDebugMsg( "rewind on empty statement" );
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
    close();
    return false;
  }

  return true;
}

bool QgsDb2FeatureIterator::close()
{
  if ( mClosed )
    return false;

  if ( mQuery )
  {
    if ( !mQuery->isActive() )
    {
      QgsDebugMsg( "QgsDb2FeatureIterator::close on inactive query" );
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

///////////////

QgsDb2FeatureSource::QgsDb2FeatureSource( const QgsDb2Provider* p )
    : mFields( p->mAttributeFields )
    , mFidColName( p->mFidColName )
    , mGeometryColName( p->mGeometryColName )
    , mGeometryColType( p->mGeometryColType )
    , mSchemaName( p->mSchemaName )
    , mTableName( p->mTableName )
    , mUserName( p->mUserName )
    , mPassword( p->mPassword )
    , mService( p->mService )
    , mDatabaseName( p->mDatabaseName )
    , mDriver( p->mDriver )
    , mHost( p->mHost )
    , mPort( p->mPort )    
    , mSqlWhereClause( p->mSqlWhereClause )
{   //TODO set mEnvironment to LUW or ZOS? -David
  mSRId = p->mSRId;
}

QgsDb2FeatureSource::~QgsDb2FeatureSource()
{
}

QgsFeatureIterator QgsDb2FeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsDb2FeatureIterator( this, false, request ) );
}
