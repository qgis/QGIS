/***************************************************************************
    qgssqlanywhereprovider.cpp - QGIS data provider for SQL Anywhere DBMS
    --------------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author		 : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QLibrary>

#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsgeometry.h>
#include <qgsmessageoutput.h>
#include <qgsrectangle.h>
#include <qgscoordinatereferencesystem.h>
#include <qgslogger.h>
#include <qgscredentials.h>

#include "qgssqlanywhereprovider.h"

static const QString SQLANYWHERE_KEY = "sqlanywhere";
static const QString SQLANYWHERE_DESCRIPTION = "SQL Anywhere data provider";
static const QString SQLANYWHERE_STORAGETYPE = "SQL Anywhere database";
static const int sGeomTypeSelectLimit = 100;

QgsSqlAnywhereProvider::QgsSqlAnywhereProvider( QString const &uri )
    : QgsVectorDataProvider( uri )
    , mValid( true )
    , mKeyConstrained( false )
    , mGeomType( QGis::WKBUnknown )
    , mSrid( -1 )
    , mNumberFeatures( 0 )
    , mCapabilities( 0 )
    , mStmt( NULL )
    , mIdStmt( NULL )
    , mConnRO( NULL )
    , mConnRW( NULL )
{
  SaDebugMsg( "SQL Anywhere Provider constructor");

  // parsing members from the uri structure
  QgsDataSourceURI anUri = QgsDataSourceURI( uri );
  mConnectInfo = anUri.connectionInfo();
  mSchemaName = anUri.schema();
  mTableName = anUri.table();
  mQuotedTableName = anUri.quotedTablename();
  mGeometryColumn = anUri.geometryColumn();
  mSubsetString = anUri.sql();
  mKeyColumn = anUri.keyColumn();
  mUseEstimatedMetadata = anUri.useEstimatedMetadata();

  SaDebugMsg( "Connection info is " + mConnectInfo );
  SaDebugMsg( "Schema is: " + mSchemaName );
  SaDebugMsg( "Table name is: " + mTableName );
  SaDebugMsg( "Geometry column is: " + mGeometryColumn );
  SaDebugMsg( "Where clause is: " + mSubsetString );
  SaDebugMsg( "Supplied key column: " + mKeyColumn );
  SaDebugMsg( QString( "Estimate metadata?: %1" ).arg( mUseEstimatedMetadata ) );

  // load the SQL Anywhere interface
  if( !SqlAnyConnection::initApi() ) {
    mValid = false;

    showMessageBox( tr("Failed to load interface" ),
	    tr( SqlAnyConnection::failedInitMsg() ) );
    SaDebugMsg( "Invalid SQL Anywhere layer: unable to load C API" );
    return;
  }

  // open read-only and read-write connections to SQL Anywhere
  if( !ensureConnRO() || !ensureConnRW() ) {
    mValid = false;

    SaDebugMsg( "Invalid SQL Anywhere layer: no database connection" );
    showMessageBox( tr( "Failed to connect to database" ),
		  tr( "A connection to the SQL Anywhere database cannot "
		      "be established." ) );
    closeDb();
    return;
  }

  // sets mIsTable, mTableId, mIsComputed, mGeomType, mSrid
  if ( !checkLayerType() )  {
    mValid = false;

    // message box shown by checkLayerType()
    SaDebugMsg( "Invalid SQL Anywhere layer: bad layer type" );
    closeDb();
    return;
  }
 
  // sets mCrs, mSrsExtent
  if( !checkSrs() ) {
    mValid = false;
   
    // message box shown by checkSrs()
    SaDebugMsg( "Invalid SQL Anywhere layer: unsupported SRS" );
    closeDb();
    return;
  }

  // load the columns list to mAttributeFields and mAttributeDefaults
  if( !loadFields() ) {
    mValid = false;

    // message box shown by loadFields()
    SaDebugMsg( "Invalid SQL Anywhere layer: could not load fields" );
    closeDb();
    return;
  }
 
  // Sets mKeyColumn and mKeyConstrained 
  if( !findKeyColumn() ) {
    mValid = false;

    SaDebugMsg( "Invalid SQL Anywhere layer: no suitable key" );
    showMessageBox( tr( "No suitable key column" ),
		  tr( "The source relation %1 has no column suitable "
		      "for use as a unique key.\n\n"
		      "Quantum GIS requires that the relation has an integer "
		      "column no larger than 32 bits containing unique values. "
		      ) .arg( mQuotedTableName ) );
    closeDb();
    return;
  } else {
    SaDebugMsg( QString( "Using key column %1" ).arg( mKeyColumn ) );
  }

  // Sets mCapabilities
  if( !checkPermissions() ) {
    mValid = false;

    // message box shown by checkPermissions()
    SaDebugMsg( "Invalid SQL Anywhere layer: bad permissions" );
    closeDb();
    return;
  }

  // close read-write connection until user edits for first time
  closeConnRW();  

  // Set mNumberFeatures
  countFeatures();

  // Populate QgsVectorDataProvider::mNativeTypes
  setNativeTypes();
 
  // Reset QgsDataProvider::mDataSourceURI 
  // 1. add key column
  // 2. remove password so that it isn't written out in the layer definition
  anUri.setKeyColumn( mKeyColumn );
  setDataSourceUri( QgsDataSourceURI::removePassword( anUri.uri() ) );
 
} // QgsSqlAnywhereProvider::QgsSqlAnywhereProvider

QgsSqlAnywhereProvider::~QgsSqlAnywhereProvider()
{
  SaDebugMsg( "SQL Anywhere Provider destructor");
  closeDb();
} // QgsSqlAnywhereProvider::~QgsSqlAnywhereProvider()

void
QgsSqlAnywhereProvider::setNativeTypes()
{
    const SqlAnyTypeMap *map = mConnRO->typeMap();
    for( SqlAnyTypeMap::const_iterator it = map->constBegin() 
	    ; it != map->constEnd()
	    ; it++ ) {
	if( it.key() != DT_NOTYPE )
	    mNativeTypes << it.value();
    }
} // QgsSqlAnywhereProvider::setNativeTypes()

/**
 * Query the database schema to retrieve attribute names, types, default values
 */
bool 
QgsSqlAnywhereProvider::loadFields()
{
    QString	    sql;
    SqlAnyStatement *stmt;
    QSet<QString>   fieldNames;

    if( !ensureConnRO() ) {
	SaDebugMsg( "No database connection." );
	return false;
    }

    mAttributeFields.clear();
    mAttributeDefaults.clear();
    SaDebugMsg( QString("Loading fields for table %1").arg( mTableName ) );
    
    // Load field list by querying meta-data for a "SELECT FIRST *" query
    sql = QString( "SELECT FIRST * FROM %1" ).arg( mQuotedTableName );
    stmt = mConnRO->execute_direct( sql );
    if( !stmt->isValid() ) {
	reportError( tr( "Error loading attributes" ), stmt );
	delete stmt;
	return false;
    }

    for( int i = 0 ; i < stmt->numCols() ; i++ ) {
	a_sqlany_column_info	info;
	QString	    fieldName;
	int	    fieldLength;
	int	    fieldPrecision;
	QString	    fieldDefault;

	stmt->getColumnInfo( i, &info );

	fieldName = QString::fromUtf8( info.name );
	fieldLength = (int) info.max_size;
	fieldPrecision = (int) info.precision;
	const QgsVectorDataProvider::NativeType fieldNativeType = stmt->mapType( info.native_type );

	// skip the geom column
	if( fieldName == mGeometryColumn ) continue;

	// look for duplicates 
	if ( fieldNames.contains( fieldName ) ) {
	  showMessageBox( tr( "Ambiguous field!" ),
			  tr( "Duplicate field %1 found\n" ).arg( fieldName ) );
	  delete stmt;
	  return false;
	}


	mAttributeFields.insert( 
		fieldNames.size(),
		QgsField( fieldName, fieldNativeType.mType, fieldNativeType.mTypeName, fieldLength, fieldPrecision ) 
	    );

	fieldDefault = getDefaultValue( fieldName );
	mAttributeDefaults.insert( fieldNames.size(), fieldDefault );

	SaDebugMsg( QString("Loaded field %1: %2 with default value: %3")
			.arg( fieldNames.size() )
			.arg( fieldName ) 
			.arg( fieldDefault ) );

	fieldNames << fieldName;
    }

    delete stmt;
    return true;
} // QgsSqlAnywhereProvider::loadFields()


QString 
QgsSqlAnywhereProvider::storageType() const
{
    return "SQL Anywhere database";
} // QgsSqlAnywhereProvider::storageType() 

