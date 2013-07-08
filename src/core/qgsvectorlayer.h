/***************************************************************************
                          qgsvectorlayer.h  -  description
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

#include <QMap>
#include <QSet>
#include <QList>
#include <QStringList>

#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfield.h"
#include "qgssnapper.h"
#include "qgsfield.h"

class QPainter;
class QImage;

class QgsAttributeAction;
class QgsCoordinateTransform;
class QgsFeatureRequest;
class QgsGeometry;
class QgsGeometryVertexIndex;
class QgsMapToPixel;
class QgsLabel;
class QgsRectangle;
class QgsVectorDataProvider;
class QgsSingleSymbolRendererV2;
class QgsRectangle;
class QgsVectorLayerJoinBuffer;
class QgsFeatureRendererV2;
class QgsDiagramRendererV2;
class QgsDiagramLayerSettings;
class QgsGeometryCache;
class QgsVectorLayerEditBuffer;
class QgsSymbolV2;

typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;

/** @note Added in 1.9 */
class CORE_EXPORT QgsAttributeEditorElement : public QObject
{
    Q_OBJECT
  public:

    enum AttributeEditorType
    {
      AeTypeContainer,
      AeTypeField,
      AeTypeInvalid
    };

    QgsAttributeEditorElement( AttributeEditorType type, QString name, QObject *parent = NULL )
        : QObject( parent ), mType( type ), mName( name ) {}

    virtual ~QgsAttributeEditorElement() {}

    QString name() const { return mName; }
    AttributeEditorType type() const { return mType; }

    virtual QDomElement toDomElement( QDomDocument& doc ) const = 0;

  protected:
    AttributeEditorType mType;
    QString mName;
};

/** @note Added in 1.9 */
class CORE_EXPORT QgsAttributeEditorContainer : public QgsAttributeEditorElement
{
  public:
    QgsAttributeEditorContainer( QString name, QObject *parent )
        : QgsAttributeEditorElement( AeTypeContainer, name, parent ) {}

    ~QgsAttributeEditorContainer() {}

    virtual QDomElement toDomElement( QDomDocument& doc ) const;
    virtual void addChildElement( QgsAttributeEditorElement *widget );
    QList<QgsAttributeEditorElement*> children() const { return mChildren; }

  private:
    QList<QgsAttributeEditorElement*> mChildren;
};

/** @note Added in 1.9 */
class CORE_EXPORT QgsAttributeEditorField : public QgsAttributeEditorElement
{
  public:
    QgsAttributeEditorField( QString name , int idx, QObject *parent )
        : QgsAttributeEditorElement( AeTypeField, name, parent ), mIdx( idx ) {}

    ~QgsAttributeEditorField() {}

    virtual QDomElement toDomElement( QDomDocument& doc ) const;
    int idx() const { return mIdx; }

  private:
    int mIdx;
};

/** @note added in 1.7 */
struct CORE_EXPORT QgsVectorJoinInfo
{
  /**Join field in the target layer*/
  QString targetFieldName;
  /**Source layer*/
  QString joinLayerId;
  /**Join field in the source layer*/
  QString joinFieldName;
  /**True if the join is cached in virtual memory*/
  bool memoryCache;
  /**Cache for joined attributes to provide fast lookup (size is 0 if no memory caching)
    @note not available in python bindings
    */
  QHash< QString, QgsAttributes> cachedAttributes;

  /**Join field index in the target layer. For backward compatibility with 1.x (x>=7)*/
  int targetFieldIndex;
  /**Join field index in the source layer. For backward compatibility with 1.x (x>=7)*/
  int joinFieldIndex;
};


