/***************************************************************************
    qgsgrassprovider.cpp -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <string.h>
#include <iostream>
#include <vector>
#include <cfloat>

#include <qpixmap.h>
#include <qiconset.h>
#include <qdir.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qmessagebox.h>

#include "../../src/qgis.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"

extern "C" {
#include <gis.h>
#include <dbmi.h>
#include <Vect.h>
}

#include "qgsgrass.h"
#include "qgsgrassprovider.h"

std::vector<GLAYER> QgsGrassProvider::mLayers;
std::vector<GMAP> QgsGrassProvider::mMaps;

QgsGrassProvider::QgsGrassProvider(QString uri):mDataSourceUri(uri)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassProvider URI: " << uri << std::endl;
    #endif

    QTime time;
    time.start();

    mValid = false;
    checkEndian();
    
    // Parse URI 
    QDir dir ( uri );  // it is not a directory in fact
    uri = dir.path();  // no dupl '/'

    mLayer = dir.dirName();
    uri = uri.left( dir.path().findRev('/') );
    dir = QDir(uri);
    mMapName = dir.dirName();
    dir.cdUp(); 
    mMapset = dir.dirName();
    dir.cdUp(); 
    mLocation = dir.dirName();
    dir.cdUp(); 
    mGisdbase = dir.path();
    
    #ifdef QGISDEBUG
    std::cerr << "gisdbase: " << mGisdbase << std::endl;
    std::cerr << "location: " << mLocation << std::endl;
    std::cerr << "mapset: "   << mMapset << std::endl;
    std::cerr << "mapName: "  << mMapName << std::endl;
    std::cerr << "layer: "    << mLayer << std::endl;
    #endif

    /* Parse Layer, supported layers <field>_point, <field>_line, <field>_area
    *  Layer is opened even if it is empty (has no features) 
    */
    mLayerField = -1;       
    if ( mLayer.compare("boundary") == 0 ) { // currently not used
        mLayerType = BOUNDARY;
	mGrassType = GV_BOUNDARY;
    } else if ( mLayer.compare("centroid") == 0 ) { // currently not used
        mLayerType = CENTROID;
	mGrassType = GV_CENTROID;
    } else {
	// Get field number
	int pos = mLayer.find('_');

	if ( pos == -1 ) {
	    std::cerr << "Invalid layer name, no underscore found: " << mLayer << std::endl;
	    return;
	}

	mLayerField = mLayer.left(pos).toInt();

	QString ts = mLayer.right( mLayer.length() - pos - 1 );
	if ( ts.compare("point") == 0 ) {
	    mLayerType = POINT;
	    mGrassType = GV_POINT; // ?! centroids may be points
	} else if ( ts.compare("line") == 0 ) {
	    mLayerType = LINE;
	    mGrassType = GV_LINE | GV_BOUNDARY; 
	} else if ( ts.compare("polygon") == 0 ) {
	    mLayerType = POLYGON;
	    mGrassType = GV_AREA; 
	} else {
	    std::cerr << "Invalid layer name, wrong type: " << ts << std::endl;
	    return;
	}
    }
    #ifdef QGISDEBUG
    std::cerr << "mLayerField: " << mLayerField << std::endl;
    std::cerr << "mLayerType: " << mLayerType << std::endl;
    #endif

    if ( mLayerType == BOUNDARY || mLayerType == CENTROID ) {
	std::cerr << "Layer type not supported." << std::endl;
	return;
    }

    // Set QGIS type
    switch ( mLayerType ) {
	case POINT:
	case CENTROID:
            mQgisType = QGis::WKBPoint; 
	    break;
	case LINE:
	case BOUNDARY:
            mQgisType = QGis::WKBLineString; 
	    break;
	case POLYGON:
	    mQgisType = QGis::WKBPolygon;
	    break;
    }

    mLayerId = openLayer(mGisdbase, mLocation, mMapset, mMapName, mLayerField);
    if ( mLayerId < 0 ) {
	std::cerr << "Cannot open GRASS layer:" << uri << std::endl;
	return;
    }
    #ifdef QGISDEBUG
    std::cerr << "mLayerId: " << mLayerId << std::endl;
    #endif

    mMap = layerMap(mLayerId);

    // Extent
    Vect_get_map_box ( mMap, &mMapBox );

    // Getting the total number of features in the layer
    mNumberFeatures = 0;
    mCidxFieldIndex = -1;
    if ( mLayerField >= 0 ) {
	mCidxFieldIndex = Vect_cidx_get_field_index ( mMap, mLayerField);
	if ( mCidxFieldIndex >= 0 ) {
	    mNumberFeatures = Vect_cidx_get_type_count ( mMap, mLayerField, mGrassType );
	    mCidxFieldNumCats = Vect_cidx_get_num_cats_by_index ( mMap, mCidxFieldIndex );
	}
    } else {
	// TODO nofield layers
	mNumberFeatures = 0;
    }
    mNextCidx = 0;

    #ifdef QGISDEBUG
    std::cerr << "mNumberFeatures = " << mNumberFeatures << std::endl;
    #endif

    // Create selection array
    int nlines = Vect_get_num_lines ( mMap );
    int nareas = Vect_get_num_areas ( mMap );
    #ifdef QGISDEBUG
    std::cerr << "nlines = " << nlines << " nareas = " << nareas << std::endl;
    #endif

    if ( mLayerType == POINT || mLayerType == CENTROID || mLayerType == LINE || mLayerType == BOUNDARY ) {
	mSelectionSize = nlines + 1;
    } else if ( mLayerType == POLYGON ) {
	mSelectionSize = nareas + 1;
    }
    mSelection = (char *) malloc ( mSelectionSize );

    mPoints = Vect_new_line_struct ();
    mCats = Vect_new_cats_struct ();
    mList = Vect_new_list ();

    resetSelection(1); // TODO ? - where what reset

    mValid = true;

    #ifdef QGISDEBUG
    std::cerr << "New GRASS layer opened, time (ms): " << time.elapsed() << std::endl;
    #endif
}

