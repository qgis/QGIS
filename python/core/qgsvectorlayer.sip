/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayer.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;


class QgsVectorLayer : QgsMapLayer, QgsExpressionContextGenerator, QgsExpressionContextScopeGenerator, QgsFeatureSink, QgsFeatureSource
{
%Docstring
Represents a vector layer which manages a vector based data sets.

The QgsVectorLayer is instantiated by specifying the name of a data provider,
such as postgres or wfs, and url defining the specific data set to connect to.
The vector layer constructor in turn instantiates a QgsVectorDataProvider subclass
corresponding to the provider type, and passes it the url. The data provider
connects to the data source.

The QgsVectorLayer provides a common interface to the different data types. It also
manages editing transactions.

Sample usage of the QgsVectorLayer class:

.. code-block::

         QString uri = "point?crs=epsg:4326&field=id:integer";
         QgsVectorLayer *scratchLayer = new QgsVectorLayer(uri, "Scratch point layer",  "memory");

The main data providers supported by QGIS are listed below.

\section providers Vector data providers

\subsection memory Memory data providerType (memory)

The memory data provider is used to construct in memory data, for example scratch
data or data generated from spatial operations such as contouring. There is no
inherent persistent storage of the data. The data source uri is constructed. The
url specifies the geometry type ("point", "linestring", "polygon",
"multipoint","multilinestring","multipolygon"), optionally followed by url parameters
as follows:

- crs=definition
Defines the coordinate reference system to use for the layer.
definition is any string accepted by :py:func:`QgsCoordinateReferenceSystem.createFromString()`

- index=yes
Specifies that the layer will be constructed with a spatial index

- field=name:type(length,precision)
Defines an attribute of the layer. Multiple field parameters can be added
to the data provider definition. type is one of "integer", "double", "string".

An example url is "Point?crs=epsg:4326&field=id:integer&field=name:string(20)&index=yes"

\subsection ogr OGR data provider (ogr)

Accesses data using the OGR drivers (http://www.gdal.org/ogr/ogr_formats.html). The url
is the OGR connection string. A wide variety of data formats can be accessed using this
driver, including file based formats used by many GIS systems, database formats, and
web services. Some of these formats are also supported by custom data providers listed
below.

\subsection spatialite SpatiaLite data provider (spatialite)

Access data in a SpatiaLite database. The url defines the connection parameters, table,
geometry column, and other attributes. The url can be constructed using the
QgsDataSourceUri class.

\subsection postgres PostgreSQL data provider (postgres)

Connects to a PostgreSQL database. The url defines the connection parameters, table,
geometry column, and other attributes. The url can be constructed using the
QgsDataSourceUri class.

\subsection mssql Microsoft SQL server data provider (mssql)

Connects to a Microsoft SQL server database. The url defines the connection parameters, table,
geometry column, and other attributes. The url can be constructed using the
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
any string accepted by :py:func:`QgsCoordinateReferenceSystem.createFromString()`

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

.. seealso:: :py:class:`QgsVectorLayerUtils`
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

    struct LayerOptions
    {

      explicit LayerOptions( bool loadDefaultStyle = true, bool readExtentFromXml = false );
%Docstring
Constructor for LayerOptions.
%End

      bool loadDefaultStyle;

      bool readExtentFromXml;

    };

    explicit QgsVectorLayer( const QString &path = QString(), const QString &baseName = QString(),
                             const QString &providerLib = "ogr", const QgsVectorLayer::LayerOptions &options = QgsVectorLayer::LayerOptions() );
%Docstring
Constructor - creates a vector layer

The QgsVectorLayer is constructed by instantiating a data provider.  The provider
interprets the supplied path (url) of the data source to connect to and access the
data.

:param path:  The path or url of the parameter.  Typically this encodes
parameters used by the data provider as url query items.
:param baseName: The name used to represent the layer in the legend
:param providerLib:  The name of the data provider, e.g., "memory", "postgres"
:param options: layer load options
%End


    ~QgsVectorLayer();


    virtual QgsVectorLayer *clone() const /Factory/;

%Docstring
Returns a new instance equivalent to this one. A new provider is
created for the same data source and renderers for features and diagrams
are cloned too. Moreover, each attributes (transparency, extent, selected
features and so on) are identicals.

:return: a new layer instance

.. versionadded:: 3.0
%End

    QString storageType() const;
%Docstring
Returns the permanent storage type for this layer as a friendly name.
This is obtained from the data provider and does not follow any standard.
%End

    QString capabilitiesString() const;
%Docstring
Capabilities for this layer, comma separated and translated.
%End

    QString dataComment() const;
%Docstring
Returns a description for this layer as defined in the data provider.
%End

    QString displayField() const;
%Docstring
This is a shorthand for accessing the displayExpression if it is a simple field.
If the displayExpression is more complex than a simple field, a null string will
be returned.

.. seealso:: :py:func:`displayExpression`
%End

    void setDisplayExpression( const QString &displayExpression );
%Docstring
Set the preview expression, used to create a human readable preview string.
Used e.g. in the attribute table feature list. Uses :py:class:`QgsExpression`.

:param displayExpression: The expression which will be used to preview features
for this layer
%End

    QString displayExpression() const;
%Docstring
Get the preview expression, used to create a human readable preview string.
Uses :py:class:`QgsExpression`

:return: The expression which will be used to preview features for this layer
%End

    virtual QgsVectorDataProvider *dataProvider();


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

:param joinInfo: join object containing join layer id, target and source field

.. note::

   since 2.6 returns bool indicating whether the join can be added *
%End

    bool removeJoin( const QString &joinLayerId );
%Docstring
Removes a vector layer join

:return: true if join was found and successfully removed *
%End

    QgsVectorLayerJoinBuffer *joinBuffer();
%Docstring
Accessor to the join buffer object

.. versionadded:: 2.14.7
%End
    const QList<QgsVectorLayerJoinInfo> vectorJoins() const;

    virtual bool setDependencies( const QSet<QgsMapLayerDependency> &layers );

%Docstring
Sets the list of dependencies.

.. seealso:: :py:func:`dependencies`

:param layers: set of :py:class:`QgsMapLayerDependency`. Only user-defined dependencies will be added

:return: false if a dependency cycle has been detected

.. versionadded:: 3.0
%End

    virtual QSet<QgsMapLayerDependency> dependencies() const;

%Docstring
Gets the list of dependencies. This includes data dependencies set by the user (:py:func:`setDataDependencies`)
as well as dependencies given by the provider

:return: a set of :py:class:`QgsMapLayerDependency`

.. versionadded:: 3.0
%End

    int addExpressionField( const QString &exp, const QgsField &fld );
%Docstring
Add a new field which is calculated by the expression specified

:param exp: The expression which calculates the field
:param fld: The field to calculate

:return: The index of the new field


.. versionadded:: 2.9
%End

    void removeExpressionField( int index );
%Docstring
Remove an expression field

:param index: The index of the field

.. versionadded:: 2.6
%End

    QString expressionField( int index ) const;
%Docstring
Returns the expression used for a given expression field

:param index: An index of an epxression based (virtual) field

:return: The expression for the field at index


.. versionadded:: 2.9
%End

    void updateExpressionField( int index, const QString &exp );
%Docstring
Changes the expression used to define an expression based (virtual) field

:param index: The index of the expression to change

:param exp: The new expression to set

.. versionadded:: 2.9
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

:return: See description
%End

    void selectByRect( QgsRectangle &rect, SelectBehavior behavior = SetSelection );
%Docstring
Select features found within the search rectangle (in layer's coordinates)

:param rect: search rectangle
:param behavior: selection type, allows adding to current selection, removing
from selection, etc.

.. seealso:: :py:func:`invertSelectionInRectangle`

.. seealso:: :py:func:`selectByExpression`

.. seealso:: :py:func:`selectByIds`
%End

    void selectByExpression( const QString &expression, SelectBehavior behavior = SetSelection );
%Docstring
Select matching features using an expression.

:param expression: expression to evaluate to select features
:param behavior: selection type, allows adding to current selection, removing
from selection, etc.

.. versionadded:: 2.16

.. seealso:: :py:func:`selectByRect`

.. seealso:: :py:func:`selectByIds`
%End

    void selectByIds( const QgsFeatureIds &ids, SelectBehavior behavior = SetSelection );
%Docstring
Select matching features using a list of feature IDs. Will emit the
selectionChanged() signal with the clearAndSelect flag set.

:param ids: feature IDs to select
:param behavior: selection type, allows adding to current selection, removing
from selection, etc.

.. versionadded:: 2.16

.. seealso:: :py:func:`selectByRect`

.. seealso:: :py:func:`selectByExpression`
%End

    void modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds );
%Docstring
Modifies the current selection on this layer

:param selectIds:    Select these ids
:param deselectIds:  Deselect these ids

.. seealso:: :py:func:`select`

.. seealso:: :py:func:`select`

.. seealso:: :py:func:`deselect`

.. seealso:: :py:func:`deselect`

.. seealso:: :py:func:`selectByExpression`
%End

    void invertSelection();
%Docstring
Select not selected features and deselect selected ones
%End

    void selectAll();
%Docstring
Select all the features
%End

    void invertSelectionInRectangle( QgsRectangle &rect );
%Docstring
Invert selection of features found within the search rectangle (in layer's coordinates)

:param rect:  The rectangle in which the selection of features will be inverted

.. seealso:: :py:func:`invertSelection`
%End

    QgsFeatureList selectedFeatures() const;
%Docstring
Get a copy of the user-selected features

:return: A list of :py:class:`QgsFeature`

.. seealso:: :py:func:`selectedFeatureIds`

.. seealso:: :py:func:`getSelectedFeatures`
%End

    QgsFeatureIterator getSelectedFeatures( QgsFeatureRequest request = QgsFeatureRequest() ) const;
%Docstring
Get an iterator of the selected features

:param request: You may specify a request, e.g. to limit the set of requested attributes.
Any filter on the request will be discarded.

:return: Iterator over the selected features


.. seealso:: :py:func:`selectedFeatureIds`

.. seealso:: :py:func:`selectedFeatures`
%End

    const QgsFeatureIds &selectedFeatureIds() const;
%Docstring
Return reference to identifiers of selected features

:return: A list of :py:class:`QgsFeatureId`

.. seealso:: :py:func:`selectedFeatures`
%End

    QgsRectangle boundingBoxOfSelected() const;
%Docstring
Returns the bounding box of the selected features. If there is no selection, QgsRectangle(0,0,0,0) is returned
%End

    bool labelsEnabled() const;
%Docstring
Returns whether the layer contains labels which are enabled and should be drawn.

:return: true if layer contains enabled labels

.. versionadded:: 2.9
%End

    bool diagramsEnabled() const;
%Docstring
Returns whether the layer contains diagrams which are enabled and should be drawn.

:return: true if layer contains enabled diagrams

.. versionadded:: 2.9
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


    void setRenderer( QgsFeatureRenderer *r /Transfer/ );
%Docstring
Set renderer which will be invoked to represent this layer.
Ownership is transferred.
%End

    QgsWkbTypes::GeometryType geometryType() const;
%Docstring
Returns point, line or polygon
%End

    virtual QgsWkbTypes::Type wkbType() const;

%Docstring
Returns the WKBType or WKBUnknown in case of error
%End

    QString providerType() const;
%Docstring
Return the provider type for this layer
%End

    virtual QgsCoordinateReferenceSystem sourceCrs() const;

    virtual QString sourceName() const;


    virtual bool readXml( const QDomNode &layer_node, const QgsReadWriteContext &context );

%Docstring
Reads vector layer specific state from project file Dom node.

.. note::

   Called by :py:func:`QgsMapLayer.readXml()`
%End

    virtual bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const;

%Docstring
Write vector layer specific state to project file Dom node.

.. note::

   Called by :py:func:`QgsMapLayer.writeXml()`
%End

    virtual void resolveReferences( QgsProject *project );

%Docstring
Resolve references to other layers (kept as layer IDs after reading XML) into layer objects.

.. versionadded:: 3.0
%End

    virtual void saveStyleToDatabase( const QString &name, const QString &description,
                                      bool useAsDefault, const QString &uiFileContent,
                                      QString &msgError /Out/ );
%Docstring
Save named and sld style of the layer to the style table in the db.

:param name:
:param description:
:param useAsDefault:
:param uiFileContent:
:param msgError:
%End

    virtual int listStylesInDatabase( QStringList &ids /Out/, QStringList &names /Out/,
                                      QStringList &descriptions /Out/, QString &msgError /Out/ );
%Docstring
Lists all the style in db split into related to the layer and not related to

:param ids: the list in which will be stored the style db ids
:param names: the list in which will be stored the style names
:param descriptions: the list in which will be stored the style descriptions
:param msgError:

:return: the number of styles related to current layer
%End

    virtual QString getStyleFromDatabase( const QString &styleId, QString &msgError /Out/ );
%Docstring
Will return the named style corresponding to style id provided
%End

    virtual bool deleteStyleFromDatabase( const QString &styleId, QString &msgError /Out/ );
%Docstring
Delete a style from the database

:param styleId: the provider's layer_styles table id of the style to delete
:param msgError: reference to string that will be updated with any error messages

:return: true in case of success

.. versionadded:: 3.0
%End

    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag /Out/, bool loadFromLocalDb );
%Docstring
Load a named style from file/local db/datasource db

:param theURI: the URI of the style or the URI of the layer
:param resultFlag: will be set to true if a named style is correctly loaded
:param loadFromLocalDb: if true forces to load from local db instead of datasource one
%End

    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag /Out/ );

%Docstring
Calls loadNamedStyle( theURI, resultFlag, false );
Retained for backward compatibility
%End