/** \ingroup core
 * Represents a vector layer which manages a vector based data sets.
 *
 * The QgsVectorLayer is instantiated by specifying the name of a data provider,
 * such as postgres or wfs, and url defining the specific data set to connect to.
 * The vector layer constructor in turn instantiates a QgsVectorDataProvider subclass
 * corresponding to the provider type, and passes it the url.  The data provider
 * connects to the data source.
 *
 * The QgsVectorLayer provides a common interface to the different data types.  It also
 * manages editing transactions.
 *
 *  Sample usage of the QgsVectorLayer class:
 *
 * \code
 *     QString uri = "point?crs=epsg:4326&field=id:integer";
 *     QgsVectorLayer *scratchLayer = new QgsVectorLayer(uri, "Scratch point layer",  "memory");
 * \endcode
 *
 * The main data providers supported by QGIS are listed below.
 *
 * \section providers Vector data providers
 *
 * \subsection memory Memory data providerType (memory)
 *
 * The memory data provider is used to construct in memory data, for example scratch
 * data or data generated from spatial operations such as contouring.  There is no
 * inherent persistent storage of the data.  The data source uri is constructed.  The
 * url specifies the geometry type ("point", "linestring", "polygon",
 * "multipoint","multilinestring","multipolygon"), optionally followed by url parameters
 * as follows:
 *
 * - crs=definition
 *   Defines the coordinate reference system to use for the layer.
 *   definition is any string accepted by QgsCoordinateReferenceSystem::createFromString()
 *
 * - index=yes
 *   Specifies that the layer will be constructed with a spatial index
 *
 * - field=name:type(length,precision)
 *   Defines an attribute of the layer.  Multiple field parameters can be added
 *   to the data provider definition.  type is one of "integer", "double", "string".
 *
 * An example url is "Point?crs=epsg:4326&field=id:integer&field=name:string(20)&index=yes"
 *
 * \subsection ogr OGR data provider (ogr)
 *
 * Accesses data using the OGR drivers (http://www.gdal.org/ogr/ogr_formats.html). The url
 * is the OGR connection string.  A wide variety of data formats can be accessed using this
 * driver, including file based formats used by many GIS systems, database formats, and
 * web services.  Some of these formats are also supported by custom data providers listed
 * below.
 *
 * \subsection spatialite Spatialite data provider (spatialite)
 *
 * Access data in a spatialite database. The url defines the connection parameters, table,
 * geometry column, and other attributes.  The url can be constructed using the
 * QgsDataSourceURI class.
 *
 * \subsection postgres Postgresql data provider (postgres)
 *
 * Connects to a postgresql database.  The url defines the connection parameters, table,
 * geometry column, and other attributes.  The url can be constructed using the
 * QgsDataSourceURI class.
 *
 * \subsection mssql Microsoft SQL server data provider (mssql)
 *
 * Connects to a Microsoft SQL server database.  The url defines the connection parameters, table,
 * geometry column, and other attributes.  The url can be constructed using the
 * QgsDataSourceURI class.
 *
 * \subsection sqlanywhere SQL Anywhere data provider (sqlanywhere)
 *
 * Connects to an SQLanywhere database.  The url defines the connection parameters, table,
 * geometry column, and other attributes.  The url can be constructed using the
 * QgsDataSourceURI class.
 *
 * \subsection wfs WFS (web feature service) data provider (wfs)
 *
 * Used to access data provided by a web feature service.
 *
 * The url can be a HTTP url to a WFS 1.0.0 server or a GML2 data file path.
 * Examples are http://foobar/wfs or /foo/bar/file.gml
 *
 * If a GML2 file path is provided the driver will attempt to read the schema from a
 * file in the same directory with the same basename + “.xsd”. This xsd file must be
 * in the same format as a WFS describe feature type response. If no xsd file is provide
 * then the driver will attempt to guess the attribute types from the file.
 *
 * In the case of a HTTP URL the ‘FILTER’ query string parameter can be used to filter
 * the WFS feature type. The ‘FILTER’ key value can either be a QGIS expression
 * or an OGC XML filter. If the value is set to a QGIS expression the driver will
 * turn it into OGC XML filter before passing it to the WFS server. Beware the
 * QGIS expression filter only supports” =, != ,<,> ,<= ,>= ,AND ,OR ,NOT, LIKE, IS NULL”
 * attribute operators, “BBOX, Disjoint, Intersects, Touches, Crosses, Contains, Overlaps, Within”
 * spatial binary operators and the QGIS local “geomFromWKT, geomFromGML”
 * geometry constructor functions.
 *
 * Also note:
 *
 * - You can use various functions available in the QGIS Expression list,
 *   however the function must exist server side and have the same name and arguments to work.
 *
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
 *   -  "csv" splits the file based on three sets of characters:
 *      delimiter characters, quote characters,
 *      and escape characters.  Delimiter characters mark the end
 *      of a field. Quote characters enclose a field which can contain
 *      delimiter characters, and newlines.  Escape characters cause the
 *      following character to be treated literally (including delimiter,
 *      quote, and newline characters).  Escape and quote characters must
 *      be different from delimiter characters. Escape characters that are
 *      also quote characters are treated specially - they can only
 *      escape themselves within quotes.  Elsewhere they are treated as
 *      quote characters.  The defaults for delimiter, quote, and escape
 *      are ',', '"', '"'.
 *   -  "regexp" splits each record using a regular expression (see QRegExp
 *      documentation for details).
 *   -  "whitespace" splits each record based on whitespace (on or more whitespace
 *      characters.  Leading whitespace in the record is ignored.
 *   -  "plain" is provided for backwards compatibility.  It is equivalent to
 *      CSV except that the default quote characters are single and double quotes,
 *      and there is no escape characters.
 *
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
 *   If yes then empty fields will be discarded (eqivalent to concatenating consecutive
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
 * -subsetIndex=(yes|no)
 *
 *   Determines whether the provider generates an index to improve the efficiency
 *   of subsets.  The default is yes
 *
 * -spatialIndex=(yes|no)
 *
 *   Determines whether the provider generates a spatial index.  The default is no.
 *
 * -watchFile=(yes|no)
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
 */


class CORE_EXPORT QgsVectorLayer : public QgsMapLayer
{
    Q_OBJECT

  public:
    /** The different types to layout the attribute editor. @note added in 1.9 */
    enum EditorLayout
    {
      GeneratedLayout = 0,
      TabLayout = 1,
      UiFileLayout = 2
    };

    enum EditType
    {
      LineEdit,
      UniqueValues,
      UniqueValuesEditable,
      ValueMap,
      Classification,
      EditRange,
      SliderRange,
      CheckBox,      /* added in 1.4 */
      FileName,
      Enumeration,
      Immutable,     /* The attribute value should not be changed in the attribute form */
      Hidden,        /* The attribute value should not be shown in the attribute form @added in 1.4 */
      TextEdit,      /* multiline edit @added in 1.4*/
      Calendar,      /* calendar widget @added in 1.5 */
      DialRange,     /* dial range @added in 1.5 */
      ValueRelation, /* value map from an table @added in 1.8 */
      UuidGenerator, /* uuid generator - readonly and automatically intialized @added in 1.9 */
      Photo,         /* phote widget @added in 1.9 */
      WebView,       /* webview widget @added in 1.9 */
      Color,         /* color @added in 1.9 */
    };

    struct RangeData
    {
      RangeData() {}
      RangeData( QVariant theMin, QVariant theMax, QVariant theStep )
          : mMin( theMin ), mMax( theMax ), mStep( theStep ) {}

      QVariant mMin;
      QVariant mMax;
      QVariant mStep;
    };

    struct ValueRelationData
    {
      ValueRelationData() {}
      ValueRelationData( QString layer, QString key, QString value, bool allowNull, bool orderByValue,
                         bool allowMulti = false,
                         QString filterExpression = QString::null )
          : mLayer( layer )
          , mKey( key )
          , mValue( value )
          , mFilterExpression( filterExpression )
          , mAllowNull( allowNull )
          , mOrderByValue( orderByValue )
          , mAllowMulti( allowMulti )
      {}

