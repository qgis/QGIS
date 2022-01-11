
/***************************************************************************
                             -------------------
    begin                : Oct 29, 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYER_H
#define QGSVECTORLAYER_H


#include "qgis_core.h"
#include <QMap>
#include <QSet>
#include <QList>
#include <QStringList>
#include <QFont>
#include <QMutex>

#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfeaturesource.h"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorsimplifymethod.h"
#include "qgseditformconfig.h"
#include "qgsattributetableconfig.h"
#include "qgsaggregatecalculator.h"
#include "qgsfeatureiterator.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsexpressioncontextscopegenerator.h"
#include "qgsexpressioncontext.h"

class QPainter;
class QImage;

class QgsAbstractGeometrySimplifier;
class QgsActionManager;
class QgsConditionalLayerStyles;
class QgsCurve;
class QgsDiagramLayerSettings;
class QgsDiagramRenderer;
class QgsEditorWidgetWrapper;
class QgsExpressionFieldBuffer;
class QgsFeatureRenderer;
class QgsGeometry;
class QgsGeometryVertexIndex;
class QgsMapToPixel;
class QgsRectangle;
class QgsRectangle;
class QgsRelation;
class QgsWeakRelation;
class QgsRelationManager;
class QgsSingleSymbolRenderer;
class QgsStoredExpressionManager;
class QgsSymbol;
class QgsVectorLayerJoinInfo;
class QgsVectorLayerEditBuffer;
class QgsVectorLayerJoinBuffer;
class QgsVectorLayerFeatureCounter;
class QgsAbstractVectorLayerLabeling;
class QgsPalLayerSettings;
class QgsPoint;
class QgsFeedback;
class QgsAuxiliaryStorage;
class QgsAuxiliaryLayer;
class QgsGeometryOptions;
class QgsStyleEntityVisitorInterface;
class QgsVectorLayerTemporalProperties;
class QgsFeatureRendererGenerator;
class QgsVectorLayerElevationProperties;

typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;

// TODO QGIS4: Remove virtual from non-inherited methods (like isModified)

/**
 * \ingroup core
 * \brief Represents a vector layer which manages a vector based data sets.
 *
 * The QgsVectorLayer is instantiated by specifying the name of a data provider,
 * such as postgres or wfs, and url defining the specific data set to connect to.
 * The vector layer constructor in turn instantiates a QgsVectorDataProvider subclass
 * corresponding to the provider type, and passes it the url. The data provider
 * connects to the data source.
 *
 * The QgsVectorLayer provides a common interface to the different data types. It also
 * manages editing transactions by buffering layer edits until they are written to the
 * underlying QgsVectorDataProvider. Before edits can be made a call to startEditing()
 * is required. Any edits made to a QgsVectorLayer are then held in memory only, and
 * are not written to the underlying QgsVectorDataProvider until a call to commitChanges()
 * is made. Buffered edits can be rolled back and discarded without altering the
 * underlying provider by calling rollBack().
 *
 *  Sample usage of the QgsVectorLayer class:
 *
 * \code{.py}
 *     uri = "point?crs=epsg:4326&field=id:integer"
 *     scratchLayer = QgsVectorLayer(uri, "Scratch point layer",  "memory")
 * \endcode
 *
 * The main data providers supported by QGIS are listed below.
 *
 * \section providers Vector data providers
 *
 * \subsection memory Memory data providerType (memory)
 *
 * The memory data provider is used to construct in memory data, for example scratch
 * data or data generated from spatial operations such as contouring. There is no
 * inherent persistent storage of the data. The data source uri is constructed. The
 * url specifies the geometry type ("point", "linestring", "polygon",
 * "multipoint","multilinestring","multipolygon"), optionally followed by url parameters
 * as follows:
 *
 * - crs=definition
 *   Defines the coordinate reference system to use for the layer.
 *   definition is any string accepted by QgsCoordinateReferenceSystem::createFromString()
 * - index=yes
 *   Specifies that the layer will be constructed with a spatial index
 * - field=name:type(length,precision)
 *   Defines an attribute of the layer. Multiple field parameters can be added
 *   to the data provider definition. type is one of "integer", "double", "string".
 *
 * An example url is "Point?crs=epsg:4326&field=id:integer&field=name:string(20)&index=yes"
 *
 * Since QGIS 3.4 when closing a project, the application shows a warning about potential data
 * loss if there are any non-empty memory layers present. If your memory layer should not
 * trigger such warning, it is possible to suppress that by setting the following custom variable:
 *
 * \code{.py}
 *     layer.setCustomProperty("skipMemoryLayersCheck", 1)
 * \endcode
 *
 *
 * \subsection ogr OGR data provider (ogr)
 *
 * Accesses data using the OGR drivers (https://gdal.org/drivers/vector/index.html). The url
 * is the OGR connection string. A wide variety of data formats can be accessed using this
 * driver, including file based formats used by many GIS systems, database formats, and
 * web services. Some of these formats are also supported by custom data providers listed
 * below.
 *
 * \subsection spatialite SpatiaLite data provider (spatialite)
 *
 * Access data in a SpatiaLite database. The url defines the connection parameters, table,
 * geometry column, and other attributes. The url can be constructed using the
 * QgsDataSourceUri class.
 *
 * \subsection postgres PostgreSQL data provider (postgres)
 *
 * Connects to a PostgreSQL database. The url defines the connection parameters, table,
 * geometry column, and other attributes. The url can be constructed using the
 * QgsDataSourceUri class.
 *
 * \subsection mssql Microsoft SQL server data provider (mssql)
 *
 * Connects to a Microsoft SQL server database. The url defines the connection parameters, table,
 * geometry column, and other attributes. The url can be constructed using the
 * QgsDataSourceUri class.
 *
 * \subsection wfs WFS (web feature service) data provider (wfs)
 *
 * Used to access data provided by a web feature service.
 *
 * The url can be a HTTP url to a WFS server (legacy, e.g. http://foobar/wfs?TYPENAME=xxx&SRSNAME=yyy[&FILTER=zzz]), or,
 * starting with QGIS 2.16, a URI constructed using the QgsDataSourceUri class with the following parameters :
 *
 * - url=string (mandatory): HTTP url to a WFS server endpoint. e.g http://foobar/wfs
 * - typename=string (mandatory): WFS typename
 * - srsname=string (recommended): SRS like 'EPSG:XXXX'
 * - username=string
 * - password=string
 * - authcfg=string
 * - version=auto/1.0.0/1.1.0/2.0.0
 * - sql=string: full SELECT SQL statement with optional WHERE, ORDER BY and possibly with JOIN if supported on server
 * - filter=string: QGIS expression or OGC/FES filter
 * - restrictToRequestBBOX=1: to download only features in the view extent (or more generally
 *   in the bounding box of the feature iterator)
 * - pageSize=number: number of features to retrieve in a single request (WFS 2)
 * - maxNumFeatures=number: maximum number of features to retrieve (possibly across several multiple paging requests)
 * - IgnoreAxisOrientation=1: to ignore EPSG axis order for WFS 1.1 or 2.0
 * - InvertAxisOrientation=1: to invert axis order
 * - hideDownloadProgressDialog=1: to hide the download progress dialog
 *
 * The ‘FILTER’ query string parameter can be used to filter
 * the WFS feature type. The ‘FILTER’ key value can either be a QGIS expression
 * or an OGC XML filter. If the value is set to a QGIS expression the driver will
 * turn it into OGC XML filter before passing it to the WFS server. Beware the
 * QGIS expression filter only supports” =, !=, <, >, <=, >=, AND, OR, NOT, LIKE, IS NULL”
 * attribute operators, “BBOX, Disjoint, Intersects, Touches, Crosses, Contains, Overlaps, Within”
 * spatial binary operators and the QGIS local “geomFromWKT, geomFromGML”
 * geometry constructor functions.
 *
 * \subsection oapif OGC API - Features data provider (oapif)
 *
 * Used to access data provided by a OGC API - Features server.
 *
 * The URI should be constructed using the QgsDataSourceUri class with the following parameters:
 *
 * - url=string (mandatory): HTTP url to a OGC API - Features landing page.
 * - typename=string (mandatory): Collection id
 * - username=string
 * - password=string
 * - authcfg=string
 * - filter=string: QGIS expression (only datetime filtering is forwarded to the server)
 * - restrictToRequestBBOX=1: to download only features in the view extent (or more generally
 *   in the bounding box of the feature iterator)
 * - pageSize=number: number of features to retrieve in a single request
 * - maxNumFeatures=number: maximum number of features to retrieve (possibly across several multiple paging requests)
 * - hideDownloadProgressDialog=1: to hide the download progress dialog.
 *
 * Also note:
 *
 * - You can use various functions available in the QGIS Expression list,
 *   however the function must exist server side and have the same name and arguments to work.
 * - Use the special $geometry parameter to provide the layer geometry column as input
 *   into the spatial binary operators e.g intersects($geometry, geomFromWKT('POINT (5 6)'))
 *
 * \subsection delimitedtext Delimited text file data provider (delimitedtext)
 *
 * Accesses data in a delimited text file, for example CSV files generated by
 * spreadsheets. The contents of the file are split into columns based on specified
 * delimiter characters.  Each record may be represented spatially either by an
 * X and Y coordinate column, or by a WKT (well known text) formatted columns.
 *
 * The url defines the filename, the formatting options (how the
 * text in the file is divided into data fields, and which fields contain the
 * X,Y coordinates or WKT text definition.  The options are specified as url query
 * items.
 *
 * At its simplest the url can just be the filename, in which case it will be loaded
 * as a CSV formatted file.
 *
 * The url may include the following items:
 *
 * - encoding=UTF-8
 *
 *   Defines the character encoding in the file.  The default is UTF-8.  To use
 *   the default encoding for the operating system use "System".
 *
 * - type=(csv|regexp|whitespace|plain)
 *
 *   Defines the algorithm used to split records into columns. Records are
 *   defined by new lines, except for csv format files for which quoted fields
 *   may span multiple records.  The default type is csv.
 *
 * - "csv" splits the file based on three sets of characters:
 *   delimiter characters, quote characters,
 *   and escape characters.  Delimiter characters mark the end
 *   of a field. Quote characters enclose a field which can contain
 *   delimiter characters, and newlines.  Escape characters cause the
 *   following character to be treated literally (including delimiter,
 *   quote, and newline characters).  Escape and quote characters must
 *   be different from delimiter characters. Escape characters that are
 *   also quote characters are treated specially - they can only
 *   escape themselves within quotes.  Elsewhere they are treated as
 *   quote characters.  The defaults for delimiter, quote, and escape
 *   are ',', '"', '"'.
 * - "regexp" splits each record using a regular expression (see QRegularExpression
 *   documentation for details).
 * - "whitespace" splits each record based on whitespace (on or more whitespace
 *   characters.  Leading whitespace in the record is ignored.
 * - "plain" is provided for backwards compatibility.  It is equivalent to
 *   CSV except that the default quote characters are single and double quotes,
 *   and there is no escape characters.
 * - delimiter=characters
 *
 *   Defines the delimiter characters used for csv and plain type files, or the
 *   regular expression for regexp type files.  It is a literal string of characters
 *   except that "\t" may be used to represent a tab character.
 *
 * - quote=characters
 *
 *   Defines the characters that are used as quote characters for csv and plain type
 *   files.
 *
 * - escape=characters
 *
 *   Defines the characters used to escape delimiter, quote, and newline characters.
 *
 * - skipLines=n
 *
 *   Defines the number of lines to ignore at the beginning of the file (default 0)
 *
 * - useHeader=(yes|no)
 *
 *   Defines whether the first record in the file (after skipped lines) contains
 *   column names (default yes)
 *
 * - trimFields=(yes|no)
 *
 *   If yes then leading and trailing whitespace will be removed from fields
 *
 * - skipEmptyFields=(yes|no)
 *
 *   If yes then empty fields will be discarded (equivalent to concatenating consecutive
 *   delimiters)
 *
 * - maxFields=#
 *
 *   Specifies the maximum number of fields to load for each record.  Additional
 *   fields will be discarded.  Default is 0 - load all fields.
 *
 * - decimalPoint=c
 *
 *   Defines a character that is used as a decimal point in the numeric columns
 *   The default is '.'.
 *
 * - xField=column yField=column
 *
 *   Defines the name of the columns holding the x and y coordinates for XY point geometries.
 *   If the useHeader is no (ie there are no column names), then this is the column
 *   number (with the first column as 1).
 *
 * - xyDms=(yes|no)
 *
 *   If yes then the X and Y coordinates are interpreted as
 *   degrees/minutes/seconds format (fairly permissively),
 *   or degree/minutes format.
 *
 * - wktField=column
 *
 *   Defines the name of the columns holding the WKT geometry definition for WKT geometries.
 *   If the useHeader is no (ie there are no column names), then this is the column
 *   number (with the first column as 1).
 *
 * - geomType=(point|line|polygon|none)
 *
 *   Defines the geometry type for WKT type geometries.  QGIS will only display one
 *   type of geometry for the layer - any others will be ignored when the file is
 *   loaded.  By default the provider uses the type of the first geometry in the file.
 *   Use geomType to override this type.
 *
 *   geomType can also be set to none, in which case the layer is loaded without
 *   geometries.
 *
 * - subset=expression
 *
 *   Defines an expression that will identify a subset of records to display
 *
 * - crs=crsstring
 *
 *   Defines the coordinate reference system used for the layer.  This can be
 *   any string accepted by QgsCoordinateReferenceSystem::createFromString()
 *
 * - subsetIndex=(yes|no)
 *
 *   Determines whether the provider generates an index to improve the efficiency
 *   of subsets.  The default is yes
 *
 * - spatialIndex=(yes|no)
 *
 *   Determines whether the provider generates a spatial index.  The default is no.
 *
 * - watchFile=(yes|no)
 *
 *   Defines whether the file will be monitored for changes. The default is
 *   to monitor for changes.
 *
 * - quiet
 *
 *   Errors encountered loading the file will not be reported in a user dialog if
 *   quiet is included (They will still be shown in the output log).
 *
 * \subsection gpx GPX data provider (gpx)
 *
 * Provider reads tracks, routes, and waypoints from a GPX file.  The url
 * defines the name of the file, and the type of data to retrieve from it
 * ("track", "route", or "waypoint").
 *
 * An example url is "/home/user/data/holiday.gpx?type=route"
 *
 * \subsection grass Grass data provider (grass)
 *
 * Provider to display vector data in a GRASS GIS layer.
 *
 * \see QgsVectorLayerUtils()
 */