    bool loadAuxiliaryLayer( const QgsAuxiliaryStorage &storage, const QString &key = QString() );
%Docstring
Loads the auxiliary layer for this vector layer. If there's no
corresponding table in the database, then nothing happens and false is
returned. The key is optional because if this layer has been read from
a XML document, then the key read in this document is used by default.

:param storage: The auxiliary storage where to look for the table
:param key: The key to use for joining.

:return: true if the auxiliary layer is well loaded, false otherwise


.. versionadded:: 3.0
%End

    void setAuxiliaryLayer( QgsAuxiliaryLayer *layer /Transfer/ = 0 );
%Docstring
Sets the current auxiliary layer. The auxiliary layer is automatically
put in editable mode and fields are updated. Moreover, a join is created
between the current layer and the auxiliary layer. Ownership is
transferred.

.. versionadded:: 3.0
%End

    QgsAuxiliaryLayer *auxiliaryLayer();
%Docstring
Returns the current auxiliary layer.

.. versionadded:: 3.0
%End


    virtual bool readSymbology( const QDomNode &layerNode, QString &errorMessage, const QgsReadWriteContext &context );

%Docstring
Read the symbology for the current layer from the Dom node supplied.

:param layerNode: node that will contain the symbology definition for this layer.
:param errorMessage: reference to string that will be updated with any error messages
:param context: reading context (used for transform from relative to absolute paths)

:return: true in case of success.
%End