QgsGrassProvider::~QgsGrassProvider()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassProvider::~QgsGrassProvider()" << std::endl;
    #endif
    closeLayer ( mLayerId );
}

/**
* Get the first feature resutling from a select operation
* @return QgsFeature
*/
QgsFeature *QgsGrassProvider::getFirstFeature(bool fetchAttributes)
{
    mNextCidx = 0;
	
    return ( getNextFeature(fetchAttributes) );
}

/**
* Get the next feature resulting from a select operation
* @return false if there are no features in the selection set
*/
bool QgsGrassProvider::getNextFeature(QgsFeature &feature, bool fetchAttributes)
{
    // TODO once clear how to do that 
    return false;
}

/**
* Get the next feature resulting from a select operation
* Return 0 if there are no features in the selection set
* @return QgsFeature
*/
QgsFeature *QgsGrassProvider::getNextFeature(bool fetchAttributes)
{
    int cat, type, id, idx;
    unsigned char *wkb;
    int wkbsize;

    #ifdef QGISDEBUG
    std::cout << "QgsGrassProvider::getNextFeature() mNextCidx = " << mNextCidx 
    	      << " fetchAttributes = " << fetchAttributes << std::endl;
    #endif
    
    // Get next line/area id
    int found = 0;
    while ( mNextCidx < mCidxFieldNumCats ) {
	Vect_cidx_get_cat_by_index ( mMap, mCidxFieldIndex, mNextCidx++, &cat, &type, &id );
	// Warning: selection array is only of type line/area of current layer -> check type first

	if ( !(type & mGrassType) ) continue;
	if ( !mSelection[id] ) continue;
        found = 1;
	break;
    }
    if ( !found ) return 0; // No more features
    #ifdef QGISDEBUG
    std::cout << "cat = " << cat << " type = " << type << " id = " << id << std::endl;
    #endif

    QgsFeature *f = new QgsFeature(id);

    // TODO int may be 64 bits (memcpy)
    if ( type & (GV_POINTS | GV_LINES) ) { /* points or lines */
	Vect_read_line ( mMap, mPoints, mCats, id);
	int npoints = mPoints->n_points;
	
	if ( type & GV_POINTS ) {
	    wkbsize = 1 + 4 + 2*8;
	} else { // GV_LINES
	    wkbsize = 1+4+4+npoints*2*8;
	}	    
	wkb = new unsigned char[wkbsize];
	unsigned char *wkbp = wkb;
	wkbp[0] = (unsigned char) mEndian;
	wkbp += 1;

	/* WKB type */
	memcpy (wkbp, &mQgisType, 4);
	wkbp += 4;
	
	/* number of points */
	if ( type & GV_LINES ) {
	    memcpy (wkbp, &npoints, 4);
	    wkbp += 4;
	}
	
	for ( int i = 0; i < npoints; i++ ) {
	    memcpy (wkbp, &(mPoints->x[i]), 8);
	    memcpy (wkbp+8, &(mPoints->y[i]), 8);
	    wkbp += 16;
	}
    } else { // GV_AREA
	Vect_get_area_points ( mMap, id, mPoints );
	int npoints = mPoints->n_points;

	wkbsize = 1+4+4+4+npoints*2*8; // size without islands
	wkb = new unsigned char[wkbsize];
	wkb[0] = (unsigned char) mEndian;
	int offset = 1;

	/* WKB type */
	memcpy ( wkb+offset, &mQgisType, 4);
	offset += 4;

	/* Number of rings */
	int nisles = Vect_get_area_num_isles ( mMap, id );
	int nrings = 1 + nisles; 
	memcpy (wkb+offset, &nrings, 4);
	offset += 4;

	/* Outer ring */
	memcpy (wkb+offset, &npoints, 4);
	offset += 4;
	for ( int i = 0; i < npoints; i++ ) {
	    memcpy (wkb+offset, &(mPoints->x[i]), 8);
	    memcpy (wkb+offset+8, &(mPoints->y[i]), 8);
	    offset += 16;
	}
	
	/* Isles */
	for ( int i = 0; i < nisles; i++ ) {
	    Vect_get_isle_points ( mMap, Vect_get_area_isle (mMap, id, i), mPoints );
	    npoints = mPoints->n_points;
	    
	    // add space
	    wkbsize += 4+npoints*2*8;
	    wkb = (unsigned char *) realloc (wkb, wkbsize);

	    memcpy (wkb+offset, &npoints, 4);
	    offset += 4;
	    for ( int i = 0; i < npoints; i++ ) {
		memcpy (wkb+offset, &(mPoints->x[i]), 8);
		memcpy (wkb+offset+8, &(mPoints->y[i]), 8);
		offset += 16;
	    }
	}
    }

    f->setGeometry(wkb, wkbsize);

    if ( fetchAttributes ) {
	QgsGrassProvider::setFeatureAttributes( mLayerId, cat, f );  
    }
    
    return f;
}

