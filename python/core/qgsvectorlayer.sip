/******************************************************************
 * This file has been generated automatically by sipify.pl        *
 * Do not edit manually ! Edit header file and generate it again. *
 *****************************************************************/








typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;
typedef QList<QgsPointV2> QgsPointSequence;



class QgsVectorLayer : QgsMapLayer, QgsExpressionContextGenerator
{
%Docstring
 \ingroup core
 Represents a vector layer which manages a vector based data sets.

 The QgsVectorLayer is instantiated by specifying the name of a data provider,
 such as postgres or wfs, and url defining the specific data set to connect to.
 The vector layer constructor in turn instantiates a QgsVectorDataProvider subclass
 corresponding to the provider type, and passes it the url.  The data provider
 connects to the data source.

 The QgsVectorLayer provides a common interface to the different data types.  It also
 manages editing transactions.

  Sample usage of the QgsVectorLayer class:

 \code
     QString uri = "point?crs=epsg:4326&field=id:integer";
     QgsVectorLayer *scratchLayer = new QgsVectorLayer(uri, "Scratch point layer",  "memory");
 \endcode

 The main data providers supported by QGIS are listed below.

 \section providers Vector data providers

 \subsection memory Memory data providerType (memory)

 The memory data provider is used to construct in memory data, for example scratch
 data or data generated from spatial operations such as contouring.  There is no
 inherent persistent storage of the data.  The data source uri is constructed.  The
 url specifies the geometry type ("point", "linestring", "polygon",
 "multipoint","multilinestring","multipolygon"), optionally followed by url parameters
 as follows:

 - crs=definition
   Defines the coordinate reference system to use for the layer.
   definition is any string accepted by QgsCoordinateReferenceSystem::createFromString()

 - index=yes
   Specifies that the layer will be constructed with a spatial index

 - field=name:type(length,precision)
   Defines an attribute of the layer.  Multiple field parameters can be added
   to the data provider definition.  type is one of "integer", "double", "string".

 An example url is "Point?crs=epsg:4326&field=id:integer&field=name:string(20)&index=yes"

 \subsection ogr OGR data provider (ogr)