    virtual bool readStyle( const QDomNode &node, QString &errorMessage, const QgsReadWriteContext &context );

%Docstring
Read the style for the current layer from the Dom node supplied.

:param node: node that will contain the style definition for this layer.
:param errorMessage: reference to string that will be updated with any error messages
:param context: reading context (used for transform from relative to absolute paths)

:return: true in case of success.
%End

    virtual bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const;

%Docstring
Write the symbology for the layer into the docment provided.

:param node: the node that will have the style element added to it.
:param doc: the document that will have the QDomNode added.
:param errorMessage: reference to string that will be updated with any error messages
:param context: writing context (used for transform from absolute to relative paths)

:return: true in case of success.
%End

    virtual bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const;

%Docstring
Write just the style information for the layer into the document

:param node: the node that will have the style element added to it.
:param doc: the document that will have the QDomNode added.
:param errorMessage: reference to string that will be updated with any error messages
:param context: writing context (used for transform from absolute to relative paths)

:return: true in case of success.
%End

    bool writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsStringMap &props = QgsStringMap() ) const;
%Docstring
Writes the symbology of the layer into the document provided in SLD 1.1 format

:param node: the node that will have the style element added to it.
:param doc: the document that will have the QDomNode added.
:param errorMessage: reference to string that will be updated with any error messages
:param props: a open ended set of properties that can drive/inform the SLD encoding

:return: true in case of success
%End

    virtual bool readSld( const QDomNode &node, QString &errorMessage );


    long featureCount( const QString &legendKey ) const;
%Docstring
Number of features rendered with specified legend key. Features must be first
calculated by countSymbolFeatures()

:return: number of features rendered by symbol or -1 if failed or counts are not available
%End

    void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, bool loadDefaultStyleFlag = false );
%Docstring
Update the data source of the layer. The layer's renderer and legend will be preserved only
if the geometry type of the new data source matches the current geometry type of the layer.

:param dataSource: new layer data source
:param baseName: base name of the layer
:param provider: provider string
:param loadDefaultStyleFlag: set to true to reset the layer's style to the default for the
data source

.. versionadded:: 2.10
%End

    QgsVectorLayerFeatureCounter *countSymbolFeatures();
%Docstring
Count features for symbols.
The method will return the feature counter task. You will need to
connect to the symbolFeatureCountMapChanged() signal to be
notified when the freshly updated feature counts are ready.

.. note::

   If the count features for symbols has been already done a
None is returned. If you need to wait for the results,
you can call waitForFinished() on the feature counter.

.. versionadded:: 3.0
%End

    virtual bool setSubsetString( const QString &subset );
%Docstring
Set the string (typically sql) used to define a subset of the layer

:param subset: The subset string. This may be the where clause of a sql statement
or other definition string specific to the underlying dataprovider
and data store.

:return: true, when setting the subset string was successful, false otherwise
%End

    virtual QString subsetString() const;
%Docstring
Get the string (typically sql) used to define a subset of the layer

:return: The subset string or null QString if not implemented by the provider
%End

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const;

%Docstring
Query the layer for features specified in request.

:param request: feature request describing parameters of features to return

:return: iterator for matching features from provider
%End

    QgsFeatureIterator getFeatures( const QString &expression );
%Docstring
Query the layer for features matching a given expression.
%End

    QgsFeature getFeature( QgsFeatureId fid ) const;
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

    virtual bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = 0 );


    bool updateFeature( const QgsFeature &feature, bool skipDefaultValues = false );
%Docstring
Updates an existing ``feature`` in the layer, replacing the attributes and geometry for the feature
with matching QgsFeature.id() with the attributes and geometry from ``feature``.
Changes are not immediately committed to the layer.

If ``skipDefaultValue`` is set to true, default field values will not
be updated. This can be used to override default field value expressions.

Returns true if the feature's attribute was successfully changed.

.. note::

   Calls to updateFeature() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().

.. warning::

   This method needs to query the underlying data provider to fetch the feature