bool    
QgsSqlAnywhereProvider::nextFeature( QgsFeature & feature, SqlAnyStatement *stmt )
{
    bool    ok;
    int	    col_idx = 0;
    int	    id;
    a_sqlany_data_value geom;
    unsigned char	*geomBuf;
 
    ok = ( stmt != NULL && stmt->fetchNext() );
    if( ok ) {
	// first column contains mKeyColumn
	ok = stmt->getInt( col_idx++, id );
	feature.setFeatureId( id );
    }

    if( ok && mFetchGeom ) {
	// second column contains WKB geometry value
	ok = stmt->getColumn( col_idx++, &geom );
	if( ok ) {
	    geomBuf = new unsigned char[ *geom.length + 1 ];
	    memset( geomBuf, '\0', *geom.length + 1 );
	    memcpy( geomBuf, geom.buffer, *geom.length );
	    feature.setGeometryAndOwnership( geomBuf, *geom.length + 1 );
	} else {
	    SaDebugMsg( "Error retrieving feature geometry WKB." );
	    feature.setGeometryAndOwnership( 0, 0 );
	}
    }

    if( ok ) {
	// iterate mAttributesToFetch
	feature.clearAttributeMap();
	for ( QgsAttributeList::const_iterator it = mAttributesToFetch.constBegin()
		; it != mAttributesToFetch.constEnd()
		; it++ ) {
	    QVariant val;
	    if( field(*it).name() == mKeyColumn ) {
		stmt->getQVariant( 0, val );
	    } else {
		stmt->getQVariant( col_idx++, val );
	    }
	    feature.addAttribute( *it, val );
	}
	ok = ( col_idx == stmt->numCols() );
    }

    feature.setValid( ok );
    return ok;

} // QgsSqlAnywhereProvider::nextFeature( .. )

/**
 * Sets mNumberFeatures
 */
void 
QgsSqlAnywhereProvider::countFeatures() 
{
    QString		    sql;
    SqlAnyStatement	    *stmt;

    // Query total number of features
    mNumberFeatures = 0;
    if( mUseEstimatedMetadata ) {
	sql = QString( "SELECT 0 FROM %1 WHERE %2 " )
			.arg( mQuotedTableName )
			.arg( getWhereClause() );
	stmt = mConnRO->execute_direct( sql );
	if( stmt->isValid() ) {
	    mNumberFeatures = (long) stmt->numRows();
	    if( mNumberFeatures < 0 ) { 
		// indicates optimizer estimate
		mNumberFeatures = -mNumberFeatures; 
	    }
	}
	delete stmt;
    } else {
	sql = QString( "SELECT COUNT(*) FROM %1 WHERE %2 " )
			.arg( mQuotedTableName )
			.arg( getWhereClause() );
	stmt = mConnRO->execute_direct( sql );
	if( stmt->isValid() && stmt->fetchNext() ) {
	    stmt->getLong( 0, mNumberFeatures );
	}
	delete stmt;
    }
	
    SaDebugMsg( QString( "Number of features: %1 %2" )
		.arg( mNumberFeatures ) 
		.arg( mUseEstimatedMetadata ? "(estimated)" : "" ) );

} // QgsSqlAnywhereProvider::countFeatures() 

/** 
 * Return the extent for this data layer
*/
QgsRectangle
QgsSqlAnywhereProvider::extent()
{
    QString	    sql;
    SqlAnyStatement *stmt;
    double	    xmin, xmax, ymin, ymax;

    if( isValid() && mLayerExtent.isEmpty() ) {
	// test whether layer is empty
	sql = QString( "SELECT FIRST 0 FROM %1 " )
		.arg( geomSampleSet() ) ;
	stmt = mConnRO->execute_direct( sql );
	if( !stmt->isValid() || !stmt->fetchNext() ) {
	    delete stmt;
	    return mSrsExtent;
	}
	delete stmt;

	// need to recompute extent 
	sql = QString( "SELECT COALESCE( MIN( %1.ST_XMin() ), %3), "
			    "COALESCE( MAX( %1.ST_XMax() ), %4), "
			    "COALESCE( MIN( %1.ST_YMin() ), %5), "
			    "COALESCE( MAX( %1.ST_YMax() ), %6) "
			"FROM %2 " )
	    .arg( geomColIdent() )
	    .arg( geomSampleSet() ) 
	    .arg( mSrsExtent.xMinimum() )
	    .arg( mSrsExtent.xMaximum() )
	    .arg( mSrsExtent.yMinimum() )
	    .arg( mSrsExtent.yMaximum() );
	
	stmt = mConnRO->execute_direct( sql );
	if( !stmt->isValid() || !stmt->fetchNext() ) {
	    delete stmt;
	    return mSrsExtent;
	}
	stmt->getDouble( 0, xmin );
	stmt->getDouble( 1, xmax );
	stmt->getDouble( 2, ymin );
	stmt->getDouble( 3, ymax );
	delete stmt;

	// set mLayerExtent
	mLayerExtent.setXMinimum( xmin );
	mLayerExtent.setXMaximum( xmax );
	mLayerExtent.setYMinimum( ymin );
	mLayerExtent.setYMaximum( ymax );

	// SQL Anywhere allows data to extend a certain amount past
	// the SRS extent.  Therefore, adjust mSrsExtent
	// to accommodate any overage.
	mSrsExtent.unionRect( mLayerExtent );

	SaDebugMsg( QString( "Adjusting mSrsExtent = %1" )
			.arg( mSrsExtent.toString() ) );

    }

    SaDebugMsg( QString( "Returning mLayerExtent = %1" )
		    .arg( mLayerExtent.toString() ) );

    return mLayerExtent;
} // QgsSqlAnywhereProvider::extent()


QString 
QgsSqlAnywhereProvider::subsetString()
{
    return mSubsetString;
} // QgsSqlAnywhereProvider::subsetString()

bool 
QgsSqlAnywhereProvider::setSubsetString( QString theSQL )
{
  QString   prevSubsetString = mSubsetString;
  QString   prevKeyColumn = mKeyColumn;

  mSubsetString = theSQL.trimmed();
  SaDebugMsg( QString( "Setting new WHERE clause: %2 " ).arg( mSubsetString ) );

  if( !mKeyConstrained ) {
    // check whether existing key is still valid or, if not,
    // whether a different key column could be used instead
    // (possibly changes mKeyColumn and mKeyConstrained)
    if( !findKeyColumn() ) {
	SaDebugMsg( "Reverting WHERE clause due to no suitable key." );
	mSubsetString = prevSubsetString;
	mKeyColumn = prevKeyColumn;
	mKeyConstrained = false;
	return false;
    } else if( prevKeyColumn != mKeyColumn ) {
	SaDebugMsg( "Changed key column from " + prevKeyColumn + " to " + mKeyColumn );
    }
  }

  // update datasource URI
  QgsDataSourceURI uri = QgsDataSourceURI( dataSourceUri() );
  uri.setSql( mSubsetString );
  uri.setKeyColumn( mKeyColumn );
  setDataSourceUri( uri.uri() );

  // recalculate mNumberFeatures
  countFeatures();

  // indicate that mLayerExtent needs to be recalculated upon next access
  mLayerExtent.setMinimal();

  // clear cursors
  closeCursors();

  return true;
} // QgsSqlAnywhereProvider::setSubsetString()

/**
 * Checks to see if the given column contains unique data
 */
bool 
QgsSqlAnywhereProvider::hasUniqueData( QString colName )
{
    SqlAnyStatement *stmt;
    bool isUnique = true;
    QString sql = QString( "SELECT 0 "
		      "FROM %1 "
		      "WHERE %2 "
		      "HAVING COUNT(DISTINCT %3)<COUNT(%3)" )
			.arg( mQuotedTableName )
			.arg( getWhereClause() )
			.arg( quotedIdentifier( colName ) );
    stmt = mConnRO->execute_direct( sql );
    if( !stmt->isValid() || stmt->fetchNext() ) {
	SaDebugMsg( QString( "Column %1 is not unique.")
		 .arg( mQuotedTableName + "(" + colName + ")" ) ); 
	isUnique = false;
    }
    delete stmt;
    return isUnique;
} // QgsSqlAnywhereProvider::hasUniqueData()

