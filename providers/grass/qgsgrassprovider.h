/***************************************************************************
    qgsgrassprovider.h  -  Data provider for GRASS format
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
#ifndef QGSGRASSPROVIDER_H
#define QGSGRASSPROVIDER_H

class QgsFeature;
class QgsField;

/* Attributes. Cache of database attributes (because selection from database is slow). */
struct GATT {
    int cat;       // category
    char **values; // pointer to array of pointers to values
};

/* Grass layer (unique vector+field). */
struct GLAYER {
    QString path;                  // path to the layer gisdbase+location+mapset+mapName
    int     field;                 // field number
    bool    valid;                 // valid is true if layer is opened, once the layer is closed, 
                                   // valid is set to false and no more used
    int     mapId;                 // map ID in maps vector
    struct  Map_info   *map;       // map header
    struct  field_info *fieldInfo; // field info
    int     nColumns;              // number of columns in database table, if 0, attributes are not available
                                   // and category (column name 'cat') is used instead
    int     keyColumn;             // number of key column
    std::vector<QgsField> fields;  // description of layer fields
    int     nAttributes;           // number of attributes read to the memory (may be < nRecords)
    GATT    *attributes;           // vector of attributes
    double  (*minmax)[2];          // minimum and maximum values of attributes
    int     nUsers;                // number of instances using this layer, increased by open(), 
                                   // decreased by close()
};

/* Grass vector map. */
struct GMAP {
    QString gisdbase;      // map gisdabase
    QString location;      // map location name (not path!)
    QString mapset;        // map mapset
    QString mapName;       // map name
    QString path;          // path to the layer gisdbase+location+mapset+mapName
    bool    valid;         // true if map is opened, once the map is closed, 
                           // valid is set to false and no more used
    struct  Map_info *map; // map header
    int     nUsers;        // number layers using this map
};

/**
  \class QgsGrassProvider
  \brief Data provider for GRASS vectors
*/
class QgsGrassProvider : public QgsDataProvider {
public:
	QgsGrassProvider(QString uri=0);
	virtual ~QgsGrassProvider();

	/**
	* Get the first feature resulting from a select operation
	* @return QgsFeature
	*/
	QgsFeature * getFirstFeature(bool fetchAttributes=false);

	/** 
	* Get the next feature resutling from a select operation
	* @return QgsFeature
	*/
	QgsFeature * getNextFeature(bool fetchAttributes=false);
	bool getNextFeature(QgsFeature &feature, bool fetchAttributes=false);
	QgsFeature* getNextFeature(std::list<int>& attlist);
	
	/** 
	* Get the feature type as defined in WKBTYPE (qgis.h). 
	* @return int representing the feature type
	*/
	int geometryType();

	/** 
	* Get the number of features in the layer
	*/
	long featureCount();

	/** 
	* Get the number of fields in the layer
	*/
	int fieldCount();

	/**
	* Select features based on a bounding rectangle. Features can be retrieved 
	* with calls to getFirstFeature and getNextFeature.
	* @param mbr QgsRect containing the extent to use in selecting features
	*/
	void select(QgsRect *mbr, bool useIntersect=false);

	/** 
	* Set the data source specification. This may be a path or database
	* connection string
	* @uri data source specification
	*/
	void setDataSourceUri(QString uri);
	
	/** 
	* Get the data source specification. This may be a path or database
	* connection string
	* @return data source specification
	*/
	QString getDataSourceUri();
	
	/**
	* Identify features within the search radius specified by rect
	* @param rect Bounding rectangle of search radius
	* @return std::vector containing QgsFeature objects that intersect rect
	*/
	virtual std::vector<QgsFeature>& identify(QgsRect *rect);

        /** Return endian-ness for this layer
        */	
	int endian();

	/** Return the extent for this data layer
	*/
	virtual QgsRect *extent();

	/**
	* Get the field information for the layer
	*/
	std::vector<QgsField>& fields();
	 
	/* Reset the layer - for an OGRLayer, this means clearing the
	* spatial filter and calling ResetReading
	*/
	void reset();

	/**Returns the minimum value of an attribut
	   @param position the number of the attribute*/
	QString minValue(int position);

	/**Returns the maximum value of an attribut
	   @param position the number of the attribute*/
	QString maxValue(int position);

	/**Returns true if this is a valid shapefile
	*/
	bool isValid();

	/**Adds a feature
	   @return true in case of success and false in case of failure*/
	bool addFeature(QgsFeature* f);

        /**Deletes a feature
	   @param id the number of the feature
	   @return true in case of success and false in case of failure*/
	bool deleteFeature(int id);

private:
	enum ENDIAN {
		NDR = 1,
		XDR = 0
	};