QgsFeature* QgsGrassProvider::getNextFeature(std::list<int>& attlist)
{
    return 0;//soon
}

void QgsGrassProvider::resetSelection( bool sel)
{
    memset ( mSelection, (int) sel, mSelectionSize );
    mNextCidx = 0;
}

/**
* Select features based on a bounding rectangle. Features can be retrieved
* with calls to getFirstFeature and getNextFeature.
* @param mbr QgsRect containing the extent to use in selecting features
*/
void QgsGrassProvider::select(QgsRect *rect, bool useIntersect)
{
    #ifdef QGISDEBUG
    std::cout << "QgsGrassProvider::select() useIntersect = " << useIntersect << std::endl;
    #endif

    resetSelection(0);

    if ( !useIntersect ) { // select by bounding boxes only
	BOUND_BOX box;
	box.N = rect->yMax(); box.S = rect->yMin(); 
	box.E = rect->xMax(); box.W = rect->xMin(); 
	box.T = PORT_DOUBLE_MAX; box.B = -PORT_DOUBLE_MAX; 
	if ( mLayerType == POINT || mLayerType == CENTROID || mLayerType == LINE || mLayerType == BOUNDARY ) {
	    Vect_select_lines_by_box(mMap, &box, mGrassType, mList);
	} else if ( mLayerType == POLYGON ) {
	    Vect_select_areas_by_box(mMap, &box, mList);
	}

    } else { // check intersection
	struct line_pnts *Polygon;
	
	Polygon = Vect_new_line_struct();

	Vect_append_point( Polygon, rect->xMin(), rect->yMin(), 0);
	Vect_append_point( Polygon, rect->xMax(), rect->yMin(), 0);
	Vect_append_point( Polygon, rect->xMax(), rect->yMax(), 0);
	Vect_append_point( Polygon, rect->xMin(), rect->yMax(), 0);
	Vect_append_point( Polygon, rect->xMin(), rect->yMin(), 0);

	if ( mLayerType == POINT || mLayerType == CENTROID || mLayerType == LINE || mLayerType == BOUNDARY ) {
	    Vect_select_lines_by_polygon ( mMap, Polygon, 0, NULL, mGrassType, mList);
	} else if ( mLayerType == POLYGON ) {
	    Vect_select_areas_by_polygon ( mMap, Polygon, 0, NULL, mList);
	}

	Vect_destroy_line_struct (Polygon);
    }
    for ( int i = 0; i < mList->n_values; i++ ) {
        if ( mList->value[i] <= mSelectionSize ) {
	    mSelection[mList->value[i]] = 1;
	} else {
	    std::cerr << "Selected element out of range" << std::endl;
	}
    }
	
    #ifdef QGISDEBUG
    std::cout << mList->n_values << " features selected" << std::endl;
    #endif
}