      QString mLayer;
      QString mKey;
      QString mValue;
      QString mFilterExpression;
      bool mAllowNull;
      bool mOrderByValue;
      bool mAllowMulti;  /* allow selection of multiple keys @added in 1.9 */
    };

    struct GroupData
    {
      GroupData() {}
      GroupData( QString name , QList<QString> fields )
          : mName( name ), mFields( fields ) {}
      QString mName;
      QList<QString> mFields;
    };

    struct TabData
    {
      TabData() {}
      TabData( QString name , QList<QString> fields , QList<GroupData> groups )
          : mName( name ), mFields( fields ), mGroups( groups ) {}
      QString mName;
      QList<QString> mFields;
      QList<GroupData> mGroups;
    };

    /** Constructor - creates a vector layer
     *
     * The QgsVectorLayer is constructed by instantiating a data provider.  The provider
     * interprets the supplied path (url) of the data source to connect to and access the
     * data.
     *
     * @param  path  The path or url of the parameter.  Typically this encodes
     *               parameters used by the data provider as url query items.
     * @param  baseName The name used to represent the layer in the legend
     * @param  providerLib  The name of the data provider, eg "memory", "postgres"
     * @param  loadDefaultStyleFlag whether to load the default style
     *
     */
    QgsVectorLayer( QString path = QString::null, QString baseName = QString::null,
                    QString providerLib = QString::null, bool loadDefaultStyleFlag = true );

    /** Destructor */
    virtual ~QgsVectorLayer();

    /** Returns the permanent storage type for this layer as a friendly name. */
    QString storageType() const;

    /** Capabilities for this layer in a friendly format. */
    QString capabilitiesString() const;

    /** Returns a comment for the data in the layer */
    QString dataComment() const;

    /** Set the primary display field to be used in the identify results dialog */
    void setDisplayField( QString fldName = "" );

    /** Returns the primary display field name used in the identify results dialog */
    const QString displayField() const;

    /** Set the preview expression, used to create a human readable preview string.
     *  Used e.g. in the attribute table feature list. Uses { @link QgsExpression }.
     *
     *  @param displayExpression The expression which will be used to preview features
     *                           for this layer
     *  @note added in 2.0
     */
    void setDisplayExpression( const QString displayExpression );

    /**
     *  Get the preview expression, used to create a human readable preview string.
     *  Uses { @link QgsExpression }
     *
     *  @return The expression which will be used to preview features for this layer
     *
     *  @note added in 2.0
     */
    const QString displayExpression();

    /** Returns the data provider */
    QgsVectorDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner
        @note not available in python bindings
      */
    const QgsVectorDataProvider* dataProvider() const;

    /** Sets the textencoding of the data provider */
    void setProviderEncoding( const QString& encoding );

    /** Setup the coordinate system tranformation for the layer */
    void setCoordinateSystem();

    /** Joins another vector layer to this layer
      @param joinInfo join object containing join layer id, target and source field
      @note added in 1.7 */
    void addJoin( const QgsVectorJoinInfo& joinInfo );

    /** Removes  a vector layer join
      @note added in 1.7 */
    void removeJoin( const QString& joinLayerId );

    /** @note added in 1.7 */
    const QList< QgsVectorJoinInfo >& vectorJoins() const;

    /** Get the label object associated with this layer */
    QgsLabel *label();

    const QgsLabel *label() const;

    QgsAttributeAction *actions() { return mActions; }

    /**
     * The number of features that are selected in this layer
     *
     * @return See description
     */
    int selectedFeatureCount();

    /**
     * Select features found within the search rectangle (in layer's coordinates)
     *
     * @param rect            The search rectangle
     * @param addToSelection  If set to true will not clear before selecting
     *
     * @see   invertSelectionInRectangle(QgsRectangle & rect)
     */
    void select( QgsRectangle & rect, bool addToSelection );

    /**
     * Modifies the current selection on this layer
     *
     * @param selectIds    Select these ids
     * @param deselectIds  Deselect these ids
     *
     * @see   select(QgsFeatureIds)
     * @see   select(QgsFeatureId)
     * @see   deselect(QgsFeatureIds)
     * @see   deselect(QgsFeatureId)
     */
    void modifySelection( QgsFeatureIds selectIds, QgsFeatureIds deselectIds );

    /** Select not selected features and deselect selected ones */
    void invertSelection();

    /** Select all the features */
    void selectAll();

    /**
     * Invert selection of features found within the search rectangle (in layer's coordinates)
     *
     * @param rect  The rectangle in which the selection of features will be inverted
     *
     * @see   invertSelection()
     */
    void invertSelectionInRectangle( QgsRectangle & rect );

    /**
     * Get a copy of the user-selected features
     *
     * @return A list of { @link QgsFeature } 's
     *
     * @see    selectedFeaturesIds()
     */
    QgsFeatureList selectedFeatures();

    /**
     * Return reference to identifiers of selected features
     *
     * @return A list of { @link QgsFeatureId } 's
     * @see selectedFeatures()
     */
    const QgsFeatureIds &selectedFeaturesIds() const;

    /**
     * Change selection to the new set of features. Dismisses the current selection.
     * Will emit the { @link selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) } signal with the
     * clearAndSelect flag set.
     *
     * @param ids   The ids which will be the new selection
     */
    void setSelectedFeatures( const QgsFeatureIds &ids );

    /** Returns the bounding box of the selected features. If there is no selection, QgsRectangle(0,0,0,0) is returned */
    QgsRectangle boundingBoxOfSelected();

    /** Sets diagram rendering object (takes ownership) */
    void setDiagramRenderer( QgsDiagramRendererV2* r );
    const QgsDiagramRendererV2* diagramRenderer() const { return mDiagramRenderer; }

    void setDiagramLayerSettings( const QgsDiagramLayerSettings& s );
    const QgsDiagramLayerSettings *diagramLayerSettings() const { return mDiagramLayerSettings; }