with matching QgsFeature.id() on every call. Depending on the underlying data source this
can be slow to execute. Consider using the more efficient changeAttributeValue() or
changeGeometry() methods instead.


.. seealso:: :py:func:`startEditing`

.. seealso:: :py:func:`commitChanges`

.. seealso:: :py:func:`changeGeometry`

.. seealso:: :py:func:`changeAttributeValue`
%End

    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );
%Docstring
Insert a new vertex before the given vertex number,
in the given ring, item (first number is index 0), and feature
Not meaningful for Point geometries

.. note::

   Calls to insertVertex() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    bool insertVertex( const QgsPoint &point, QgsFeatureId atFeatureId, int beforeVertex );
%Docstring
Insert a new vertex before the given vertex number,
in the given ring, item (first number is index 0), and feature
Not meaningful for Point geometries

.. note::

   Calls to insertVertex() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );
%Docstring
Moves the vertex at the given position number,
ring and item (first number is index 0), and feature
to the given coordinates

.. note::

   Calls to moveVertex() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    bool moveVertex( const QgsPoint &p, QgsFeatureId atFeatureId, int atVertex ) /PyName=moveVertexV2/;
%Docstring
Moves the vertex at the given position number,
ring and item (first number is index 0), and feature
to the given coordinates

.. note::

   available in Python as moveVertexV2

.. note::

   Calls to moveVertex() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    EditResult deleteVertex( QgsFeatureId featureId, int vertex );
%Docstring
Deletes a vertex from a feature.

:param featureId: ID of feature to remove vertex from
:param vertex: index of vertex to delete

.. versionadded:: 2.14

.. note::

   Calls to deleteVertex() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    bool deleteSelectedFeatures( int *deletedCount = 0 );
%Docstring
Deletes the selected features

:return: true in case of success and false otherwise
%End

    QgsGeometry::OperationResult addRing( const QVector<QgsPointXY> &ring, QgsFeatureId *featureId = 0 );
%Docstring
Adds a ring to polygon/multipolygon features

:param ring: ring to add
:param featureId: if specified, feature ID for feature ring was added to will be stored in this parameter

:return: QgsGeometry.OperationResult
- Success
- LayerNotEditable
- AddRingNotInExistingFeature
- InvalidInputGeometryType
- AddRingNotClosed
- AddRingNotValid
- AddRingCrossesExistingRings

.. note::

   Calls to addRing() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    QgsGeometry::OperationResult addRing( QgsCurve *ring /Transfer/, QgsFeatureId *featureId = 0 ) /PyName=addCurvedRing/;
%Docstring
Adds a ring to polygon/multipolygon features (takes ownership)

:param ring: ring to add
:param featureId: if specified, feature ID for feature ring was added to will be stored in this parameter

:return: QgsGeometry.OperationResult
- Success
- LayerNotEditable
- AddRingNotInExistingFeature
- InvalidInputGeometryType
- AddRingNotClosed
- AddRingNotValid
- AddRingCrossesExistingRings

.. note::

   available in Python as addCurvedRing

.. note::

   Calls to addRing() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    QgsGeometry::OperationResult addPart( const QList<QgsPointXY> &ring );
%Docstring
Adds a new part polygon to a multipart feature

:return: QgsGeometry.OperationResult
- Success
- LayerNotEditable
- SelectionIsEmpty
- SelectionIsGreaterThanOne
- AddPartSelectedGeometryNotFound
- AddPartNotMultiGeometry
- InvalidBaseGeometry
- InvalidInputGeometryType

.. note::

   Calls to addPart() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    QgsGeometry::OperationResult addPart( const QgsPointSequence &ring ) /PyName=addPartV2/;
%Docstring
Adds a new part polygon to a multipart feature

:return: QgsGeometry.OperationResult
- Success
- LayerNotEditable
- SelectionIsEmpty
- SelectionIsGreaterThanOne
- AddPartSelectedGeometryNotFound
- AddPartNotMultiGeometry
- InvalidBaseGeometry
- InvalidInputGeometryType

.. note::

   available in Python bindings as addPartV2

.. note::

   Calls to addPart() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    QgsGeometry::OperationResult addPart( QgsCurve *ring /Transfer/ ) /PyName=addCurvedPart/;
%Docstring

.. note::

   available in Python as addCurvedPart

.. note::

   Calls to addPart() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    int translateFeature( QgsFeatureId featureId, double dx, double dy );
%Docstring
Translates feature by dx, dy

:param featureId: id of the feature to translate
:param dx: translation of x-coordinate
:param dy: translation of y-coordinate

:return: 0 in case of success

.. note::

   Calls to translateFeature() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    QgsGeometry::OperationResult splitParts( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false );
%Docstring
Splits parts cut by the given line

:param splitLine: line that splits the layer features
:param topologicalEditing: true if topological editing is enabled

:return: QgsGeometry.OperationResult
- Success
- NothingHappened
- LayerNotEditable
- InvalidInputGeometryType
- InvalidBaseGeometry
- GeometryEngineError
- SplitCannotSplitPoint

.. note::

   Calls to splitParts() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    QgsGeometry::OperationResult splitFeatures( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false );
%Docstring
Splits features cut by the given line

:param splitLine: line that splits the layer features
:param topologicalEditing: true if topological editing is enabled

:return: QgsGeometry.OperationResult
- Success
- NothingHappened
- LayerNotEditable
- InvalidInputGeometryType
- InvalidBaseGeometry
- GeometryEngineError
- SplitCannotSplitPoint

.. note::

   Calls to splitFeatures() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    int addTopologicalPoints( const QgsGeometry &geom );
%Docstring
Adds topological points for every vertex of the geometry.

:param geom: the geometry where each vertex is added to segments of other features

:return: 0 in case of success

.. note::

   geom is not going to be modified by the function

.. note::

   Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    int addTopologicalPoints( const QgsPointXY &p );
%Docstring
Adds a vertex to segments which intersect point p but don't
already have a vertex there. If a feature already has a vertex at position p,
no additional vertex is inserted. This method is useful for topological
editing.

:param p: position of the vertex

:return: 0 in case of success

.. note::

   Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End


    QgsAbstractVectorLayerLabeling *labeling();
%Docstring
Access to labeling configuration. May be null if labeling is not used.

.. versionadded:: 3.0
%End

    void setLabeling( QgsAbstractVectorLayerLabeling *labeling /Transfer/ );
%Docstring
Set labeling configuration. Takes ownership of the object.