// findKeyColumn - sets mKeyColumn and mKeyConstrained
bool 
QgsSqlAnywhereProvider::findKeyColumn()
{
    QSet<QString>   testedCols;
    QString	    sql;
    SqlAnyStatement *stmt;

    SaDebugMsg( QString("Finding key column for table %1").arg( mTableName ) );

    // test suitability of given key
    if( !mKeyColumn.isEmpty() ) {
	bool		keyIsValid = true;
	unsigned int	colId;

	// test whether key column is a member of table and appropriate type
	sql = QString( "SELECT FIRST column_id "
			"FROM SYSTABCOL "
			"WHERE table_id = %1 "
			"AND column_name = %2 "
			"AND base_type_str IN ( 'int', 'unsigned int', 'smallint', 'unsigned smallint', 'tinyint' ) " ) 
		.arg( mTableId )
		.arg( quotedValue( mKeyColumn ) );
	stmt = mConnRO->execute_direct( sql );
	keyIsValid = stmt->isValid() && stmt->fetchNext() && stmt->getUInt( 0, colId );
	delete stmt;

	if( keyIsValid ) {
	    // see if sa_describe_query identifies this column
	    // as the primary key (works for tables and views)
	    QString keyCol;
	    sql = QString( "SELECT name "
			    "FROM sa_describe_query( "
				"'SELECT * FROM %1', 1 ) "
			    "WHERE is_key_column = 1" )
		    .arg( mQuotedTableName );
	    stmt = mConnRO->execute_direct( sql );
	    mKeyConstrained = stmt->isValid()
		&& stmt->fetchNext()
		&& stmt->getString( 0, keyCol )
		&& keyCol == mKeyColumn
		&& !stmt->fetchNext();
	    delete stmt;

	    if( !mKeyConstrained ) {
		// look for a unique index on the column (works for tables,
		// or materialized views, not for logical views)
		sql = QString( "SELECT 0 "
				"FROM SYSIDX i "
				"JOIN SYSIDXCOL ic "
				    "ON ic.table_id = i.table_id "
				    "AND ic.index_id = i.index_id "
				"WHERE i.table_id = %1 "
				"AND ic.column_id = %2 "
				"AND i.\"unique\" IN ( 1, 2 ) "
				"AND NOT EXISTS ( "
				    "SELECT column_id "
				    "FROM SYSIDXCOL ic2 "
				    "WHERE ic2.table_id = ic.table_id "
				    "AND ic2.index_id = ic.index_id "
				    "AND ic2.sequence > 0 ) " ) 
				.arg( mTableId )
				.arg( colId );
		stmt = mConnRO->execute_direct( sql );
		mKeyConstrained = stmt->isValid() && stmt->fetchNext();
		delete stmt;
	    }

	    keyIsValid = mKeyConstrained || hasUniqueData( mKeyColumn );
	}
	
	if( keyIsValid ) {
	    // success!
	    return true;
	} else {
	    SaDebugMsg( QString( "Given column %1 is not a suitable key." )
			    .arg( mKeyColumn ) );
	    testedCols << mKeyColumn;
	    mKeyColumn.clear();
	}
    }

    // see if sa_describe_query returns a single key of the correct type
    sql = QString( "SELECT name "
		    "FROM sa_describe_query( "
			"'SELECT * FROM %1', 1 ) dq1 "
		    "WHERE is_key_column = 1 " 
		    "AND is_added_key_column = 0 "
		    "AND domain_name IN ( 'int', 'unsigned int', 'smallint', 'unsigned smallint', 'tinyint' ) "
		    "AND NOT EXISTS ( "
			"SELECT column_number "
			"FROM sa_describe_query( "
			    "'SELECT * FROM %1', 1 ) dq2 "
			"WHERE is_key_column = 1 "
			"AND dq2.column_number <> dq1.column_number "
			")"
		    )
	    .arg( mQuotedTableName );
    stmt = mConnRO->execute_direct( sql );
    if( stmt->isValid() && stmt->fetchNext() ) {
	// valid constrained key found!
	stmt->getString( 0, mKeyColumn );
	mKeyConstrained = true;
	delete stmt;
	return true;
    }
    delete stmt;

    // iterate over single-column unique constraints 
    sql = QString( "SELECT c.column_name "
		    "FROM SYSIDX i "
		    "JOIN SYSIDXCOL ic "
			"ON ic.table_id = i.table_id "
			"AND ic.index_id = i.index_id "
		    "JOIN SYSTABCOL c "
			"ON c.table_id = ic.table_id "
			"AND c.column_id = ic.column_id "
		    "WHERE i.table_id = %1 "
		    "AND i.\"unique\" IN ( 1, 2 ) "
		    "AND c.base_type_str IN ( 'int', 'unsigned int', 'smallint', 'unsigned smallint', 'tinyint' ) "
		    "AND NOT EXISTS ( "
			"SELECT column_id "
			"FROM SYSIDXCOL ic2 "
			"WHERE ic2.table_id = ic.table_id "
			"AND ic2.index_id = ic.index_id "
			"AND ic2.sequence > 0 ) " )
		    .arg( mTableId );
    stmt = mConnRO->execute_direct( sql );
    if( stmt->isValid() && stmt->fetchNext() ) {
	// valid constrained key found!
	stmt->getString( 0, mKeyColumn );
	mKeyConstrained = true;
	delete stmt;
	return true;
    }
    delete stmt;


    // iterate over remaining columns of integer type with <= 32 bits
    sql = QString( "SELECT column_name "
		    "FROM SYSTABCOL "
		    "WHERE table_id = %1 "
		    "AND base_type_str IN ( 'int', 'unsigned int', 'smallint', 'unsigned smallint', 'tinyint' ) " ) 
	    .arg( mTableId );
    stmt = mConnRO->execute_direct( sql );
    while( stmt->isValid() && stmt->fetchNext() ) {
	QString colName;
	stmt->getString( 0, colName );
	if( hasUniqueData( colName ) ) {
	    // valid unconstrained key found
	    mKeyColumn = colName;
	    mKeyConstrained = false;
	    delete stmt;
	    return true;
	}
    }
    delete stmt;

    SaDebugMsg( "No key column found." );
    return false;
} // QgsSqlAnywhereProvider::findKeyColumn()


bool 
QgsSqlAnywhereProvider::featureAtId( int featureId, QgsFeature & feature, bool fetchGeometry, QgsAttributeList fetchAttributes )
{
    a_sqlany_bind_param	    idParam;
    size_t		    idLen = sizeof( int );

    if( !ensureConnRO() ) {
	SaDebugMsg( "No database connection." );
	return false;
    }

    SaDebugMsg( QString( "Fetching feature for id=%1" ).arg(featureId) );

    mAttributesToFetch = fetchAttributes;
    mFetchGeom = fetchGeometry;

    // set or reset mIdStmt cursor
    if( mIdStmt == NULL 
	    || !mIdStmt->isValid() 
	    || fetchAttributes != mIdStmtAttributesToFetch
	    || fetchGeometry != mIdStmtFetchGeom 
	    || !mIdStmt->reset() ) {
	
	// prepare a new cursor
	mIdStmtAttributesToFetch = fetchAttributes;
	mIdStmtFetchGeom = fetchGeometry;

	QString whereClause = getWhereClause() 
		    + QString( "AND %1 = ? " )
			.arg( quotedIdentifier( mKeyColumn ) );

	if( mIdStmt ) { delete mIdStmt; }
	mIdStmt = mConnRO->prepare( makeSelectSql( whereClause ) );
    }

    // bind parameter
    mIdStmt->describe_bind_param( 0, idParam );
    idParam.value.buffer = (char *) &featureId;
    idParam.value.length = & idLen;
    idParam.value.type = A_VAL32;
    mIdStmt->bind_param( 0, idParam );

    mIdStmt->execute();
    return nextFeature( feature, mIdStmt );

} // QgsSqlAnywhereProvider::featureAtId()


void 
QgsSqlAnywhereProvider::select( QgsAttributeList fetchAttributes, QgsRectangle rect, bool fetchGeometry, bool useIntersect )
{
    double xmin;
    double ymin;
    double xmax;
    double ymax;

    a_sqlany_bind_param	xminParam;
    a_sqlany_bind_param	yminParam;
    a_sqlany_bind_param	xmaxParam;
    a_sqlany_bind_param	ymaxParam;

    size_t xminLen = sizeof( double );
    size_t yminLen = sizeof( double );
    size_t xmaxLen = sizeof( double );
    size_t ymaxLen = sizeof( double );

    if( !ensureConnRO() ) {
	SaDebugMsg( "No database connection." );
	return;
    }

    SaDebugMsg( "initial rectangle =" + rect.toString() );

    rect = rect.intersect( &mSrsExtent );

    SaDebugMsg( "rectangle clipped to SRS extent =" + rect.toString() );

    mAttributesToFetch = fetchAttributes;
    mFetchGeom = fetchGeometry;

    // set or reset mStmt cursor
    if( mStmt == NULL 
	    || !mStmt->isValid()
	    || fetchAttributes != mStmtAttributesToFetch
	    || mStmtRect.isEmpty() != rect.isEmpty()
	    || fetchGeometry != mStmtFetchGeom 
	    || useIntersect != mStmtUseIntersect
	    || !mStmt->reset() ) {

	// prepare a new cursor
	mStmtAttributesToFetch = fetchAttributes;
	mStmtFetchGeom = fetchGeometry;
	mStmtUseIntersect = useIntersect;

	QString whereClause = getWhereClause();
	if( !rect.isEmpty() ) {
	    whereClause += QString( "AND %1 .%2 ( new ST_Polygon( "
				    "new ST_Point( ?, ?, %3), "
				    "new ST_Point( ?, ?, %3 ) ) ) = 1 " )
			.arg( geomColIdent() )
			.arg( ( useIntersect ? "ST_Intersects" : "ST_IntersectsFilter" ) )
			.arg( mSrid );
	}

	if( mStmt ) { delete mStmt; }
	mStmt = mConnRO->prepare( makeSelectSql( whereClause ) );
    }

    // bind parameters if necessary
    mStmtRect = rect;
    if( !rect.isEmpty() ) {
	xmin = mStmtRect.xMinimum();
	ymin = mStmtRect.yMinimum();
	xmax = mStmtRect.xMaximum();
	ymax = mStmtRect.yMaximum();

	if( !mStmt->describe_bind_param( 0, xminParam )
		|| !mStmt->describe_bind_param( 1, yminParam )
		|| !mStmt->describe_bind_param( 2, xmaxParam )
		|| !mStmt->describe_bind_param( 3, ymaxParam ) ) {
	    reportError( tr( "Error describing bind parameters" ), mStmt );
	    return;
	}

	xminParam.value.buffer = (char *) &xmin;
	yminParam.value.buffer = (char *) &ymin;
	xmaxParam.value.buffer = (char *) &xmax;
	ymaxParam.value.buffer = (char *) &ymax;

	xminParam.value.length = &xminLen;
	yminParam.value.length = &yminLen;
	xmaxParam.value.length = &xmaxLen;
	ymaxParam.value.length = &ymaxLen;

	xminParam.value.type = A_DOUBLE;
	yminParam.value.type = A_DOUBLE;
	xmaxParam.value.type = A_DOUBLE;
	ymaxParam.value.type = A_DOUBLE;

	if( !mStmt->bind_param( 0, xminParam )
		|| !mStmt->bind_param( 1, yminParam )
		|| !mStmt->bind_param( 2, xmaxParam )
		|| !mStmt->bind_param( 3, ymaxParam ) ) {
	    reportError( tr( "Error binding parameters" ), mStmt );
	    return;
	}
    }

    mStmt->execute();

} // QgsSqlAnywhereProvider::select()