    /** Return renderer V2.
     * @note added in 1.4 */
    QgsFeatureRendererV2* rendererV2();
    /** Set renderer V2.
     * @note added in 1.4
     */
    void setRendererV2( QgsFeatureRendererV2* r );

    /** Draw layer with renderer V2. QgsFeatureRenderer::startRender() needs to be called before using this method
     * @note added in 1.4
     */
    void drawRendererV2( QgsFeatureIterator &fit, QgsRenderContext& rendererContext, bool labeling );

    /** Draw layer with renderer V2 using symbol levels. QgsFeatureRenderer::startRender() needs to be called before using this method
     * @note added in 1.4
     */
    void drawRendererV2Levels( QgsFeatureIterator &fit, QgsRenderContext& rendererContext, bool labeling );

    /** Returns point, line or polygon */
    QGis::GeometryType geometryType() const;

    /** Returns true if this is a geometry layer and false in case of NoGeometry (table only) or UnknownGeometry
     * @note added in 1.7
     */
    bool hasGeometryType() const;

    /**Returns the WKBType or WKBUnknown in case of error*/
    QGis::WkbType wkbType() const;

    /** Return the provider type for this layer */
    QString providerType() const;

    /** reads vector layer specific state from project file Dom node.
     *  @note Called by QgsMapLayer::readXML().
     */
    virtual bool readXml( const QDomNode& layer_node );

    /** write vector layer specific state to project file Dom node.
     *  @note Called by QgsMapLayer::writeXML().
     */
    virtual bool writeXml( QDomNode & layer_node, QDomDocument & doc );

    /**
     * Save named and sld style of the layer to the style table in the db.
     * @param name
     * @param description
     * @param useAsDefault
     * @param uiFileContent
     * @param msgError
     */
    virtual void saveStyleToDatabase( QString name, QString description,
                                      bool useAsDefault, QString uiFileContent,
                                      QString &msgError );

    /**
     * Lists all the style in db split into related to the layer and not related to
     * @param ids the list in which will be stored the style db ids
     * @param names the list in which will be stored the style names
     * @param descriptions the list in which will be stored the style descriptions
     * @param msgError
     * @return the number of styles related to current layer
     */
    virtual int listStylesInDatabase( QStringList &ids, QStringList &names,
                                      QStringList &descriptions, QString &msgError );

    /**
     * Will return the named style corresponding to style id provided
     */
    virtual QString getStyleFromDatabase( QString styleId, QString &msgError );

    /**
     * Load a named style from file/local db/datasource db
     * @param theURI the URI of the style or the URI of the layer
     * @param theResultFlag will be set to true if a named style is correctly loaded
     * @param loadFromLocalDb if true forces to load from local db instead of datasource one
     */
    virtual QString loadNamedStyle( const QString theURI, bool &theResultFlag, bool loadFromLocalDb );

    /**
     * Calls loadNamedStyle( theURI, theResultFlag, false );
     * Retained for backward compatibility
     */
    virtual QString loadNamedStyle( const QString theURI, bool &theResultFlag );

    virtual bool applyNamedStyle( QString namedStyle , QString errorMsg );

    /** convert a saved attribute editor element into a AttributeEditor structure as it's used internally.
     * @param elem the DOM element
     * @param parent the QObject which will own this object
     */
    QgsAttributeEditorElement* attributeEditorElementFromDomElement( QDomElement &elem, QObject* parent );

    /** Read the symbology for the current layer from the Dom node supplied.
     * @param node node that will contain the symbology definition for this layer.
     * @param errorMessage reference to string that will be updated with any error messages
     * @return true in case of success.
     */
    bool readSymbology( const QDomNode& node, QString& errorMessage );

    /** Write the symbology for the layer into the docment provided.
     *  @param node the node that will have the style element added to it.
     *  @param doc the document that will have the QDomNode added.
     *  @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     */
    bool writeSymbology( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;

    bool writeSld( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;
    bool readSld( const QDomNode& node, QString& errorMessage );

    /**
     * Number of features in the layer. This is necessary if features are
     * added/deleted or the layer has been subsetted. If the data provider
     * chooses not to support this feature, the total number of features
     * can be returned.
     * @return long containing number of features
     */
    virtual long featureCount() const;

    /**
     * Number of features rendered with specified symbol. Features must be first
     * calculated by countSymbolFeatures()
     * @param symbol the symbol
     * @return number of features rendered by symbol or -1 if failed or counts are not available
     */
    long featureCount( QgsSymbolV2* symbol );

    /**
     * Count features for symbols. Feature counts may be get by featureCount( QgsSymbolV2*).
     * @param showProgress show progress dialog
     * @return true if calculated, false if failed or was canceled by user
     */
    bool countSymbolFeatures( bool showProgress = true );

    /**
     * Set the string (typically sql) used to define a subset of the layer
     * @param subset The subset string. This may be the where clause of a sql statement
     *               or other defintion string specific to the underlying dataprovider
     *               and data store.
     * @return true, when setting the subset string was successful, false otherwise (added in 1.4)
     */
    virtual bool setSubsetString( QString subset );

    /**
     * Get the string (typically sql) used to define a subset of the layer
     * @return The subset string or QString::null if not implemented by the provider
     */
    virtual QString subsetString();

    /**
     * Query the provider for features specified in request.
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() );

    /** Adds a feature
        @param f feature to add
        @param alsoUpdateExtent If True, will also go to the effort of e.g. updating the extents.
        @return                    True in case of success and False in case of error
     */
    bool addFeature( QgsFeature& f, bool alsoUpdateExtent = true );

    /** Updates an existing feature
        @param f feature to update
        @return                    True in case of success and False in case of error
        @note added in 1.8
     */
    bool updateFeature( QgsFeature &f );

    /** Insert a new vertex before the given vertex number,
     *  in the given ring, item (first number is index 0), and feature
     *  Not meaningful for Point geometries
     */
    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );

    /** Moves the vertex at the given position number,
     *  ring and item (first number is index 0), and feature
     *  to the given coordinates
     */
    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );

    /** Deletes a vertex from a feature
     */
    bool deleteVertex( QgsFeatureId atFeatureId, int atVertex );

    /** Deletes the selected features
     *  @return true in case of success and false otherwise
     */
    bool deleteSelectedFeatures();

    /**Adds a ring to polygon/multipolygon features
     @return
       0 in case of success,
       1 problem with feature type,
       2 ring not closed,
       3 ring not valid,
       4 ring crosses existing rings,
       5 no feature found where ring can be inserted
       6 layer not editable */
    int addRing( const QList<QgsPoint>& ring );

    /**Adds a new part polygon to a multipart feature
     @return
       0 in case of success,
       1 if selected feature is not multipart,
       2 if ring is not a valid geometry,
       3 if new polygon ring not disjoint with existing rings,
       4 if no feature was selected,
       5 if several features are selected,
       6 if selected geometry not found
       7 layer not editable */
    int addPart( const QList<QgsPoint>& ring );

    /**Translates feature by dx, dy
       @param featureId id of the feature to translate
       @param dx translation of x-coordinate
       @param dy translation of y-coordinate
       @return 0 in case of success*/
    int translateFeature( QgsFeatureId featureId, double dx, double dy );

    /**Splits features cut by the given line
     *  @param splitLine line that splits the layer features
     *  @param topologicalEditing true if topological editing is enabled
     *  @return
     *   0 in case of success,
     *   4 if there is a selection but no feature split
     */
    int splitFeatures( const QList<QgsPoint>& splitLine, bool topologicalEditing = false );

    /**Changes the specified geometry such that it has no intersections with other
     *  polygon (or multipolygon) geometries in this vector layer
     *  @param geom geometry to modify
     *  @param ignoreFeatures list of feature ids where intersections should be ignored
     *  @return 0 in case of success
     */
    int removePolygonIntersections( QgsGeometry* geom, QgsFeatureIds ignoreFeatures = QgsFeatureIds() );

    /** Adds topological points for every vertex of the geometry.
     * @param geom the geometry where each vertex is added to segments of other features
     * @note geom is not going to be modified by the function
     * @return 0 in case of success
     */
    int addTopologicalPoints( QgsGeometry* geom );

    /** Adds a vertex to segments which intersect point p but don't
     * already have a vertex there. If a feature already has a vertex at position p,
     * no additional vertex is inserted. This method is useful for topological
     * editing.
     * @param p position of the vertex
     * @return 0 in case of success
     */
    int addTopologicalPoints( const QgsPoint& p );