 Accesses data using the OGR drivers (http://www.gdal.org/ogr/ogr_formats.html). The url
 is the OGR connection string.  A wide variety of data formats can be accessed using this
 driver, including file based formats used by many GIS systems, database formats, and
 web services.  Some of these formats are also supported by custom data providers listed
 below.

 \subsection spatialite Spatialite data provider (spatialite)

 Access data in a spatialite database. The url defines the connection parameters, table,
 geometry column, and other attributes.  The url can be constructed using the
 QgsDataSourceUri class.

 \subsection postgres Postgresql data provider (postgres)

 Connects to a postgresql database.  The url defines the connection parameters, table,
 geometry column, and other attributes.  The url can be constructed using the
 QgsDataSourceUri class.

 \subsection mssql Microsoft SQL server data provider (mssql)

 Connects to a Microsoft SQL server database.  The url defines the connection parameters, table,
 geometry column, and other attributes.  The url can be constructed using the
 QgsDataSourceUri class.

 \subsection wfs WFS (web feature service) data provider (wfs)

 Used to access data provided by a web feature service.

 The url can be a HTTP url to a WFS server (legacy, e.g. http://foobar/wfs?TYPENAME=xxx&SRSNAME=yyy[&FILTER=zzz]), or,
 starting with QGIS 2.16, a URI constructed using the QgsDataSourceUri class with the following parameters :
 - url=string (mandatory): HTTP url to a WFS server endpoint. e.g http://foobar/wfs
 - typename=string (mandatory): WFS typename
 - srsname=string (recommended): SRS like 'EPSG:XXXX'
 - username=string
 - password=string
 - authcfg=string
 - version=auto/1.0.0/1.1.0/2.0.0
  -sql=string: full SELECT SQL statement with optional WHERE, ORDER BY and possibly with JOIN if supported on server
 - filter=string: QGIS expression or OGC/FES filter
 - restrictToRequestBBOX=1: to download only features in the view extent (or more generally
   in the bounding box of the feature iterator)
 - maxNumFeatures=number
 - IgnoreAxisOrientation=1: to ignore EPSG axis order for WFS 1.1 or 2.0
 - InvertAxisOrientation=1: to invert axis order
 - hideDownloadProgressDialog=1: to hide the download progress dialog

 The ‘FILTER’ query string parameter can be used to filter
 the WFS feature type. The ‘FILTER’ key value can either be a QGIS expression
 or an OGC XML filter. If the value is set to a QGIS expression the driver will
 turn it into OGC XML filter before passing it to the WFS server. Beware the
 QGIS expression filter only supports” =, !=, <, >, <=, >=, AND, OR, NOT, LIKE, IS NULL”
 attribute operators, “BBOX, Disjoint, Intersects, Touches, Crosses, Contains, Overlaps, Within”
 spatial binary operators and the QGIS local “geomFromWKT, geomFromGML”
 geometry constructor functions.

 Also note:

 - You can use various functions available in the QGIS Expression list,
   however the function must exist server side and have the same name and arguments to work.

 - Use the special $geometry parameter to provide the layer geometry column as input
   into the spatial binary operators e.g intersects($geometry, geomFromWKT('POINT (5 6)'))

 \subsection delimitedtext Delimited text file data provider (delimitedtext)

 Accesses data in a delimited text file, for example CSV files generated by
 spreadsheets. The contents of the file are split into columns based on specified
 delimiter characters.  Each record may be represented spatially either by an
 X and Y coordinate column, or by a WKT (well known text) formatted columns.

 The url defines the filename, the formatting options (how the
 text in the file is divided into data fields, and which fields contain the
 X,Y coordinates or WKT text definition.  The options are specified as url query
 items.

 At its simplest the url can just be the filename, in which case it will be loaded
 as a CSV formatted file.

 The url may include the following items:

 - encoding=UTF-8

   Defines the character encoding in the file.  The default is UTF-8.  To use
   the default encoding for the operating system use "System".

 - type=(csv|regexp|whitespace|plain)

   Defines the algorithm used to split records into columns. Records are
   defined by new lines, except for csv format files for which quoted fields
   may span multiple records.  The default type is csv.

   -  "csv" splits the file based on three sets of characters:
      delimiter characters, quote characters,
      and escape characters.  Delimiter characters mark the end
      of a field. Quote characters enclose a field which can contain
      delimiter characters, and newlines.  Escape characters cause the
      following character to be treated literally (including delimiter,
      quote, and newline characters).  Escape and quote characters must
      be different from delimiter characters. Escape characters that are
      also quote characters are treated specially - they can only
      escape themselves within quotes.  Elsewhere they are treated as
      quote characters.  The defaults for delimiter, quote, and escape
      are ',', '"', '"'.
   -  "regexp" splits each record using a regular expression (see QRegExp
      documentation for details).
   -  "whitespace" splits each record based on whitespace (on or more whitespace
      characters.  Leading whitespace in the record is ignored.
   -  "plain" is provided for backwards compatibility.  It is equivalent to
      CSV except that the default quote characters are single and double quotes,
      and there is no escape characters.

 - delimiter=characters

   Defines the delimiter characters used for csv and plain type files, or the
   regular expression for regexp type files.  It is a literal string of characters
   except that "\t" may be used to represent a tab character.

 - quote=characters

   Defines the characters that are used as quote characters for csv and plain type
   files.

 - escape=characters

   Defines the characters used to escape delimiter, quote, and newline characters.

 - skipLines=n

   Defines the number of lines to ignore at the beginning of the file (default 0)

 - useHeader=(yes|no)

   Defines whether the first record in the file (after skipped lines) contains
   column names (default yes)

 - trimFields=(yes|no)

   If yes then leading and trailing whitespace will be removed from fields

 - skipEmptyFields=(yes|no)

   If yes then empty fields will be discarded (equivalent to concatenating consecutive
   delimiters)

 - maxFields=#

   Specifies the maximum number of fields to load for each record.  Additional
   fields will be discarded.  Default is 0 - load all fields.

 - decimalPoint=c

   Defines a character that is used as a decimal point in the numeric columns
   The default is '.'.

 - xField=column yField=column

   Defines the name of the columns holding the x and y coordinates for XY point geometries.
   If the useHeader is no (ie there are no column names), then this is the column
   number (with the first column as 1).

 - xyDms=(yes|no)

   If yes then the X and Y coordinates are interpreted as
   degrees/minutes/seconds format (fairly permissively),
   or degree/minutes format.

 - wktField=column

   Defines the name of the columns holding the WKT geometry definition for WKT geometries.
   If the useHeader is no (ie there are no column names), then this is the column
   number (with the first column as 1).

 - geomType=(point|line|polygon|none)

   Defines the geometry type for WKT type geometries.  QGIS will only display one
   type of geometry for the layer - any others will be ignored when the file is
   loaded.  By default the provider uses the type of the first geometry in the file.
   Use geomType to override this type.

   geomType can also be set to none, in which case the layer is loaded without
   geometries.

 - subset=expression

   Defines an expression that will identify a subset of records to display

 - crs=crsstring

   Defines the coordinate reference system used for the layer.  This can be
   any string accepted by QgsCoordinateReferenceSystem::createFromString()

 -subsetIndex=(yes|no)

   Determines whether the provider generates an index to improve the efficiency
   of subsets.  The default is yes

 -spatialIndex=(yes|no)

   Determines whether the provider generates a spatial index.  The default is no.

 -watchFile=(yes|no)

   Defines whether the file will be monitored for changes. The default is
   to monitor for changes.

 - quiet

   Errors encountered loading the file will not be reported in a user dialog if
   quiet is included (They will still be shown in the output log).

 \subsection gpx GPX data provider (gpx)

 Provider reads tracks, routes, and waypoints from a GPX file.  The url
 defines the name of the file, and the type of data to retrieve from it
 ("track", "route", or "waypoint").

 An example url is "/home/user/data/holiday.gpx?type=route"

 \subsection grass Grass data provider (grass)

 Provider to display vector data in a GRASS GIS layer.

 TODO QGIS3: Remove virtual from non-inherited methods (like isModified)
 @see QgsVectorLayerUtils()
%End

%TypeHeaderCode
#include "qgsvectorlayer.h"
%End


  public:

    enum EditResult
    {
      Success,
      EmptyGeometry,
      EditFailed,
      FetchFeatureFailed,
      InvalidLayer,
    };

    enum SelectBehavior
    {
      SetSelection,
      AddToSelection,
      IntersectSelection,
      RemoveFromSelection,
    };

    QgsVectorLayer( const QString &path = QString::null, const QString &baseName = QString::null,
                    const QString &providerLib = "ogr", bool loadDefaultStyleFlag = true );
%Docstring
 Constructor - creates a vector layer

 The QgsVectorLayer is constructed by instantiating a data provider.  The provider
 interprets the supplied path (url) of the data source to connect to and access the
 data.

 @param  path  The path or url of the parameter.  Typically this encodes
               parameters used by the data provider as url query items.
 @param  baseName The name used to represent the layer in the legend
 @param  providerLib  The name of the data provider, e.g., "memory", "postgres"
 @param  loadDefaultStyleFlag whether to load the default style
%End


    virtual ~QgsVectorLayer();

//    QgsVectorLayer( const QgsVectorLayer &rhs ) = delete;
//    QgsVectorLayer &operator=( QgsVectorLayer const &rhs ) = delete;

    QString storageType() const;
%Docstring
Returns the permanent storage type for this layer as a friendly name.
%End

    QString capabilitiesString() const;
%Docstring
Capabilities for this layer in a friendly format.
%End

    QString dataComment() const;
%Docstring
Returns a comment for the data in the layer
%End

    QString displayField() const;
%Docstring
 This is a shorthand for accessing the displayExpression if it is a simple field.
 If the displayExpression is more complex than a simple field, a null string will
 be returned.

 @see displayExpression
%End

    void setDisplayExpression( const QString &displayExpression );
%Docstring
 Set the preview expression, used to create a human readable preview string.
  Used e.g. in the attribute table feature list. Uses { @link QgsExpression }.

  @param displayExpression The expression which will be used to preview features
                           for this layer
%End

    QString displayExpression() const;
%Docstring
  Get the preview expression, used to create a human readable preview string.
  Uses { @link QgsExpression }

  @return The expression which will be used to preview features for this layer
%End

    QgsVectorDataProvider *dataProvider();
%Docstring
Returns the data provider
%End

    // const QgsVectorDataProvider *dataProvider() const;  // SIP_SKIP


    void setProviderEncoding( const QString &encoding );
%Docstring
Sets the textencoding of the data provider
%End

    void setCoordinateSystem();
%Docstring
Setup the coordinate system transformation for the layer
%End

    bool addJoin( const QgsVectorLayerJoinInfo &joinInfo );
%Docstring
 Joins another vector layer to this layer
@param joinInfo join object containing join layer id, target and source field
@note since 2.6 returns bool indicating whether the join can be added *
%End

    bool removeJoin( const QString &joinLayerId );
%Docstring
 Removes a vector layer join
@returns true if join was found and successfully removed *
%End

    QgsVectorLayerJoinBuffer *joinBuffer();
%Docstring
 Accessor to the join buffer object
 @note added 2.14.7
%End
    const QList<QgsVectorLayerJoinInfo> vectorJoins() const;

    virtual bool setDependencies( const QSet<QgsMapLayerDependency> &layers );
%Docstring
 Sets the list of dependencies.
 @see dependencies()

 @param layers set of QgsMapLayerDependency. Only user-defined dependencies will be added
 @returns false if a dependency cycle has been detected
 @note added in QGIS 3.0
%End

    virtual QSet<QgsMapLayerDependency> dependencies() const;
%Docstring
 Gets the list of dependencies. This includes data dependencies set by the user (@see setDataDependencies)
 as well as dependencies given by the provider

 @returns a set of QgsMapLayerDependency
 @note added in QGIS 3.0
%End

    int addExpressionField( const QString &exp, const QgsField &fld );
%Docstring
 Add a new field which is calculated by the expression specified

 @param exp The expression which calculates the field
 @param fld The field to calculate

 @return The index of the new field

 @note added in 2.6, return value added in 2.9
%End

    void removeExpressionField( int index );
%Docstring
 Remove an expression field

 @param index The index of the field

 @note added in 2.6
%End

    QString expressionField( int index ) const;
%Docstring
 Returns the expression used for a given expression field

 @param index An index of an epxression based (virtual) field

 @return The expression for the field at index

 @note added in 2.9
%End

    void updateExpressionField( int index, const QString &exp );
%Docstring
 Changes the expression used to define an expression based (virtual) field

 @param index The index of the expression to change

 @param exp The new expression to set

 @note added in 2.9
%End

    QgsActionManager *actions();
%Docstring
 Get all layer actions defined on this layer.

 The pointer which is returned directly points to the actions object
 which is used by the layer, so any changes are immediately applied.
%End

    int selectedFeatureCount() const;
%Docstring
 The number of features that are selected in this layer

 @return See description
%End

    void selectByRect( QgsRectangle &rect, SelectBehavior behavior = SetSelection );
%Docstring
 Select features found within the search rectangle (in layer's coordinates)
 @param rect search rectangle
 @param behavior selection type, allows adding to current selection, removing
 from selection, etc.
 @see invertSelectionInRectangle(QgsRectangle & rect)
 @see selectByExpression()
 @see selectByIds()
%End

    void selectByExpression( const QString &expression, SelectBehavior behavior = SetSelection );
%Docstring
 Select matching features using an expression.
 @param expression expression to evaluate to select features
 @param behavior selection type, allows adding to current selection, removing
 from selection, etc.
 @note added in QGIS 2.16
 @see selectByRect()
 @see selectByIds()
%End

    void selectByIds( const QgsFeatureIds &ids, SelectBehavior behavior = SetSelection );
%Docstring
 Select matching features using a list of feature IDs. Will emit the
 selectionChanged() signal with the clearAndSelect flag set.
 @param ids feature IDs to select
 @param behavior selection type, allows adding to current selection, removing
 from selection, etc.
 @note added in QGIS 2.16
 @see selectByRect()
 @see selectByExpression()
%End

    void modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds );
%Docstring
 Modifies the current selection on this layer

 @param selectIds    Select these ids
 @param deselectIds  Deselect these ids

 @see   select(QgsFeatureIds)
 @see   select(QgsFeatureId)
 @see   deselect(QgsFeatureIds)
 @see   deselect(QgsFeatureId)
 @see selectByExpression()
%End

    void invertSelection();
%Docstring
Select not selected features and deselect selected ones
%End

    void selectAll();
%Docstring
Select all the features
%End

    QgsFeatureIds allFeatureIds() const;
%Docstring
Get all feature Ids
%End

    void invertSelectionInRectangle( QgsRectangle &rect );
%Docstring
 Invert selection of features found within the search rectangle (in layer's coordinates)

 @param rect  The rectangle in which the selection of features will be inverted

 @see   invertSelection()
%End

    QgsFeatureList selectedFeatures() const;
%Docstring
 Get a copy of the user-selected features

 @return A list of { @link QgsFeature } 's

 @see    selectedFeatureIds()
 @see    selectedFeaturesIterator() which is more memory friendly when handling large selections
%End

    QgsFeatureIterator selectedFeaturesIterator( QgsFeatureRequest request = QgsFeatureRequest() ) const;
%Docstring
 Get an iterator of the selected features

 @param request You may specify a request, e.g. to limit the set of requested attributes.
                Any filter on the request will be discarded.

 @return Iterator over the selected features

 @see    selectedFeatureIds()
 @see    selectedFeatures()
%End

    const QgsFeatureIds &selectedFeatureIds() const;
%Docstring
 Return reference to identifiers of selected features

 @return A list of { @link QgsFeatureId } 's
 @see selectedFeatures()
%End

    QgsRectangle boundingBoxOfSelected() const;
%Docstring
Returns the bounding box of the selected features. If there is no selection, QgsRectangle(0,0,0,0) is returned
%End

    bool labelsEnabled() const;
%Docstring
 Returns whether the layer contains labels which are enabled and should be drawn.
 @return true if layer contains enabled labels
 @note added in QGIS 2.9
%End

    bool diagramsEnabled() const;
%Docstring
 Returns whether the layer contains diagrams which are enabled and should be drawn.
 @return true if layer contains enabled diagrams
 @note added in QGIS 2.9
%End

    void setDiagramRenderer( QgsDiagramRenderer *r /Transfer/ );
%Docstring
Sets diagram rendering object (takes ownership)
%End
    const QgsDiagramRenderer *diagramRenderer() const;

    void setDiagramLayerSettings( const QgsDiagramLayerSettings &s );
    const QgsDiagramLayerSettings *diagramLayerSettings() const;

    QgsFeatureRenderer *renderer();
%Docstring
Return renderer.
%End

    // const QgsFeatureRenderer *renderer() const { return mRenderer; } // SIP_SKIP


    void setRenderer( QgsFeatureRenderer *r /Transfer/ );
%Docstring
 Set renderer which will be invoked to represent this layer.
 Ownership is transferred.
%End

    QgsWkbTypes::GeometryType geometryType() const;
%Docstring
Returns point, line or polygon
%End

    bool hasGeometryType() const;
%Docstring
Returns true if this is a geometry layer and false in case of NoGeometry (table only) or UnknownGeometry
%End

    QgsWkbTypes::Type wkbType() const;
%Docstring
Returns the WKBType or WKBUnknown in case of error
%End

    QString providerType() const;
%Docstring
Return the provider type for this layer
%End

    virtual bool readXml( const QDomNode &layer_node );
%Docstring
 Reads vector layer specific state from project file Dom node.
 @note Called by QgsMapLayer::readXml().
%End

    virtual bool writeXml( QDomNode &layer_node, QDomDocument &doc ) const;
%Docstring
 Write vector layer specific state to project file Dom node.
 @note Called by QgsMapLayer::writeXml().
%End

    void resolveReferences( QgsProject *project );
%Docstring
 Resolve references to other layers (kept as layer IDs after reading XML) into layer objects.
 @note added in 3.0
%End

    virtual void saveStyleToDatabase( const QString &name, const QString &description,
                                      bool useAsDefault, const QString &uiFileContent,
                                      QString &msgError /Out/ );
%Docstring
 Save named and sld style of the layer to the style table in the db.
 @param name
 @param description
 @param useAsDefault
 @param uiFileContent
 @param msgError
%End

    virtual int listStylesInDatabase( QStringList &ids /Out/, QStringList &names /Out/,
                                      QStringList &descriptions /Out/, QString &msgError /Out/ );
%Docstring
 Lists all the style in db split into related to the layer and not related to
 @param ids the list in which will be stored the style db ids
 @param names the list in which will be stored the style names
 @param descriptions the list in which will be stored the style descriptions
 @param msgError
 @return the number of styles related to current layer
%End

    virtual QString getStyleFromDatabase( const QString &styleId, QString &msgError /Out/ );
%Docstring
 Will return the named style corresponding to style id provided
%End

    virtual bool deleteStyleFromDatabase( const QString &styleId, QString &msgError /Out/ );
%Docstring
 Delete a style from the database
 @note added in QGIS 3.0
 @param styleId the provider's layer_styles table id of the style to delete
 @param msgError reference to string that will be updated with any error messages
 @return true in case of success
%End

    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag /Out/, bool loadFromLocalDb );
%Docstring
 Load a named style from file/local db/datasource db
 @param theURI the URI of the style or the URI of the layer
 @param resultFlag will be set to true if a named style is correctly loaded
 @param loadFromLocalDb if true forces to load from local db instead of datasource one
%End

    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag /Out/ );
%Docstring
 Calls loadNamedStyle( theURI, resultFlag, false );
 Retained for backward compatibility
%End

    bool readSymbology( const QDomNode &layerNode, QString &errorMessage );
%Docstring
 Read the symbology for the current layer from the Dom node supplied.
 @param layerNode node that will contain the symbology definition for this layer.
 @param errorMessage reference to string that will be updated with any error messages
 @return true in case of success.
%End

    bool readStyle( const QDomNode &node, QString &errorMessage );
%Docstring
 Read the style for the current layer from the Dom node supplied.
 @param node node that will contain the style definition for this layer.
 @param errorMessage reference to string that will be updated with any error messages
 @return true in case of success.
%End

    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage ) const;
%Docstring
 Write the symbology for the layer into the docment provided.
  @param node the node that will have the style element added to it.
  @param doc the document that will have the QDomNode added.
  @param errorMessage reference to string that will be updated with any error messages
  @return true in case of success.
%End

    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage ) const;
%Docstring
 Write just the style information for the layer into the document
  @param node the node that will have the style element added to it.
  @param doc the document that will have the QDomNode added.
  @param errorMessage reference to string that will be updated with any error messages
  @return true in case of success.
%End

    bool writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsStringMap &props = QgsStringMap() ) const;
%Docstring
 Writes the symbology of the layer into the document provided in SLD 1.1 format
 @param node the node that will have the style element added to it.
 @param doc the document that will have the QDomNode added.
 @param errorMessage reference to string that will be updated with any error messages
 @param props a open ended set of properties that can drive/inform the SLD encoding
 @return true in case of success
%End