.. versionadded:: 3.0
%End

    virtual bool isEditable() const;

%Docstring
Returns true if the provider is in editing mode
%End

    virtual bool isSpatial() const;

%Docstring
Returns true if this is a geometry layer and false in case of NoGeometry (table only) or UnknownGeometry
%End

    virtual bool isModified() const;
%Docstring
Returns true if the provider has been modified since the last commit
%End

    bool isAuxiliaryField( int index, int &srcIndex ) const;
%Docstring
Returns true if the field comes from the auxiliary layer,
false otherwise.

.. versionadded:: 3.0
%End

    virtual void reload();

%Docstring
Synchronises with changes in the datasource
%End

    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) /Factory/;

%Docstring
Return new instance of QgsMapLayerRenderer that will be used for rendering of given context

.. versionadded:: 2.4
%End

    virtual QgsRectangle extent() const;

    virtual QgsRectangle sourceExtent() const;


    virtual QgsFields fields() const;
%Docstring
Returns the list of fields of this layer.
This also includes fields which have not yet been saved to the provider.

:return: A list of fields
%End

    QgsFields pendingFields() const;
%Docstring
Returns the list of fields of this layer.
This also includes fields which have not yet been saved to the provider.
Alias for fields()

:return: A list of fields
%End

    QgsAttributeList pendingAllAttributesList() const;
%Docstring
Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
Alias for attributeList()
%End

    QgsAttributeList attributeList() const;
%Docstring
Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
Alias for attributeList()
%End

    QgsAttributeList pendingPkAttributesList() const;
%Docstring
Returns list of attributes making up the primary key
Alias for pkAttributeList()
%End

    QgsAttributeList pkAttributeList() const;
%Docstring
Returns list of attributes making up the primary key
%End

    long pendingFeatureCount() const;
%Docstring
Returns feature count including changes which have not yet been committed
Alias for featureCount()
%End

    virtual long featureCount() const;

%Docstring
Returns feature count including changes which have not yet been committed
If you need only the count of committed features call this method on this layer's provider.
%End

    bool setReadOnly( bool readonly = true );
%Docstring
Make layer read-only (editing disabled) or not

:return: false if the layer is in editing yet
%End

    bool changeGeometry( QgsFeatureId fid, const QgsGeometry &geometry, bool skipDefaultValue = false );
%Docstring
Changes a feature's ``geometry`` within the layer's edit buffer
(but does not immediately commit the changes). The ``fid`` argument
specifies the ID of the feature to be changed.

If ``skipDefaultValue`` is set to true, default field values will not
be updated. This can be used to override default field value expressions.

Returns true if the feature's geometry was successfully changed.

.. note::

   Calls to changeGeometry() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().

.. seealso:: :py:func:`startEditing`

.. seealso:: :py:func:`commitChanges`

.. seealso:: :py:func:`changeAttributeValue`

.. seealso:: :py:func:`updateFeature`
%End

    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant(), bool skipDefaultValues = false );
%Docstring
Changes an attribute value for a feature (but does not immediately commit the changes).
The ``fid`` argument specifies the ID of the feature to be changed.

The ``field`` argument must specify a valid field index for the layer (where an index of 0
corresponds to the first field).

The new value to be assigned to the field is given by ``newValue``.

If a valid QVariant is specified for ``oldValue``, it will be used as the field value in the
case of an undo operation corresponding to this attribute value change. If an invalid
QVariant is used (the default behavior), then the feature's current value will be automatically
retrieved and used. Note that this involves a feature request to the underlying data provider,
so it is more efficient to explicitly pass an ``oldValue`` if it is already available.

If ``skipDefaultValue`` is set to true, default field values will not
be updated. This can be used to override default field value expressions.

Returns true if the feature's attribute was successfully changed.

.. note::

   Calls to changeAttributeValue() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().

.. seealso:: :py:func:`startEditing`

.. seealso:: :py:func:`commitChanges`

.. seealso:: :py:func:`changeGeometry`

.. seealso:: :py:func:`updateFeature`
%End

    bool addAttribute( const QgsField &field );
%Docstring
Add an attribute field (but does not commit it)
returns true if the field was added

.. note::

   Calls to addAttribute() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    void setFieldAlias( int index, const QString &aliasString );
%Docstring
Sets an alias (a display name) for attributes to display in dialogs

.. versionadded:: 3.0
%End

    void removeFieldAlias( int index );
%Docstring
Removes an alias (a display name) for attributes to display in dialogs

.. versionadded:: 3.0
%End

    bool renameAttribute( int index, const QString &newName );
%Docstring
Renames an attribute field  (but does not commit it).

:param index: attribute index
:param newName: new name of field

.. versionadded:: 2.16

.. note::

   Calls to renameAttribute() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    QString attributeAlias( int index ) const;
%Docstring
Returns the alias of an attribute name or a null string if there is no alias.

\see {attributeDisplayName( int attributeIndex )} which returns the field name
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

    virtual bool deleteAttribute( int attr );
%Docstring
Deletes an attribute field (but does not commit it).

.. note::

   Calls to deleteAttribute() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    bool deleteAttributes( QList<int> attrs );
%Docstring
Deletes a list of attribute fields (but does not commit it)

\param  attrs the indices of the attributes to delete

:return: true if at least one attribute has been deleted
%End

    virtual bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = 0 );


    bool deleteFeature( QgsFeatureId fid );
%Docstring
Deletes a feature from the layer (but does not commit it).

.. note::

   Calls to deleteFeature() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    bool deleteFeatures( const QgsFeatureIds &fids );
%Docstring
Deletes a set of features from the layer (but does not commit it)

:param fids: The feature ids to delete

:return: false if the layer is not in edit mode or does not support deleting
in case of an active transaction depends on the provider implementation

.. note::

   Calls to deleteFeatures() are only valid for layers in which edits have been enabled
by a call to startEditing(). Changes made to features using this method are not committed
to the underlying data provider until a commitChanges() call is made. Any uncommitted
changes can be discarded by calling rollBack().
%End

    bool commitChanges();
%Docstring
Attempts to commit to the underlying data provider any buffered changes made since the
last to call to startEditing().

Returns the result of the attempt. If a commit fails (i.e. false is returned), the
in-memory changes are left untouched and are not discarded. This allows editing to
continue if the commit failed on e.g. a disallowed value in a Postgres
database - the user can re-edit and try again.