/**
* Set the data source specification. This may be a path or database
* connection string
* @uri data source specification
*/
void QgsGrassProvider::setDataSourceUri(QString uri)
{
	mDataSourceUri = uri;
}

/**
* Get the data source specification. This may be a path or database
* connection string
* @return data source specification
*/
QString QgsGrassProvider::getDataSourceUri()
{
	return mDataSourceUri;
}

/**
* Identify features within the search radius specified by rect
* @param rect Bounding rectangle of search radius
* @return std::vector containing QgsFeature objects that intersect rect
*/
std::vector<QgsFeature>& QgsGrassProvider::identify(QgsRect * rect)
{
    select(rect, true);
}

/* set endian */
void QgsGrassProvider::checkEndian()
{
	char *chkEndian = new char[4];
	memset(chkEndian, '\0', 4);
	chkEndian[0] = 0xE8;

	int *ce = (int *) chkEndian;
	if (232 == *ce)
	    mEndian = NDR;
	else
	    mEndian = XDR;
	delete[]chkEndian;
}

int QgsGrassProvider::endian()
{
	return mEndian;
}

QgsRect *QgsGrassProvider::extent()
{
    return new QgsRect( mMapBox.W, mMapBox.S, mMapBox.E, mMapBox.N);
}

/** 
* Return the feature type
*/
int QgsGrassProvider::geometryType(){
    return mQgisType;
}
/** 
* Return the feature type
*/
long QgsGrassProvider::featureCount(){
    return mNumberFeatures;
}

/**
* Return the number of fields
*/
int QgsGrassProvider::fieldCount(){
      return mLayers[mLayerId].fields.size();
}

/**
* Return fields
*/
std::vector<QgsField>& QgsGrassProvider::fields(){
      return mLayers[mLayerId].fields;

}

void QgsGrassProvider::reset(){
    resetSelection(1);
    mNextCidx = 0;
}