bool 
QgsSqlAnywhereProvider::nextFeature( QgsFeature & feature )
{
  feature.setValid( false );
  if ( !isValid() ) {
    SaDebugMsg( "Read attempt on an invalid SQL Anywhere data source" );
    return false;
  }

  if ( mStmt == NULL || !mStmt->isValid() ) {
    SaDebugMsg( "nextFeature() called with invalid cursor()" );
    return false;
  }

  return nextFeature( feature, mStmt );
} // QgsSqlAnywhereProvider::nextFeature()


/**
 * makeSelectSql - returns a query such that
 *  1. first column is mKeyColumn
 *  2. if mFetchGeom then next column is mGeomColumn
 *  3. remaining columns are mAttributesToFetch less mKeyColumn 
 */
QString
QgsSqlAnywhereProvider::makeSelectSql( QString whereClause ) const
{
    QString sql = QString( "SELECT %1" ).arg( quotedIdentifier( mKeyColumn ) );
    if( mFetchGeom ) {
	sql += QString( ", %1 .ST_AsBinary('WKB(Version=1.2;endian=%2)') " )
		.arg( geomColIdent() )
		.arg( QgsApplication::endian() == QgsApplication::XDR ?  "xdr" : "ndr" );
    }

    for ( QgsAttributeList::const_iterator it = mAttributesToFetch.constBegin()
	    ; it != mAttributesToFetch.constEnd()
	    ; it++ ) {
	QString attrName = field(*it).name();
	if ( !attrName.isEmpty() && attrName != mKeyColumn ) { 
	    sql += "," + quotedIdentifier( attrName );
	}
    }

    sql += QString( " FROM %1 WHERE %2 OPTIONS(FORCE OPTIMIZATION)" )
		.arg( mQuotedTableName )
		.arg( whereClause );

    return sql;

} // QgsSqlAnywhereProvider::makeSelectSql()

/**
 * Restart reading features from previous select operation
 */
void 
QgsSqlAnywhereProvider::rewind() 
{
    select( mStmtAttributesToFetch, mStmtRect, mStmtFetchGeom, mStmtUseIntersect );
}


QString 
QgsSqlAnywhereProvider::name() const
{
    SaDebugMsg( "Returning name " + SQLANYWHERE_KEY );
    return SQLANYWHERE_KEY;
} // QgsSqlAnywhereProvider::name() 


QString 
QgsSqlAnywhereProvider::description() const
{
    return SQLANYWHERE_DESCRIPTION;
} // QgsSqlAnywhereProvider::description() 


// Returns the minimum or maximum value of an attribute
QVariant 
QgsSqlAnywhereProvider::minmaxValue( int index, const QString minmax )
{
    QString	    attrName;
    QString	    sql;
    SqlAnyStatement *stmt;
    QVariant	    val;

    attrName = field(index).name();
    sql = QString( "SELECT %1( %2 ) FROM %3 WHERE %4 " )
	.arg( minmax )
	.arg( quotedIdentifier( attrName ) )
	.arg( mQuotedTableName )
	.arg( getWhereClause() );

    stmt = mConnRO->execute_direct( sql );
    if( !stmt->isValid() || !stmt->fetchNext() || !stmt->getQVariant( 0, val ) ) {
	val = QVariant( QString::null );
    }
    delete stmt;
    return val;

} // QgsSqlAnywhereProvider::minmaxValue()


// Returns the list of unique values of an attribute
void 
QgsSqlAnywhereProvider::uniqueValues( int index, QList < QVariant > &uniqueValues, int limit )
{
    QString	    attrName;
    QString	    sql;
    SqlAnyStatement *stmt;

    uniqueValues.clear();

    attrName = field(index).name();
    if( !attrName.isEmpty() ) {
	sql = QString( "SELECT DISTINCT %1 FROM %2 WHERE %3 " )
	    .arg( quotedIdentifier( attrName ) )
	    .arg( mQuotedTableName )
	    .arg( getWhereClause() );

	if( limit >= 0 ) {
	    sql = QString( "SELECT TOP %1 %2 FROM ( %3 ) AS uniqueVals " )
		    .arg(limit)
		    .arg( quotedIdentifier( attrName ) )
		    .arg( sql );
	}
	sql += QString( "ORDER BY %1 " )
	    .arg( quotedIdentifier( attrName ) );

	stmt = mConnRO->execute_direct( sql );
	while( stmt->isValid() && stmt->fetchNext() ) {
	    QVariant val;
	    stmt->getQVariant( 0, val );
	    uniqueValues.append( val );
	}
	delete stmt;
    }
} // QgsSqlAnywhereProvider::uniqueValues()

QString
QgsSqlAnywhereProvider::getDefaultValue( QString attrName )
{
    QString	    sql;
    SqlAnyStatement *stmt;
    QString	    val;

    sql = QString( "SELECT \"default\" "
		    "FROM SYSTABCOL " 
		    "WHERE table_id = %1 " 
		    "AND column_name = %2 "
		    )
	.arg( mTableId )
	.arg( quotedValue( attrName ) );

    stmt = mConnRO->execute_direct( sql );
    if( !stmt->isValid() || !stmt->fetchNext() || !stmt->getString( 0, val ) ) {
	val = QString( QString::null );
    } else {
	val.replace( "'", "" );
    }
    delete stmt;

    return val;
} // QgsSqlAnywhereProvider::defaultValue()

bool 
QgsSqlAnywhereProvider::addFeatures( QgsFeatureList & flist )
{
    bool	    ok = true;
    sacapi_i32	    code;
    char	    errbuf[SACAPI_ERROR_SIZE];
    QgsRectangle    rect;

    if( !(mCapabilities & QgsVectorDataProvider::AddFeatures) ) {
	return false;
    }

    if( flist.size() == 0 ) {
	return true;
    }

    if( !ensureConnRW() ) {
	SaDebugMsg( "No read-write database connection." );
	return false;
    }

    mConnRW->begin();

    // iterate features
    for ( QgsFeatureList::iterator fit = flist.begin()
	    ; ok && fit != flist.end()
	    ; fit++ ) {

	SqlAnyStatement	*stmt;
	QString		sql;
	QString		values;
	const QgsAttributeMap &attrMap = fit->attributeMap();

	// retrieve geometry value
	QgsGeometry	geom = *fit->geometry();
	QByteArray	wktBa = geom.exportToWkt().toUtf8();
	size_t		wktLen = wktBa.length();
	a_sqlany_bind_param wktParam;
	rect.unionRect( geom.boundingBox() );

	// construct insert statement with geometry as parameter
	sql = QString( "INSERT INTO %1 ( %2" )
			.arg( mQuotedTableName )
			.arg( geomColIdent() );
	values = QString( ") VALUES ( ST_Geometry::ST_GeomFromText(?, %1)" )
			.arg( mSrid );

	// iterate attributes
	for ( QgsAttributeMap::const_iterator ait = attrMap.constBegin()
		; ait != attrMap.constEnd()
		; ait++ ) {

	    QString attr = field( ait.key() ).name();
	    if ( !attr.isEmpty() && attr != mGeometryColumn ) {
		sql += ", " + attr;
		values += ", " + quotedValue( ait->toString() );
	    }
	}
	sql += values + " )";

	// prepare statement, bind geometry value, execute insert

	stmt = mConnRW->prepare( sql );
	ok = stmt->isValid();
	ok = ok && stmt->describe_bind_param( 0, wktParam );
	wktParam.value.buffer = wktBa.data();
	wktParam.value.length = &wktLen;
	wktParam.value.type = A_STRING;
	ok = ok && stmt->bind_param( 0, wktParam );
	ok = ok && stmt->execute();
	delete stmt;
    }


    if( ok ) {
	SaDebugMsg( "Committing feature insertions" );
	ok = mConnRW->commit( code, errbuf, sizeof( errbuf ) );
    } else {
	mConnRW->getError( code, errbuf, sizeof( errbuf ) );

	SaDebugMsg( "Rolling back feature insertions" );
	mConnRW->rollback();
    }

    if( ok ) {
	// expand extents to accomodate new geometries
	mSrsExtent.unionRect( rect );
	if( !mLayerExtent.isEmpty() ) {
	    mLayerExtent.unionRect( rect );
	}
    } else {
	reportError( tr( "Error inserting features" ), code, errbuf );
    }

    return ok;

} // QgsSqlAnywhereProvider::addFeatures()