The commits occur in distinct stages,
(add attributes, add features, change attribute values, change
geometries, delete features, delete attributes)
so if a stage fails, it can be difficult to roll back cleanly.
Therefore any error message returned by commitErrors() also includes which stage failed so
that the user has some chance of repairing the damage cleanly.

.. seealso:: :py:func:`startEditing`

.. seealso:: :py:func:`commitErrors`

.. seealso:: :py:func:`rollBack`
%End

    QStringList commitErrors() const;
%Docstring
Returns a list containing any error messages generated when attempting
to commit changes to the layer.

.. seealso:: :py:func:`commitChanges`
%End

    bool rollBack( bool deleteBuffer = true );
%Docstring
Stops a current editing operation and discards any uncommitted edits.

If ``deleteBuffer`` is true the editing buffer will be completely deleted (the default
behavior).

.. seealso:: :py:func:`startEditing`

.. seealso:: :py:func:`commitChanges`
%End

    QList<QgsRelation> referencingRelations( int idx ) const;
%Docstring
Get relations, where the foreign key is on this layer

:param idx: Only get relations, where idx forms part of the foreign key

:return: A list of relations
%End

    QgsVectorLayerEditBuffer *editBuffer();
%Docstring
Buffer with uncommitted editing operations. Only valid after editing has been turned on.
%End


    void beginEditCommand( const QString &text );
%Docstring
Create edit command for undo/redo operations

:param text: text which is to be displayed in undo window
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
Will regenerate the `fields` property of this layer by obtaining all fields
from the dataProvider, joined fields and virtual fields. It will also
take any changes made to default values into consideration.

.. note::

   Unless the fields on the provider have directly been modified, there is
no reason to call this method.
%End

    QVariant defaultValue( int index, const QgsFeature &feature = QgsFeature(),
                           QgsExpressionContext *context = 0 ) const;
%Docstring
Returns the calculated default value for the specified field index. The default
value may be taken from a client side default value expression (see setDefaultValueDefinition())
or taken from the underlying data provider.

:param index: field index
:param feature: optional feature to use for default value evaluation. If passed,
then properties from the feature (such as geometry) can be used when calculating
the default value.
:param context: optional expression context to evaluate expressions again. If not
specified, a default context will be created

:return: calculated default value

.. versionadded:: 3.0

.. seealso:: :py:func:`setDefaultValueDefinition`
%End

    void setDefaultValueDefinition( int index, const QgsDefaultValue &definition );
%Docstring
Sets the definition of the expression to use when calculating the default value for a field.

:param index: field index
:param definition: default value definition to use and evaluate
when calculating default values for field. Pass
an empty expression to clear the default.

.. versionadded:: 3.0

.. seealso:: :py:func:`defaultValue`

.. seealso:: :py:func:`defaultValueDefinition`
%End

    QgsDefaultValue defaultValueDefinition( int index ) const;
%Docstring
Returns the definition of the expression used when calculating the default value for a field.

:param index: field index

:return: definition of the default value with the expression evaluated
when calculating default values for field, or definition with an
empty string if no default is set

.. versionadded:: 3.0

.. seealso:: :py:func:`defaultValue`

.. seealso:: :py:func:`setDefaultValueDefinition`
%End

    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;
%Docstring
Returns any constraints which are present for a specified
field index. These constraints may be inherited from the layer's data provider
or may be set manually on the vector layer from within QGIS.

.. versionadded:: 3.0

.. seealso:: :py:func:`setFieldConstraint`
%End

    QMap< QgsFieldConstraints::Constraint, QgsFieldConstraints::ConstraintStrength> fieldConstraintsAndStrength( int fieldIndex ) const;
%Docstring
Returns a map of constraint with their strength for a specific field of the layer.

:param fieldIndex: field index

.. versionadded:: 3.0
%End

    void setFieldConstraint( int index, QgsFieldConstraints::Constraint constraint, QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthHard );
%Docstring
Sets a constraint for a specified field index. Any constraints inherited from the layer's
data provider will be kept intact and cannot be modified. Ie, calling this method only allows for new
constraints to be added on top of the existing provider constraints.

.. versionadded:: 3.0

.. seealso:: :py:func:`fieldConstraints`

.. seealso:: :py:func:`removeFieldConstraint`
%End

    void removeFieldConstraint( int index, QgsFieldConstraints::Constraint constraint );
%Docstring
Removes a constraint for a specified field index. Any constraints inherited from the layer's
data provider will be kept intact and cannot be removed.

.. versionadded:: 3.0

.. seealso:: :py:func:`fieldConstraints`

.. seealso:: :py:func:`setFieldConstraint`
%End

    QString constraintExpression( int index ) const;
%Docstring
Returns the constraint expression for for a specified field index, if set.

.. versionadded:: 3.0

.. seealso:: :py:func:`fieldConstraints`

.. seealso:: :py:func:`constraintDescription`

.. seealso:: :py:func:`setConstraintExpression`
%End

    QString constraintDescription( int index ) const;
%Docstring
Returns the descriptive name for the constraint expression for a specified field index.

.. versionadded:: 3.0

.. seealso:: :py:func:`constraints`

.. seealso:: :py:func:`constraintExpression`

.. seealso:: :py:func:`setConstraintExpression`
%End

    void setConstraintExpression( int index, const QString &expression, const QString &description = QString() );
%Docstring
Set the constraint expression for the specified field index. An optional descriptive name for the constraint
can also be set. Setting an empty expression will clear any existing expression constraint.

.. versionadded:: 3.0

.. seealso:: :py:func:`constraintExpression`

.. seealso:: :py:func:`constraintDescription`

.. seealso:: :py:func:`constraints`
%End

    void setEditorWidgetSetup( int index, const QgsEditorWidgetSetup &setup );
%Docstring
\copydoc editorWidgetSetup
%End

    QgsEditorWidgetSetup editorWidgetSetup( int index ) const;
%Docstring
The editor widget setup defines which QgsFieldFormatter and editor widget will be used
for the field at `index`.

.. versionadded:: 3.0
%End

    virtual QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const;

%Docstring
Calculates a list of unique values contained within an attribute in the layer. Note that
in some circumstances when unsaved changes are present for the layer then the returned list
may contain outdated values (for instance when the attribute value in a saved feature has
been changed inside the edit buffer then the previous saved value will be included in the
returned list).

:param fieldIndex: column index for attribute
:param limit: maximum number of values to return (or -1 if unlimited)

.. seealso:: :py:func:`minimumValue`

