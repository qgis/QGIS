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


    virtual ~QgsVectorLayer();

//    QgsVectorLayer( const QgsVectorLayer &rhs ) = delete;
//    QgsVectorLayer &operator=( QgsVectorLayer const &rhs ) = delete;

    QString storageType() const;

    QString capabilitiesString() const;

    QString dataComment() const;

    QString displayField() const;

    void setDisplayExpression( const QString &displayExpression );

    QString displayExpression() const;

    QgsVectorDataProvider *dataProvider();

    // const QgsVectorDataProvider *dataProvider() const;  // SIP_SKIP


    void setProviderEncoding( const QString &encoding );

    void setCoordinateSystem();

    bool addJoin( const QgsVectorLayerJoinInfo &joinInfo );

    bool removeJoin( const QString &joinLayerId );

    QgsVectorLayerJoinBuffer *joinBuffer();
    const QList<QgsVectorLayerJoinInfo> vectorJoins() const;

    virtual bool setDependencies( const QSet<QgsMapLayerDependency> &layers );

    virtual QSet<QgsMapLayerDependency> dependencies() const;

    int addExpressionField( const QString &exp, const QgsField &fld );

    void removeExpressionField( int index );

    QString expressionField( int index ) const;

    void updateExpressionField( int index, const QString &exp );

    QgsActionManager *actions();

    int selectedFeatureCount() const;

    void selectByRect( QgsRectangle &rect, SelectBehavior behavior = SetSelection );

    void selectByExpression( const QString &expression, SelectBehavior behavior = SetSelection );

    void selectByIds( const QgsFeatureIds &ids, SelectBehavior behavior = SetSelection );

    void modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds );

    void invertSelection();

    void selectAll();

    QgsFeatureIds allFeatureIds() const;

    void invertSelectionInRectangle( QgsRectangle &rect );

    QgsFeatureList selectedFeatures() const;

    QgsFeatureIterator selectedFeaturesIterator( QgsFeatureRequest request = QgsFeatureRequest() ) const;

    const QgsFeatureIds &selectedFeatureIds() const;

    QgsRectangle boundingBoxOfSelected() const;

    bool labelsEnabled() const;

    bool diagramsEnabled() const;

    void setDiagramRenderer( QgsDiagramRenderer *r /Transfer/ );
    const QgsDiagramRenderer *diagramRenderer() const;

    void setDiagramLayerSettings( const QgsDiagramLayerSettings &s );
    const QgsDiagramLayerSettings *diagramLayerSettings() const;

    QgsFeatureRenderer *renderer();

    // const QgsFeatureRenderer *renderer() const { return mRenderer; } // SIP_SKIP


    void setRenderer( QgsFeatureRenderer *r /Transfer/ );

    QgsWkbTypes::GeometryType geometryType() const;

    bool hasGeometryType() const;

    QgsWkbTypes::Type wkbType() const;

    QString providerType() const;

    virtual bool readXml( const QDomNode &layer_node );

    virtual bool writeXml( QDomNode &layer_node, QDomDocument &doc ) const;

    void resolveReferences( QgsProject *project );

    virtual void saveStyleToDatabase( const QString &name, const QString &description,
                                      bool useAsDefault, const QString &uiFileContent,
                                      QString &msgError /Out/ );

    virtual int listStylesInDatabase( QStringList &ids /Out/, QStringList &names /Out/,
                                      QStringList &descriptions /Out/, QString &msgError /Out/ );

    virtual QString getStyleFromDatabase( const QString &styleId, QString &msgError /Out/ );

    virtual bool deleteStyleFromDatabase( const QString &styleId, QString &msgError /Out/ );

    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag /Out/, bool loadFromLocalDb );

    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag /Out/ );

    bool readSymbology( const QDomNode &layerNode, QString &errorMessage );

    bool readStyle( const QDomNode &node, QString &errorMessage );

    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage ) const;

    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage ) const;

    bool writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsStringMap &props = QgsStringMap() ) const;

    bool readSld( const QDomNode &node, QString &errorMessage );

    long featureCount( const QString &legendKey ) const;

    void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, bool loadDefaultStyleFlag = false );

    bool countSymbolFeatures( bool showProgress = true );

    virtual bool setSubsetString( const QString &subset );

    virtual QString subsetString() const;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const;

    QgsFeatureIterator getFeatures( const QString &expression );

    QgsFeature getFeature( QgsFeatureId fid );

    QgsFeatureIterator getFeatures( const QgsFeatureIds &fids );

    QgsFeatureIterator getFeatures( const QgsRectangle &rectangle );

    bool addFeature( QgsFeature &feature, bool alsoUpdateExtent = true );

    bool updateFeature( QgsFeature &f );

    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );

    bool insertVertex( const QgsPointV2 &point, QgsFeatureId atFeatureId, int beforeVertex );

    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );

    bool moveVertex( const QgsPointV2 &p, QgsFeatureId atFeatureId, int atVertex ) /PyName=moveVertexV2/;

    EditResult deleteVertex( QgsFeatureId featureId, int vertex );

    bool deleteSelectedFeatures( int *deletedCount = 0 );

    int addRing( const QList<QgsPoint> &ring, QgsFeatureId *featureId = 0 );

    int addRing( QgsCurve *ring /Transfer/, QgsFeatureId *featureId = 0 ) /PyName=addCurvedRing/;

    int addPart( const QList<QgsPoint> &ring );

    int addPart( const QgsPointSequence &ring ) /PyName=addPartV2/;

    int addPart( QgsCurve *ring /Transfer/ ) /PyName=addCurvedPart/;

    int translateFeature( QgsFeatureId featureId, double dx, double dy );

    int splitParts( const QList<QgsPoint> &splitLine, bool topologicalEditing = false );

    int splitFeatures( const QList<QgsPoint> &splitLine, bool topologicalEditing = false );

    int addTopologicalPoints( const QgsGeometry &geom );

    int addTopologicalPoints( const QgsPoint &p );

    int insertSegmentVerticesForSnap( const QList<QgsSnappingResult> &snapResults );

    // const QgsAbstractVectorLayerLabeling *labeling() const { return mLabeling; } // SIP_SKIP


    // void setLabeling( QgsAbstractVectorLayerLabeling *labeling ); // SIP_SKIP


    virtual bool isEditable() const;

    virtual bool isSpatial() const;

    virtual bool isModified() const;

    bool snapPoint( QgsPoint &point, double tolerance );

    int snapWithContext( const QgsPoint &startPoint,
                         double snappingTolerance,
                         QMultiMap < double, QgsSnappingResult > &snappingResults /Out/,
                         QgsSnapper::SnappingType snap_to );

    virtual void reload();

    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) /Factory/;

    QgsRectangle extent() const;

    QgsFields fields() const;

    QgsFields pendingFields() const;

    QgsAttributeList pendingAllAttributesList() const;

    QgsAttributeList attributeList() const;

    QgsAttributeList pendingPkAttributesList() const;

    QgsAttributeList pkAttributeList() const;

    long pendingFeatureCount() const;

    long featureCount() const;

    bool setReadOnly( bool readonly = true );

    bool changeGeometry( QgsFeatureId fid, const QgsGeometry &geom );

    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );

    bool addAttribute( const QgsField &field );

    void setFieldAlias( int index, const QString &aliasString );

    void removeFieldAlias( int index );

    bool renameAttribute( int index, const QString &newName );

    QString attributeAlias( int index ) const;

    QString attributeDisplayName( int index ) const;

    QgsStringMap attributeAliases() const;

    QSet<QString> excludeAttributesWms() const;

    void setExcludeAttributesWms( const QSet<QString> &att );

    QSet<QString> excludeAttributesWfs() const;

    void setExcludeAttributesWfs( const QSet<QString> &att );

    bool deleteAttribute( int attr );

    bool deleteAttributes( QList<int> attrs );

    bool addFeatures( QgsFeatureList features, bool makeSelected = true );

    bool deleteFeature( QgsFeatureId fid );

    bool deleteFeatures( const QgsFeatureIds &fids );

    bool commitChanges();

    QStringList commitErrors() const;

    bool rollBack( bool deleteBuffer = true );

    QString annotationForm() const;

    void setAnnotationForm( const QString &ui );

    QList<QgsRelation> referencingRelations( int idx ) const;

    QgsVectorLayerEditBuffer *editBuffer();

    // const QgsVectorLayerEditBuffer *editBuffer() const { return mEditBuffer; } // SIP_SKIP


    void beginEditCommand( const QString &text );

    void endEditCommand();

    void destroyEditCommand();

    enum VertexMarkerType
    {
      SemiTransparentCircle,
      Cross,
      NoMarker
    };

    static void drawVertexMarker( double x, double y, QPainter &p, QgsVectorLayer::VertexMarkerType type, int vertexSize );

    void updateFields();

    QVariant defaultValue( int index, const QgsFeature &feature = QgsFeature(),
                           QgsExpressionContext *context = 0 ) const;

    void setDefaultValueExpression( int index, const QString &expression );

    QString defaultValueExpression( int index ) const;

    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;

    void setFieldConstraint( int index, QgsFieldConstraints::Constraint constraint, QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthHard );

    void removeFieldConstraint( int index, QgsFieldConstraints::Constraint constraint );

    QString constraintExpression( int index ) const;

    QString constraintDescription( int index ) const;

    void setConstraintExpression( int index, const QString &expression, const QString &description = QString() );

    void setEditorWidgetSetup( int index, const QgsEditorWidgetSetup &setup );

    QgsEditorWidgetSetup editorWidgetSetup( int index ) const;

    void uniqueValues( int index, QList<QVariant> &uniqueValues /Out/, int limit = -1 ) const;

    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = 0 ) const;

    QVariant minimumValue( int index ) const;

    QVariant maximumValue( int index ) const;

    QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                        const QString &fieldOrExpression,
                        const QgsAggregateCalculator::AggregateParameters &parameters = QgsAggregateCalculator::AggregateParameters(),
                        QgsExpressionContext *context = 0,
                        bool *ok = 0 ) const;

    QList< QVariant > getValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, QgsFeedback *feedback = 0 ) const;

    QList< double > getDoubleValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, int *nullCount = 0, QgsFeedback *feedback = 0 ) const;

    void setFeatureBlendMode( QPainter::CompositionMode blendMode );
    QPainter::CompositionMode featureBlendMode() const;

    void setLayerTransparency( int layerTransparency );
    int layerTransparency() const;

    QString metadata() const;

    // inline QgsGeometryCache *cache() { return mCache; } // SIP_SKIP


    void setSimplifyMethod( const QgsVectorSimplifyMethod &simplifyMethod );

    const QgsVectorSimplifyMethod &simplifyMethod() const;

    bool simplifyDrawingCanbeApplied( const QgsRenderContext &renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const;

    QgsConditionalLayerStyles *conditionalStyles() const;

    QgsAttributeTableConfig attributeTableConfig() const;

    void setAttributeTableConfig( const QgsAttributeTableConfig &attributeTableConfig );

    QString mapTipTemplate() const;

    void setMapTipTemplate( const QString &mapTipTemplate );

    QgsExpressionContext createExpressionContext() const;

    QgsEditFormConfig editFormConfig() const;

    void setEditFormConfig( const QgsEditFormConfig &editFormConfig );

  public slots:

    void select( QgsFeatureId featureId );

    void select( const QgsFeatureIds &featureIds );

    void deselect( const QgsFeatureId featureId );

    void deselect( const QgsFeatureIds &featureIds );

    void removeSelection();

    virtual void updateExtents();

    bool startEditing();


  protected slots:
    void invalidateSymbolCountedFlag();

  signals:

    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, const bool clearAndSelect );

    void layerModified();

    void beforeModifiedCheck() const;

    void beforeEditingStarted();

    void editingStarted();

    void editingStopped();

    void beforeCommitChanges();

    void beforeRollBack();

    void attributeAdded( int idx );

    void beforeAddingExpressionField( const QString &fieldName );

    void attributeDeleted( int idx );

    void beforeRemovingExpressionField( int idx );

    void featureAdded( QgsFeatureId fid );

    void featureDeleted( QgsFeatureId fid );

    void featuresDeleted( const QgsFeatureIds &fids );

    void updatedFields();


    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );

    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geometry );

    void committedAttributesDeleted( const QString &layerId, const QgsAttributeList &deletedAttributes );
    void committedAttributesAdded( const QString &layerId, const QList<QgsField> &addedAttributes );
    void committedFeaturesAdded( const QString &layerId, const QgsFeatureList &addedFeatures );
    void committedFeaturesRemoved( const QString &layerId, const QgsFeatureIds &deletedFeatureIds );
    void committedAttributeValuesChanges( const QString &layerId, const QgsChangedAttributesMap &changedAttributesValues );
    void committedGeometriesChanges( const QString &layerId, const QgsGeometryMap &changedGeometries );

    void labelingFontNotFound( QgsVectorLayer *layer, const QString &fontfamily );

    void featureBlendModeChanged( QPainter::CompositionMode blendMode );

    void layerTransparencyChanged( int layerTransparency );

    void editCommandStarted( const QString &text );

    void editCommandEnded();

    void editCommandDestroyed();

    void readCustomSymbology( const QDomElement &element, QString &errorMessage );

    void writeCustomSymbology( QDomElement &element, QDomDocument &doc, QString &errorMessage ) const;

    void mapTipTemplateChanged();

    void displayExpressionChanged();

    void raiseError( const QString &msg );

    void editFormConfigChanged();

    void readOnlyChanged();


  protected:
    void setExtent( const QgsRectangle &rect );

  private:                       // Private methods
    QgsVectorLayer( const QgsVectorLayer &rhs );
};