bool 
QgsSqlAnywhereProvider::deleteFeatures( const QgsFeatureIds & id )
{
    bool	    ok;
    QString	    sql;
    sacapi_i32	    code;
    char	    errbuf[SACAPI_ERROR_SIZE];

    if( !(mCapabilities & QgsVectorDataProvider::DeleteFeatures) ) {
	return false;
    }

    if( id.size() == 0 ) {
	return true;
    }

    if( !ensureConnRW() ) {
	SaDebugMsg( "No read-write database connection." );
	return false;
    }

    sql = QString( "DELETE FROM %1 WHERE %2 AND %3 IN ( " )
	    .arg( mQuotedTableName )
	    .arg( getWhereClause() )
	    .arg( mKeyColumn );
    for( QgsFeatureIds::const_iterator it = id.constBegin()
	    ; it != id.constEnd() 
	    ; it++ ) {
	if( it != id.constBegin() ) {
	    sql += ", ";
	}
	sql += QString( "%1" ).arg( *it );
    }
    sql += " ) ";

    mConnRW->begin();
    ok = mConnRW->execute_immediate( sql, code, errbuf, sizeof( errbuf ) );
    if( ok ) {
	SaDebugMsg( "Committing feature deletions" );
	ok = mConnRW->commit( code, errbuf, sizeof( errbuf ) );
    } else {
	SaDebugMsg( "Rolling back feature deletions" );
	mConnRW->rollback();
    }

    if( !ok ) {
	reportError( tr( "Error deleting features" ), code, errbuf );
    }

    return ok;
} // QgsSqlAnywhereProvider::deleteFeatures()


bool 
QgsSqlAnywhereProvider::addAttributes( const QList<QgsField> &attributes )
{
    bool	    ok=true;
    QString	    sql;
    sacapi_i32	    code;
    char	    errbuf[SACAPI_ERROR_SIZE];

    if( !(mCapabilities & QgsVectorDataProvider::AddAttributes) ) {
	return false;
    }

    if( attributes.size() == 0 ) {
	return true;
    }

    if( !ensureConnRW() ) {
	SaDebugMsg( "No read-write database connection." );
	return false;
    }
 
    sql = QString( "ALTER TABLE %1 " ).arg( mQuotedTableName );
    for( QList<QgsField>::const_iterator it = attributes.constBegin()
	    ; ok && it != attributes.constEnd()
	    ; it++ ) {

	QString datatype = it->typeName();
	if( datatype == "CHAR" 
		|| datatype == "VARCHAR"
		|| datatype == "BINARY"
		) {
	    datatype += QString( "(%1)" ).arg( it->length() );
	} else if ( datatype == "DECIMAL" ) {
	    datatype += QString( "(%1, %2)" )
		.arg( it->length() )
		.arg( it->precision() );
	}

	sql += ( it != attributes.constBegin() ? ", " : "" );
	sql += QString( "ADD %2 AS %3 " )
		.arg( quotedIdentifier( it->name() ) )
		.arg( datatype );
    }

    // clear cursors to release table locks
    closeCursors();

    mConnRW->begin();
    ok = mConnRW->execute_immediate( sql, code, errbuf, sizeof( errbuf ) );
    if( ok ) {
	// The ALTER TABLE statement already forced an implicit COMMIT 
	// on the database server, but we need an explicit commit here
	// to release the mutex held on the connection object
	SaDebugMsg( "Committing alter table" );
	ok = mConnRW->commit( code, errbuf, sizeof( errbuf ) );

	// Add column comments.  The ALTER TABLE statement was
	// already committed, so treat this operation as best-effort.
	mConnRW->begin();
	for( QList<QgsField>::const_iterator it = attributes.constBegin()
		; ok && it != attributes.constEnd()
		; it++ ) {

	    if( !it->comment().isEmpty() ) {
		sql = QString( "COMMENT ON COLUMN %1.%2 IS %3 " )
		    .arg( mQuotedTableName )
		    .arg( quotedIdentifier( it->name() ) )
		    .arg( quotedValue( it->comment() ) );
		mConnRW->execute_immediate( sql, code, errbuf, sizeof(errbuf) );
	    }
	}
	mConnRW->commit( code, errbuf, sizeof( errbuf ) );

    } else {
	SaDebugMsg( "Rolling back alter table" );
	mConnRW->rollback();
    }

    if( !ok ) {
	reportError( tr( "Error adding attributes" ), code, errbuf );
    }

    // reset the layer
    loadFields();

    return ok;
} // QgsSqlAnywhereProvider::addAttributes() 


bool 
QgsSqlAnywhereProvider::deleteAttributes( const QgsAttributeIds & ids )
{
    bool	    ok;
    QString	    sql;
    sacapi_i32	    code;
    char	    errbuf[SACAPI_ERROR_SIZE];

    if( !(mCapabilities & QgsVectorDataProvider::AddAttributes) ) {
	return false;
    }

    if( ids.size() == 0 ) {
	return true;
    }

    if( !ensureConnRW() ) {
	SaDebugMsg( "No read-write database connection." );
	return false;
    }

    sql = QString( "ALTER TABLE %1 " ).arg( mQuotedTableName );
    int cnt = 0;
    for( QgsAttributeIds::const_iterator it = ids.constBegin()
	    ; it != ids.constEnd()
	    ; it++ ) {

	QString attr = field(*it).name();
	if( !attr.isEmpty() ) {
	    sql += ( cnt++ != 0 ? ", " : "" );
	    sql += QString( "DROP %1 " )
		.arg( quotedIdentifier( attr ) );
	}
    }

    // clear cursors to release table locks
    closeCursors();

    mConnRW->begin();
    ok = mConnRW->execute_immediate( sql, code, errbuf, sizeof( errbuf ) );
    if( ok ) {
	SaDebugMsg( "Committing alter table" );
	ok = mConnRW->commit( code, errbuf, sizeof( errbuf ) );
    } else {
	SaDebugMsg( "Rolling back alter table" );
	mConnRW->rollback();
    }

    if( !ok ) {
	reportError( tr( "Error deleting attributes" ), code, errbuf );
    }

    // reset the layer
    loadFields();

    return ok;
} // QgsSqlAnywhereProvider::deleteAttributes() 


bool 
QgsSqlAnywhereProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
    bool	    ok = true;
    sacapi_i32	    code;
    char	    errbuf[SACAPI_ERROR_SIZE];

    if( !(mCapabilities & QgsVectorDataProvider::ChangeAttributeValues) ) {
	return 0;
    }

    if( attr_map.size() == 0 ) {
	return true;
    }

    if( !ensureConnRW() ) {
	SaDebugMsg( "No read-write database connection." );
	return false;
    }

    mConnRW->begin();

    // iterate each feature requiring update
    for( QgsChangedAttributesMap::const_iterator fit = attr_map.constBegin()
	    ; ok && fit != attr_map.constEnd()
	    ; fit++ ) {

	// skip added features
	if ( fit.key() < 0 )
	  continue;

	// iterate each attribute requiring update
	QString sql = QString( "UPDATE %1 SET " ).arg( mQuotedTableName );
	int cnt=0;
	for( QgsAttributeMap::const_iterator ait = fit->constBegin()
		; ok && ait != fit->constEnd()
		; ait++ ) {

	    QString attr = field( ait.key() ).name();
	    sql += ( cnt++ != 0 ? ", " : "" );
	    sql += QString( "%1=%2" )
		    .arg( attr )
		    .arg( quotedValue( ait->toString() ) );
	    ok = !attr.isEmpty();
	}

	sql += QString( " WHERE %1 AND %2=%3 " )
		.arg( getWhereClause() )
		.arg( mKeyColumn )
		.arg( fit.key() );

	if( ok ) {
	    ok = mConnRW->execute_immediate( sql, code, errbuf, sizeof(errbuf) );
	} else {
	    strcpy( tr( "Attribute not found" ).toUtf8().data(), errbuf );
	}
    }

    if( ok ) {
	SaDebugMsg( "Committing attribute updates" );
	ok = mConnRW->commit( code, errbuf, sizeof( errbuf ) );
    } else {
	SaDebugMsg( "Rolling back attribute updates" );
	mConnRW->rollback();
    }

    if( !ok ) {
	reportError( tr( "Error updating attributes" ), code, errbuf );
    }

    return ok;
} // QgsSqlAnywhereProvider::changeAttributeValues() 