.. seealso:: :py:func:`maximumValue`
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

:param index: column index for attribute
:param substring: substring to match (case insensitive)
:param limit: maxmum number of the values to return, or -1 to return all unique values
:param feedback: optional feedback object for canceling request

:return: list of unique strings containing substring
%End

    virtual QVariant minimumValue( int index ) const;

%Docstring
Returns the minimum value for an attribute column or an invalid variant in case of error.
Note that in some circumstances when unsaved changes are present for the layer then the
returned value may be outdated (for instance when the attribute value in a saved feature has
been changed inside the edit buffer then the previous saved value may be returned as the minimum).

.. seealso:: :py:func:`maximumValue`

.. seealso:: :py:func:`uniqueValues`
%End

    virtual QVariant maximumValue( int index ) const;

%Docstring
Returns the maximum value for an attribute column or an invalid variant in case of error.
Note that in some circumstances when unsaved changes are present for the layer then the
returned value may be outdated (for instance when the attribute value in a saved feature has
been changed inside the edit buffer then the previous saved value may be returned as the maximum).

.. seealso:: :py:func:`minimumValue`

.. seealso:: :py:func:`uniqueValues`
%End

    QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                        const QString &fieldOrExpression,
                        const QgsAggregateCalculator::AggregateParameters &parameters = QgsAggregateCalculator::AggregateParameters(),
                        QgsExpressionContext *context = 0,
                        bool *ok = 0 ) const;
%Docstring
Calculates an aggregated value from the layer's features.

:param aggregate: aggregate to calculate
:param fieldOrExpression: source field or expression to use as basis for aggregated values.
:param parameters: parameters controlling aggregate calculation
:param context: expression context for expressions and filters
:param ok: if specified, will be set to true if aggregate calculation was successful

:return: calculated aggregate value

.. versionadded:: 2.16
%End

    QList< QVariant > getValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, QgsFeedback *feedback = 0 ) const;
%Docstring
Fetches all values from a specified field name or expression.

:param fieldOrExpression: field name or an expression string
:param ok: will be set to false if field or expression is invalid, otherwise true
:param selectedOnly: set to true to get values from selected features only
:param feedback: optional feedback object to allow cancelation

:return: list of fetched values

.. versionadded:: 2.9

.. seealso:: :py:func:`getDoubleValues`
%End

    QList< double > getDoubleValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, int *nullCount = 0, QgsFeedback *feedback = 0 ) const;
%Docstring
Fetches all double values from a specified field name or expression. Null values or
invalid expression results are skipped.

:param fieldOrExpression: field name or an expression string evaluating to a double value
:param ok: will be set to false if field or expression is invalid, otherwise true
:param selectedOnly: set to true to get values from selected features only
:param nullCount: optional pointer to integer to store number of null values encountered in
:param feedback: optional feedback object to allow cancelation

:return: list of fetched values

.. versionadded:: 2.9

.. seealso:: :py:func:`getValues`
%End

    void setFeatureBlendMode( QPainter::CompositionMode blendMode );
%Docstring
Set the blending mode used for rendering each feature
%End
    QPainter::CompositionMode featureBlendMode() const;
%Docstring
Returns the current blending mode for features
%End

    void setOpacity( double opacity );
%Docstring
Sets the ``opacity`` for the vector layer, where ``opacity`` is a value between 0 (totally transparent)
and 1.0 (fully opaque).

.. seealso:: :py:func:`opacity`

.. seealso:: :py:func:`opacityChanged`

.. versionadded:: 3.0
%End

    double opacity() const;
%Docstring
Returns the opacity for the vector layer, where opacity is a value between 0 (totally transparent)
and 1.0 (fully opaque).

.. seealso:: :py:func:`setOpacity`

.. seealso:: :py:func:`opacityChanged`

.. versionadded:: 3.0
%End

    virtual QString htmlMetadata() const;


    void setSimplifyMethod( const QgsVectorSimplifyMethod &simplifyMethod );
%Docstring
Set the simplification settings for fast rendering of features

.. versionadded:: 2.2
%End

    const QgsVectorSimplifyMethod &simplifyMethod() const;
%Docstring
Returns the simplification settings for fast rendering of features

.. versionadded:: 2.2
%End

    bool simplifyDrawingCanbeApplied( const QgsRenderContext &renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const;
%Docstring
Returns whether the VectorLayer can apply the specified simplification hint

.. note::

   Do not use in 3rd party code - may be removed in future version!

.. versionadded:: 2.2
%End

    QgsConditionalLayerStyles *conditionalStyles() const;
%Docstring
 Return the conditional styles that are set for this layer. Style information is
used to render conditional formatting in the attribute table.

:return: Return a QgsConditionalLayerStyles object holding the conditional attribute
style information. Style information is generic and can be used for anything.

.. versionadded:: 2.12
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

.. versionadded:: 3.0
%End

    void setMapTipTemplate( const QString &mapTipTemplate );
%Docstring
The mapTip is a pretty, html representation for feature information.

It may also contain embedded expressions.

.. versionadded:: 3.0
%End

    virtual QgsExpressionContext createExpressionContext() const;


    virtual QgsExpressionContextScope *createExpressionContextScope() const /Factory/;


    QgsEditFormConfig editFormConfig() const;
%Docstring
Get the configuration of the form used to represent this vector layer.
This is a writable configuration that can directly be changed in place.

:return: The configuration of this layers' form

.. versionadded:: 2.14
%End

    void setEditFormConfig( const QgsEditFormConfig &editFormConfig );
%Docstring
Get the configuration of the form used to represent this vector layer.
This is a writable configuration that can directly be changed in place.

:return: The configuration of this layers' form

.. versionadded:: 3.0
%End

    void setReadExtentFromXml( bool readExtentFromXml );
%Docstring
Flag allowing to indicate if the extent has to be read from the XML
document when data source has no metadata or if the data provider has
to determine it.

.. versionadded:: 3.0
%End

    bool readExtentFromXml() const;
%Docstring
Returns true if the extent is read from the XML document when data
source has no metadata, false if it's the data provider which determines
it.

.. versionadded:: 3.0
%End

    bool isEditCommandActive() const;
%Docstring
Test if an edit command is active

.. versionadded:: 3.0
%End

  public slots:

    void select( QgsFeatureId featureId );
%Docstring
Select feature by its ID

:param featureId:  The id of the feature to select

.. seealso:: :py:func:`select`
%End

    void select( const QgsFeatureIds &featureIds );
%Docstring
Select features by their ID

:param featureIds: The ids of the features to select

.. seealso:: :py:func:`select`
%End

    void deselect( const QgsFeatureId featureId );
%Docstring
Deselect feature by its ID

:param featureId:  The id of the feature to deselect

.. seealso:: :py:func:`deselect`
%End

    void deselect( const QgsFeatureIds &featureIds );
%Docstring
Deselect features by their ID

:param featureIds: The ids of the features to deselect

.. seealso:: :py:func:`deselect`
%End

    void removeSelection();
%Docstring
Clear selection

.. seealso:: :py:func:`selectByIds`
%End

    virtual void updateExtents( bool force = false );
%Docstring
Update the extents for the layer. This is necessary if features are
added/deleted or the layer has been subsetted.

:param force: true to update layer extent even if it's read from xml by default, false otherwise
%End

    bool startEditing();
%Docstring
Makes the layer editable.

This starts an edit session on this layer. Changes made in this edit session will not
be made persistent until commitChanges() is called, and can be reverted by calling
rollBack().

Returns true if the layer was successfully made editable, or false if the operation
failed (e.g. due to an underlying read-only data source, or lack of edit support
by the backend data provider).

.. seealso:: :py:func:`commitChanges`

.. seealso:: :py:func:`rollBack`
%End

  signals:

    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, const bool clearAndSelect );