	// Layer type (layerType)
	enum    TYPE {   // layer name:
	    POINT = 1,   // <field>_point
	    LINE,        // <field>_line
	    POLYGON,     // <field>_polygon
	    BOUNDARY,    // boundary (currently not used)
	    CENTROID     // centroid (currently not used)
	};

        QString mGisdbase;      // map gisdabase
        QString mLocation;      // map location name (not path!)
        QString mMapset;        // map mapset
        QString mMapName;       // map name
        QString mLayer;         // layer name
	int     mLayerField;    // field part of layer or -1 if no field specified
	int     mLayerType;     // layer type POINT, LINE, ...
	int     mGrassType;     // grass feature type: GV_POINT, GV_LINE | GV_BOUNDARY, GV_AREA, 
	                       // ( GV_BOUNDARY, GV_CENTROID )
	int     mQgisType;      // WKBPoint, WKBLineString, ...
	int     mLayerId;       // ID used in layers
        struct  Map_info *mMap; // vector header pointer
	
        struct line_pnts *mPoints; // points structure 
        struct line_cats *mCats;   // cats structure
	struct ilist     *mList; 
	BOUND_BOX mMapBox;         // map bounding box
	int    mCidxFieldIndex;    // index for layerField in category index or -1 if no such field
	int    mCidxFieldNumCats;  // Number of records in field index
	int    mNextCidx;          // next index in cidxFieldIndex to be read, used to find nextFeature

	// selection: array of size nlines or nareas + 1, set to 1 - selected or 0 - not selected, 2 - read
	// Code 2 means that the line was already read in this cycle, all 2 must be reset to 1
	// if getFirstFeature() or select() is calles. 
	// Distinction between 1 and 2 is used if attribute table exists, in that case attributes are
	// read from the table and geometry is append and selection set to 2.
	// In the end the selection array is scanned for 1 (attributes missing), and the geometry 
	// is returned without attributes
	char    *mSelection;   
	int     mSelectionSize; // size of selection array

	QString mDataSourceUri;
	bool    mValid;
	long    mNumberFeatures;
	int     mEndian;               // endian
	
	void resetSelection(bool sel); // reset selection
	void checkEndian();            // set endian

	// -----------------------------------------------------------------------------------------
	/* Static variables and methods.
         * These methods opens GRASS vectors and loads some parts of vectors to the memory.
         * it maintains the list of opened layers so that sources are not duplicated in the memory. 
         * Layers are identified by layer ID. 
         * The layers have unique URI, if next layer of the same URI is requested, 
         * nUsers is increased and ID of the layer which is already opened is returned.
         * Attributes are loaded from DB and stored in the memory when layer is opened.
         */

	/*! Open layer. Layer for QgsGrassVector means Map+field
	 *  @param gisdbase 
	 *  @param location
	 *  @param mapset
	 *  @param mapName
	 *  @param field
	 *  @return layer ID
	 *  @return -1 cannot open
	 */
	static int openLayer(QString gisdbase, QString location, QString mapset, QString mapName, int field);

	/*! Close layer. 
	 *  @param layerId 
	 */
	static void closeLayer( int layerId );

	/*! Open map. 
	 *  @param gisdbase 
	 *  @param location
	 *  @param mapset
	 *  @param mapName
	 *  @return map ID
	 *  @return -1 cannot open
	 */
	static int openMap(QString gisdbase, QString location, QString mapset, QString mapName);

	/*! Close map. 
	 *  @param mapId 
	 */
	static void closeMap( int mapId );
	
	/*! Get layer map. 
	 *  @param layerId 
	 *  @return pointer to Map_info structure
	 */
	static struct Map_info *layerMap( int layerId );
	
	/*! Get attribute by category(key) and attribute number. 
	 *  @param layerId 
	 *  @param category (key) 
	 *  @param column column number ( < nColumns ) 
	 *  @return pointer to string representation of the value or NULL, this value must not be changed
	 */
	static char *attribute( int layerId, int cat, int column );
	
	/*! Set feature attributes. 
	 *  @param layerId
	 *  @param feature
	 *  @param cat category number
	 */
	static void setFeatureAttributes ( int layerId, int cat, QgsFeature *feature);

	/*! Set feature attributes. 
	 *  @param layerId
	 *  @param feature
	 *  @param cat category number
	 *  @param attlist a list containing the index number of the fields to set
	 */
	static void setFeatureAttributes ( int layerId, int cat, QgsFeature *feature, std::list<int>& attlist);

	//
	/* Static arrays of opened layers and vectors */
	static 	std::vector<GLAYER> mLayers; // Map + field/attributes
	static 	std::vector<GMAP> mMaps;     // Map
};

#endif // QGSGRASSPROVIDER_H