bool 
QgsSqlAnywhereProvider::changeGeometryValues( QgsGeometryMap & gmap )
{
    bool		ok;
    QString		sql;
    sacapi_i32		code;
    char		errbuf[SACAPI_ERROR_SIZE];
    SqlAnyStatement	*stmt;
    QgsRectangle	rect;
    size_t		wktLen;
    a_sqlany_bind_param wktParam;
    size_t		keyLen = sizeof( int );
    a_sqlany_bind_param keyParam;

    if( !(mCapabilities & QgsVectorDataProvider::ChangeGeometries) ) {
	return false;
    }

    if( gmap.size() == 0 ) {
	return true;
    }

    if( !ensureConnRW() ) {
	SaDebugMsg( "No read-write database connection." );
	return false;
    }

    mConnRW->begin();
    sql = QString( "UPDATE %1 SET %2=ST_Geometry::ST_GeomFromText(?, %3) WHERE %4 AND %5=? " )
	    .arg( mQuotedTableName )
	    .arg( geomColIdent() )
	    .arg( mSrid )
	    .arg( getWhereClause() )
	    .arg( mKeyColumn );
    stmt = mConnRW->prepare( sql );
    ok = stmt->isValid();

    for( QgsGeometryMap::const_iterator it = gmap.constBegin()
	    ; ok && it != gmap.constEnd()
	    ; it++ ) {
	int key = it.key();
	QgsGeometry geom = *it;
	QString wkt = geom.exportToWkt();
	QByteArray wktBa = wkt.toUtf8();
	rect.unionRect( geom.boundingBox() );

	SaDebugMsg( QString("Setting geometry value for feature id %1 to '%2'")
		.arg( key ) 
		.arg( wkt ) );

	ok = stmt->reset();
	ok = ok && stmt->describe_bind_param( 0, wktParam );
	ok = ok && stmt->describe_bind_param( 1, keyParam );

	wktParam.value.buffer = wktBa.data();
	wktLen = (size_t) wktBa.length();
	wktParam.value.length = &wktLen;
	wktParam.value.type = A_STRING;

	keyParam.value.buffer = (char *) &key;
	keyParam.value.length = &keyLen;
	keyParam.value.type = A_VAL32;

	ok = ok && stmt->bind_param( 0, wktParam );
	ok = ok && stmt->bind_param( 1, keyParam );

	ok = ok && stmt->execute();
    }
    delete stmt;

    if( ok ) {
	SaDebugMsg( "Committing feature updates" );
	ok = mConnRW->commit( code, errbuf, sizeof( errbuf ) );

    } else {
	mConnRW->getError( code, errbuf, sizeof( errbuf ) );

	SaDebugMsg( "Rolling back feature updates" );
	mConnRW->rollback();
    }

    if( ok ) {
	// expand SRS and layer extents to accomodate new geometries
	mSrsExtent.unionRect( rect );
	if( !mLayerExtent.isEmpty() ) {
	    mLayerExtent.unionRect( rect );
	}
    } else {
	reportError( tr( "Error updating features" ), code, errbuf );
    }

    return ok;
} // QgsSqlAnywhereProvider::changeGeometryValues()


bool
QgsSqlAnywhereProvider::ensureConnRO()
{
    if( mConnRO && !mConnRO->isAlive() ) {
	closeConnRO();
    }

    if( !mConnRO ) {
	mConnRO = SqlAnyConnection::connect( mConnectInfo, true );
	if( mConnRO ) {
	    mConnectInfo = mConnRO->uri(); // get updated username/password
	}
    }

    return mConnRO != NULL;
}

bool
QgsSqlAnywhereProvider::ensureConnRW()
{
    if( mConnRW && !mConnRW->isAlive() ) {
	closeConnRW();
    }

    if( !mConnRW ) {
	mConnRW = SqlAnyConnection::connect( mConnectInfo, false );
	if( mConnRW ) {
	    mConnectInfo = mConnRW->uri(); // get updated username/password
	}
    }

    return mConnRW != NULL;
}

void 
QgsSqlAnywhereProvider::closeDb()
{
    closeConnRO();
    closeConnRW();
    SqlAnyConnection::releaseApi();
}

void
QgsSqlAnywhereProvider::closeConnRO()
{
  closeConnROCursors();
  if( mConnRO ) {
      mConnRO->release();
      mConnRO = NULL;
  }
} // QgsSqlAnywhereProvider::closeConnRO()

void
QgsSqlAnywhereProvider::closeConnROCursors()
{
  if( mStmt ) {
      delete mStmt;
      mStmt = NULL;
  }

  if( mIdStmt ) {
      delete mIdStmt;
      mIdStmt = NULL;
  }
} // QgsSqlAnywhereProvider::closeConnROCursors()

void
QgsSqlAnywhereProvider::closeConnRW()
{
  if( mConnRW ) {
      mConnRW->release();
      mConnRW = NULL;
  }
} // QgsSqlAnywhereProvider::closeConnRW()


QString 
QgsSqlAnywhereProvider::quotedIdentifier( QString id ) const
{
  id.replace( "\"", "\"\"" );
  return id.prepend( "\"" ).append( "\"" );
} // QgsSqlAnywhereProvider::quotedIdentifier() 


QString 
QgsSqlAnywhereProvider::quotedValue( QString value ) const
{
  if ( value.isNull() )
    return "NULL";

  if ( value.toUpper() == "AUTOINCREMENT" 
	  || value.toUpper() == "TIMESTAMP" )
    return "DEFAULT";

  if ( value.toUpper() == "DEFAULT" 
	  || value.toUpper() == "CURRENT USER" 
	  || value.toUpper() == "CURRENT_USER" 
	  || value.toUpper() == "CURRENT DATE" 
	  || value.toUpper() == "CURRENT_DATE" 
	  || value.toUpper() == "TODAY()"
	  || value.toUpper() == "CURRENT TIME" 
	  || value.toUpper() == "CURRENT_TIME" 
	  || value.toUpper() == "CURRENT TIMESTAMP"
	  || value.toUpper() == "CURRENT_TIMESTAMP")
    return value.toUpper();

  value.replace( "'", "''" );
  return value.prepend( "'" ).append( "'" );
} // QgsSqlAnywhereProvider::quotedValue()


/**
 * Verifies that schema.table.column is a valid geometry column,
 * and sets values of: 
 * mIsTable, mTableId, mIsComputed, mGeomType, mSrid
 */