    bool readSld( const QDomNode &node, QString &errorMessage );

    long featureCount( const QString &legendKey ) const;
%Docstring
 Number of features rendered with specified legend key. Features must be first
 calculated by countSymbolFeatures()
 @return number of features rendered by symbol or -1 if failed or counts are not available
%End

    void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, bool loadDefaultStyleFlag = false );
%Docstring
 Update the data source of the layer. The layer's renderer and legend will be preserved only
 if the geometry type of the new data source matches the current geometry type of the layer.
 @param dataSource new layer data source
 @param baseName base name of the layer
 @param provider provider string
 @param loadDefaultStyleFlag set to true to reset the layer's style to the default for the
 data source
 @note added in QGIS 2.10
%End

    bool countSymbolFeatures( bool showProgress = true );
%Docstring
 Count features for symbols. Feature counts may be get by featureCount().
 @param showProgress show progress dialog
 @return true if calculated, false if failed or was canceled by user
%End

    virtual bool setSubsetString( const QString &subset );
%Docstring
 Set the string (typically sql) used to define a subset of the layer
 @param subset The subset string. This may be the where clause of a sql statement
               or other definition string specific to the underlying dataprovider
               and data store.
 @return true, when setting the subset string was successful, false otherwise
%End

    virtual QString subsetString() const;