QString QgsGrassProvider::minValue(int position)
{
    if ( position >= fieldCount() ) {
	std::cerr << "Warning: access requested to invalid position in QgsGrassProvider::minValue()" 
	          << std::endl;
    }
    return QString::number( mLayers[mLayerId].minmax[position][0], 'f', 2 );
}

 
QString QgsGrassProvider::maxValue(int position)
{
    if ( position >= fieldCount() ) {
	std::cerr << "Warning: access requested to invalid position in QgsGrassProvider::maxValue()" 
	          << std::endl;
    }
    return QString::number( mLayers[mLayerId].minmax[position][1], 'f', 2 );
}

bool QgsGrassProvider::isValid(){
    #ifdef QGISDEBUG
    QString validString = mValid?"true":"false";
    std::cerr << "QgsGrassProvider::isValid() returned: " << validString << std::endl;
    #endif
    return mValid;
}

// ------------------------------------------------------------------------------------------------------
// Compare categories in GATT
static int cmpAtt ( const void *a, const void *b ) {
    GATT *p1 = (GATT *) a;
    GATT *p2 = (GATT *) b;
    return (p1->cat - p2->cat);
}

/* returns layerId or -1 on error */
int QgsGrassProvider::openLayer(QString gisdbase, QString location, QString mapset, QString mapName, int field)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassProvider::openLayer()" << std::endl;
    std::cerr << "gisdbase: " << gisdbase << std::endl;
    std::cerr << "location: " << location << std::endl;
    std::cerr << "mapset: "   << mapset << std::endl;
    std::cerr << "mapName: "  << mapName << std::endl;
    std::cerr << "field: "    << field << std::endl;
    #endif

    // Check if this layer is already opened

    for ( int i = 0; i <  mLayers.size(); i++) {
	if ( !(mLayers[i].valid) ) continue;

	GMAP *mp = &(mMaps[mLayers[i].mapId]);

	if ( mp->gisdbase == gisdbase && mp->location == location && 
	     mp->mapset == mapset && mp->mapName == mapName && mLayers[i].field == field )
	{
	    // the layer already exists, return layer id
	    #ifdef QGISDEBUG
	    std::cerr << "The layer is already opened with ID = " << i << std::endl;
	    #endif
	    mLayers[i].nUsers++;
	    return i;
	}
    }

    // Create a new layer
    GLAYER layer;
    layer.valid = false;
    layer.field = field; 

    // Open map
    layer.mapId = openMap ( gisdbase, location, mapset, mapName );
    if ( layer.mapId < 0 ) {
	std::cerr << "Cannot open vector map" << std::endl;
	return -1;
    }
    #ifdef QGISDEBUG
    std::cerr << "layer.mapId = " << layer.mapId << std::endl;
    #endif
    layer.map = mMaps[layer.mapId].map;

    // Get field info
    layer.fieldInfo = Vect_get_field( layer.map, layer.field); // should work also with field = 0

    // Read attributes
    layer.nColumns = 0;
    layer.nAttributes = 0;
    layer.attributes = 0;
    layer.keyColumn = -1;
    if ( layer.fieldInfo == NULL ) {
        #ifdef QGISDEBUG
	std::cerr << "No field info -> no attribute table" << std::endl;
        #endif
    } else { 
        #ifdef QGISDEBUG
	std::cerr << "Field info found -> open database" << std::endl;
        #endif
	dbDriver *databaseDriver = db_start_driver_open_database ( layer.fieldInfo->driver, 
								   layer.fieldInfo->database );

	if ( databaseDriver == NULL ) {
	    std::cerr << "Cannot open database " << layer.fieldInfo->database << " by driver " 
		      << layer.fieldInfo->driver << std::endl;
	} else {
            #ifdef QGISDEBUG
	    std::cerr << "Database opened -> open select cursor" << std::endl;
            #endif
	    dbString dbstr; 
	    db_init_string (&dbstr);
	    db_set_string (&dbstr, "select * from ");
	    db_append_string (&dbstr, layer.fieldInfo->table);
	    
            #ifdef QGISDEBUG
	    std::cerr << "SQL: " << db_get_string(&dbstr) << std::endl;
            #endif
	    dbCursor databaseCursor;
	    if ( db_open_select_cursor(databaseDriver, &dbstr, &databaseCursor, DB_SCROLL) != DB_OK ){
		layer.nColumns = 0;
		db_close_database_shutdown_driver ( databaseDriver );
		QMessageBox::warning( 0, "Warning", "Cannot select attributes from table '" + 
			         QString(layer.fieldInfo->table) + "'" );
	    } else {
		int nRecords = db_get_num_rows ( &databaseCursor );
                #ifdef QGISDEBUG
		std::cerr << "Number of records: " << nRecords << std::endl;
                #endif
		
		dbTable  *databaseTable = db_get_cursor_table (&databaseCursor);
		layer.nColumns = db_get_table_number_of_columns(databaseTable);

		layer.minmax = new double[layer.nColumns][2];

		// Read columns' description 
		for (int i = 0; i < layer.nColumns; i++) {
		    layer.minmax[i][0] = DBL_MAX;
		    layer.minmax[i][1] = -DBL_MAX;

		    dbColumn *column = db_get_table_column (databaseTable, i);

		    int ctype = db_sqltype_to_Ctype ( db_get_column_sqltype(column) );
                    #ifdef QGISDEBUG
		    std::cerr << "column = " << db_get_column_name(column) 
			      << " ctype = " << ctype << std::endl;
                    #endif
		    
		    QString ctypeStr;
		    switch ( ctype ) {
			case DB_C_TYPE_INT:
			    ctypeStr = "integer";
			    break; 
			case DB_C_TYPE_DOUBLE:
			    ctypeStr = "double";
			    break; 
			case DB_C_TYPE_STRING:
			    ctypeStr = "string";
			    break; 
			case DB_C_TYPE_DATETIME:
			    ctypeStr = "datetime";
			    break; 
		    }
		    layer.fields.push_back ( QgsField( db_get_column_name(column), ctypeStr, 
		                     db_get_column_length(column), db_get_column_precision(column) ) );
		    
		    if ( G_strcasecmp ( db_get_column_name(column), layer.fieldInfo->key) == 0 ) {
			layer.keyColumn = i;
		    }
		}

		if ( layer.keyColumn < 0 ) {
		    layer.fields.clear();
                    layer.nColumns = 0;

		    QMessageBox::warning( 0, "Warning", "Key column '" + QString(layer.fieldInfo->key) + 
			         "' not found in the table '" + QString(layer.fieldInfo->table) + "'" );
		} else {
		    // Read attributes to the memory
		    layer.attributes = (GATT *) malloc ( nRecords * sizeof(GATT) );
		    while ( 1 ) {
			int more;
				
			if ( db_fetch (&databaseCursor, DB_NEXT, &more) != DB_OK ) {
			    std::cout << "Cannot fetch DB record" << std::endl;
			    break;
			}
			if ( !more ) break; // no more records

			// Check cat value
			dbColumn *column = db_get_table_column (databaseTable, layer.keyColumn);
			dbValue *value = db_get_column_value(column);
			layer.attributes[layer.nAttributes].cat = db_get_value_int (value);
			if ( layer.attributes[layer.nAttributes].cat < 1 ) continue; 

			layer.attributes[layer.nAttributes].values = (char **) malloc ( layer.nColumns * sizeof(char*) );

			for (int i = 0; i < layer.nColumns; i++) {
			    column = db_get_table_column (databaseTable, i);
			    int sqltype = db_get_column_sqltype(column);
			    int ctype = db_sqltype_to_Ctype ( sqltype );
			    value = db_get_column_value(column);
			    db_convert_value_to_string ( value, sqltype, &dbstr);

			    #ifdef QGISDEBUG
			    std::cout << "column: " << db_get_column_name(column) << std::endl;
			    std::cout << "value: " << db_get_string(&dbstr) << std::endl;
			    #endif

			    layer.attributes[layer.nAttributes].values[i] = strdup ( db_get_string(&dbstr) );

			    double dbl;
			    if ( ctype == DB_C_TYPE_INT ) {
				dbl = db_get_value_int ( value );
			    } else if ( ctype == DB_C_TYPE_DOUBLE ) {
				dbl = db_get_value_double ( value );
			    } else {
				dbl = 0;
			    }
			    
			    if ( dbl < layer.minmax[i][0] ) {
				layer.minmax[i][0] = dbl;
			    }
			    if ( dbl > layer.minmax[i][1] ) {
				layer.minmax[i][1] = dbl;
			    }
			}
			layer.nAttributes++;
		    }
		    // Sort attributes by category
		    qsort ( layer.attributes, layer.nAttributes, sizeof(GATT), cmpAtt );
		}
		db_close_cursor (&databaseCursor);
		db_close_database_shutdown_driver ( databaseDriver );
		db_free_string(&dbstr);

                #ifdef QGISDEBUG
		std::cerr << "number of attributes = " << layer.nAttributes << std::endl;
                #endif

	    }
	}
    }
    // Add cat if no attribute fields exist (otherwise qgis crashes)
    if ( layer.nColumns == 0 ) {
	layer.fields.push_back ( QgsField( "cat", "integer", 10, 0) );
	layer.minmax = new double[1][2];
	layer.minmax[0][0] = 0; 
	layer.minmax[0][1] = 0; 

	int cidx = Vect_cidx_get_field_index ( layer.map, field );
	if ( cidx >= 0 ) {
	    int ncats, cat, type, id;
	    
	    ncats = Vect_cidx_get_num_cats_by_index ( layer.map, cidx );

	    if ( ncats > 0 ) {
		Vect_cidx_get_cat_by_index ( layer.map, cidx, 0, &cat, &type, &id );
	        layer.minmax[0][0] = cat; 

	        Vect_cidx_get_cat_by_index ( layer.map, cidx, ncats-1, &cat, &type, &id );
	        layer.minmax[0][1] = cat; 
	    }
	}
    }

    layer.valid = true;

    // Add new layer to layers
    mLayers.push_back(layer);
	
    #ifdef QGISDEBUG
    std::cerr << "New layer successfully opened" << layer.nAttributes << std::endl;
    #endif
        
    return mLayers.size() - 1; 
    
}
void QgsGrassProvider::closeLayer( int layerId )
{
    #ifdef QGISDEBUG
    std::cerr << "Close layer " << layerId << std::endl;
    #endif

    // TODO: not tested because delete is never used for providers
    mLayers[layerId].nUsers--;

    if ( mLayers[layerId].nUsers == 0 ) { // No more users, free sources
        #ifdef QGISDEBUG
        std::cerr << "No more users -> delete layer" << std::endl;
        #endif

        mLayers[layerId].valid = false;

	// Column names/types
	mLayers[layerId].fields.resize(0);
	
	// Attributes
	for ( int i = 0; i < mLayers[layerId].nAttributes; i++ ) {
	    free ( mLayers[layerId].attributes[i].values );
	}
	free ( mLayers[layerId].attributes );
		
	delete[] mLayers[layerId].minmax;

	// Field info
	free ( mLayers[layerId].fieldInfo );

	closeMap ( mLayers[layerId].mapId );
    }
}