bool 
QgsSqlAnywhereProvider::checkLayerType()
{
    QString	    sql;
    SqlAnyStatement *stmt;
    QString	    geomType;  
    bool	    foundSrid;
    int		    colSrid;

    // checking database catalog to verify geometry column and table
    sql = QString( "SELECT g.table_id, "
		    "(CASE t.table_type_str WHEN 'BASE' THEN 1 ELSE 0 END) AS isTable, "
		    "(CASE c.column_type WHEN 'C' THEN 1 ELSE 0 END) AS isComputed, "
		    "COALESCE( g.geometry_type_name, 'ST_GEOMETRY' ), "
		    "g.srs_id "
		 "FROM SYS.ST_GEOMETRY_COLUMNS g "
		 "JOIN SYS.SYSTAB t ON g.table_id = t.table_id "
		 "JOIN SYS.SYSTABCOL c "
		     "ON g.table_id = c.table_id "
		     "AND g.column_id = c.column_id "
		 "WHERE g.table_schema=%1 " 
		 "AND g.table_name=%2 "
		 "AND g.column_name=%3 " )
		.arg( quotedValue( mSchemaName ) )
		.arg( quotedValue( mTableName ) )
		.arg( quotedValue( mGeometryColumn ) );

    stmt = mConnRO->execute_direct( sql );
    if( !stmt->isValid() || !stmt->fetchNext() ) {
	reportError( tr( "Error verifying geometry column %1" )
		 .arg( mQuotedTableName + "(" + mGeometryColumn + ")" )
		, stmt );
	delete stmt;
	return false;
    }

    stmt->getUInt( 0, mTableId );
    stmt->getBool( 1, mIsTable );
    stmt->getBool( 2, mIsComputed );
    stmt->getString( 3, geomType );
    foundSrid = stmt->getInt( 4, colSrid );

    delete stmt;

    // Set mGeomType and mSrid
    mGeomType = lookupWkbType( geomType );
    mSrid = colSrid;
    if( mGeomType == QGis::WKBUnknown || !foundSrid ) {
	// Schema type is too general to be handled by QGis.
	// Access table to see if there is a unique geometry type and srid
	sql = QString( "SELECT DISTINCT "
		 "CASE "
		     "WHEN UCASE(%1.ST_GeometryType()) IN ('ST_POINT','ST_MULTIPOINT') THEN 'ST_POINT' "
		     "WHEN UCASE(%1.ST_GeometryType()) IN ('ST_LINESTRING','ST_MULTILINESTRING') THEN 'ST_LINESTRING' "
		     "WHEN UCASE(%1.ST_GeometryType()) IN ('ST_POLYGON','ST_MULTIPOLYGON') THEN 'ST_POLYGON' "
		     "ELSE 'ST_GEOMETRY' "
		 "END, "
		 "%1.ST_SRID() "
		 "FROM %3 " )
	    .arg( quotedIdentifier( mGeometryColumn ) )
	    .arg( geomSampleSet() ) ;

	// Set mGeomType and mSrid 
	stmt = mConnRO->execute_direct( sql );
	if( stmt->isValid() ) {
	    for( int i=0; stmt->fetchNext(); i++ ) {
		QString		rowGeomTypeStr;
		QGis::WkbType   rowGeomType;
		int		rowSrid;

		stmt->getString( 0, rowGeomTypeStr );
		rowGeomType = lookupWkbType( rowGeomTypeStr );
		stmt->getInt( 1, rowSrid );

		if( i == 0 ) {
		    // set values based upon first row
		    mGeomType = rowGeomType;
		    mSrid = rowSrid;
		    foundSrid = true;
		} else {
		    // Determine which value changed
		    if( rowGeomType != mGeomType ) {
			mGeomType = QGis::WKBUnknown;
		    }
		    if( rowSrid != mSrid ) {
			foundSrid = false;
		    }
		}
	    }
	}
	delete stmt;

	if( mGeomType == QGis::WKBUnknown ) {
	    showMessageBox( tr( "Unknown geometry type" ),
		      tr( "Column %1 has a geometry type of %2, which Quantum GIS does not currently support." )
		     .arg( mQuotedTableName + "(" + mGeometryColumn + ")" ) 
		     .arg( geomType ) );
	    return false;
	}

	if( !foundSrid ) {
	    showMessageBox( tr( "Mixed Spatial Reference Systems" ),
		      tr( "Column %1 is not restricted to a single SRID, which Quantum GIS requires." )
		     .arg( mQuotedTableName + "(" + mGeometryColumn + ")" ) );
	    return false;
	}
    }

    return true;
} // QgsSqlAnywhereProvider::checkLayerType()

/**
 * Sets mCapabilities
 * For now, liberally enable permissions
 * TODO: check against database permissions
 */
bool
QgsSqlAnywhereProvider::checkPermissions()
{
    bool	    dbRO;
    QString	    sql;
    SqlAnyStatement *stmt;

    // set read-only capabilities
    mCapabilities = QgsVectorDataProvider::NoCapabilities;
    mCapabilities |= QgsVectorDataProvider::SelectAtId;
    mCapabilities |= QgsVectorDataProvider::SelectGeometryAtId;

    // test for read-only database
    sql = "SELECT CASE UCASE(DB_PROPERTY('ReadOnly')) WHEN 'ON' THEN 1 ELSE 0 END";
    stmt = mConnRW->execute_direct( sql );
    if( !stmt->isValid() || !stmt->fetchNext() ) {
	reportError( tr( "Error checking database ReadOnly property" ), stmt );
	delete stmt;
	return false;
    }
    stmt->getBool( 0, dbRO );
    delete stmt;

    // No more capabilities if database is read-only
    if( !dbRO ) {
	// set capability bits based upon database permissions
	if( testDeletePermission() ) {
	    mCapabilities |= QgsVectorDataProvider::DeleteFeatures;
	}
	if( !mIsComputed && testInsertPermission() ) {
	    mCapabilities |= QgsVectorDataProvider::AddFeatures;
	}
	if( !mIsComputed && testUpdateGeomPermission() ) {
	    mCapabilities |= QgsVectorDataProvider::ChangeGeometries;
	}
	if( testUpdateOtherPermission() ) {
	    mCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
	}
	if( testAlterTable() ) {
	    mCapabilities |= QgsVectorDataProvider::AddAttributes;
	    mCapabilities |= QgsVectorDataProvider::DeleteAttributes;
	}

	// explicitly disable adding/editing features 
	// if table contains measured or 3D geometries,
	// as this could lead to corruption of measures and/or z values
	if( (mCapabilities & QgsVectorDataProvider::AddFeatures)
		|| (mCapabilities & QgsVectorDataProvider::ChangeGeometries) ) {
	    if( testMeasuredOr3D() ) { 
		mCapabilities &= ~( QgsVectorDataProvider::AddFeatures | QgsVectorDataProvider::ChangeGeometries);
	    }
	}
    }

    return true;
} // QgsSqlAnywhereProvider::checkPermissions()

bool
QgsSqlAnywhereProvider::testDMLPermission( QString sql)
{
    SqlAnyStatement *stmt;
    bool	    isValid;

    if( !ensureConnRW() ) {
	SaDebugMsg( "No read-write database connection." );
	return false;
    }

    mConnRW->begin();
    stmt = mConnRW->prepare( sql );
    isValid = stmt->isValid();
    delete stmt;
    mConnRW->rollback();

    if( isValid ) {
	SaDebugMsg( "DML statement successfully prepared." );
    } else {
	SaDebugMsg( "DML statement rejected by server." );
    }

    return isValid;
} // QgsSqlAnywhereProvider::testDMLPermission()

bool
QgsSqlAnywhereProvider::testDeletePermission()
{
    SaDebugMsg( "Testing for DELETE permission" );
    QString sql = QString( "DELETE FROM %1 WHERE 1=0" )
		    .arg( mQuotedTableName );
    return testDMLPermission( sql );
} // QgsSqlAnywhereProvider::testDeletePermission()

bool
QgsSqlAnywhereProvider::testInsertPermission()
{
    SaDebugMsg( "Testing for INSERT permission" );
    QString sql = QString( "INSERT INTO %1 ( %2" )
		    .arg( mQuotedTableName )
		    .arg( geomColIdent() );
    QString values = ") VALUES ( ?";
    for ( QgsFieldMap::const_iterator it = mAttributeFields.constBegin()
	    ; it != mAttributeFields.constEnd()
	    ; it++ ) {
	QString attr = it->name();
	if( attr != mGeometryColumn ) {
	    sql += ", " + attr;
	    values += ", ?";
	}
    }
    return testDMLPermission( sql + values + " ) " );
} // QgsSqlAnywhereProvider::testInsertPermission()

bool
QgsSqlAnywhereProvider::testUpdateGeomPermission()
{
    return testUpdateColumn( mGeometryColumn );
} // QgsSqlAnywhereProvider::testUpdateGeomPermission()

bool
QgsSqlAnywhereProvider::testUpdateOtherPermission()
{
    for ( QgsFieldMap::const_iterator it = mAttributeFields.constBegin()
	    ; it != mAttributeFields.constEnd()
	    ; it++ ) {
	QString colname = it->name();
	if( colname != mGeometryColumn && testUpdateColumn( colname ) ) {
	    return true;
	}
    }
    return false;
} // QgsSqlAnywhereProvider::testUpdateOtherPermission()

bool
QgsSqlAnywhereProvider::testUpdateColumn( QString colname )
{
    SaDebugMsg( QString( "Testing for UPDATE permission on column %1" )
		    .arg( colname ) );
    QString sql = QString( "UPDATE %1 SET %2=? WHERE 1=0" )
		    .arg( mQuotedTableName )
		    .arg( colname );
    return testDMLPermission( sql );
} // QgsSqlAnywhereProvider::testUpdateColumn()

bool
QgsSqlAnywhereProvider::testAlterTable()
{
    SaDebugMsg( QString( "Testing for ALTER TABLE permission" ) );
    QString sql = QString( "ALTER TABLE %1 ADD myTempCol INTEGER" )
		    .arg( mQuotedTableName );
    return testDMLPermission( sql );
} // QgsSqlAnywhereProvider::AlterTable()


bool
QgsSqlAnywhereProvider::testMeasuredOr3D( )
// Determine whether table contains features that are measured or 3D.
{
    SqlAnyStatement *stmt;
    QString	    sql;
    bool	    isMeasuredOr3D;

    sql = QString( "SELECT FIRST 0 FROM %1 "
	      " AND ( %2 .ST_Is3D() = 1 OR %2 .ST_IsMeasured() = 1)" )
	.arg( geomSampleSet() ) 
	.arg( quotedIdentifier( mGeometryColumn ) ) ;

    stmt = mConnRO->execute_direct( sql );
    isMeasuredOr3D = !stmt->isValid() || stmt->fetchNext();
    delete stmt;

    return isMeasuredOr3D;
} // QgsSqlAnywhereProvider::testMeasuredOr3D() 