%Docstring
 Get the string (typically sql) used to define a subset of the layer
 @return The subset string or QString::null if not implemented by the provider
%End

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const;
%Docstring
 Query the layer for features specified in request.
 @param request feature request describing parameters of features to return
 @returns iterator for matching features from provider
%End

    QgsFeatureIterator getFeatures( const QString &expression );
%Docstring
 Query the layer for features matching a given expression.
%End

    QgsFeature getFeature( QgsFeatureId fid );
%Docstring
 Query the layer for the feature with the given id.
 If there is no such feature, the returned feature will be invalid.
%End

    QgsFeatureIterator getFeatures( const QgsFeatureIds &fids );
%Docstring
 Query the layer for the features with the given ids.
%End

    QgsFeatureIterator getFeatures( const QgsRectangle &rectangle );
%Docstring
 Query the layer for the features which intersect the specified rectangle.
%End

    bool addFeature( QgsFeature &feature, bool alsoUpdateExtent = true );
%Docstring
 Adds a feature
@param feature feature to add
@param alsoUpdateExtent If True, will also go to the effort of e.g. updating the extents.
@return                    True in case of success and False in case of error
%End

    bool updateFeature( QgsFeature &f );
%Docstring
 Updates an existing feature. This method needs to query the datasource
on every call. Consider using {@link changeAttributeValue()} or
{@link changeGeometry()} instead.
@param f  Feature to update
@return   True in case of success and False in case of error
%End

    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );
%Docstring
 Insert a new vertex before the given vertex number,
  in the given ring, item (first number is index 0), and feature
  Not meaningful for Point geometries
%End

    bool insertVertex( const QgsPointV2 &point, QgsFeatureId atFeatureId, int beforeVertex );
%Docstring
 Insert a new vertex before the given vertex number,
  in the given ring, item (first number is index 0), and feature
  Not meaningful for Point geometries
%End

    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );
%Docstring
 Moves the vertex at the given position number,
  ring and item (first number is index 0), and feature
  to the given coordinates
%End

    bool moveVertex( const QgsPointV2 &p, QgsFeatureId atFeatureId, int atVertex ) /PyName=moveVertexV2/;
%Docstring
 Moves the vertex at the given position number,
 ring and item (first number is index 0), and feature
 to the given coordinates
 @note available in python as moveVertexV2
%End

    EditResult deleteVertex( QgsFeatureId featureId, int vertex );
%Docstring
 Deletes a vertex from a feature.
 @param featureId ID of feature to remove vertex from
 @param vertex index of vertex to delete
 @note added in QGIS 2.14
%End

    bool deleteSelectedFeatures( int *deletedCount = 0 );
%Docstring
 Deletes the selected features
  @return true in case of success and false otherwise
%End

    int addRing( const QList<QgsPoint> &ring, QgsFeatureId *featureId = 0 );
%Docstring
TODO QGIS 3.0 returns an enum instead of a magic constant
%End

    int addRing( QgsCurve *ring /Transfer/, QgsFeatureId *featureId = 0 ) /PyName=addCurvedRing/;
%Docstring
TODO QGIS 3.0 returns an enum instead of a magic constant
%End

    int addPart( const QList<QgsPoint> &ring );
%Docstring
TODO QGIS 3.0 returns an enum instead of a magic constant
%End

    int addPart( const QgsPointSequence &ring ) /PyName=addPartV2/;
%Docstring
TODO QGIS 3.0 returns an enum instead of a magic constant
%End

    int addPart( QgsCurve *ring /Transfer/ ) /PyName=addCurvedPart/;