%Docstring
This signal is emitted when selection was changed

:param selected:        Newly selected feature ids
:param deselected:      Ids of all features which have previously been selected but are not any more
:param clearAndSelect:  In case this is set to true, the old selection was dismissed and the new selection corresponds to selected
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
Applies only to types QgsFields.OriginEdit, QgsFields.OriginProvider and QgsFields.OriginExpression

:param idx: The index of the new attribute

.. seealso:: :py:func:`updatedFields`
%End

    void beforeAddingExpressionField( const QString &fieldName );
%Docstring
Will be emitted, when an expression field is going to be added to this vector layer.
Applies only to types QgsFields.OriginExpression

:param fieldName: The name of the attribute to be added
%End

    void attributeDeleted( int idx );
%Docstring
Will be emitted, when an attribute has been deleted from this vector layer.
Applies only to types QgsFields.OriginEdit, QgsFields.OriginProvider and QgsFields.OriginExpression

:param idx: The index of the deleted attribute

.. seealso:: :py:func:`updatedFields`
%End

    void beforeRemovingExpressionField( int idx );
%Docstring
Will be emitted, when an expression field is going to be deleted from this vector layer.
Applies only to types QgsFields.OriginExpression

:param idx: The index of the attribute to be deleted
%End

    void featureAdded( QgsFeatureId fid );
%Docstring
Emitted when a new feature has been added to the layer

:param fid: The id of the new feature
%End

    void featureDeleted( QgsFeatureId fid );
%Docstring
Emitted when a feature has been deleted.

If you do expensive operations in a slot connected to this, you should prefer to use
featuresDeleted( const :py:class:`QgsFeatureIds`& ).

:param fid: The id of the feature which has been deleted
%End

    void featuresDeleted( const QgsFeatureIds &fids );
%Docstring
Emitted when features have been deleted.

If features are deleted within an edit command, this will only be emitted once at the end
to allow connected slots to minimize the overhead.
If features are deleted outside of an edit command, this signal will be emitted once per feature.

:param fids: The feature ids that have been deleted.
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

:param fid: The id of the changed feature
:param idx: The attribute index of the changed attribute
:param value: The new value of the attribute
%End

    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geometry );
%Docstring
Is emitted whenever a geometry change is done in the edit buffer.
Note that at this point the geometry change is not yet saved to the provider.

:param fid: The id of the changed feature
:param geometry: The new geometry
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

    void opacityChanged( double opacity );
%Docstring
Emitted when the layer's opacity is changed, where ``opacity`` is a value between 0 (transparent)
and 1 (opaque).

.. versionadded:: 3.0

.. seealso:: :py:func:`setOpacity`

.. seealso:: :py:func:`opacity`
%End

    void editCommandStarted( const QString &text );
%Docstring
Signal emitted when a new edit command has been started

:param text: Description for this edit command
%End

    void editCommandEnded();
%Docstring
Signal emitted, when an edit command successfully ended

.. note::

   This does not mean it is also committed, only that it is written
to the edit buffer. See beforeCommitChanges()
%End

    void editCommandDestroyed();
%Docstring
Signal emitted, whan an edit command is destroyed

.. note::

   This is not a rollback, it is only related to the current edit command.
See beforeRollBack()
%End

    void readCustomSymbology( const QDomElement &element, QString &errorMessage );
%Docstring
Signal emitted whenever the symbology (QML-file) for this layer is being read.
If there is custom style information saved in the file, you can connect to this signal
and update the layer style accordingly.

:param element: The XML layer style element.

:param errorMessage: Write error messages into this string.
%End

    void writeCustomSymbology( QDomElement &element, QDomDocument &doc, QString &errorMessage ) const;
%Docstring
Signal emitted whenever the symbology (QML-file) for this layer is being written.
If there is custom style information you want to save to the file, you can connect
to this signal and update the element accordingly.

:param element:  The XML element where you can add additional style information to.
:param doc:      The XML document that you can use to create new XML nodes.
:param errorMessage: Write error messages into this string.
%End

    void mapTipTemplateChanged();
%Docstring
Emitted when the map tip changes

.. versionadded:: 3.0
%End

    void displayExpressionChanged();
%Docstring
Emitted when the display expression changes

.. versionadded:: 3.0
%End

    void raiseError( const QString &msg );
%Docstring
Signals an error related to this vector layer.
%End

    void editFormConfigChanged();
%Docstring
Will be emitted whenever the edit form configuration of this layer changes.

.. versionadded:: 3.0
%End

    void readOnlyChanged();
%Docstring
Emitted when the read only state of this layer is changed.
Only applies to manually set readonly state, not to the edit mode.

.. versionadded:: 3.0
%End

    void symbolFeatureCountMapChanged();
%Docstring
Emitted when the feature count for symbols on this layer has been recalculated.

.. versionadded:: 3.0
%End

  protected:
    virtual void setExtent( const QgsRectangle &rect );

%Docstring
Set the extent
%End

  private:                       // Private methods
    QgsVectorLayer( const QgsVectorLayer &rhs );
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayer.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