class CORE_EXPORT QgsVectorLayer : public QgsMapLayer, public QgsExpressionContextGenerator, public QgsExpressionContextScopeGenerator, public QgsFeatureSink, public QgsFeatureSource
{
    Q_OBJECT

    Q_PROPERTY( QString subsetString READ subsetString WRITE setSubsetString NOTIFY subsetStringChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString mapTipTemplate READ mapTipTemplate WRITE setMapTipTemplate NOTIFY mapTipTemplateChanged )
    Q_PROPERTY( QgsEditFormConfig editFormConfig READ editFormConfig WRITE setEditFormConfig NOTIFY editFormConfigChanged )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged )
    Q_PROPERTY( bool supportsEditing READ supportsEditing NOTIFY supportsEditingChanged )

  public:

    /**
     * Setting options for loading vector layers.
     * \since QGIS 3.0
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions.
       */
      explicit LayerOptions( bool loadDefaultStyle = true,
                             bool readExtentFromXml = false )
        : loadDefaultStyle( loadDefaultStyle )
        , readExtentFromXml( readExtentFromXml )
      {}

      /**
       * Constructor for LayerOptions.
       * \since QGIS 3.8
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext,
                             bool loadDefaultStyle = true,
                             bool readExtentFromXml = false
                           )
        : loadDefaultStyle( loadDefaultStyle )
        , readExtentFromXml( readExtentFromXml )
        , transformContext( transformContext )
      {}

      //! Set to TRUE if the default layer style should be loaded
      bool loadDefaultStyle = true;

      /**
       * If TRUE, the layer extent will be read from XML (i.e. stored in the
       * project file). If FALSE, the extent will be determined by the provider on layer load.
       */
      bool readExtentFromXml = false;

      /**
       * Coordinate transform context
       * \since QGIS 3.8
       */
      QgsCoordinateTransformContext transformContext = QgsCoordinateTransformContext();

      /**
       * Fallback geometry type.
       *
       * This may be set for layers where the geometry type is known in advance, and where
       * the layer path may not be initially resolvable. (E.g. layers with a URI pointing
       * to a non-existent file). It is only ever used if the layer cannot be resolved,
       * otherwise the actual layer geometry type will be detected and used for the layer.
       *
       * \see fallbackCrs
       * \since QGIS 3.8
       */
      QgsWkbTypes::Type fallbackWkbType = QgsWkbTypes::Unknown;

      /**
       * Fallback layer coordinate reference system.
       *
       * This may be set for layers where the coordinate reference system is known in advance, and where
       * the layer path may not be initially resolvable. (E.g. layers with a URI pointing
       * to a non-existent file). It is only ever used if the layer cannot be resolved,
       * otherwise the actual layer CRS will be detected and used for the layer.
       *
       * \see fallbackWkbType
       * \since QGIS 3.8
       */
      QgsCoordinateReferenceSystem fallbackCrs;

      /**
       * Controls whether the layer is allowed to have an invalid/unknown CRS.
       *
       * If TRUE, then no validation will be performed on the layer's CRS and the layer
       * layer's crs() may be invalid() (i.e. the layer will have no georeferencing available
       * and will be treated as having purely numerical coordinates).
       *
       * If FALSE (the default), the layer's CRS will be validated using QgsCoordinateReferenceSystem::validate(),
       * which may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the
       * layer.
       *
       * \since QGIS 3.10
       */
      bool skipCrsValidation = false;

    };

    /**
     * Context for cascade delete features
     * \since QGIS 3.14
     */
    struct CORE_EXPORT DeleteContext
    {

      /**
       * Constructor for DeleteContext.
       */
      explicit DeleteContext( bool cascade = false, QgsProject *project = nullptr ): cascade( cascade ), project( project ) {}

      /**
       * Returns a list of all layers affected by the delete operation.
       *
       * If \a includeAuxiliaryLayers is FALSE then auxiliary layers will not be included in the
       * returned list.
       */
      QList<QgsVectorLayer *> handledLayers( bool includeAuxiliaryLayers = true ) const;

      /**
       * Returns a list of feature IDs from the specified \a layer affected by the delete operation.
       */
      QgsFeatureIds handledFeatures( QgsVectorLayer *layer ) const;

      QMap<QgsVectorLayer *, QgsFeatureIds> mHandledFeatures SIP_SKIP;
      bool cascade;
      QgsProject *project;
    };

    /**
     * Constructor - creates a vector layer
     *
     * The QgsVectorLayer is constructed by instantiating a data provider.  The provider
     * interprets the supplied path (url) of the data source to connect to and access the
     * data.
     *
     * \param path  The path or url of the parameter.  Typically this encodes
     *               parameters used by the data provider as url query items.
     * \param baseName The name used to represent the layer in the legend
     * \param providerLib  The name of the data provider, e.g., "memory", "postgres"
     * \param options layer load options
     */
    explicit QgsVectorLayer( const QString &path = QString(), const QString &baseName = QString(),
                             const QString &providerLib = "ogr", const QgsVectorLayer::LayerOptions &options = QgsVectorLayer::LayerOptions() );

    ~QgsVectorLayer() override;

    //! QgsVectorLayer cannot be copied.
    QgsVectorLayer( const QgsVectorLayer &rhs ) = delete;
    //! QgsVectorLayer cannot be copied.
    QgsVectorLayer &operator=( QgsVectorLayer const &rhs ) = delete;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsVectorLayer: '%1' (%2)>" ).arg( sipCpp->name(), sipCpp->dataProvider() ? sipCpp->dataProvider()->name() : QStringLiteral( "Invalid" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns a new instance equivalent to this one. A new provider is
     *  created for the same data source and renderers for features and diagrams
     *  are cloned too. Moreover, each attributes (transparency, extent, selected
     *  features and so on) are identical.
     * \returns a new layer instance
     * \since QGIS 3.0
     */
    QgsVectorLayer *clone() const override SIP_FACTORY;

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     * This is obtained from the data provider and does not follow any standard.
     */
    QString storageType() const;

    /**
     * Capabilities for this layer, comma separated and translated.
     */
    QString capabilitiesString() const;


    /**
     * Returns TRUE if the layer is a query (SQL) layer.
     *
     * \note this is simply a shortcut to check if the SqlQuery flag
     *       is set.
     *
     * \see vectorLayerTypeFlags()
     * \since QGIS 3.24
     */
    bool isSqlQuery() const;

    /**
     * Returns the vector layer type flags.
     *
     * \see isSqlQuery()
     * \since QGIS 3.24
     */
    Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const;


    /**
     * Returns a description for this layer as defined in the data provider.
     */
    QString dataComment() const;

    /**
     * This is a shorthand for accessing the displayExpression if it is a simple field.
     * If the displayExpression is more complex than a simple field, a null string will
     * be returned.
     *
     * \see displayExpression
     */
    QString displayField() const;

    /**
     * Set the preview expression, used to create a human readable preview string.
     *  Used e.g. in the attribute table feature list. Uses QgsExpression.
     *
     *  \param displayExpression The expression which will be used to preview features
     *                           for this layer
     */
    void setDisplayExpression( const QString &displayExpression );

    /**
     *  Returns the preview expression, used to create a human readable preview string.
     *  Uses QgsExpression
     *
     *  \returns The expression which will be used to preview features for this layer
     */
    QString displayExpression() const;

    QgsVectorDataProvider *dataProvider() FINAL;
    const QgsVectorDataProvider *dataProvider() const FINAL SIP_SKIP;
    QgsMapLayerTemporalProperties *temporalProperties() override;
    QgsMapLayerElevationProperties *elevationProperties() override;

    /**
     * Sets the text \a encoding of the data provider.
     *
     * An empty \a encoding string indicates that the provider should automatically
     * select the most appropriate encoding.
     *
     * \warning Support for setting the provider encoding depends on the underlying data
     * provider. Check dataProvider().capabilities() for the QgsVectorDataProvider::SelectEncoding
     * capability in order to determine if the provider supports this ability.
     */
    void setProviderEncoding( const QString &encoding );

    //! Setup the coordinate system transformation for the layer
    void setCoordinateSystem();

    /**
     * Joins another vector layer to this layer
     * \param joinInfo join object containing join layer id, target and source field
     * \note since 2.6 returns bool indicating whether the join can be added
    */
    bool addJoin( const QgsVectorLayerJoinInfo &joinInfo );

    /**
     * Removes a vector layer join
     * \returns TRUE if join was found and successfully removed
    */
    bool removeJoin( const QString &joinLayerId );

    /**
     * Returns the join buffer object.
     * \since QGIS 2.14.7
     */
    QgsVectorLayerJoinBuffer *joinBuffer() { return mJoinBuffer; }

    /**
     * Returns a const pointer on join buffer object.
     * \since QGIS 3.10
     */
    const QgsVectorLayerJoinBuffer *joinBuffer() const { return mJoinBuffer; } SIP_SKIP;

    const QList<QgsVectorLayerJoinInfo> vectorJoins() const;

    /**
     * Sets the list of dependencies.
     * \see dependencies()
     *
     * \param layers set of QgsMapLayerDependency. Only user-defined dependencies will be added
     * \returns FALSE if a dependency cycle has been detected
     * \since QGIS 3.0
     */
    bool setDependencies( const QSet<QgsMapLayerDependency> &layers ) FINAL;

    /**
     * Gets the list of dependencies. This includes data dependencies set by the user (\see setDataDependencies)
     * as well as dependencies given by the provider
     *
     * \returns a set of QgsMapLayerDependency
     * \since QGIS 3.0
     */
    QSet<QgsMapLayerDependency> dependencies() const FINAL;

    /**
     * Add a new field which is calculated by the expression specified
     *
     * \param exp The expression which calculates the field
     * \param fld The field to calculate
     *
     * \returns The index of the new field
     *
     * \since QGIS 2.9
     */
    int addExpressionField( const QString &exp, const QgsField &fld );

    /**
     * Removes an expression field
     *
     * \param index The index of the field
     *
     * \since QGIS 2.6
     */
    void removeExpressionField( int index );

    /**
     * Returns the expression used for a given expression field
     *
     * \param index An index of an epxression based (virtual) field
     *
     * \returns The expression for the field at index
     *
     * \since QGIS 2.9
     */
    QString expressionField( int index ) const;

    /**
     * Changes the expression used to define an expression based (virtual) field
     *
     * \param index The index of the expression to change
     *
     * \param exp The new expression to set
     *
     * \since QGIS 2.9
     */
    void updateExpressionField( int index, const QString &exp );

    /**
     * Returns all layer actions defined on this layer.
     *
     * The pointer which is returned directly points to the actions object
     * which is used by the layer, so any changes are immediately applied.
     */
    QgsActionManager *actions() { return mActions; }

    /**
     * Returns all layer actions defined on this layer.
     *
     * The pointer which is returned is const.
     */
    const QgsActionManager *actions() const SIP_SKIP { return mActions; }

    /**
     * Returns the number of features that are selected in this layer.
     *
     * \see selectedFeatureIds()
     */
    int selectedFeatureCount() const;

    /**
     * Selects features found within the search rectangle (in layer's coordinates)
     * \param rect search rectangle
     * \param behavior selection type, allows adding to current selection, removing
     * from selection, etc.
     * \see invertSelectionInRectangle(QgsRectangle & rect)
     * \see selectByExpression()
     * \see selectByIds()
     */
    Q_INVOKABLE void selectByRect( QgsRectangle &rect, Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection );

    /**
     * Selects matching features using an expression.
     * \param expression expression to evaluate to select features
     * \param behavior selection type, allows adding to current selection, removing
     * from selection, etc.
     * \see selectByRect()
     * \see selectByIds()
     * \since QGIS 2.16
     */
    Q_INVOKABLE void selectByExpression( const QString &expression, Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection );

    /**
     * Selects matching features using a list of feature IDs. Will emit the
     * selectionChanged() signal with the clearAndSelect flag set.
     * \param ids feature IDs to select
     * \param behavior selection type, allows adding to current selection, removing
     * from selection, etc.
     * \see selectByRect()
     * \see selectByExpression()
     * \since QGIS 2.16
     */
    Q_INVOKABLE void selectByIds( const QgsFeatureIds &ids, Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection );

    /**
     * Modifies the current selection on this layer
     *
     * \param selectIds    Select these ids
     * \param deselectIds  Deselect these ids
     *
     * \see   selectByIds
     * \see   deselect(const QgsFeatureIds&)
     * \see   deselect(const QgsFeatureId)
     * \see selectByExpression()
     */
    Q_INVOKABLE void modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds );

    //! Selects not selected features and deselects selected ones
    Q_INVOKABLE void invertSelection();

    //! Select all the features
    Q_INVOKABLE void selectAll();

    /**
     * Inverts selection of features found within the search rectangle (in layer's coordinates)
     *
     * \param rect  The rectangle in which the selection of features will be inverted
     *
     * \see   invertSelection()
     */
    Q_INVOKABLE void invertSelectionInRectangle( QgsRectangle &rect );

    /**
     * Returns a copy of the user-selected features.
     *
     * \warning Calling this method triggers a request for all attributes and geometry for the selected features.
     * Consider using the much more efficient selectedFeatureIds() or selectedFeatureCount() if you do not
     * require access to the feature attributes or geometry.
     *
     * \returns A list of QgsFeature
     *
     * \see    selectedFeatureIds()
     * \see    getSelectedFeatures() which is more memory friendly when handling large selections
     */
    Q_INVOKABLE QgsFeatureList selectedFeatures() const;

    /**
     * Returns an iterator of the selected features.
     *
     * \param request You may specify a request, e.g. to limit the set of requested attributes.
     *                Any filter on the request will be discarded.
     *
     * \returns Iterator over the selected features
     *
     * \warning Calling this method returns an iterator for all attributes and geometry for the selected features.
     * Consider using the much more efficient selectedFeatureIds() or selectedFeatureCount() if you do not
     * require access to the feature attributes or geometry.
     *
     * \see    selectedFeatureIds()
     * \see    selectedFeatures()
     */
    QgsFeatureIterator getSelectedFeatures( QgsFeatureRequest request = QgsFeatureRequest() ) const;

    /**
     * Returns a list of the selected features IDs in this layer.
     *
     * \see selectedFeatures()
     * \see selectedFeatureCount()
     * \see selectByIds()
     */
    Q_INVOKABLE const QgsFeatureIds &selectedFeatureIds() const;

    //! Returns the bounding box of the selected features. If there is no selection, QgsRectangle(0,0,0,0) is returned
    Q_INVOKABLE QgsRectangle boundingBoxOfSelected() const;

    /**
     * Returns whether the layer contains labels which are enabled and should be drawn.
     * \returns TRUE if layer contains enabled labels
     *
     * \see setLabelsEnabled()
     * \since QGIS 2.9
     */
    bool labelsEnabled() const;

    /**
     * Sets whether labels should be \a enabled for the layer.
     *
     * \note Labels will only be rendered if labelsEnabled() is TRUE and a labeling
     * object is returned by labeling().
     *
     * \see labelsEnabled()
     * \see labeling()
     */
    void setLabelsEnabled( bool enabled );

    /**
     * Returns whether the layer contains diagrams which are enabled and should be drawn.
     * \returns TRUE if layer contains enabled diagrams
     * \since QGIS 2.9
     */
    bool diagramsEnabled() const;

    //! Sets diagram rendering object (takes ownership)
    void setDiagramRenderer( QgsDiagramRenderer *r SIP_TRANSFER );
    const QgsDiagramRenderer *diagramRenderer() const { return mDiagramRenderer; }

    void setDiagramLayerSettings( const QgsDiagramLayerSettings &s );
    const QgsDiagramLayerSettings *diagramLayerSettings() const { return mDiagramLayerSettings; }

    /**
     * Returns the feature renderer used for rendering the features in the layer in 2D
     * map views.
     *
     * \see setRenderer()
     */
    QgsFeatureRenderer *renderer() { return mRenderer; }

    /**
     * Returns the feature renderer used for rendering the features in the layer in 2D
     * map views.
     *
     * \see setRenderer()
     * \note not available in Python bindings
     */
    const QgsFeatureRenderer *renderer() const SIP_SKIP { return mRenderer; }

    /**
     * Sets the feature renderer which will be invoked to represent this layer in 2D map views.
     * Ownership is transferred.
     *
     * \see renderer()
     */
    void setRenderer( QgsFeatureRenderer *r SIP_TRANSFER );

    /**
     * Adds a new feature renderer \a generator to the layer.
     *
     * Ownership of \a generator is transferred to the layer.
     *
     * \see removeFeatureRendererGenerator()
     * \see featureRendererGenerators()
     * \since QGIS 3.18
     */
    void addFeatureRendererGenerator( QgsFeatureRendererGenerator *generator SIP_TRANSFER );

    /**
     * Removes the feature renderer with matching \a id from the layer.
     *
     * The corresponding generator will be deleted.
     *
     * \see addFeatureRendererGenerator()
     * \see featureRendererGenerators()
     * \since QGIS 3.18
     */
    void removeFeatureRendererGenerator( const QString &id );

    /**
     * Returns a list of the feature renderer generators owned by the layer.
     *
     * \see addFeatureRendererGenerator()
     * \see removeFeatureRendererGenerator()
     * \since QGIS 3.18
     */
    QList< const QgsFeatureRendererGenerator * > featureRendererGenerators() const;

    //! Returns point, line or polygon
    Q_INVOKABLE QgsWkbTypes::GeometryType geometryType() const;

    //! Returns the WKBType or WKBUnknown in case of error
    Q_INVOKABLE QgsWkbTypes::Type wkbType() const FINAL;

    QgsCoordinateReferenceSystem sourceCrs() const FINAL;
    QString sourceName() const FINAL;

    /**
     * Reads vector layer specific state from project file Dom node.
     * \note Called by QgsMapLayer::readXml().
     */
    bool readXml( const QDomNode &layer_node, QgsReadWriteContext &context ) FINAL;

    /**
     * Writes vector layer specific state to project file Dom node.
     * \note Called by QgsMapLayer::writeXml().
     */
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const FINAL;

    QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const FINAL;
    QString decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const FINAL;

    /**
     * Resolves references to other layers (kept as layer IDs after reading XML) into layer objects.
     * \since QGIS 3.0
     */
    void resolveReferences( QgsProject *project ) FINAL;

    /**
     * Saves named and sld style of the layer to the style table in the db.
     * \param name Style name
     * \param description A description of the style
     * \param useAsDefault Set to TRUE if style should be used as the default style for the layer
     * \param uiFileContent
     * \param msgError will be set to a descriptive error message if any occurs
     *
     *
     * \note Prior to QGIS 3.24, this method would show a message box warning when a
     * style with the same \a styleName already existed to confirm replacing the style with the user.
     * Since 3.24, calling this method will ALWAYS overwrite any existing style with the same name.
     * Use QgsProviderRegistry::styleExists() to test in advance if a style already exists and handle this appropriately
     * in your client code.
     */
    virtual void saveStyleToDatabase( const QString &name, const QString &description,
                                      bool useAsDefault, const QString &uiFileContent,
                                      QString &msgError SIP_OUT );

    /**
     * Lists all the style in db split into related to the layer and not related to
     * \param ids the list in which will be stored the style db ids
     * \param names the list in which will be stored the style names
     * \param descriptions the list in which will be stored the style descriptions
     * \param msgError will be set to a descriptive error message if any occurs
     * \returns the number of styles related to current layer (-1 on not implemented)
     * \note Since QGIS 3.2 Styles related to the layer are ordered with the default style first then by update time for Postgres, MySQL and Spatialite.
     */
    virtual int listStylesInDatabase( QStringList &ids SIP_OUT, QStringList &names SIP_OUT,
                                      QStringList &descriptions SIP_OUT, QString &msgError SIP_OUT );

    /**
     * Returns the named style corresponding to style id provided
     */
    virtual QString getStyleFromDatabase( const QString &styleId, QString &msgError SIP_OUT );

    /**
     * Deletes a style from the database
     * \param styleId the provider's layer_styles table id of the style to delete
     * \param msgError will be set to a descriptive error message if any occurs
     * \returns TRUE in case of success
     * \since QGIS 3.0
     */
    virtual bool deleteStyleFromDatabase( const QString &styleId, QString &msgError SIP_OUT );

    /**
     * Loads a named style from file/local db/datasource db
     * \param theURI the URI of the style or the URI of the layer
     * \param resultFlag will be set to TRUE if a named style is correctly loaded
     * \param loadFromLocalDb if TRUE forces to load from local db instead of datasource one
     * \param categories the style categories to be loaded.
     */
    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag SIP_OUT, bool loadFromLocalDb,
                                    QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories );

    /**
     * Calls loadNamedStyle( theURI, resultFlag, FALSE );
     * Retained for backward compatibility
     */
    QString loadNamedStyle( const QString &theURI, bool &resultFlag SIP_OUT,
                            QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) FINAL;

    /**
     * Loads the auxiliary layer for this vector layer. If there's no
     * corresponding table in the database, then nothing happens and FALSE is
     * returned. The key is optional because if this layer has been read from
     * a XML document, then the key read in this document is used by default.
     *
     * \param storage The auxiliary storage where to look for the table
     * \param key The key to use for joining.
     *
     * \returns TRUE if the auxiliary layer is well loaded, FALSE otherwise
     *
     * \since QGIS 3.0
     */
    bool loadAuxiliaryLayer( const QgsAuxiliaryStorage &storage, const QString &key = QString() );

    /**
     * Sets the current auxiliary layer. The auxiliary layer is automatically
     * put in editable mode and fields are updated. Moreover, a join is created
     * between the current layer and the auxiliary layer. Ownership is
     * transferred.
     *
     *
     * \since QGIS 3.0
     */
    void setAuxiliaryLayer( QgsAuxiliaryLayer *layer SIP_TRANSFER = nullptr );

    /**
     * Returns the current auxiliary layer.
     *
     * \since QGIS 3.0
     */
    QgsAuxiliaryLayer *auxiliaryLayer();

    /**
     * Returns the current const auxiliary layer.
     *
     * \since QGIS 3.0
     */
    const QgsAuxiliaryLayer *auxiliaryLayer() const SIP_SKIP;

    bool readSymbology( const QDomNode &layerNode, QString &errorMessage,
                        QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) FINAL;
    bool readStyle( const QDomNode &node, QString &errorMessage,
                    QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) FINAL;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                         const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const FINAL;
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                     const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const FINAL;

    /**
     * Writes the symbology of the layer into the document provided in SLD 1.1 format
     * \param node the node that will have the style element added to it.
     * \param doc the document that will have the QDomNode added.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param props a open ended set of properties that can drive/inform the SLD encoding
     * \returns TRUE in case of success
     */
    bool writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QVariantMap &props = QVariantMap() ) const;

    bool readSld( const QDomNode &node, QString &errorMessage ) FINAL;

    /**
     * Number of features rendered with specified legend key. Features must be first
     * calculated by countSymbolFeatures()
     * \returns number of features rendered by symbol or -1 if failed or counts are not available
     */
    long long featureCount( const QString &legendKey ) const;

    /**
     * Ids of features rendered with specified legend key. Features must be first
     * calculated by countSymbolFeatures()
     * \returns Ids of features rendered by symbol or -1 if failed or Ids are not available
     * \since QGIS 3.10
     */
    QgsFeatureIds symbolFeatureIds( const QString &legendKey ) const;

    /**
     * Determines if this vector layer has features.
     *
     * \warning when a layer is editable and some features
     * have been deleted, this will return
     * QgsFeatureSource::FeatureAvailability::FeaturesMayBeAvailable
     * to avoid a potentially expensive call to the dataprovider.
     *
     * \since QGIS 3.4
     */
    FeatureAvailability hasFeatures() const FINAL;


    QString loadDefaultStyle( bool &resultFlag SIP_OUT ) FINAL;

    /**
     * Count features for symbols.
     * The method will return the feature counter task. You will need to
     * connect to the symbolFeatureCountMapChanged() signal to be
     * notified when the freshly updated feature counts are ready.
     *
     * \param storeSymbolFids If TRUE will gather the feature ids (fids) per symbol, otherwise only the count. Default FALSE.
     * \note If the count features for symbols has been already done a
     *       NULLPTR is returned. If you need to wait for the results,
     *       you can call waitForFinished() on the feature counter.
     *
     * \since This is asynchronous since QGIS 3.0
     */
    QgsVectorLayerFeatureCounter *countSymbolFeatures( bool storeSymbolFids = false );

    /**
     * Sets the string (typically sql) used to define a subset of the layer
     * \param subset The subset string. This may be the where clause of a sql statement
     *               or other definition string specific to the underlying dataprovider
     *               and data store.
     * \returns TRUE, when setting the subset string was successful, FALSE otherwise
     */
    virtual bool setSubsetString( const QString &subset );

    /**
     * Returns the string (typically sql) used to define a subset of the layer.
     * \returns The subset string or null QString if not implemented by the provider
     */
    virtual QString subsetString() const;

    /**
     * Queries the layer for features specified in request.
     * \param request feature request describing parameters of features to return
     * \returns iterator for matching features from provider
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const FINAL;

    /**
     * Queries the layer for features matching a given expression.
     */
    inline QgsFeatureIterator getFeatures( const QString &expression )
    {
      return getFeatures( QgsFeatureRequest( expression ) );
    }

    /**
     * Queries the layer for the feature with the given id.
     * If there is no such feature, the returned feature will be invalid.
     */
    inline QgsFeature getFeature( QgsFeatureId fid ) const
    {
      QgsFeature feature;
      getFeatures( QgsFeatureRequest( fid ) ).nextFeature( feature );
      return feature;
    }

    /**
     * Queries the layer for the geometry at the given id.
     * If there is no such feature, the returned geometry will be invalid.
     */
    QgsGeometry getGeometry( QgsFeatureId fid ) const;

    /**
     * Queries the layer for the features with the given ids.
     */
    inline QgsFeatureIterator getFeatures( const QgsFeatureIds &fids )
    {
      return getFeatures( QgsFeatureRequest( fids ) );
    }

    /**
     * Queries the layer for the features which intersect the specified rectangle.
     */
    inline QgsFeatureIterator getFeatures( const QgsRectangle &rectangle )
    {
      return getFeatures( QgsFeatureRequest( rectangle ) );
    }

    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) FINAL;

    /**
     * Updates an existing \a feature in the layer, replacing the attributes and geometry for the feature
     * with matching QgsFeature::id() with the attributes and geometry from \a feature.
     * Changes are not immediately committed to the layer.
     *
     * If \a skipDefaultValue is set to TRUE, default field values will not
     * be updated. This can be used to override default field value expressions.
     *
     * Returns TRUE if the feature's attribute was successfully changed.
     *
     * \note Calls to updateFeature() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     *
     * \warning This method needs to query the underlying data provider to fetch the feature
     * with matching QgsFeature::id() on every call. Depending on the underlying data source this
     * can be slow to execute. Consider using the more efficient changeAttributeValue() or
     * changeGeometry() methods instead.
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeGeometry()
     * \see changeAttributeValue()
    */
    bool updateFeature( QgsFeature &feature, bool skipDefaultValues = false );

    /**
     * Inserts a new vertex before the given vertex number,
     * in the given ring, item (first number is index 0), and feature.
     *
     * Not meaningful for Point geometries
     *
     * \note Calls to insertVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );

    /**
     * Inserts a new vertex before the given vertex number,
     * in the given ring, item (first number is index 0), and feature.
     *
     * Not meaningful for Point geometries
     *
     * \note Calls to insertVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool insertVertex( const QgsPoint &point, QgsFeatureId atFeatureId, int beforeVertex );

    /**
     * Moves the vertex at the given position number,
     * ring and item (first number is index 0), and feature
     * to the given coordinates.
     *
     * \note Calls to moveVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );

    /**
     * Moves the vertex at the given position number,
     * ring and item (first number is index 0), and feature
     * to the given coordinates.
     * \note available in Python as moveVertexV2
     * \note Calls to moveVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool moveVertex( const QgsPoint &p, QgsFeatureId atFeatureId, int atVertex ) SIP_PYNAME( moveVertexV2 );

    /**
     * Deletes a vertex from a feature.
     * \param featureId ID of feature to remove vertex from
     * \param vertex index of vertex to delete
     * \note Calls to deleteVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \since QGIS 2.14
     */
    Qgis::VectorEditResult deleteVertex( QgsFeatureId featureId, int vertex );

    /**
     * Deletes the selected features
     * \param deletedCount The number of successfully deleted features
     * \param context The chain of features who will be deleted for feedback and to avoid endless recursions
     *
     * \returns TRUE in case of success and FALSE otherwise
     */
    Q_INVOKABLE bool deleteSelectedFeatures( int *deletedCount = nullptr, DeleteContext *context = nullptr );

    /**
     * Adds a ring to polygon/multipolygon features
     * \param ring ring to add
     * \param featureId if specified, feature ID for feature ring was added to will be stored in this parameter
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - LayerNotEditable
     * - AddRingNotInExistingFeature
     * - InvalidInputGeometryType
     * - AddRingNotClosed
     * - AddRingNotValid
     * - AddRingCrossesExistingRings
     *
     * \note Calls to addRing() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED Qgis::GeometryOperationResult addRing( const QVector<QgsPointXY> &ring, QgsFeatureId *featureId = nullptr ) SIP_DEPRECATED;


    /**
     * Adds a ring to polygon/multipolygon features
     * \param ring ring to add
     * \param featureId if specified, feature ID for feature ring was added to will be stored in this parameter
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - LayerNotEditable
     * - AddRingNotInExistingFeature
     * - InvalidInputGeometryType
     * - AddRingNotClosed
     * - AddRingNotValid
     * - AddRingCrossesExistingRings
     *
     * \note Calls to addRing() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    Q_INVOKABLE Qgis::GeometryOperationResult addRing( const QgsPointSequence &ring, QgsFeatureId *featureId = nullptr );

    /**
     * Adds a ring to polygon/multipolygon features (takes ownership)
     * \param ring ring to add
     * \param featureId if specified, feature ID for feature ring was added to will be stored in this parameter
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - LayerNotEditable
     * - AddRingNotInExistingFeature
     * - InvalidInputGeometryType
     * - AddRingNotClosed
     * - AddRingNotValid
     * - AddRingCrossesExistingRings
     *
     * \note available in Python as addCurvedRing
     * \note Calls to addRing() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    Q_INVOKABLE Qgis::GeometryOperationResult addRing( QgsCurve *ring SIP_TRANSFER, QgsFeatureId *featureId = nullptr ) SIP_PYNAME( addCurvedRing );

    /**
     * Adds a new part polygon to a multipart feature
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - LayerNotEditable
     * - SelectionIsEmpty
     * - SelectionIsGreaterThanOne
     * - AddPartSelectedGeometryNotFound
     * - AddPartNotMultiGeometry
     * - InvalidBaseGeometry
     * - InvalidInputGeometryType
     *
     * \note Calls to addPart() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED Qgis::GeometryOperationResult addPart( const QList<QgsPointXY> &ring ) SIP_DEPRECATED;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

    /**
     * Adds a new part polygon to a multipart feature
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - LayerNotEditable
     * - SelectionIsEmpty
     * - SelectionIsGreaterThanOne
     * - AddPartSelectedGeometryNotFound
     * - AddPartNotMultiGeometry
     * - InvalidBaseGeometry
     * - InvalidInputGeometryType
     *
     * \note available in Python bindings as addPartV2
     * \note Calls to addPart() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED Qgis::GeometryOperationResult addPart( const QVector<QgsPointXY> &ring ) SIP_PYNAME( addPartV2 ) SIP_DEPRECATED;
#endif

    /**
     * Adds a new part polygon to a multipart feature
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - LayerNotEditable
     * - SelectionIsEmpty
     * - SelectionIsGreaterThanOne
     * - AddPartSelectedGeometryNotFound
     * - AddPartNotMultiGeometry
     * - InvalidBaseGeometry
     * - InvalidInputGeometryType
     *
     * \note available in Python bindings as addPartV2
     * \note Calls to addPart() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    Q_INVOKABLE Qgis::GeometryOperationResult addPart( const QgsPointSequence &ring ) SIP_PYNAME( addPartV2 );

    /**
     * \note available in Python as addCurvedPart
     * \note Calls to addPart() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    Q_INVOKABLE Qgis::GeometryOperationResult addPart( QgsCurve *ring SIP_TRANSFER ) SIP_PYNAME( addCurvedPart );

    /**
     * Translates feature by dx, dy
     * \param featureId id of the feature to translate
     * \param dx translation of x-coordinate
     * \param dy translation of y-coordinate
     * \returns 0 in case of success
     * \note Calls to translateFeature() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    Q_INVOKABLE int translateFeature( QgsFeatureId featureId, double dx, double dy );

    /**
     * Splits parts cut by the given line
     * \param splitLine line that splits the layer features
     * \param topologicalEditing TRUE if topological editing is enabled
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - NothingHappened
     * - LayerNotEditable
     * - InvalidInputGeometryType
     * - InvalidBaseGeometry
     * - GeometryEngineError
     * - SplitCannotSplitPoint
     *
     * \note Calls to splitParts() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED Qgis::GeometryOperationResult splitParts( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false ) SIP_DEPRECATED;

    /**
     * Splits parts cut by the given line
     * \param splitLine line that splits the layer features
     * \param topologicalEditing TRUE if topological editing is enabled
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - NothingHappened
     * - LayerNotEditable
     * - InvalidInputGeometryType
     * - InvalidBaseGeometry
     * - GeometryEngineError
     * - SplitCannotSplitPoint
     *
     * \note Calls to splitParts() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    Q_INVOKABLE Qgis::GeometryOperationResult splitParts( const QgsPointSequence &splitLine, bool topologicalEditing = false );

    /**
     * Splits features cut by the given line
     * \param splitLine line that splits the layer features
     * \param topologicalEditing TRUE if topological editing is enabled
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - NothingHappened
     * - LayerNotEditable
     * - InvalidInputGeometryType
     * - InvalidBaseGeometry
     * - GeometryEngineError
     * - SplitCannotSplitPoint
     *
     * \note Calls to splitFeatures() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED Qgis::GeometryOperationResult splitFeatures( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false ) SIP_DEPRECATED;

    /**
     * Splits features cut by the given line
     * \param splitLine line that splits the layer features
     * \param topologicalEditing TRUE if topological editing is enabled
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - NothingHappened
     * - LayerNotEditable
     * - InvalidInputGeometryType
     * - InvalidBaseGeometry
     * - GeometryEngineError
     * - SplitCannotSplitPoint
     *
     * \note Calls to splitFeatures() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    Q_INVOKABLE Qgis::GeometryOperationResult splitFeatures( const QgsPointSequence &splitLine, bool topologicalEditing = false );

    /**
     * Splits features cut by the given curve
     * \param curve curve that splits the layer features
     * \param[out] topologyTestPoints topological points to be tested against other layers
     * \param preserveCircular whether circular strings are preserved after splitting
     * \param topologicalEditing TRUE if topological editing is enabled
     * \returns Qgis::GeometryOperationResult
     *
     * - Success
     * - NothingHappened
     * - LayerNotEditable
     * - InvalidInputGeometryType
     * - InvalidBaseGeometry
     * - GeometryEngineError
     * - SplitCannotSplitPoint
     *
     * \note Calls to splitFeatures() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \since QGIS 3.16
     */
    Q_INVOKABLE Qgis::GeometryOperationResult splitFeatures( const QgsCurve *curve, QgsPointSequence &topologyTestPoints SIP_OUT, bool preserveCircular = false, bool topologicalEditing = false );

    /**
     * Adds topological points for every vertex of the geometry.
     * \param geom the geometry where each vertex is added to segments of other features
     * \returns -1 in case of layer error (invalid or non editable)
     * \returns 0 in case of success
     * \returns 1 in case of geometry error (non spatial or null geometry)
     * \returns 2 in case no vertices needed to be added
     * \note geom is not going to be modified by the function
     * \note Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    int addTopologicalPoints( const QgsGeometry &geom );

    /**
     * Adds a vertex to segments which intersect point \a p but don't
     * already have a vertex there. If a feature already has a vertex at position \a p,
     * no additional vertex is inserted. This method is useful for topological
     * editing.
     * \param p position of the vertex
     * \returns -1 in case of layer error (invalid or non editable)
     * \returns 0 in case of success
     * \returns 1 in case of geometry error (non spatial or null geometry)
     * \returns 2 in case no vertices needed to be added
     * \note Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED int addTopologicalPoints( const QgsPointXY &p )  SIP_DEPRECATED;

    /**
     * Adds a vertex to segments which intersect point \a p but don't
     * already have a vertex there. If a feature already has a vertex at position \a p,
     * no additional vertex is inserted. This method is useful for topological
     * editing.
     * \param p position of the vertex
     * \returns -1 in case of layer error (invalid or non editable)
     * \returns 0 in case of success
     * \returns 1 in case of geometry error (non spatial or null geometry)
     * \returns 2 in case no vertices needed to be added
     * \note Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \since 3.10
     */
    int addTopologicalPoints( const QgsPoint &p );

    /**
     * Adds a vertex to segments which intersect any of the points \a p but don't
     * already have a vertex there. If a feature already has a vertex at position \a p,
     * no additional vertex is inserted. This method is useful for topological
     * editing.
     * \param ps point sequence of the vertices
     * \returns -1 in case of layer error (invalid or non editable)
     * \returns 0 in case of success
     * \returns 1 in case of geometry error (non spatial or null geometry)
     * \returns 2 in case no vertices needed to be added
     * \note Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \since 3.16
     */
    int addTopologicalPoints( const QgsPointSequence &ps );

    /**
     * Access to const labeling configuration. May be NULLPTR if labeling is not used.
     * \note Labels will only be rendered if labelsEnabled() returns TRUE.
     * \see labelsEnabled()
     * \since QGIS 3.0
     */
    const QgsAbstractVectorLayerLabeling *labeling() const SIP_SKIP { return mLabeling; }

    /**
     * Access to labeling configuration. May be NULLPTR if labeling is not used.
     * \note Labels will only be rendered if labelsEnabled() returns TRUE.
     * \see labelsEnabled()
     * \since QGIS 3.0
     */
    QgsAbstractVectorLayerLabeling *labeling() { return mLabeling; }

    /**
     * Sets labeling configuration. Takes ownership of the object.
     * \since QGIS 3.0
     */
    void setLabeling( QgsAbstractVectorLayerLabeling *labeling SIP_TRANSFER );

    //! Returns TRUE if the provider is in editing mode
    bool isEditable() const FINAL;

    //! Returns TRUE if this is a geometry layer and FALSE in case of NoGeometry (table only) or UnknownGeometry
    bool isSpatial() const FINAL;

    //! Returns TRUE if the provider has been modified since the last commit
    bool isModified() const override;

    /**
     * Returns TRUE if the field comes from the auxiliary layer,
     * FALSE otherwise.
     *
     * \since QGIS 3.0
     */
    bool isAuxiliaryField( int index, int &srcIndex ) const;

    //! Synchronises with changes in the datasource
    void reload() FINAL;

    /**
     * Returns new instance of QgsMapLayerRenderer that will be used for rendering of given context
     * \since QGIS 2.4
     */
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) FINAL SIP_FACTORY;

    QgsRectangle extent() const FINAL;
    QgsRectangle sourceExtent() const FINAL;

    /**
     * Returns the list of fields of this layer.
     * This also includes fields which have not yet been saved to the provider.
     *
     * \returns A list of fields
     */
    QgsFields fields() const FINAL;

    /**
     * Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
     */
    inline QgsAttributeList attributeList() const { return mFields.allAttributesList(); }

    /**
     * Returns the list of attributes which make up the layer's primary keys.
     */
    QgsAttributeList primaryKeyAttributes() const;

    /**
     * Returns feature count including changes which have not yet been committed
     * If you need only the count of committed features call this method on this layer's provider.
     * \returns the number of features on this layer or -1 if unknown.
     */
    long long featureCount() const FINAL;

    /**
     * Makes layer read-only (editing disabled) or not
     * \returns FALSE if the layer is in editing yet
     */
    bool setReadOnly( bool readonly = true );

    /**
     * Returns whether the layer supports editing or not
     * \return FALSE if the layer is read only or the data provider has no editing capabilities
     * \since QGIS 3.18
     */
    bool supportsEditing() const override;

    /**
     * Changes a feature's \a geometry within the layer's edit buffer
     * (but does not immediately commit the changes). The \a fid argument
     * specifies the ID of the feature to be changed.
     *
     * If \a skipDefaultValue is set to TRUE, default field values will not
     * be updated. This can be used to override default field value expressions.
     *
     * \returns TRUE if the feature's geometry was successfully changed.
     *
     * \note Calls to changeGeometry() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeAttributeValue()
     * \see updateFeature()
     */
    bool changeGeometry( QgsFeatureId fid, QgsGeometry &geometry, bool skipDefaultValue = false );

    /**
     * Changes an attribute value for a feature (but does not immediately commit the changes).
     * The \a fid argument specifies the ID of the feature to be changed.
     *
     * The \a field argument must specify a valid field index for the layer (where an index of 0
     * corresponds to the first field).
     *
     * The new value to be assigned to the field is given by \a newValue.
     *
     * If a valid QVariant is specified for \a oldValue, it will be used as the field value in the
     * case of an undo operation corresponding to this attribute value change. If an invalid
     * QVariant is used (the default behavior), then the feature's current value will be automatically
     * retrieved and used. Note that this involves a feature request to the underlying data provider,
     * so it is more efficient to explicitly pass an \a oldValue if it is already available.
     *
     * If \a skipDefaultValues is set to TRUE, default field values will not
     * be updated. This can be used to override default field value expressions.
     *
     * \returns TRUE if the feature's attribute was successfully changed.
     *
     * \note Calls to changeAttributeValue() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeGeometry()
     * \see updateFeature()
     */
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant(), bool skipDefaultValues = false );

    /**
     * Changes attributes' values for a feature (but does not immediately
     * commit the changes).
     * The \a fid argument specifies the ID of the feature to be changed.
     *
     * The new values to be assigned to the fields are given by \a newValues.
     *
     * If a valid QVariant is specified for a field in \a oldValues, it will be
     * used as the field value in the case of an undo operation corresponding
     * to this attribute value change. If an invalid QVariant is used (the
     * default behavior), then the feature's current value will be
     * automatically retrieved and used. Note that this involves a feature
     * request to the underlying data provider, so it is more efficient to
     * explicitly pass an oldValue if it is already available.
     *
     * If \a skipDefaultValues is set to TRUE, default field values will not
     * be updated. This can be used to override default field value
     * expressions.
     *
     * \returns TRUE if feature's attributes was successfully changed.
     *
     * \note Calls to changeAttributeValues() are only valid for layers in
     * which edits have been enabled by a call to startEditing(). Changes made
     * to features using this method are not committed to the underlying data
     * provider until a commitChanges() call is made. Any uncommitted changes
     * can be discarded by calling rollBack().
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeGeometry()
     * \see updateFeature()
     * \see changeAttributeValue()
     *
     * \since QGIS 3.0
     */
    bool changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues = QgsAttributeMap(), bool skipDefaultValues = false );

    /**
     * Add an attribute field (but does not commit it)
     * returns TRUE if the field was added
     *
     * \note Calls to addAttribute() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool addAttribute( const QgsField &field );

    /**
     * Sets an alias (a display name) for attributes to display in dialogs
     *
     * \since QGIS 3.0
     */
    void setFieldAlias( int index, const QString &aliasString );

    /**
     * Removes an alias (a display name) for attributes to display in dialogs
     *
     * \since QGIS 3.0
     */
    void removeFieldAlias( int index );

    /**
     * Renames an attribute field  (but does not commit it).
     * \param index attribute index
     * \param newName new name of field
     * \note Calls to renameAttribute() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \since QGIS 2.16
     */
    bool renameAttribute( int index, const QString &newName );

    /**
     * Returns the alias of an attribute name or a null string if there is no alias.
     *
     * \see {attributeDisplayName( int attributeIndex )} which returns the field name
     *      if no alias is defined.
     */
    QString attributeAlias( int index ) const;

    //! Convenience function that returns the attribute alias if defined or the field name else
    QString attributeDisplayName( int index ) const;

    //! Returns a map of field name to attribute alias
    QgsStringMap attributeAliases() const;

    /**
     * A set of attributes that are not advertised in WMS requests with QGIS server.
     * \deprecated since QGIS 3.16, use fields().configurationFlags() instead
     */
    Q_DECL_DEPRECATED QSet<QString> excludeAttributesWms() const SIP_DEPRECATED;

    /**
     * A set of attributes that are not advertised in WMS requests with QGIS server.
     * \deprecated since QGIS 3.16, use setFieldConfigurationFlag instead
     */
    Q_DECL_DEPRECATED void setExcludeAttributesWms( const QSet<QString> &att ) SIP_DEPRECATED;

    /**
     * A set of attributes that are not advertised in WFS requests with QGIS server.
     * \deprecated since QGIS 3.16, use fields().configurationFlags() instead
     */
    Q_DECL_DEPRECATED QSet<QString> excludeAttributesWfs() const SIP_DEPRECATED;

    /**
     * A set of attributes that are not advertised in WFS requests with QGIS server.
     * \deprecated since QGIS 3.16, use setFieldConfigurationFlag instead
     */
    Q_DECL_DEPRECATED void setExcludeAttributesWfs( const QSet<QString> &att ) SIP_DEPRECATED;

    /**
     * Deletes an attribute field (but does not commit it).
     *
     * \note Calls to deleteAttribute() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    virtual bool deleteAttribute( int attr );

    /**
     * Deletes a list of attribute fields (but does not commit it)
     *
     * \param  attrs the indices of the attributes to delete
     * \returns TRUE if at least one attribute has been deleted
     *
     */
    bool deleteAttributes( const QList<int> &attrs );

    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) FINAL;

    /**
     * Deletes a feature from the layer (but does not commit it).
     * \param fid The feature id to delete
     * \param context The chain of features who will be deleted for feedback and to avoid endless recursions
     *
     * \note Calls to deleteFeature() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool deleteFeature( QgsFeatureId fid, DeleteContext *context = nullptr );

    /**
     * Deletes a set of features from the layer (but does not commit it)
     * \param fids The feature ids to delete
     * \param context The chain of features who will be deleted for feedback and to avoid endless recursions
     *
     * \returns FALSE if the layer is not in edit mode or does not support deleting
     *         in case of an active transaction depends on the provider implementation
     *
     * \note Calls to deleteFeatures() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool deleteFeatures( const QgsFeatureIds &fids, DeleteContext *context = nullptr );

    /**
     * Attempts to commit to the underlying data provider any buffered changes made since the
     * last to call to startEditing().
     *
     * Returns the result of the attempt. If a commit fails (i.e. FALSE is returned), the
     * in-memory changes are left untouched and are not discarded. This allows editing to
     * continue if the commit failed on e.g. a disallowed value in a Postgres
     * database - the user can re-edit and try again.
     *
     * The commits occur in distinct stages,
     * (add attributes, add features, change attribute values, change
     * geometries, delete features, delete attributes)
     * so if a stage fails, it can be difficult to roll back cleanly.
     * Therefore any error message returned by commitErrors() also includes which stage failed so
     * that the user has some chance of repairing the damage cleanly.
     *
     * By setting \a stopEditing to FALSE, the layer will stay in editing mode.
     * Otherwise the layer editing mode will be disabled if the commit is successful.
     *
     * \see startEditing()
     * \see commitErrors()
     * \see rollBack()
     */
    Q_INVOKABLE bool commitChanges( bool stopEditing = true );

    /**
     * Returns a list containing any error messages generated when attempting
     * to commit changes to the layer.
     * \see commitChanges()
     */
    QStringList commitErrors() const;

    /**
     * Stops a current editing operation and discards any uncommitted edits.
     *
     * If \a deleteBuffer is TRUE the editing buffer will be completely deleted (the default
     * behavior).
     *
     * \see startEditing()
     * \see commitChanges()
     */
    Q_INVOKABLE bool rollBack( bool deleteBuffer = true );

    /**
     * Returns the layer's relations, where the foreign key is on this layer.
     *
     * \param idx Only get relations, where idx forms part of the foreign key
     * \returns A list of relations
     */
    QList<QgsRelation> referencingRelations( int idx ) const;

    /**
     * Returns the layer's weak relations as specified in the layer's style.
     * \returns A list of weak relations
     * \note not available in Python bindings
     * \since QGIS 3.12
     */
    QList<QgsWeakRelation> weakRelations( ) const SIP_SKIP;


    //! Buffer with uncommitted editing operations. Only valid after editing has been turned on.
    Q_INVOKABLE QgsVectorLayerEditBuffer *editBuffer() { return mEditBuffer; }

    /**
     * Buffer with uncommitted editing operations. Only valid after editing has been turned on.
     * \note not available in Python bindings
     */
    const QgsVectorLayerEditBuffer *editBuffer() const SIP_SKIP { return mEditBuffer; }

    /**
     * Create edit command for undo/redo operations
     * \param text text which is to be displayed in undo window
     */
    void beginEditCommand( const QString &text );

    //! Finish edit command and add it to undo/redo stack
    void endEditCommand();

    //! Destroy active command and reverts all changes in it
    void destroyEditCommand();

    /**
     * Draws a vertex symbol at (screen) coordinates x, y. (Useful to assist vertex editing.)
     * \deprecated Use the equivalent QgsSymbolLayerUtils::drawVertexMarker function instead
     */
    Q_DECL_DEPRECATED static void drawVertexMarker( double x, double y, QPainter &p, Qgis::VertexMarkerType type, int vertexSize );

    /**
     * Will regenerate the `fields` property of this layer by obtaining all fields
     * from the dataProvider, joined fields and virtual fields. It will also
     * take any changes made to default values into consideration.
     *
     * \note Unless the fields on the provider have directly been modified, there is
     * no reason to call this method.
     */
    void updateFields();

    /**
     * Returns the calculated default value for the specified field index. The default
     * value may be taken from a client side default value expression (see setDefaultValueDefinition())
     * or taken from the underlying data provider.
     * \param index field index
     * \param feature optional feature to use for default value evaluation. If passed,
     * then properties from the feature (such as geometry) can be used when calculating
     * the default value.
     * \param context optional expression context to evaluate expressions again. If not
     * specified, a default context will be created
     * \returns calculated default value
     * \see setDefaultValueDefinition()
     * \since QGIS 3.0
     */
    QVariant defaultValue( int index, const QgsFeature &feature = QgsFeature(),
                           QgsExpressionContext *context = nullptr ) const;

    /**
     * Sets the definition of the expression to use when calculating the default value for a field.
     * \param index field index
     * \param definition default value definition to use and evaluate
     * when calculating default values for field. Pass
     * an empty expression to clear the default.
     *
     * \see defaultValue()
     * \see defaultValueDefinition()
     * \since QGIS 3.0
     */
    void setDefaultValueDefinition( int index, const QgsDefaultValue &definition );

    /**
     * Returns the definition of the expression used when calculating the default value for a field.
     * \param index field index
     * \returns definition of the default value with the expression evaluated
     * when calculating default values for field, or definition with an
     * empty string if no default is set
     * \see defaultValue()
     * \see setDefaultValueDefinition()
     * \since QGIS 3.0
     */
    QgsDefaultValue defaultValueDefinition( int index ) const;

    /**
     * Returns any constraints which are present for a specified
     * field index. These constraints may be inherited from the layer's data provider
     * or may be set manually on the vector layer from within QGIS.
     * \see setFieldConstraint()
     * \since QGIS 3.0
     */
    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;

    /**
     * Returns a map of constraint with their strength for a specific field of the layer.
     * \param fieldIndex field index
     * \since QGIS 3.0
     */
    QMap< QgsFieldConstraints::Constraint, QgsFieldConstraints::ConstraintStrength> fieldConstraintsAndStrength( int fieldIndex ) const;

    /**
     * Sets a constraint for a specified field index. Any constraints inherited from the layer's
     * data provider will be kept intact and cannot be modified. Ie, calling this method only allows for new
     * constraints to be added on top of the existing provider constraints.
     * \see fieldConstraints()
     * \see removeFieldConstraint()
     * \since QGIS 3.0
     */
    void setFieldConstraint( int index, QgsFieldConstraints::Constraint constraint, QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthHard );

    /**
     * Removes a constraint for a specified field index. Any constraints inherited from the layer's
     * data provider will be kept intact and cannot be removed.
     * \see fieldConstraints()
     * \see setFieldConstraint()
     * \since QGIS 3.0
     */
    void removeFieldConstraint( int index, QgsFieldConstraints::Constraint constraint );

    /**
     * Returns the constraint expression for for a specified field index, if set.
     * \see fieldConstraints()
     * \see constraintDescription()
     * \see setConstraintExpression()
     * \since QGIS 3.0
     */
    QString constraintExpression( int index ) const;

    /**
     * Returns the descriptive name for the constraint expression for a specified field index.
     * \see fieldConstraints()
     * \see constraintExpression()
     * \see setConstraintExpression()
     * \since QGIS 3.0
     */
    QString constraintDescription( int index ) const;

    /**
     * Sets the constraint expression for the specified field index. An optional descriptive name for the constraint
     * can also be set. Setting an empty expression will clear any existing expression constraint.
     * \see constraintExpression()
     * \see constraintDescription()
     * \see fieldConstraints()
     * \since QGIS 3.0
     */
    void setConstraintExpression( int index, const QString &expression, const QString &description = QString() );

    /**
     * Sets the configuration flags of the field at given index
     * \see QgsField::ConfigurationFlag
     * \since QGIS 3.16
     */
    void setFieldConfigurationFlags( int index, QgsField::ConfigurationFlags flags ) SIP_SKIP;

    /**
     * Sets the given configuration \a flag for the field at given \a index to be \a active or not.
     * \since QGIS 3.16
     */
    void setFieldConfigurationFlag( int index, QgsField::ConfigurationFlag flag, bool active ) SIP_SKIP;

    /**
     * Returns the configuration flags of the field at given index
     * \see QgsField::ConfigurationFlag
     * \since QGIS 3.16
     */
    QgsField::ConfigurationFlags fieldConfigurationFlags( int index ) const SIP_SKIP;

    /**
     * \copydoc editorWidgetSetup
     */
    void setEditorWidgetSetup( int index, const QgsEditorWidgetSetup &setup );

    /**
     * The editor widget setup defines which QgsFieldFormatter and editor widget will be used
     * for the field at `index`.
     *
     * \since QGIS 3.0
     */
    QgsEditorWidgetSetup editorWidgetSetup( int index ) const;

    /**
     * Calculates a list of unique values contained within an attribute in the layer. Note that
     * in some circumstances when unsaved changes are present for the layer then the returned list
     * may contain outdated values (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value will be included in the
     * returned list).
     * \param fieldIndex column index for attribute
     * \param limit maximum number of values to return (or -1 if unlimited)
     * \see minimumValue()
     * \see maximumValue()
     */
    QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const FINAL;

    /**
     * Returns unique string values of an attribute which contain a specified subset string. Subset
     * matching is done in a case-insensitive manner. Note that
     * in some circumstances when unsaved changes are present for the layer then the returned list
     * may contain outdated values (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value will be included in the
     * returned list).
     * \param index column index for attribute
     * \param substring substring to match (case insensitive)
     * \param limit maxmum number of the values to return, or -1 to return all unique values
     * \param feedback optional feedback object for canceling request
     * \returns list of unique strings containing substring
     */
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const;

    /**
     * Returns the minimum value for an attribute column or an invalid variant in case of error.
     *
     * \note In some circumstances when unsaved changes are present for the layer then the
     * returned value may be outdated (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value may be returned as the minimum).
     *
     * \note If both the minimum and maximum value are required it is more efficient to call minimumAndMaximumValue()
     * instead of separate calls to minimumValue() and maximumValue().
     *
     * \see maximumValue()
     * \see minimumAndMaximumValue()
     * \see uniqueValues()
     */
    QVariant minimumValue( int index ) const FINAL;

    /**
     * Returns the maximum value for an attribute column or an invalid variant in case of error.
     *
     * \note In some circumstances when unsaved changes are present for the layer then the
     * returned value may be outdated (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value may be returned as the maximum).
     *
     * \note If both the minimum and maximum value are required it is more efficient to call minimumAndMaximumValue()
     * instead of separate calls to minimumValue() and maximumValue().
     *
     * \see minimumValue()
     * \see minimumAndMaximumValue()
     * \see uniqueValues()
     */
    QVariant maximumValue( int index ) const FINAL;


    /**
     * Calculates both the minimum and maximum value for an attribute column.
     *
     * This is more efficient then calling both minimumValue() and maximumValue() when both the minimum
     * and maximum values are required.
     *
     * \param index index of field to calculate minimum and maximum value for.
     * \param minimum will be set to minimum attribute value or an invalid variant in case of error.
     * \param maximum will be set to maximum attribute value or an invalid variant in case of error.
     *
     * \note In some circumstances when unsaved changes are present for the layer then the
     * calculated values may be outdated (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value may be returned as the maximum).
     *
     * \see minimumValue()
     * \see maximumValue()
     *
     * \since QGIS 3.20
     */
    void minimumAndMaximumValue( int index, QVariant &minimum SIP_OUT, QVariant &maximum SIP_OUT ) const;

    /**
     * Calculates an aggregated value from the layer's features.
     * Currently any filtering expression provided will override filters in the FeatureRequest.
     * \param aggregate aggregate to calculate
     * \param fieldOrExpression source field or expression to use as basis for aggregated values.
     * \param parameters parameters controlling aggregate calculation
     * \param context expression context for expressions and filters
     * \param ok if specified, will be set to TRUE if aggregate calculation was successful
     * \param fids list of fids to filter, otherwise will use all fids
     * \param feedback optional feedback argument for early cancellation (since QGIS 3.22)
     * \param error optional storage for error messages (not available in Python bindings)
     * \returns calculated aggregate value
     * \since QGIS 2.16
     */
    QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                        const QString &fieldOrExpression,
                        const QgsAggregateCalculator::AggregateParameters &parameters = QgsAggregateCalculator::AggregateParameters(),
                        QgsExpressionContext *context = nullptr,
                        bool *ok = nullptr,
                        QgsFeatureIds *fids = nullptr,
                        QgsFeedback *feedback = nullptr,
                        QString *error SIP_PYARGREMOVE = nullptr ) const;

    //! Sets the blending mode used for rendering each feature
    void setFeatureBlendMode( QPainter::CompositionMode blendMode );
    //! Returns the current blending mode for features
    QPainter::CompositionMode featureBlendMode() const;

    QString htmlMetadata() const FINAL;

    /**
     * Sets the simplification settings for fast rendering of features
     * \since QGIS 2.2
     */
    void setSimplifyMethod( const QgsVectorSimplifyMethod &simplifyMethod ) { mSimplifyMethod = simplifyMethod; }

    /**
     * Returns the simplification settings for fast rendering of features
     * \since QGIS 2.2
     */
    inline const QgsVectorSimplifyMethod &simplifyMethod() const { return mSimplifyMethod; }

    /**
     * Returns whether the VectorLayer can apply the specified simplification hint
     * \note Do not use in 3rd party code - may be removed in future version!
     * \since QGIS 2.2
     */
    bool simplifyDrawingCanbeApplied( const QgsRenderContext &renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const;

    /**
     * Returns the conditional styles that are set for this layer. Style information is
     * used to render conditional formatting in the attribute table.
     * \returns Return a QgsConditionalLayerStyles object holding the conditional attribute
     * style information. Style information is generic and can be used for anything.
     * \since QGIS 2.12
     */
    QgsConditionalLayerStyles *conditionalStyles() const;

    /**
     * Returns the attribute table configuration object.
     * This defines the appearance of the attribute table.
     */
    QgsAttributeTableConfig attributeTableConfig() const;

    /**
     * Sets the attribute table configuration object.
     * This defines the appearance of the attribute table.
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig &attributeTableConfig );

    /**
     * The mapTip is a pretty, html representation for feature information.
     *
     * It may also contain embedded expressions.
     *
     * \since QGIS 3.0
     */
    QString mapTipTemplate() const;

    /**
     * The mapTip is a pretty, html representation for feature information.
     *
     * It may also contain embedded expressions.
     *
     * \since QGIS 3.0
     */
    void setMapTipTemplate( const QString &mapTipTemplate );

    QgsExpressionContext createExpressionContext() const FINAL;

    QgsExpressionContextScope *createExpressionContextScope() const FINAL SIP_FACTORY;

    /**
     * Returns the configuration of the form used to represent this vector layer.
     *
     * \returns The configuration of this layers' form
     *
     * \since QGIS 2.14
     */
    QgsEditFormConfig editFormConfig() const;

    /**
     * Sets the \a editFormConfig (configuration) of the form used to represent this vector layer.
     *
     * \see editFormConfig()
     * \since QGIS 3.0
     */
    void setEditFormConfig( const QgsEditFormConfig &editFormConfig );

    /**
     * Flag allowing to indicate if the extent has to be read from the XML
     * document when data source has no metadata or if the data provider has
     * to determine it.
     *
     * \since QGIS 3.0
     */
    void setReadExtentFromXml( bool readExtentFromXml );

    /**
     * Returns TRUE if the extent is read from the XML document when data
     * source has no metadata, FALSE if it's the data provider which determines
     * it.
     *
     * \since QGIS 3.0
     */
    bool readExtentFromXml() const;

    /**
     * Tests if an edit command is active
     *
     * \since QGIS 3.0
     */
    bool isEditCommandActive() const { return mEditCommandActive; }

    /**
     * Configuration and logic to apply automatically on any edit happening on this layer.
     *
     * \since QGIS 3.4
     */
    QgsGeometryOptions *geometryOptions() const;

    /**
     * Controls, if the layer is allowed to commit changes. If this is set to FALSE
     * it will not be possible to commit changes on this layer. This can be used to
     * define checks on a layer that need to be pass before the layer can be saved.
     * If you use this API, make sure that:
     *
     * - the user is visibly informed that his changes were not saved and what he needs
     *   to do in order to be able to save the changes.
     * - to set the property back to TRUE, once the user has fixed his data.
     *
     * When calling \see commitChanges() this flag is checked just after the
     * \see beforeCommitChanges() signal is emitted, so it's possible to adjust it from there.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.4
     */
    bool allowCommit() const SIP_SKIP;

    /**
     * Controls, if the layer is allowed to commit changes. If this is set to FALSE
     * it will not be possible to commit changes on this layer. This can be used to
     * define checks on a layer that need to be pass before the layer can be saved.
     * If you use this API, make sure that:
     *
     * - the user is visibly informed that his changes were not saved and what he needs
     *   to do in order to be able to save the changes.
     * - to set the property back to TRUE, once the user has fixed his data.
     *
     * When calling \see commitChanges() this flag is checked just after the
     * \see beforeCommitChanges() signal is emitted, so it's possible to adjust it from there.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.4
     */
    void setAllowCommit( bool allowCommit ) SIP_SKIP;

    /**
     * Returns the manager of the stored expressions for this layer.
     *
     * \since QGIS 3.10
     */
    QgsStoredExpressionManager *storedExpressionManager() { return mStoredExpressionManager; }

  public slots:

    /**
     * Selects feature by its ID
     *
     * \param featureId  The id of the feature to select
     *
     * \see select( const QgsFeatureIds& )
     */
    void select( QgsFeatureId featureId );

    /**
     * Selects features by their ID
     *
     * \param featureIds The ids of the features to select
     *
     * \see select(QgsFeatureId)
     */
    Q_INVOKABLE void select( const QgsFeatureIds &featureIds );

    /**
     * Deselects feature by its ID
     *
     * \param featureId  The id of the feature to deselect
     *
     * \see deselect(const QgsFeatureIds&)
     */
    void deselect( QgsFeatureId featureId );

    /**
     * Deselects features by their ID
     *
     * \param featureIds The ids of the features to deselect
     *
     * \see deselect(const QgsFeatureId)
     */
    Q_INVOKABLE void deselect( const QgsFeatureIds &featureIds );

    /**
     * Clear selection
     *
     * \see selectByIds()
     * \see reselect()
     */
    Q_INVOKABLE void removeSelection();

    /**
     * Reselects the previous set of selected features. This is only applicable
     * after a prior call to removeSelection().
     *
     * Any other modifications to the selection following a call to removeSelection() clears
     * memory of the previous selection and consequently calling reselect() has no impact.
     *
     * \see removeSelection()
     * \since QGIS 3.10
     */
    void reselect();

    /**
     * Update the extents for the layer. This is necessary if features are
     * added/deleted or the layer has been subsetted.
     *
     * \param force TRUE to update layer extent even if it's read from xml by default, FALSE otherwise
     */
    virtual void updateExtents( bool force = false );

    /**
     * Makes the layer editable.
     *
     * This starts an edit session on this layer. Changes made in this edit session will not
     * be made persistent until commitChanges() is called, and can be reverted by calling
     * rollBack().
     *
     * \returns TRUE if the layer was successfully made editable, or FALSE if the operation
     * failed (e.g. due to an underlying read-only data source, or lack of edit support
     * by the backend data provider).
     *
     * \see commitChanges()
     * \see rollBack()
     */
    Q_INVOKABLE bool startEditing();

    /**
     * Sets the coordinate transform context to \a transformContext
     *
     * \since QGIS 3.8
     */
    virtual void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;

    SpatialIndexPresence hasSpatialIndex() const override;

    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

  signals:

    /**
     * Emitted when selection was changed
     *
     * \param selected        Newly selected feature ids
     * \param deselected      Ids of all features which have previously been selected but are not any more
     * \param clearAndSelect  In case this is set to TRUE, the old selection was dismissed and the new selection corresponds to selected
     */
    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );

    /**
     * Emitted whenever the allowCommitChanged() property of this layer changes.
     *
     * \since QGIS 3.4
     */
    void allowCommitChanged();

    //! Emitted when the layer is checked for modifications. Use for last-minute additions.
    void beforeModifiedCheck() const;

    //! Emitted before editing on this layer is started.
    void beforeEditingStarted();

    /**
     * Emitted before changes are committed to the data provider.
     *
     * The \a stopEditing flag specifies if the editing mode shall be left after this commit.
     */
    void beforeCommitChanges( bool stopEditing );

    //! Emitted before changes are rolled back.
    void beforeRollBack();

    /**
     * Emitted after changes are committed to the data provider.
     * \since QGIS 3.16
     */
    void afterCommitChanges();

    /**
     * Emitted after changes are rolled back.
     * \since QGIS 3.4
     */
    void afterRollBack();

    /**
     * Will be emitted, when a new attribute has been added to this vector layer.
     * Applies only to types QgsFields::OriginEdit, QgsFields::OriginProvider and QgsFields::OriginExpression
     *
     * \param idx The index of the new attribute
     *
     * \see updatedFields()
     */
    void attributeAdded( int idx );

    /**
     * Will be emitted, when an expression field is going to be added to this vector layer.
     * Applies only to types QgsFields::OriginExpression
     *
     * \param fieldName The name of the attribute to be added
     */
    void beforeAddingExpressionField( const QString &fieldName );

    /**
     * Will be emitted, when an attribute has been deleted from this vector layer.
     * Applies only to types QgsFields::OriginEdit, QgsFields::OriginProvider and QgsFields::OriginExpression
     *
     * \param idx The index of the deleted attribute
     *
     * \see updatedFields()
     */
    void attributeDeleted( int idx );

    /**
     * Will be emitted, when an expression field is going to be deleted from this vector layer.
     * Applies only to types QgsFields::OriginExpression
     *
     * \param idx The index of the attribute to be deleted
     */
    void beforeRemovingExpressionField( int idx );

    /**
     * Emitted when a new feature has been added to the layer
     *
     * \param fid The id of the new feature
     */
    void featureAdded( QgsFeatureId fid );

    /**
     * Emitted when a feature has been deleted.
     *
     * If you do expensive operations in a slot connected to this, you should prefer to use
     * featuresDeleted( const QgsFeatureIds& ).
     *
     * \param fid The id of the feature which has been deleted
     */
    void featureDeleted( QgsFeatureId fid );

    /**
     * Emitted when features have been deleted.
     *
     * If features are deleted within an edit command, this will only be emitted once at the end
     * to allow connected slots to minimize the overhead.
     * If features are deleted outside of an edit command, this signal will be emitted once per feature.
     *
     * \param fids The feature ids that have been deleted.
     */
    void featuresDeleted( const QgsFeatureIds &fids );

    /**
     * Emitted whenever the fields available from this layer have been changed.
     * This can be due to manually adding attributes or due to a join.
     */
    void updatedFields();

    /**
     * Emitted when the layer's subset string has changed.
     * \since QGIS 3.2
     */
    void subsetStringChanged();

    /**
     * Emitted whenever an attribute value change is done in the edit buffer.
     * Note that at this point the attribute change is not yet saved to the provider.
     *
     * \param fid The id of the changed feature
     * \param idx The attribute index of the changed attribute
     * \param value The new value of the attribute
     */
    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );

    /**
     * Emitted whenever a geometry change is done in the edit buffer.
     * Note that at this point the geometry change is not yet saved to the provider.
     *
     * \param fid The id of the changed feature
     * \param geometry The new geometry
     */
    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geometry );

    //! Emitted when attributes are deleted from the provider if not in transaction mode.
    void committedAttributesDeleted( const QString &layerId, const QgsAttributeList &deletedAttributes );
    //! Emitted when attributes are added to the provider if not in transaction mode.
    void committedAttributesAdded( const QString &layerId, const QList<QgsField> &addedAttributes );
    //! Emitted when features are added to the provider if not in transaction mode.
    void committedFeaturesAdded( const QString &layerId, const QgsFeatureList &addedFeatures );
    //! Emitted when features are deleted from the provider if not in transaction mode.
    void committedFeaturesRemoved( const QString &layerId, const QgsFeatureIds &deletedFeatureIds );
    //! Emitted when attribute value changes are saved to the provider if not in transaction mode.
    void committedAttributeValuesChanges( const QString &layerId, const QgsChangedAttributesMap &changedAttributesValues );
    //! Emitted when geometry changes are saved to the provider if not in transaction mode.
    void committedGeometriesChanges( const QString &layerId, const QgsGeometryMap &changedGeometries );

    //! Emitted when the font family defined for labeling layer is not found on system
    void labelingFontNotFound( QgsVectorLayer *layer, const QString &fontfamily );

    //! Signal emitted when setFeatureBlendMode() is called
    void featureBlendModeChanged( QPainter::CompositionMode blendMode );

    /**
     * Signal emitted when a new edit command has been started
     *
     * \param text Description for this edit command
     */
    void editCommandStarted( const QString &text );

    /**
     * Signal emitted, when an edit command successfully ended
     * \note This does not mean it is also committed, only that it is written
     * to the edit buffer. See beforeCommitChanges()
     */
    void editCommandEnded();

    /**
     * Signal emitted, when an edit command is destroyed
     * \note This is not a rollback, it is only related to the current edit command.
     * See beforeRollBack()
     */
    void editCommandDestroyed();

    /**
     * Signal emitted whenever the symbology (QML-file) for this layer is being read.
     * If there is custom style information saved in the file, you can connect to this signal
     * and update the layer style accordingly.
     *
     * \param element The XML layer style element.
     *
     * \param errorMessage Write error messages into this string.
     */
    void readCustomSymbology( const QDomElement &element, QString &errorMessage );

    /**
     * Signal emitted whenever the symbology (QML-file) for this layer is being written.
     * If there is custom style information you want to save to the file, you can connect
     * to this signal and update the element accordingly.
     *
     * \param element  The XML element where you can add additional style information to.
     * \param doc      The XML document that you can use to create new XML nodes.
     * \param errorMessage Write error messages into this string.
     */
    void writeCustomSymbology( QDomElement &element, QDomDocument &doc, QString &errorMessage ) const;

    /**
     * Emitted when the map tip changes
     *
     * \since QGIS 3.0
     */
    void mapTipTemplateChanged();

    /**
     * Emitted when the display expression changes
     *
     * \since QGIS 3.0
     */
    void displayExpressionChanged();

    /**
     * Signals an error related to this vector layer.
     */
    void raiseError( const QString &msg );

    /**
     * Will be emitted whenever the edit form configuration of this layer changes.
     *
     * \since QGIS 3.0
     */
    void editFormConfigChanged();

    /**
     * Emitted when the read only state of this layer is changed.
     * Only applies to manually set readonly state, not to the edit mode.
     *
     * \since QGIS 3.0
     */
    void readOnlyChanged();

    /**
     * Emitted when the read only state or the data provider of this layer is changed.
     *
     * \since QGIS 3.18
     */
    void supportsEditingChanged();

    /**
     * Emitted when the feature count for symbols on this layer has been recalculated.
     *
     * \since QGIS 3.0
     */
    void symbolFeatureCountMapChanged();

  protected:
    //! Sets the extent
    void setExtent( const QgsRectangle &rect ) FINAL;

  private slots:
    void invalidateSymbolCountedFlag();
    void onFeatureCounterCompleted();
    void onFeatureCounterTerminated();
    void onJoinedFieldsChanged();
    void onFeatureDeleted( QgsFeatureId fid );
    void onRelationsLoaded();
    void onSymbolsCounted();
    void onDirtyTransaction( const QString &sql, const QString &name );
    void emitDataChanged();
    void onAfterCommitChangesDependency();

  private:
    void updateDefaultValues( QgsFeatureId fid, QgsFeature feature = QgsFeature() );

    /**
     * Returns TRUE if the provider is in read-only mode
     */
    bool isReadOnly() const FINAL;

    /**
     * Bind layer to a specific data provider
     * \param provider provider key string, must match a valid QgsVectorDataProvider key. E.g. "postgres", "ogr", etc.
     * \param options provider options
     * \param flags provider flags, since QGIS 3.16
     */
    bool setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    /**
     * Updates the data source of the layer. The layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     * \param dataSource new layer data source
     * \param baseName base name of the layer
     * \param provider provider string
     * \param options provider options
     * \param flags provider read flags
     * \see dataSourceChanged()
     * \since QGIS 3.20
     */
    void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags ) override;

    //! Read labeling from SLD
    void readSldLabeling( const QDomNode &node );

    //! Read settings from SLD TextSymbolizer element
    bool readSldTextSymbolizer( const QDomNode &node, QgsPalLayerSettings &settings ) const;

    //! Read simple labeling from layer's custom properties (QGIS 2.x projects)
    QgsAbstractVectorLayerLabeling *readLabelingFromCustomProperties();

    bool deleteFeatureCascade( QgsFeatureId fid, DeleteContext *context = nullptr );