%Docstring
@note available in python as addCurvedPart
%End

    int translateFeature( QgsFeatureId featureId, double dx, double dy );
%Docstring
 Translates feature by dx, dy
  @param featureId id of the feature to translate
  @param dx translation of x-coordinate
  @param dy translation of y-coordinate
  @return 0 in case of success
%End

    int splitParts( const QList<QgsPoint> &splitLine, bool topologicalEditing = false );
%Docstring
TODO QGIS 3.0 returns an enum instead of a magic constant
%End

    int splitFeatures( const QList<QgsPoint> &splitLine, bool topologicalEditing = false );
%Docstring
TODO QGIS 3.0 returns an enum instead of a magic constant
%End

    int addTopologicalPoints( const QgsGeometry &geom );
%Docstring
 Adds topological points for every vertex of the geometry.
 @param geom the geometry where each vertex is added to segments of other features
 @note geom is not going to be modified by the function
 @return 0 in case of success
%End

    int addTopologicalPoints( const QgsPoint &p );
%Docstring
 Adds a vertex to segments which intersect point p but don't
 already have a vertex there. If a feature already has a vertex at position p,
 no additional vertex is inserted. This method is useful for topological
 editing.
 @param p position of the vertex
 @return 0 in case of success
%End

    int insertSegmentVerticesForSnap( const QList<QgsSnappingResult> &snapResults );
%Docstring
 Inserts vertices to the snapped segments.
 This is useful for topological editing if snap to segment is enabled.
 @param snapResults results collected from the snapping operation
 @return 0 in case of success
%End

    // const QgsAbstractVectorLayerLabeling *labeling() const { return mLabeling; } // SIP_SKIP


    // void setLabeling( QgsAbstractVectorLayerLabeling *labeling ); // SIP_SKIP


    virtual bool isEditable() const;
%Docstring
Returns true if the provider is in editing mode
%End

    virtual bool isSpatial() const;

    virtual bool isModified() const;
%Docstring
Returns true if the provider has been modified since the last commit
%End

    bool snapPoint( QgsPoint &point, double tolerance );
%Docstring
 Snaps a point to the closest vertex if there is one within the snapping tolerance
  @param point       The point which is set to the position of a vertex if there is one within the snapping tolerance.
  If there is no point within this tolerance, point is left unchanged.
  @param tolerance   The snapping tolerance
  @return true if the point has been snapped, false if no vertex within search tolerance
%End

    int snapWithContext( const QgsPoint &startPoint,
                         double snappingTolerance,
                         QMultiMap < double, QgsSnappingResult > &snappingResults /Out/,
                         QgsSnapper::SnappingType snap_to );
%Docstring
 Snaps to segment or vertex within given tolerance
 @param startPoint point to snap (in layer coordinates)
 @param snappingTolerance distance tolerance for snapping
 @param snappingResults snapping results. Key is the distance between startPoint and snapping target
 @param snap_to to segment / to vertex
 @return 0 in case of success
%End

    virtual void reload();
%Docstring
Synchronises with changes in the datasource
%End

    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) /Factory/;
%Docstring
 Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
 @note added in 2.4
%End

    QgsRectangle extent() const;
%Docstring
Return the extent of the layer
%End

    QgsFields fields() const;
%Docstring
 Returns the list of fields of this layer.
 This also includes fields which have not yet been saved to the provider.

 @return A list of fields
%End

    QgsFields pendingFields() const;
%Docstring
 Returns the list of fields of this layer.
 This also includes fields which have not yet been saved to the provider.
 Alias for {@link fields()}

 @return A list of fields
%End

    QgsAttributeList pendingAllAttributesList() const;
%Docstring
 Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
 Alias for {@link attributeList()}
%End

    QgsAttributeList attributeList() const;
%Docstring
 Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
 Alias for {@link attributeList()}
%End

    QgsAttributeList pendingPkAttributesList() const;
%Docstring
 Returns list of attributes making up the primary key
 Alias for {@link pkAttributeList()}
%End

    QgsAttributeList pkAttributeList() const;
%Docstring
Returns list of attributes making up the primary key
%End

    long pendingFeatureCount() const;
%Docstring
 Returns feature count including changes which have not yet been committed
 Alias for {@link featureCount()}
%End

    long featureCount() const;
%Docstring
 Returns feature count including changes which have not yet been committed
 If you need only the count of committed features call this method on this layer's provider.
%End

    bool setReadOnly( bool readonly = true );
%Docstring
 Make layer read-only (editing disabled) or not
  @return false if the layer is in editing yet
%End

    bool changeGeometry( QgsFeatureId fid, const QgsGeometry &geom );
%Docstring
Change feature's geometry
%End

    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );
%Docstring
 Changes an attribute value (but does not commit it)

 @param fid   The feature id of the feature to be changed
 @param field The index of the field to be updated
 @param newValue The value which will be assigned to the field
 @param oldValue The previous value to restore on undo (will otherwise be retrieved)

 @return true in case of success
%End

    bool addAttribute( const QgsField &field );
%Docstring
 Add an attribute field (but does not commit it)
 returns true if the field was added
%End

    void setFieldAlias( int index, const QString &aliasString );
%Docstring
 Sets an alias (a display name) for attributes to display in dialogs

 @note Added in QGIS 3.0
%End

    void removeFieldAlias( int index );
%Docstring
 Removes an alias (a display name) for attributes to display in dialogs

 @note Added in QGIS 3.0
%End

    bool renameAttribute( int index, const QString &newName );
%Docstring
 Renames an attribute field  (but does not commit it).
 @param index attribute index
 @param newName new name of field
 @note added in QGIS 2.16
%End

    QString attributeAlias( int index ) const;
%Docstring
 Returns the alias of an attribute name or a null string if there is no alias.

 @see {attributeDisplayName( int attributeIndex )} which returns the field name
      if no alias is defined.
%End

    QString attributeDisplayName( int index ) const;
%Docstring
Convenience function that returns the attribute alias if defined or the field name else
%End

    QgsStringMap attributeAliases() const;
%Docstring
Returns a map of field name to attribute alias
%End

    QSet<QString> excludeAttributesWms() const;
%Docstring
 A set of attributes that are not advertised in WMS requests with QGIS server.
%End

    void setExcludeAttributesWms( const QSet<QString> &att );
%Docstring
 A set of attributes that are not advertised in WMS requests with QGIS server.
%End

    QSet<QString> excludeAttributesWfs() const;
%Docstring
 A set of attributes that are not advertised in WFS requests with QGIS server.
%End

    void setExcludeAttributesWfs( const QSet<QString> &att );
%Docstring
 A set of attributes that are not advertised in WFS requests with QGIS server.
%End

    bool deleteAttribute( int attr );
%Docstring
Delete an attribute field (but does not commit it)
%End

    bool deleteAttributes( QList<int> attrs );
%Docstring
 Deletes a list of attribute fields (but does not commit it)

 @param  attrs the indices of the attributes to delete
 @return true if at least one attribute has been deleted
%End

    bool addFeatures( QgsFeatureList features, bool makeSelected = true );
%Docstring
Insert a copy of the given features into the layer  (but does not commit it)
%End

    bool deleteFeature( QgsFeatureId fid );