QString
QgsSqlAnywhereProvider::geomSampleSet()
{
    QString sampleGeoms;
    if( mUseEstimatedMetadata ) {
	sampleGeoms = QString( "(SELECT TOP %1 %2 FROM %3 WHERE %4 IS NOT NULL AND %5 ) AS sampleGeoms WHERE 1=1 ")
	    .arg( sGeomTypeSelectLimit )
	    .arg( geomColIdent() )
	    .arg( mQuotedTableName )
	    .arg( quotedIdentifier( mGeometryColumn ) )
	    .arg( getWhereClause() );
    } else {
	sampleGeoms = mQuotedTableName + " WHERE " + getWhereClause() + " ";
    }
    return sampleGeoms;
} // QgsSqlAnywhereProvider::geomSampleSet()

// sets values of mCrs, mSrsExtent
bool
QgsSqlAnywhereProvider::checkSrs()
{
    const double    dblMax = 1.79769313e308; // SQL Anywhere's MAX_DOUBLE
    bool	    roundEarth;
    QString	    srsWkt;
    QString	    srsProj4;
    double	    xmin, xmax, ymin, ymax;
    SqlAnyStatement *stmt;
    QString	    sql;

    sql = QString( "SELECT IF round_earth='Y' THEN 1 ELSE 0 ENDIF, " 
		    "COALESCE( definition, '' ), "
		    "COALESCE( transform_definition, '' ), "
		    "COALESCE( min_x, -%1 ), "
		    "COALESCE( max_x, %1 ), "
		    "COALESCE( min_y, -%1 ), "
		    "COALESCE( max_y, %1 ) "
		  "FROM ST_SPATIAL_REFERENCE_SYSTEMS "
		  "WHERE srs_id=%2" )
	    .arg( dblMax )
	    .arg( mSrid );

    stmt = mConnRO->execute_direct( sql );
    if( !stmt->isValid() || !stmt->fetchNext() ) {
	reportError( tr( "Error loading SRS definition" ), stmt );
	delete stmt;
	return false;
    }
    stmt->getBool( 0, roundEarth );
    stmt->getString( 1, srsWkt );
    stmt->getString( 2, srsProj4 );
    stmt->getDouble( 3, xmin );
    stmt->getDouble( 4, xmax );
    stmt->getDouble( 5, ymin );
    stmt->getDouble( 6, ymax );
    delete stmt;

    if( roundEarth ) {
	// attempt to find a compatible planar srs
	bool	foundProjection = false;
	int	projSrid;
	QString projStr;
	QString projMsg;

	sql = QString( "SELECT B.srs_id "
			"FROM ST_SPATIAL_REFERENCE_SYSTEMS A, "
			    "ST_SPATIAL_REFERENCE_SYSTEMS B "
			"WHERE A.srs_id=%1 "
			"AND B.round_earth='N' "
			"AND A.transform_definition=B.transform_definition "
			"AND A.min_x >= B.min_x "
			"AND A.min_y >= B.min_y "
			"AND A.max_x <= B.max_x "
			"AND A.max_y <= B.max_y " )
		.arg( mSrid );

	stmt = mConnRO->execute_direct( sql );
	if( stmt->isValid() && stmt->fetchNext() ) {
	    stmt->getInt( 0, projSrid );
	    foundProjection = true;
	}
	delete stmt;

	if( foundProjection ) {
	    // use ST_Srid() to convert, since coordinate systems match
	    projStr = QString(".ST_SRID( %1 )").arg( projSrid );
	    projMsg = tr( 
			"Because Quantum GIS supports only planar data, "
			"the SQL Anywhere data provider will "
			"transform the data to the compatible planar "
			"projection (SRID=%1)." 
			).arg( projSrid );

	} else {
	    // use ST_Transform() to WGS 84 planar projection
	    projSrid = 1000004326;
	    xmin = -90;
	    ymin = -180;
	    xmax = 90;
	    ymax = 180;
	    projStr = QString(".ST_Transform( %1 )").arg( projSrid );
	    projMsg = tr( 
			"Because Quantum GIS supports only planar data "
			"and no compatible planar projection was found, "
			"the SQL Anywhere data provider will attempt to "
			"transform the data to planar WGS 84 (SRID=%1)."
			).arg( projSrid );
	}

	showMessageBox( tr( "Limited Support of Round Earth SRS" ),
		    tr( "Column %1 (%2) contains geometries belonging to "
			"a round earth spatial reference system (SRID=%3). "
			"%4\n\n"
			"Updates to geometry values will be "
			"disabled, and query performance may be poor "
			"because spatial indexes will not be utilized. "
			"To improve performance, consider creating a "
			"spatial index on a new (possibly computed) column "
			"containing a planar projection of these geometries. "
			"For help, refer to the descriptions of the "
			"ST_SRID(INT) and ST_Transform(INT) methods "
			"in the SQL Anywhere documentation." )
		      .arg( mQuotedTableName ) 
		      .arg( mGeometryColumn )
		      .arg( mSrid )
		      .arg( projMsg )
		    );

	mSrid = projSrid;
	mGeometryProjStr = projStr;
	mIsComputed = true;
    }

    // set mSrsExtent
    mSrsExtent.setXMinimum( xmin );
    mSrsExtent.setXMaximum( xmax );
    mSrsExtent.setYMinimum( ymin );
    mSrsExtent.setYMaximum( ymax );

    SaDebugMsg( QString( "Setting mSrsExtent = %1" )
		    .arg( mSrsExtent.toString() ) );

    // Create mCrs from the Proj4 definition retrieved from the 
    // SQL Anywhere database.  
    // If that fails, create from the SRS's WKT.
    SaDebugMsg( "Creating CRS from Proj4 description" );
    if( !mCrs.createFromProj4( srsProj4 ) ) {
      SaDebugMsg( "Failed to create CRS from Proj4 description. Trying WKT." );
      mCrs.createFromWkt( srsWkt );
    }

    return true;
} // QgsSqlAnywhereProvider::checkSrs()



QGis::WkbType 
QgsSqlAnywhereProvider::lookupWkbType( QString type )
{
    if ( type.toUpper() == "ST_POINT" ) {
	return QGis::WKBPoint;
    } else if ( type.toUpper() == "ST_MULTIPOINT" ) {
	return QGis::WKBMultiPoint;
    } else if ( type.toUpper() == "ST_LINESTRING" ) {
	return QGis::WKBLineString;
    } else if ( type.toUpper() == "ST_MULTILINESTRING" ) {
	return QGis::WKBMultiLineString;
    } else if ( type.toUpper() == "ST_POLYGON" ) {
	return QGis::WKBPolygon;
    } else if ( type.toUpper() == "ST_MULTIPOLYGON" ) {
	return QGis::WKBMultiPolygon;
    }
    return QGis::WKBUnknown;
} // QgsSqlAnywhereProvider::lookupWkbType()

const QgsField 
QgsSqlAnywhereProvider::field( int index ) const
{
    const QgsField fld = mAttributeFields[ index ];
    if( fld.name().isEmpty() ) {
	SaDebugMsg( QString( "Field %1 not found." ).arg( index ) );
    }
    return fld;
} // QgsSqlAnywhereProvider::field()


void 
QgsSqlAnywhereProvider::showMessageBox( const QString& title, const QString& text ) 
{
  QgsMessageOutput* message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
} // QgsSqlAnywhereProvider::showMessageBox()


void 
QgsSqlAnywhereProvider::showMessageBox( const QString& title, const QStringList& text ) 
{
  showMessageBox( title, text.join( "\n" ) );
} // QgsSqlAnywhereProvider::showMessageBox()

void
QgsSqlAnywhereProvider::reportError( const QString& title, sacapi_i32 code, const char *errbuf ) 
{
    reportError( title, code, QString::fromUtf8( errbuf ) );
} // QgsSqlAnywhereProvider::reportError()

void
QgsSqlAnywhereProvider::reportError( const QString& title, SqlAnyStatement *stmt ) 
{
    reportError( title, stmt->errCode(), stmt->errMsg() );
} // QgsSqlAnywhereProvider::reportError()

void
QgsSqlAnywhereProvider::reportError( const QString& title, sacapi_i32 code, QString msg ) 
{
  SaDebugMsg( title );
  SaDebugMsg( QString( "SQL Anywhere error %1: %2" ) 
			  .arg( code ) 
			  .arg( msg ) );

  QString text = QString( "SQL Anywhere error code: %1\n"
			  "Description: %2" ) 
			  .arg( code ) 
			  .arg( msg );

  QgsMessageOutput* message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();

} // QgsSqlAnywhereProvider::reportError()


/**
 * Class factory to return a pointer to a newly created
 * QgsSqlAnywhereProvider object
 */
QGISEXTERN QgsSqlAnywhereProvider *classFactory( const QString * uri )
{
  return new QgsSqlAnywhereProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  SaDebugMsg( "Returning provider key " + SQLANYWHERE_KEY );
  return SQLANYWHERE_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  SaDebugMsg( "Returning provider description " + SQLANYWHERE_DESCRIPTION );
  return SQLANYWHERE_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}