/* returns mapId or -1 on error */
int QgsGrassProvider::openMap(QString gisdbase, QString location, QString mapset, QString mapName)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassProvider::openMap()" << std::endl;
    #endif

    QString tmpPath = gisdbase + "/" + location + "/" + mapset + "/" + mapName;

    // Check if this map is already opened
    for ( int i = 0; i <  mMaps.size(); i++) {
	if ( mMaps[i].path == tmpPath ) 
	{
	    // the map is already opened, return map id
            #ifdef QGISDEBUG
	    std::cerr << "The map is already opened with ID = " << i << std::endl;
            #endif
	    mMaps[i].nUsers++;
	    return i;
	}
    }

    GMAP map;
    map.gisdbase = gisdbase;
    map.location = location;
    map.mapset = mapset;
    map.mapName = mapName;
    map.path = tmpPath;
    map.nUsers = 1;
    map.map = (struct Map_info *) malloc ( sizeof(struct Map_info) );

    // Set GRASS location
    QgsGrass::setLocation ( gisdbase, location ); 
#ifdef QGISDEBUG
	std::cerr << "Setting  gisdbase, location: " << gisdbase << ", " << location << std::endl;
#endif

    // Find the vector
    char *ms = G_find_vector2 ( (char *) mapName.ascii(), (char *) mapset.ascii()) ;

    if ( ms == NULL) {
        std::cerr << "Cannot find GRASS vector" << std::endl;
	return -1;
    }

    // Open vector
    QgsGrass::resetError(); // to "catch" error after Vect_open_old()
    Vect_set_open_level (2);
    Vect_open_old ( map.map, (char *) mapName.ascii(), (char *) mapset.ascii());

    if ( QgsGrass::getError() == QgsGrass::FATAL ) {
	std::cerr << "Cannot open GRASS vector: " << QgsGrass::getErrorMessage() << std::endl;
	return -1;
    }
    #ifdef QGISDEBUG
    std::cerr << "GRASS map successfully opened" << std::endl;
    #endif

    // Add new map to maps
    mMaps.push_back(map);

    return mMaps.size() - 1; // map id 
}