%Docstring
Delete a feature from the layer (but does not commit it)
%End

    bool deleteFeatures( const QgsFeatureIds &fids );
%Docstring
 Deletes a set of features from the layer (but does not commit it)
 @param fids The feature ids to delete

 @return false if the layer is not in edit mode or does not support deleting
         in case of an active transaction depends on the provider implementation
%End

    bool commitChanges();
%Docstring
 Attempts to commit any changes to disk.  Returns the result of the attempt.
 If a commit fails, the in-memory changes are left alone.

 This allows editing to continue if the commit failed on e.g. a
 disallowed value in a Postgres database - the user can re-edit and try
 again.

 The commits occur in distinct stages,
 (add attributes, add features, change attribute values, change
 geometries, delete features, delete attributes)
 so if a stage fails, it's difficult to roll back cleanly.
 Therefore any error message also includes which stage failed so
 that the user has some chance of repairing the damage cleanly.
 @see commitErrors()
%End

    QStringList commitErrors() const;
%Docstring
 Returns a list containing any error messages generated when attempting
 to commit changes to the layer.
 @see commitChanges()
%End

    bool rollBack( bool deleteBuffer = true );
%Docstring
 Stop editing and discard the edits
 @param deleteBuffer whether to delete editing buffer
%End

    QString annotationForm() const;
%Docstring
Get annotation form
%End

    void setAnnotationForm( const QString &ui );
%Docstring
Set annotation form for layer
%End

    QList<QgsRelation> referencingRelations( int idx ) const;
%Docstring
 Get relations, where the foreign key is on this layer

 @param idx Only get relations, where idx forms part of the foreign key
 @return A list of relations
%End

    QgsVectorLayerEditBuffer *editBuffer();
%Docstring
Buffer with uncommitted editing operations. Only valid after editing has been turned on.
%End

    // const QgsVectorLayerEditBuffer *editBuffer() const { return mEditBuffer; } // SIP_SKIP


    void beginEditCommand( const QString &text );
%Docstring
 Create edit command for undo/redo operations
 @param text text which is to be displayed in undo window
%End

    void endEditCommand();
%Docstring
Finish edit command and add it to undo/redo stack
%End

    void destroyEditCommand();
%Docstring
Destroy active command and reverts all changes in it
%End

    enum VertexMarkerType
    {
      SemiTransparentCircle,
      Cross,
      NoMarker
    };

    static void drawVertexMarker( double x, double y, QPainter &p, QgsVectorLayer::VertexMarkerType type, int vertexSize );
%Docstring
Draws a vertex symbol at (screen) coordinates x, y. (Useful to assist vertex editing.)
%End

    void updateFields();
%Docstring
Assembles mUpdatedFields considering provider fields, joined fields and added fields
%End

    QVariant defaultValue( int index, const QgsFeature &feature = QgsFeature(),
                           QgsExpressionContext *context = 0 ) const;
%Docstring
 Returns the calculated default value for the specified field index. The default
 value may be taken from a client side default value expression (see setDefaultValueExpression())
 or taken from the underlying data provider.
 @param index field index
 @param feature optional feature to use for default value evaluation. If passed,
 then properties from the feature (such as geometry) can be used when calculating
 the default value.
 @param context optional expression context to evaluate expressions again. If not
 specified, a default context will be created
 @return calculated default value
 @note added in QGIS 3.0
 @see setDefaultValueExpression()
%End

    void setDefaultValueExpression( int index, const QString &expression );
%Docstring
 Sets an expression to use when calculating the default value for a field.
 @param index field index
 @param expression expression to evaluate when calculating default values for field. Pass
 an empty expression to clear the default.
 @note added in QGIS 3.0
 @see defaultValue()
 @see defaultValueExpression()
%End

    QString defaultValueExpression( int index ) const;
%Docstring
 Returns the expression used when calculating the default value for a field.
 @param index field index
 @returns expression evaluated when calculating default values for field, or an
 empty string if no default is set
 @note added in QGIS 3.0
 @see defaultValue()
 @see setDefaultValueExpression()
%End

    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;
%Docstring
 Returns any constraints which are present for a specified
 field index. These constraints may be inherited from the layer's data provider
 or may be set manually on the vector layer from within QGIS.
 @note added in QGIS 3.0
 @see setFieldConstraint()
%End

    void setFieldConstraint( int index, QgsFieldConstraints::Constraint constraint, QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthHard );
%Docstring
 Sets a constraint for a specified field index. Any constraints inherited from the layer's
 data provider will be kept intact and cannot be modified. Ie, calling this method only allows for new
 constraints to be added on top of the existing provider constraints.
 @note added in QGIS 3.0
 @see fieldConstraints()
 @see removeFieldConstraint()
%End

    void removeFieldConstraint( int index, QgsFieldConstraints::Constraint constraint );
%Docstring
 Removes a constraint for a specified field index. Any constraints inherited from the layer's
 data provider will be kept intact and cannot be removed.
 @note added in QGIS 3.0
 @see fieldConstraints()
 @see setFieldConstraint()
%End

    QString constraintExpression( int index ) const;
%Docstring
 Returns the constraint expression for for a specified field index, if set.
 @note added in QGIS 3.0
 @see fieldConstraints()
 @see constraintDescription()
 @see setConstraintExpression()
%End

    QString constraintDescription( int index ) const;
%Docstring
 Returns the descriptive name for the constraint expression for a specified field index.
 @note added in QGIS 3.0
 @see constraints()
 @see constraintExpression()
 @see setConstraintExpression()
%End

    void setConstraintExpression( int index, const QString &expression, const QString &description = QString() );
%Docstring
 Set the constraint expression for the specified field index. An optional descriptive name for the constraint
 can also be set. Setting an empty expression will clear any existing expression constraint.
 @note added in QGIS 3.0
 @see constraintExpression()
 @see constraintDescription()
 @see constraints()
%End

    void setEditorWidgetSetup( int index, const QgsEditorWidgetSetup &setup );
%Docstring
 \copydoc editorWidgetSetup
%End

    QgsEditorWidgetSetup editorWidgetSetup( int index ) const;
%Docstring
 The editor widget setup defines which QgsFieldFormatter and editor widget will be used
 for the field at `index`.

 @note Added in QGIS 3.0
%End

    void uniqueValues( int index, QList<QVariant> &uniqueValues /Out/, int limit = -1 ) const;
%Docstring
 Calculates a list of unique values contained within an attribute in the layer. Note that
 in some circumstances when unsaved changes are present for the layer then the returned list
 may contain outdated values (for instance when the attribute value in a saved feature has
 been changed inside the edit buffer then the previous saved value will be included in the
 returned list).
 @param index column index for attribute
 @param uniqueValues out: result list
 @param limit maximum number of values to return (or -1 if unlimited)
 @see minimumValue()
 @see maximumValue()
%End

    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = 0 ) const;