#ifdef SIP_RUN
    QgsVectorLayer( const QgsVectorLayer &rhs );
#endif
    //! Returns the minimum or maximum value
    void minimumOrMaximumValue( int index, QVariant *minimum, QVariant *maximum ) const;

    void createEditBuffer();
    void clearEditBuffer();

    QgsConditionalLayerStyles *mConditionalStyles = nullptr;

    //! Pointer to data provider derived from the abastract base class QgsDataProvider
    QgsVectorDataProvider *mDataProvider = nullptr;

    //! Pointer to temporal properties
    QgsVectorLayerTemporalProperties *mTemporalProperties = nullptr;

    QgsVectorLayerElevationProperties *mElevationProperties = nullptr;

    //! The preview expression used to generate a human readable preview string for features
    QString mDisplayExpression;

    QString mMapTipTemplate;

    //! The user-defined actions that are accessed from the Identify Results dialog box
    QgsActionManager *mActions = nullptr;

    //! Flag indicating whether the layer is in read-only mode (editing disabled) or not
    bool mReadOnly = false;

    /**
     * Set holding the feature IDs that are activated.  Note that if a feature
     * subsequently gets deleted (i.e. by its addition to mDeletedFeatureIds),
     * it always needs to be removed from mSelectedFeatureIds as well.
     */
    QgsFeatureIds mSelectedFeatureIds;

    /**
     * Stores the previous set of selected features, to allow for "reselect" operations.
     */
    QgsFeatureIds mPreviousSelectedFeatureIds;

    //! Field map to commit
    QgsFields mFields;

    //! Map that stores the aliases for attributes. Key is the attribute name and value the alias for that attribute
    QgsStringMap mAttributeAliasMap;

    //! Map which stores default value expressions for fields
    QMap<QString, QgsDefaultValue> mDefaultExpressionMap;

    //! An internal structure to keep track of fields that have a defaultValueOnUpdate
    QSet<int> mDefaultValueOnUpdateFields;

    //! Map which stores constraints for fields
    QMap< QString, QgsFieldConstraints::Constraints > mFieldConstraints;

    //! Map which stores constraint strength for fields
    QMap< QPair< QString, QgsFieldConstraints::Constraint >, QgsFieldConstraints::ConstraintStrength > mFieldConstraintStrength;

    //! Map which stores expression constraints for fields. Value is a pair of expression/description.
    QMap< QString, QPair< QString, QString > > mFieldConstraintExpressions;

    QMap< QString, QgsField::ConfigurationFlags > mFieldConfigurationFlags;
    QMap< QString, QgsEditorWidgetSetup > mFieldWidgetSetups;

    //! Holds the configuration for the edit form
    QgsEditFormConfig mEditFormConfig;

    //! Geometry type as defined in enum WkbType (qgis.h)
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;

    //! Renderer object which holds the information about how to display the features
    QgsFeatureRenderer *mRenderer = nullptr;

    //! Simplification object which holds the information about how to simplify the features for fast rendering
    QgsVectorSimplifyMethod mSimplifyMethod;

    //! Labeling configuration
    QgsAbstractVectorLayerLabeling *mLabeling = nullptr;

    //! True if labels are enabled
    bool mLabelsEnabled = false;

    //! Whether 'labeling font not found' has be shown for this layer (only show once in QgsMessageBar, on first rendering)
    bool mLabelFontNotFoundNotified = false;

    //! Blend mode for features
    QPainter::CompositionMode mFeatureBlendMode = QPainter::CompositionMode_SourceOver;

    //! Flag if the vertex markers should be drawn only for selection (TRUE) or for all features (FALSE)
    bool mVertexMarkerOnlyForSelection = false;

    QStringList mCommitErrors;

    //! stores information about uncommitted changes to layer
    QgsVectorLayerEditBuffer *mEditBuffer = nullptr;
    friend class QgsVectorLayerEditBuffer;
    friend class QgsVectorLayerEditBufferGroup;
    friend class QgsVectorLayerEditPassthrough;

    //stores information about joined layers
    QgsVectorLayerJoinBuffer *mJoinBuffer = nullptr;

    //! stores information about expression fields on this layer
    QgsExpressionFieldBuffer *mExpressionFieldBuffer = nullptr;

    //diagram rendering object. 0 if diagram drawing is disabled
    QgsDiagramRenderer *mDiagramRenderer = nullptr;

    //stores infos about diagram placement (placement type, priority, position distance)
    QgsDiagramLayerSettings *mDiagramLayerSettings = nullptr;

    mutable bool mValidExtent = false;
    mutable bool mLazyExtent = true;

    //! Auxiliary layer
    std::unique_ptr<QgsAuxiliaryLayer> mAuxiliaryLayer;

    //! Key to use to join auxiliary layer
    QString mAuxiliaryLayerKey;

    // Features in renderer classes counted
    bool mSymbolFeatureCounted = false;

    // Feature counts for each renderer legend key
    QHash<QString, long long> mSymbolFeatureCountMap;
    QHash<QString, QgsFeatureIds> mSymbolFeatureIdMap;

    //! True while an undo command is active
    bool mEditCommandActive = false;

    //! True while a commit is active
    bool mCommitChangesActive = false;

    bool mReadExtentFromXml;
    QgsRectangle mXmlExtent;

    QgsFeatureIds mDeletedFids;

    QgsAttributeTableConfig mAttributeTableConfig;

    mutable QMutex mFeatureSourceConstructorMutex;

    QgsVectorLayerFeatureCounter *mFeatureCounter = nullptr;

    std::unique_ptr<QgsGeometryOptions> mGeometryOptions;

    bool mAllowCommit = true;

    //! Stored expression used for e.g. filter
    QgsStoredExpressionManager *mStoredExpressionManager = nullptr;

    friend class QgsVectorLayerFeatureSource;

    //! To avoid firing multiple time dataChanged signal on circular layer circular dependencies
    bool mDataChangedFired = false;

    QList<QgsWeakRelation> mWeakRelations;

    bool mSetLegendFromStyle = false;

    QList< QgsFeatureRendererGenerator * > mRendererGenerators;
};



// clazy:excludeall=qstring-allocations

#endif