void QgsGrassProvider::closeMap( int mapId )
{
    #ifdef QGISDEBUG
    std::cerr << "Close map " << mapId << std::endl;
    #endif

    // TODO: not tested because delete is never used for providers
    mMaps[mapId].nUsers--;

    if ( mMaps[mapId].nUsers == 0 ) { // No more users, free sources
        #ifdef QGISDEBUG
        std::cerr << "No more users -> delete map" << std::endl;
        #endif

        mMaps[mapId].valid = false;
	Vect_close ( mMaps[mapId].map );
    }
}

/** Set feature attributes */
void QgsGrassProvider::setFeatureAttributes ( int layerId, int cat, QgsFeature *feature )
{
    #ifdef QGISDEBUG
    std::cerr << "setFeatureAttributes cat = " << cat << std::endl;
    #endif
    if ( mLayers[layerId].nColumns > 0 ) {
	// find cat
	GATT key;
	key.cat = cat;
	GATT *att = (GATT *) bsearch ( &key, mLayers[layerId].attributes, mLayers[layerId].nAttributes,
		                       sizeof(GATT), cmpAtt);

	if ( att != NULL ) {
	    for (int i = 0; i < mLayers[layerId].nColumns; i++) {
		feature->addAttribute ( mLayers[layerId].fields[i].name(), att->values[i]);
	    }
	}
    } else { 
	QString tmp;
	tmp.sprintf("%d", cat );
	feature->addAttribute ( "cat", tmp);
    }
}

/** Get pointer to map */
struct Map_info *QgsGrassProvider::layerMap ( int layerId )
{
    return ( mMaps[mLayers[layerId].mapId].map );
}

//-------------------------------------------------------------------------------------------------------

/**
* Class factory to return a pointer to a newly created 
* QgsGrassProvider object
*/
extern "C" QgsGrassProvider * classFactory(const char *uri)
{
    return new QgsGrassProvider(uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
extern "C" QString providerKey(){
    return QString("grass");
}
/**
* Required description function 
*/
extern "C" QString description(){
    return QString("GRASS data provider");
} 
/**
* Required isProvider function. Used to determine if this shared library
* is a data provider plugin
*/
extern "C" bool isProvider(){
    return true;
}