%Docstring
 Returns unique string values of an attribute which contain a specified subset string. Subset
 matching is done in a case-insensitive manner. Note that
 in some circumstances when unsaved changes are present for the layer then the returned list
 may contain outdated values (for instance when the attribute value in a saved feature has
 been changed inside the edit buffer then the previous saved value will be included in the
 returned list).
 @param index column index for attribute
 @param substring substring to match (case insensitive)
 @param limit maxmum number of the values to return, or -1 to return all unique values
 @param feedback optional feedback object for canceling request
 @returns list of unique strings containing substring
%End

    QVariant minimumValue( int index ) const;
%Docstring
 Returns the minimum value for an attribute column or an invalid variant in case of error.
 Note that in some circumstances when unsaved changes are present for the layer then the
 returned value may be outdated (for instance when the attribute value in a saved feature has
 been changed inside the edit buffer then the previous saved value may be returned as the minimum).
 @see maximumValue()
 @see uniqueValues()
%End

    QVariant maximumValue( int index ) const;
%Docstring
 Returns the maximum value for an attribute column or an invalid variant in case of error.
 Note that in some circumstances when unsaved changes are present for the layer then the
 returned value may be outdated (for instance when the attribute value in a saved feature has
 been changed inside the edit buffer then the previous saved value may be returned as the maximum).
 @see minimumValue()
 @see uniqueValues()
%End

    QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                        const QString &fieldOrExpression,
                        const QgsAggregateCalculator::AggregateParameters &parameters = QgsAggregateCalculator::AggregateParameters(),
                        QgsExpressionContext *context = 0,
                        bool *ok = 0 ) const;
%Docstring
 Calculates an aggregated value from the layer's features.
 @param aggregate aggregate to calculate
 @param fieldOrExpression source field or expression to use as basis for aggregated values.
 @param parameters parameters controlling aggregate calculation
 @param context expression context for expressions and filters
 @param ok if specified, will be set to true if aggregate calculation was successful
 @return calculated aggregate value
 @note added in QGIS 2.16
%End

    QList< QVariant > getValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, QgsFeedback *feedback = 0 ) const;
%Docstring
 Fetches all values from a specified field name or expression.
 @param fieldOrExpression field name or an expression string
 @param ok will be set to false if field or expression is invalid, otherwise true
 @param selectedOnly set to true to get values from selected features only
 @param feedback optional feedback object to allow cancelation
 @returns list of fetched values
 @note added in QGIS 2.9
 @see getDoubleValues
%End

    QList< double > getDoubleValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, int *nullCount = 0, QgsFeedback *feedback = 0 ) const;
%Docstring
 Fetches all double values from a specified field name or expression. Null values or
 invalid expression results are skipped.
 @param fieldOrExpression field name or an expression string evaluating to a double value
 @param ok will be set to false if field or expression is invalid, otherwise true
 @param selectedOnly set to true to get values from selected features only
 @param nullCount optional pointer to integer to store number of null values encountered in
 @param feedback optional feedback object to allow cancelation
 @returns list of fetched values
 @note added in QGIS 2.9
 @see getValues
%End

    void setFeatureBlendMode( QPainter::CompositionMode blendMode );
%Docstring
Set the blending mode used for rendering each feature
%End
    QPainter::CompositionMode featureBlendMode() const;
%Docstring
Returns the current blending mode for features
%End

    void setLayerTransparency( int layerTransparency );
%Docstring
Set the transparency for the vector layer
%End
    int layerTransparency() const;
%Docstring
Returns the current transparency for the vector layer
%End

    QString metadata() const;

    // inline QgsGeometryCache *cache() { return mCache; } // SIP_SKIP


    void setSimplifyMethod( const QgsVectorSimplifyMethod &simplifyMethod );
%Docstring
 Set the simplification settings for fast rendering of features
  @note added in 2.2
%End

    const QgsVectorSimplifyMethod &simplifyMethod() const;
%Docstring
 Returns the simplification settings for fast rendering of features
  @note added in 2.2
%End

    bool simplifyDrawingCanbeApplied( const QgsRenderContext &renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const;
%Docstring
 Returns whether the VectorLayer can apply the specified simplification hint
  @note Do not use in 3rd party code - may be removed in future version!
  @note added in 2.2
%End

    QgsConditionalLayerStyles *conditionalStyles() const;
%Docstring
 @brief Return the conditional styles that are set for this layer. Style information is
 used to render conditional formatting in the attribute table.
 @return Return a QgsConditionalLayerStyles object holding the conditional attribute
 style information. Style information is generic and can be used for anything.
 @note added in QGIS 2.12
%End

    QgsAttributeTableConfig attributeTableConfig() const;
%Docstring
 Get the attribute table configuration object.
 This defines the appearance of the attribute table.
%End

    void setAttributeTableConfig( const QgsAttributeTableConfig &attributeTableConfig );
%Docstring
 Set the attribute table configuration object.
 This defines the appearance of the attribute table.
%End

    QString mapTipTemplate() const;
%Docstring
 The mapTip is a pretty, html representation for feature information.

 It may also contain embedded expressions.

 @note added in 3.0
%End

    void setMapTipTemplate( const QString &mapTipTemplate );
%Docstring
 The mapTip is a pretty, html representation for feature information.

 It may also contain embedded expressions.

 @note added in 3.0
%End

    QgsExpressionContext createExpressionContext() const;

    QgsEditFormConfig editFormConfig() const;
%Docstring
 Get the configuration of the form used to represent this vector layer.
 This is a writable configuration that can directly be changed in place.

 @return The configuration of this layers' form

 @note Added in QGIS 2.14
%End

    void setEditFormConfig( const QgsEditFormConfig &editFormConfig );
%Docstring
 Get the configuration of the form used to represent this vector layer.
 This is a writable configuration that can directly be changed in place.

 @return The configuration of this layers' form

 @note Added in QGIS 3.0
%End

  public slots:

    void select( QgsFeatureId featureId );
%Docstring
 Select feature by its ID

 @param featureId  The id of the feature to select

 @see select(QgsFeatureIds)
%End

    void select( const QgsFeatureIds &featureIds );
%Docstring
 Select features by their ID

 @param featureIds The ids of the features to select

 @see select(QgsFeatureId)
%End

    void deselect( const QgsFeatureId featureId );
%Docstring
 Deselect feature by its ID

 @param featureId  The id of the feature to deselect

 @see deselect(QgsFeatureIds)
%End

    void deselect( const QgsFeatureIds &featureIds );
%Docstring
 Deselect features by their ID

 @param featureIds The ids of the features to deselect

 @see deselect(QgsFeatureId)
%End

    void removeSelection();
%Docstring
 Clear selection

 @see selectByIds()
%End

    virtual void updateExtents();
%Docstring
 Update the extents for the layer. This is necessary if features are
  added/deleted or the layer has been subsetted.
%End

    bool startEditing();
%Docstring
 Make layer editable.
 This starts an edit session on this layer. Changes made in this edit session will not
 be made persistent until {@link commitChanges()} is called and can be reverted by calling
 {@link rollBack()}.
%End


  protected slots:
    void invalidateSymbolCountedFlag();

  signals:

    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, const bool clearAndSelect );
%Docstring
 This signal is emitted when selection was changed

 @param selected        Newly selected feature ids
 @param deselected      Ids of all features which have previously been selected but are not any more
 @param clearAndSelect  In case this is set to true, the old selection was dismissed and the new selection corresponds to selected