    /**Inserts vertices to the snapped segments.
     * This is useful for topological editing if snap to segment is enabled.
     * @param snapResults results collected from the snapping operation
     * @return 0 in case of success
     */
    int insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults );

    /** Set labels on */
    void enableLabels( bool on );

    /** Label is on */
    bool hasLabelsEnabled() const;

    /** Returns true if the provider is in editing mode */
    virtual bool isEditable() const;

    /** Returns true if the provider is in read-only mode
     * @note added in 1.6
     */
    virtual bool isReadOnly() const;

    /** Returns true if the provider has been modified since the last commit */
    virtual bool isModified() const;

    /**Snaps a point to the closest vertex if there is one within the snapping tolerance
     *  @param point       The point which is set to the position of a vertex if there is one within the snapping tolerance.
     *  If there is no point within this tolerance, point is left unchanged.
     *  @param tolerance   The snapping tolerance
     *  @return true if the point has been snapped, false if no vertex within search tolerance
     */
    bool snapPoint( QgsPoint& point, double tolerance );

    /**Snaps to segment or vertex within given tolerance
     * @param startPoint point to snap (in layer coordinates)
     * @param snappingTolerance distance tolerance for snapping
     * @param snappingResults snapping results. Key is the distance between startPoint and snapping target
     * @param snap_to to segment / to vertex
     * @return 0 in case of success
     */
    int snapWithContext( const QgsPoint& startPoint,
                         double snappingTolerance,
                         QMultiMap < double, QgsSnappingResult > &snappingResults,
                         QgsSnapper::SnappingType snap_to );

    /**Synchronises with changes in the datasource
      @note added in version 1.6*/
    virtual void reload();

    /** Draws the layer
     *  @return false if an error occurred during drawing
     */
    bool draw( QgsRenderContext& rendererContext );

    /** Draws the layer labels using coordinate transformation */
    void drawLabels( QgsRenderContext& rendererContext );

    /** Return the extent of the layer as a QRect */
    QgsRectangle extent();

    /** returns field list in the to-be-committed state */
    const QgsFields &pendingFields() const;

    /** returns list of attributes */
    QgsAttributeList pendingAllAttributesList();

    /** returns list of attribute making up the primary key
     * @note added in 2.0
     */
    QgsAttributeList pendingPkAttributesList();

    /** returns feature count after commit */
    int pendingFeatureCount();

    /** Make layer read-only (editing disabled) or not
     *  @return false if the layer is in editing yet
     *  @note added in 1.6
     */
    bool setReadOnly( bool readonly = true );

    /** Make layer editable */
    bool startEditing();

    /** change feature's geometry
      @note added in version 1.2 */
    bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom );

    /** changed an attribute value (but does not commit it) */
    bool changeAttributeValue( QgsFeatureId fid, int field, QVariant value, bool emitSignal = true );

    /** add an attribute field (but does not commit it)
        returns true if the field was added
      @note added in version 1.2 */
    bool addAttribute( const QgsField &field );

    /**Sets an alias (a display name) for attributes to display in dialogs
      @note added in version 1.2*/
    void addAttributeAlias( int attIndex, QString aliasString );

    /**Adds a tab (for the attribute editor form) holding groups and fields
      @note added in version 1.9*/
    void addAttributeEditorWidget( QgsAttributeEditorElement* data );
    /**Returns a list of tabs holding groups and fields
      @note added in version 1.9*/
    QList< QgsAttributeEditorElement* > &attributeEditorElements();
    /**Clears all the tabs for the attribute editor form
      @note added in version 1.9*/
    void clearAttributeEditorWidgets();

    /**Returns the alias of an attribute name or an empty string if there is no alias
      @note added in version 1.2*/
    QString attributeAlias( int attributeIndex ) const;

    /**Convenience function that returns the attribute alias if defined or the field name else
      @note added in version 1.2*/
    QString attributeDisplayName( int attributeIndex ) const;

    const QMap< QString, QString >& attributeAliases() const { return mAttributeAliasMap; }

    const QSet<QString>& excludeAttributesWMS() const { return mExcludeAttributesWMS; }
    void setExcludeAttributesWMS( const QSet<QString>& att ) { mExcludeAttributesWMS = att; }

    const QSet<QString>& excludeAttributesWFS() const { return mExcludeAttributesWFS; }
    void setExcludeAttributesWFS( const QSet<QString>& att ) { mExcludeAttributesWFS = att; }

    /** delete an attribute field (but does not commit it) */
    bool deleteAttribute( int attr );

    /**
     * Deletes a list of attribute fields (but does not commit it)
     *
     * @param  attrs the indices of the attributes to delete
     * @return true if at least one attribute has been deleted
     *
     */
    bool deleteAttributes( QList<int> attrs );

    /** Insert a copy of the given features into the layer  (but does not commit it) */
    bool addFeatures( QgsFeatureList features, bool makeSelected = true );

    /** delete a feature from the layer (but does not commit it) */
    bool deleteFeature( QgsFeatureId fid );

    /**
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
     */
    bool commitChanges();
    const QStringList &commitErrors();

    /** Stop editing and discard the edits
     * @param deleteBuffer whether to delete editing buffer (added in 1.9)
     */
    bool rollBack( bool deleteBuffer = true );

    /**get edit type*/
    EditType editType( int idx );

    /**set edit type*/
    void setEditType( int idx, EditType edit );

    /** get the active layout for the attribute editor for this layer (added in 1.9) */
    EditorLayout editorLayout();

    /** set the active layout for the attribute editor for this layer (added in 1.9) */
    void setEditorLayout( EditorLayout editorLayout );

    /** set string representing 'true' for a checkbox (added in 1.4) */
    void setCheckedState( int idx, QString checked, QString notChecked );

    /** return string representing 'true' for a checkbox (added in 1.4)
     * @note not available in python bindings
     * FIXME: need SIP binding for QPair<QString, QString>
     */
    QPair<QString, QString> checkedState( int idx );

    /** get edit form (added in 1.4) */
    QString editForm();

    /** set edit form (added in 1.4) */
    void setEditForm( QString ui );

    /** get annotation form (added in 1.5)*/
    QString annotationForm() const { return mAnnotationForm; }

    /** set annotation form for layer (added in 1.5)*/
    void setAnnotationForm( const QString& ui );

    /** get python function for edit form initialization (added in 1.4) */
    QString editFormInit();

    /** set python function for edit form initialization (added in 1.4) */
    void setEditFormInit( QString function );

    /**access value map*/
    QMap<QString, QVariant> &valueMap( int idx );

    /**access range */
    RangeData &range( int idx );

    /**access relations
     * @note added in 1.8
     **/
    ValueRelationData &valueRelation( int idx );

    /**access date format
     * @note added in 1.9
     */
    QString &dateFormat( int idx );

    /**access widget size for photo and webview widget
     * @note added in 1.9
     */
    QSize &widgetSize( int idx );

    /**is edit widget editable
     * @note added in 1.9
     **/
    bool fieldEditable( int idx );

    /**label widget on top
     * @note added in 1.9
     **/
    bool labelOnTop( int idx );

    /**set edit widget editable
     * @note added in 1.9
     **/
    void setFieldEditable( int idx, bool editable );

    /**label widget on top
     * @note added in 1.9
     **/
    void setLabelOnTop( int idx, bool onTop );

    //! Buffer with uncommitted editing operations. Only valid after editing has been turned on.
    QgsVectorLayerEditBuffer* editBuffer() { return mEditBuffer; }

    /**
     * Create edit command for undo/redo operations
     * @param text text which is to be displayed in undo window
     */
    void beginEditCommand( QString text );

    /** Finish edit command and add it to undo/redo stack */
    void endEditCommand();

    /** Destroy active command and reverts all changes in it */
    void destroyEditCommand();

    /** Returns the index of a field name or -1 if the field does not exist
      @note this method was added in version 1.4
     */
    int fieldNameIndex( const QString& fieldName ) const;

    /** Editing vertex markers
      @note public from version 1.4 */
    enum VertexMarkerType
    {
      SemiTransparentCircle,
      Cross,
      NoMarker  /* added in version 1.1 */
    };

    /** Draws a vertex symbol at (screen) coordinates x, y. (Useful to assist vertex editing.)
      @note public and static from version 1.4 */
    static void drawVertexMarker( double x, double y, QPainter& p, QgsVectorLayer::VertexMarkerType type, int vertexSize );

    /** Assembles mUpdatedFields considering provider fields, joined fields and added fields
     @note added in 1.7 */
    void updateFields();

    /** Caches joined attributes if required (and not already done)
      @note added in 1.7 */
    void createJoinCaches();

    /**Returns unique values for column
      @param index column index for attribute
      @param uniqueValues out: result list
      @param limit maximum number of values to return (-1 if unlimited)
      @note this method was added in version 1.7 */
    void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 );

    /**Returns minimum value for an attribute column or invalid variant in case of error
      @note added in 1.7*/
    QVariant minimumValue( int index );

    /**Returns maximum value for an attribute column or invalid variant in case of error
      @note added in 1.7*/
    QVariant maximumValue( int index );

    /* Set the blending mode used for rendering each feature
     * @note added in 2.0
     */
    void setFeatureBlendMode( const QPainter::CompositionMode blendMode );
    /* Returns the current blending mode for features
     * @note added in 2.0
     */
    QPainter::CompositionMode featureBlendMode() const;

    /* Set the transparency for the vector layer
     * @note added in 2.0
     */
    void setLayerTransparency( int layerTransparency );
    /* Returns the current transparency for the vector layer
     * @note added in 2.0
     */
    int layerTransparency() const;

  public slots:
    /**
     * Select feature by its ID
     *
     * @param featureId  The id of the feature to select
     *
     * @see select(QgsFeatureIds)
     */
    void select( const QgsFeatureId &featureId );

    /**
     * Select features by their ID
     *
     * @param featureIds The ids of the features to select
     *
     * @see select(QgsFeatureId)
     */
    void select( const QgsFeatureIds& featureIds );

    /**
     * Deselect feature by its ID
     *
     * @param featureId  The id of the feature to deselect
     *
     * @see deselect(QgsFeatureIds)
     */
    void deselect( const QgsFeatureId featureId );

    /**
     * Deselect features by their ID
     *
     * @param featureIds The ids of the features to deselect
     *
     * @see deselect(QgsFeatureId)
     */
    void deselect( const QgsFeatureIds& featureIds );

    /**
     * Clear selection
     *
     * @see setSelectedFeatures(const QgsFeatureIds&)
     */
    void removeSelection();

    void triggerRepaint();

    /** Update the extents for the layer. This is necessary if features are
     *  added/deleted or the layer has been subsetted.
     */
    virtual void updateExtents();

    /** Check if there is a join with a layer that will be removed
      @note added in 1.7 */
    void checkJoinLayerRemove( QString theLayerId );

    QString metadata();

    /** @note not available in python bindings */
    inline QgsGeometryCache* cache() { return mCache; }

    /**
     * @brief Is called when the cache image is being deleted. Overwrite and use to clean up.
     * @note added in 2.0
     */
    virtual void onCacheImageDelete();

  signals:

    /**
     * This signal is emitted when selection was changed
     *
     * @param selected        Newly selected feature ids
     * @param deselected      Ids of all features which have previously been selected but are not any more
     * @param clearAndSelect  In case this is set to true, the old selection was dismissed and the new selection corresponds to selected
     */
    void selectionChanged( const QgsFeatureIds selected, const QgsFeatureIds deselected, const bool clearAndSelect );

    /** This signal is emitted when selection was changed */
    void selectionChanged();

    /** This signal is emitted when modifications has been done on layer */
    void layerModified();

    /** Is emitted, when editing on this layer has started*/
    void editingStarted();

    /** Is emitted, when edited changes successfully have been written to the data provider */
    void editingStopped();

    /** Is emitted, before changes are commited to the data provider */
    void beforeCommitChanges();

    /** Is emitted, before changes are rolled back*/
    void beforeRollBack();

    /**
     * Will be emitted, when a new attribute has been added to this vector layer.
     * Applies only to types {@link QgsFields::OriginEdit} and {@link QgsFields::OriginProvider}
     *
     * @param idx The index of the new attribute
     *
     * @see updatedFields()
     */
    void attributeAdded( int idx );
    /**
     * Will be emitted, when an attribute has been deleted from this vector layer.
     * Applies only to types {@link QgsFields::OriginEdit} and {@link QgsFields::OriginProvider}
     *
     * @param idx The index of the deleted attribute
     *
     * @see updatedFields()
     */
    void attributeDeleted( int idx );
    void featureAdded( QgsFeatureId fid );  // added in 1.7
    void featureDeleted( QgsFeatureId fid );
    /**
     * Is emitted, whenever the fields available from this layer have been changed.
     * This can be due to manually adding attributes or due to a join.
     *
     * @note Added in 2.0
     */
    void updatedFields();
    void layerDeleted();

    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant & );
    void geometryChanged( QgsFeatureId fid, QgsGeometry &geom ); // added in 1.9

    /** Signals emitted after committing changes
      \note added in v1.6 */
    void committedAttributesDeleted( const QString& layerId, const QgsAttributeList& deletedAttributes );
    void committedAttributesAdded( const QString& layerId, const QList<QgsField>& addedAttributes );
    void committedFeaturesAdded( const QString& layerId, const QgsFeatureList& addedFeatures );
    void committedFeaturesRemoved( const QString& layerId, const QgsFeatureIds& deletedFeatureIds );
    void committedAttributeValuesChanges( const QString& layerId, const QgsChangedAttributesMap& changedAttributesValues );
    void committedGeometriesChanges( const QString& layerId, const QgsGeometryMap& changedGeometries );

    /** Emitted when the font family defined for labeling layer is not found on system
     * @note added in 1.9
     */
    void labelingFontNotFound( QgsVectorLayer* layer, const QString& fontfamily );

  protected:
    /** Set the extent */
    void setExtent( const QgsRectangle &rect );

  private:                       // Private methods

    /** vector layers are not copyable */
    QgsVectorLayer( const QgsVectorLayer & rhs );

    /** vector layers are not copyable */
    QgsVectorLayer & operator=( QgsVectorLayer const & rhs );

    /** bind layer to a specific data provider
       @param provider should be "postgres", "ogr", or ??
       @todo XXX should this return bool?  Throw exceptions?
    */
    bool setDataProvider( QString const & provider );

    /** Goes through all features and finds a free id (e.g. to give it temporarily to a not-commited feature) */
    QgsFeatureId findFreeId();

    /**Snaps to a geometry and adds the result to the multimap if it is within the snapping result
     @param startPoint start point of the snap
     @param featureId id of feature
     @param geom geometry to snap
     @param sqrSnappingTolerance squared search tolerance of the snap
     @param snappingResults list to which the result is appended
     @param snap_to snap to vertex or to segment
    */
    void snapToGeometry( const QgsPoint& startPoint,
                         QgsFeatureId featureId,
                         QgsGeometry* geom,
                         double sqrSnappingTolerance,
                         QMultiMap<double, QgsSnappingResult>& snappingResults,
                         QgsSnapper::SnappingType snap_to ) const;

    /**Reads vertex marker type from settings*/
    static QgsVectorLayer::VertexMarkerType currentVertexMarkerType();

    /**Reads vertex marker size from settings*/
    static int currentVertexMarkerSize();

    /** Add joined attributes to a feature */
    //void addJoinedAttributes( QgsFeature& f, bool all = false );

    /** Stop version 2 renderer and selected renderer (if required) */
    void stopRendererV2( QgsRenderContext& rendererContext, QgsSingleSymbolRendererV2* selRenderer );

    /**Registers label and diagram layer
      @param rendererContext render context
      @param attributes attributes needed for labeling and diagrams will be added to the list
      @param labeling out: true if there will be labeling (ng) for this layer*/
    void prepareLabelingAndDiagrams( QgsRenderContext& rendererContext, QgsAttributeList& attributes, bool& labeling );

  private:                       // Private attributes

    /** Update threshold for drawing features as they are read. A value of zero indicates
     *  that no features will be drawn until all have been read
     */
    int mUpdateThreshold;

    /** Enables backbuffering for the map window. This improves graphics performance,
     *  but the possibility to cancel rendering and incremental feature drawing will be lost.
     *
     */
    bool mEnableBackbuffer;

    /** Pointer to data provider derived from the abastract base class QgsDataProvider */
    QgsVectorDataProvider *mDataProvider;

    QgsFeatureIterator mProviderIterator;

    /** index of the primary label field */
    QString mDisplayField;

    /** the preview expression used to generate a human readable preview string for features */
    QString mDisplayExpression;

    /** Data provider key */
    QString mProviderKey;

    /** The user-defined actions that are accessed from the Identify Results dialog box */
    QgsAttributeAction* mActions;

    /** Flag indicating whether the layer is in read-only mode (editing disabled) or not */
    bool mReadOnly;

    /** Set holding the feature IDs that are activated.  Note that if a feature
        subsequently gets deleted (i.e. by its addition to mDeletedFeatureIds),
        it always needs to be removed from mSelectedFeatureIds as well.
     */
    QgsFeatureIds mSelectedFeatureIds;

    /** field map to commit */
    QgsFields mUpdatedFields;

    /**Map that stores the aliases for attributes. Key is the attribute name and value the alias for that attribute*/
    QMap< QString, QString > mAttributeAliasMap;

    /**Stores a list of attribute editor elements (Each holding a tree structure for a tab in the attribute editor)*/
    QList< QgsAttributeEditorElement* > mAttributeEditorElements;

    /**Attributes which are not published in WMS*/
    QSet<QString> mExcludeAttributesWMS;
    /**Attributes which are not published in WFS*/
    QSet<QString> mExcludeAttributesWFS;

    /**Map that stores the tab for attributes in the edit form. Key is the tab order and value the tab name*/
    QList< TabData > mTabs;

    /** Geometry type as defined in enum WkbType (qgis.h) */
    int mWkbType;

    /** Renderer object which holds the information about how to display the features */
    QgsFeatureRendererV2 *mRendererV2;

    /** Label */
    QgsLabel *mLabel;

    /** Display labels */
    bool mLabelOn;

    /** Whether 'labeling font not found' has be shown for this layer (only show once in QgsMessageBar, on first rendering) */
    bool mLabelFontNotFoundNotified;

    /** Blend mode for features */
    QPainter::CompositionMode mFeatureBlendMode;

    /** Layer transparency */
    int mLayerTransparency;

    /**The current type of editing marker*/
    QgsVectorLayer::VertexMarkerType mCurrentVertexMarkerType;

    /** The current size of editing marker */
    int mCurrentVertexMarkerSize;

    /** Flag if the vertex markers should be drawn only for selection (true) or for all features (false) */
    bool mVertexMarkerOnlyForSelection;

    QStringList mCommitErrors;

    QMap< QString, EditType > mEditTypes;
    QMap< QString, bool> mFieldEditables;
    QMap< QString, bool> mLabelOnTop;
    QMap< QString, QMap<QString, QVariant> > mValueMaps;
    QMap< QString, RangeData > mRanges;
    QMap< QString, QPair<QString, QString> > mCheckedStates;
    QMap< QString, ValueRelationData > mValueRelations;
    QMap< QString, QString> mDateFormats;
    QMap< QString, QSize> mWidgetSize;

    /** Defines the default layout to use for the attribute editor (Drag and drop, UI File, Generated) */
    EditorLayout mEditorLayout;

    QString mEditForm, mEditFormInit;
    //annotation form for this layer
    QString mAnnotationForm;

    //! cache for some vector layer data - currently only geometries for faster editing
    QgsGeometryCache* mCache;

    //! stores information about uncommitted changes to layer
    QgsVectorLayerEditBuffer* mEditBuffer;
    friend class QgsVectorLayerEditBuffer;

    //stores information about joined layers
    QgsVectorLayerJoinBuffer* mJoinBuffer;

    //diagram rendering object. 0 if diagram drawing is disabled
    QgsDiagramRendererV2* mDiagramRenderer;

    //stores infos about diagram placement (placement type, priority, position distance)
    QgsDiagramLayerSettings *mDiagramLayerSettings;

    bool mValidExtent;

    // Features in renderer classes counted
    bool mSymbolFeatureCounted;

    // Feature counts for each renderer symbol
    QMap<QgsSymbolV2*, long> mSymbolFeatureCountMap;

    QgsRenderContext* mCurrentRendererContext;

    friend class QgsVectorLayerFeatureIterator;
};

#endif