%End

    void layerModified();
%Docstring
This signal is emitted when modifications has been done on layer
%End

    void beforeModifiedCheck() const;
%Docstring
Is emitted, when layer is checked for modifications. Use for last-minute additions
%End

    void beforeEditingStarted();
%Docstring
Is emitted, before editing on this layer is started
%End

    void editingStarted();
%Docstring
Is emitted, when editing on this layer has started
%End

    void editingStopped();
%Docstring
Is emitted, when edited changes successfully have been written to the data provider
%End

    void beforeCommitChanges();
%Docstring
Is emitted, before changes are committed to the data provider
%End

    void beforeRollBack();
%Docstring
Is emitted, before changes are rolled back
%End

    void attributeAdded( int idx );
%Docstring
 Will be emitted, when a new attribute has been added to this vector layer.
 Applies only to types {@link QgsFields::OriginEdit}, {@link QgsFields::OriginProvider} and {@link QgsFields::OriginExpression }

 @param idx The index of the new attribute

 @see updatedFields()
%End

    void beforeAddingExpressionField( const QString &fieldName );
%Docstring
 Will be emitted, when an expression field is going to be added to this vector layer.
 Applies only to types {@link QgsFields::OriginExpression }

 @param fieldName The name of the attribute to be added
%End

    void attributeDeleted( int idx );
%Docstring
 Will be emitted, when an attribute has been deleted from this vector layer.
 Applies only to types {@link QgsFields::OriginEdit}, {@link QgsFields::OriginProvider} and {@link QgsFields::OriginExpression }

 @param idx The index of the deleted attribute

 @see updatedFields()
%End

    void beforeRemovingExpressionField( int idx );
%Docstring
 Will be emitted, when an expression field is going to be deleted from this vector layer.
 Applies only to types {@link QgsFields::OriginExpression }

 @param idx The index of the attribute to be deleted
%End

    void featureAdded( QgsFeatureId fid );
%Docstring
 Emitted when a new feature has been added to the layer

 @param fid The id of the new feature
%End

    void featureDeleted( QgsFeatureId fid );
%Docstring
 Emitted when a feature has been deleted.

 If you do expensive operations in a slot connected to this, you should prever to use
 {@link featuresDeleted( const QgsFeatureIds& )}.

 @param fid The id of the feature which has been deleted
%End

    void featuresDeleted( const QgsFeatureIds &fids );
%Docstring
 Emitted when features have been deleted.

 If features are deleted within an edit command, this will only be emitted once at the end
 to allow connected slots to minimize the overhead.
 If features are deleted outside of an edit command, this signal will be emitted once per feature.

 @param fids The feature ids that have been deleted.
%End

    void updatedFields();
%Docstring
 Is emitted, whenever the fields available from this layer have been changed.
 This can be due to manually adding attributes or due to a join.
%End


    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );
%Docstring
 Is emitted whenever an attribute value change is done in the edit buffer.
 Note that at this point the attribute change is not yet saved to the provider.

 @param fid The id of the changed feature
 @param idx The attribute index of the changed attribute
 @param value The new value of the attribute
%End

    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geometry );
%Docstring
 Is emitted whenever a geometry change is done in the edit buffer.
 Note that at this point the geometry change is not yet saved to the provider.

 @param fid The id of the changed feature
 @param geometry The new geometry
%End

    void committedAttributesDeleted( const QString &layerId, const QgsAttributeList &deletedAttributes );
%Docstring
This signal is emitted, when attributes are deleted from the provider
%End
    void committedAttributesAdded( const QString &layerId, const QList<QgsField> &addedAttributes );
%Docstring
This signal is emitted, when attributes are added to the provider
%End
    void committedFeaturesAdded( const QString &layerId, const QgsFeatureList &addedFeatures );
%Docstring
This signal is emitted, when features are added to the provider
%End
    void committedFeaturesRemoved( const QString &layerId, const QgsFeatureIds &deletedFeatureIds );
%Docstring
This signal is emitted, when features are deleted from the provider
%End
    void committedAttributeValuesChanges( const QString &layerId, const QgsChangedAttributesMap &changedAttributesValues );
%Docstring
This signal is emitted, when attribute value changes are saved to the provider
%End
    void committedGeometriesChanges( const QString &layerId, const QgsGeometryMap &changedGeometries );
%Docstring
This signal is emitted, when geometry changes are saved to the provider
%End

    void labelingFontNotFound( QgsVectorLayer *layer, const QString &fontfamily );
%Docstring
Emitted when the font family defined for labeling layer is not found on system
%End

    void featureBlendModeChanged( QPainter::CompositionMode blendMode );
%Docstring
Signal emitted when setFeatureBlendMode() is called
%End

    void layerTransparencyChanged( int layerTransparency );
%Docstring
Signal emitted when setLayerTransparency() is called
%End

    void editCommandStarted( const QString &text );
%Docstring
 Signal emitted when a new edit command has been started

 @param text Description for this edit command
%End

    void editCommandEnded();
%Docstring
 Signal emitted, when an edit command successfully ended
 @note This does not mean it is also committed, only that it is written
 to the edit buffer. See {@link beforeCommitChanges()}
%End

    void editCommandDestroyed();
%Docstring
 Signal emitted, whan an edit command is destroyed
 @note This is not a rollback, it is only related to the current edit command.
 See {@link beforeRollBack()}
%End

    void readCustomSymbology( const QDomElement &element, QString &errorMessage );
%Docstring
 Signal emitted whenever the symbology (QML-file) for this layer is being read.
 If there is custom style information saved in the file, you can connect to this signal
 and update the layer style accordingly.

 @param element The XML layer style element.

 @param errorMessage Write error messages into this string.
%End

    void writeCustomSymbology( QDomElement &element, QDomDocument &doc, QString &errorMessage ) const;
%Docstring
 Signal emitted whenever the symbology (QML-file) for this layer is being written.
 If there is custom style information you want to save to the file, you can connect
 to this signal and update the element accordingly.

 @param element  The XML element where you can add additional style information to.
 @param doc      The XML document that you can use to create new XML nodes.
 @param errorMessage Write error messages into this string.
%End

    void mapTipTemplateChanged();
%Docstring
 Emitted when the map tip changes

 @note added in QGIS 3.0
%End

    void displayExpressionChanged();
%Docstring
 Emitted when the display expression changes

 @note added in QGIS 3.0
%End

    void raiseError( const QString &msg );
%Docstring
 Signals an error related to this vector layer.
%End

    void editFormConfigChanged();
%Docstring
 Will be emitted whenever the edit form configuration of this layer changes.

 @note added in QGIS 3.0
%End

    void readOnlyChanged();
%Docstring
 Emitted when the read only state of this layer is changed.
 Only applies to manually set readonly state, not to the edit mode.

 @note Added in QGIS 3.0
%End


  protected:
    void setExtent( const QgsRectangle &rect );
%Docstring
Set the extent
%End

  private:                       // Private methods
    QgsVectorLayer( const QgsVectorLayer &rhs );
};

