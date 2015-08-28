// QMap<QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined*> is implemented as a Python dictionary.
%MappedType QMap<QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined*> /DocType="dict-of-QgsPalLayerSettings.DataDefinedProperties-QgsDataDefined*"/
{
%TypeHeaderCode
#include <qmap.h>
#include <qgspallabeling.h>
#include <qgsdatadefined.h>
%End
%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();
    if (!d)
        return NULL;
    // Set the dictionary elements.
    QMap<QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined*>::const_iterator i = sipCpp->constBegin();
    while (i != sipCpp->constEnd())
    {
      QgsDataDefined *t = i.value();
      PyObject *kobj = sipConvertFromEnum(i.key(), sipType_QgsPalLayerSettings_DataDefinedProperties);
      PyObject *tobj = sipConvertFromType(t, sipType_QgsDataDefined, sipTransferObj);
      if (kobj == NULL || tobj == NULL || PyDict_SetItem(d, kobj, tobj) < 0)
      {
	Py_DECREF(d);
	if (kobj)
	{
	  Py_DECREF(kobj);
	}
	if (tobj)
	{
	  Py_DECREF(tobj);
	}
	else
	{
	  delete t;
	}
	return NULL;
      }
      Py_DECREF(kobj);
      Py_DECREF(tobj);
      ++i;
    }
    return d;
%End
%ConvertToTypeCode
    PyObject *kobj, *tobj;
    SIP_SSIZE_T i = 0;
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;
        while (PyDict_Next(sipPy, &i, &kobj, &tobj))
            if (!sipCanConvertToType(tobj, sipType_QgsDataDefined, SIP_NOT_NONE))
                return 0;
        return 1;
    }
    QMap<QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined*> *qm = new QMap<QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined*>;

    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
        int state, k = SIPLong_AsLong(kobj);
        QgsDataDefined *t = reinterpret_cast<QgsDataDefined *>(sipConvertToType(tobj, sipType_QgsDataDefined, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t, sipType_QgsDataDefined, state);
            delete qm;
            return 0;
        }
        qm->insert(QgsPalLayerSettings::DataDefinedProperties(k), t);
        sipReleaseType(t, sipType_QgsDataDefined, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

class QgsPalLayerSettings
{
%TypeHeaderCode
#include <qgspallabeling.h>
#include <qgsdiagramrendererv2.h>
%End
  public:
    QgsPalLayerSettings();
    QgsPalLayerSettings( const QgsPalLayerSettings& s );
    ~QgsPalLayerSettings();

    //! @note added in 2.4
    static QgsPalLayerSettings fromLayer( QgsVectorLayer* layer );

    enum Placement
    {
      AroundPoint, // Point / Polygon
      OverPoint, // Point / Polygon
      Line, // Line / Polygon
      Curved, // Line
      Horizontal, // Polygon
      Free // Polygon
    };

    enum LinePlacementFlags
    {
      OnLine,
      AboveLine,
      BelowLine,
      MapOrientation,
    };

    enum QuadrantPosition
    {
      QuadrantAboveLeft,
      QuadrantAbove,
      QuadrantAboveRight,
      QuadrantLeft,
      QuadrantOver,
      QuadrantRight,
      QuadrantBelowLeft,
      QuadrantBelow,
      QuadrantBelowRight
    };

    enum UpsideDownLabels
    {
      Upright, /*!< upside-down labels (90 <= angle < 270) are shown upright */
      ShowDefined, /*!< show upside down when rotation is layer- or data-defined */
      ShowAll /*!< show upside down for all labels, including dynamic ones */
    };

    enum DirectionSymbols
    {
      SymbolLeftRight, /*!< place direction symbols on left/right of label */
      SymbolAbove, /*!< place direction symbols on above label */
      SymbolBelow /*!< place direction symbols on below label */
    };

    enum MultiLineAlign
    {
      MultiLeft,
      MultiCenter,
      MultiRight,
      MultiFollowPlacement /*!< Alignment follows placement of label, eg labels to the left of a feature
                               will be drawn with right alignment*/
    };

    /** Valid obstacle types, which affect how features within the layer will act as obstacles
     * for labels.
     */
    enum ObstacleType
    {
      PolygonInterior, /*!< avoid placing labels over interior of polygon (prefer placing labels totally
       outside or just slightly inside polygon) */
      PolygonBoundary /*!< avoid placing labels over boundary of polygon (prefer placing outside or
       completely inside polygon) */
    };

    enum ShapeType
    {
      ShapeRectangle,
      ShapeSquare,
      ShapeEllipse,
      ShapeCircle,
      ShapeSVG
    };

    enum SizeType
    {
      SizeBuffer,
      SizeFixed,
      SizePercent
    };

    enum RotationType
    {
      RotationSync,
      RotationOffset,
      RotationFixed
    };

    /** Units used for option sizes, before being converted to rendered sizes */
    enum SizeUnit
    {
      Points,
      MM,
      MapUnits,
      Percent
    };

    enum ShadowType
    {
      ShadowLowest,
      ShadowText,
      ShadowBuffer,
      ShadowShape
    };

    // update mDataDefinedNames QMap in constructor when adding/deleting enum value
    enum DataDefinedProperties
    {
      // text style
      Size,
      Bold,
      Italic,
      Underline,
      Color,
      Strikeout,
      Family,
      FontStyle,
      FontSizeUnit,
      FontTransp,
      FontCase,
      FontLetterSpacing,
      FontWordSpacing,
      FontBlendMode,

      // text formatting
      MultiLineWrapChar,
      MultiLineHeight,
      MultiLineAlignment,
      DirSymbDraw,
      DirSymbLeft,
      DirSymbRight,
      DirSymbPlacement,
      DirSymbReverse,
      NumFormat,
      NumDecimals,
      NumPlusSign,

      // text buffer
      BufferDraw,
      BufferSize,
      BufferUnit,
      BufferColor,
      BufferTransp,
      BufferJoinStyle,
      BufferBlendMode,

      // background
      ShapeDraw,
      ShapeKind,
      ShapeSVGFile,
      ShapeSizeType,
      ShapeSizeX,
      ShapeSizeY,
      ShapeSizeUnits,
      ShapeRotationType,
      ShapeRotation,
      ShapeOffset,
      ShapeOffsetUnits,
      ShapeRadii,
      ShapeRadiiUnits,
      ShapeTransparency,
      ShapeBlendMode,
      ShapeFillColor,
      ShapeBorderColor,
      ShapeBorderWidth,
      ShapeBorderWidthUnits,
      ShapeJoinStyle,

      // drop shadow
      ShadowDraw,
      ShadowUnder,
      ShadowOffsetAngle,
      ShadowOffsetDist,
      ShadowOffsetUnits,
      ShadowRadius,
      ShadowRadiusUnits,
      ShadowTransparency,
      ShadowScale,
      ShadowColor,
      ShadowBlendMode,

      // placement
      CentroidWhole,
      OffsetQuad,
      OffsetXY,
      OffsetUnits,
      LabelDistance,
      DistanceUnits,
      OffsetRotation,
      CurvedCharAngleInOut,
      // (data defined only)
      PositionX, //x-coordinate data defined label position
      PositionY, //y-coordinate data defined label position
      Hali, //horizontal alignment for data defined label position (Left, Center, Right)
      Vali, //vertical alignment for data defined label position (Bottom, Base, Half, Cap, Top)
      Rotation, //data defined rotation
      RepeatDistance,
      RepeatDistanceUnit,
      Priority,

      // rendering
      ScaleVisibility,
      MinScale,
      MaxScale,
      FontLimitPixel,
      FontMinPixel,
      FontMaxPixel,
      IsObstacle,
      ObstacleFactor,

      // (data defined only)
      Show,
      AlwaysShow
    };

    // whether to label this layer
    bool enabled;

    /** Whether to draw labels for this layer. For some layers it may be desirable
     * to register their features as obstacles for other labels without requiring
     * labels to be drawn for the layer itself. In this case drawLabels can be set
     * to false and obstacle set to true, which will result in the layer acting
     * as an obstacle but having no labels of its own.
     * @note added in QGIS 2.12
     */
    bool drawLabels;

    //-- text style

    QString fieldName;

    /** Is this label made from a expression string eg FieldName || 'mm'
      */
    bool isExpression;

    /** Returns the QgsExpression for this label settings.
      */
    QgsExpression* getLabelExpression();

    QFont textFont;
    QString textNamedStyle;
    bool fontSizeInMapUnits; //true if font size is in map units (otherwise in points)
    QgsMapUnitScale fontSizeMapUnitScale; // scale range for map units for font size
    QColor textColor;
    int textTransp;
    QPainter::CompositionMode blendMode;
    QColor previewBkgrdColor;

    //-- text formatting

    QString wrapChar;
    double multilineHeight; //0.0 to 10.0, leading between lines as multiplyer of line height
    MultiLineAlign multilineAlign; // horizontal alignment of multi-line labels

    // Adds '<' or '>', or user-defined symbol to the label string pointing to the
    // direction of the line / polygon ring
    // Works only if Placement == Line
    bool addDirectionSymbol;
    QString leftDirectionSymbol;
    QString rightDirectionSymbol;
    DirectionSymbols placeDirectionSymbol; // whether to place left/right, above or below label
    bool reverseDirectionSymbol;

    bool formatNumbers;
    int decimals;
    bool plusSign;

    //-- text buffer

    bool bufferDraw;
    double bufferSize; // buffer size
    bool bufferSizeInMapUnits; //true if buffer is in map units (otherwise in mm)
    QgsMapUnitScale bufferSizeMapUnitScale; // scale range for map units for buffer size
    QColor bufferColor;
    bool bufferNoFill; //set interior of buffer to 100% transparent
    int bufferTransp;
    Qt::PenJoinStyle bufferJoinStyle;
    QPainter::CompositionMode bufferBlendMode;

    //-- shape background

    bool shapeDraw;
    ShapeType shapeType;
    QString shapeSVGFile;
    SizeType shapeSizeType;
    QPointF shapeSize;
    SizeUnit shapeSizeUnits;
    QgsMapUnitScale shapeSizeMapUnitScale;
    RotationType shapeRotationType;
    double shapeRotation;
    QPointF shapeOffset;
    SizeUnit shapeOffsetUnits;
    QgsMapUnitScale shapeOffsetMapUnitScale;
    QPointF shapeRadii;
    SizeUnit shapeRadiiUnits;
    QgsMapUnitScale shapeRadiiMapUnitScale;
    int shapeTransparency;
    QPainter::CompositionMode shapeBlendMode;
    QColor shapeFillColor;
    QColor shapeBorderColor;
    double shapeBorderWidth;
    SizeUnit shapeBorderWidthUnits;
    QgsMapUnitScale shapeBorderWidthMapUnitScale;
    Qt::PenJoinStyle shapeJoinStyle;

    //-- drop shadow

    bool shadowDraw;
    ShadowType shadowUnder;
    int shadowOffsetAngle;
    double shadowOffsetDist;
    SizeUnit shadowOffsetUnits;
    QgsMapUnitScale shadowOffsetMapUnitScale;
    bool shadowOffsetGlobal;
    double shadowRadius;
    SizeUnit shadowRadiusUnits;
    QgsMapUnitScale shadowRadiusMapUnitScale;
    bool shadowRadiusAlphaOnly;
    int shadowTransparency;
    int shadowScale;
    QColor shadowColor;
    QPainter::CompositionMode shadowBlendMode;

    //-- placement

    Placement placement;
    unsigned int placementFlags;

    bool centroidWhole; // whether centroid calculated from whole or visible polygon
    bool centroidInside; // whether centroid-point calculated must be inside polygon

    /** True if only labels which completely fit within a polygon are allowed.
     */
    bool fitInPolygonOnly;
    double dist; // distance from the feature (in mm)
    bool distInMapUnits; //true if distance is in map units (otherwise in mm)
    QgsMapUnitScale distMapUnitScale;

    double repeatDistance;
    SizeUnit repeatDistanceUnit;
    QgsMapUnitScale repeatDistanceMapUnitScale;

    // offset labels of point/centroid features default to center
    // move label to quadrant: left/down, don't move, right/up (-1, 0, 1)
    QuadrantPosition quadOffset;

    double xOffset; // offset from point in mm or map units
    double yOffset; // offset from point in mm or map units
    bool labelOffsetInMapUnits; //true if label offset is in map units (otherwise in mm)
    QgsMapUnitScale labelOffsetMapUnitScale;
    double angleOffset; // rotation applied to offset labels
    bool preserveRotation; // preserve predefined rotation data during label pin/unpin operations

    double maxCurvedCharAngleIn; // maximum angle between inside curved label characters (defaults to 20.0, range 20.0 to 60.0)
    double maxCurvedCharAngleOut; // maximum angle between outside curved label characters (defaults to -20.0, range -20.0 to -95.0)

    int priority; // 0 = low, 10 = high

    //-- rendering

    bool scaleVisibility;
    int scaleMin;
    int scaleMax;

    bool fontLimitPixelSize; // true is label should be limited by fontMinPixelSize/fontMaxPixelSize
    int fontMinPixelSize; // minimum pixel size for showing rendered map unit labels (1 - 1000)
    int fontMaxPixelSize; // maximum pixel size for showing rendered map unit labels (1 - 10000)

    bool displayAll;  // if true, all features will be labelled even though overlaps occur
    unsigned int upsidedownLabels; // whether, or how, to show upsidedown labels

    bool labelPerPart; // whether to label every feature's part or only the biggest one
    bool mergeLines;

    bool limitNumLabels; // whether to limit the number of labels to be drawn
    int maxNumLabels; // maximum number of labels to be drawn

    double minFeatureSize; // minimum feature size to be labelled (in mm)
    bool obstacle; // whether features for layer are obstacles to labels of other layers

    /** Obstacle factor, where 1.0 = default, < 1.0 more likely to be covered by labels,
     * > 1.0 less likely to be covered
     */
    double obstacleFactor;

    /** Controls how features act as obstacles for labels
     */
    ObstacleType obstacleType;

    //-- scale factors
    double vectorScaleFactor; //scale factor painter units->pixels
    double rasterCompressFactor; //pixel resolution scale factor

    // called from register feature hook
    void calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY, QgsFeature* f = 0, const QgsRenderContext* context = 0 );

    /** Register a feature for labelling.
     * @param f feature to label
     * @param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     * @param dxfLayer dxfLayer name
     */
    void registerFeature( QgsFeature& f, const QgsRenderContext& context, QString dxfLayer );

    void readFromLayer( QgsVectorLayer* layer );
    void writeToLayer( QgsVectorLayer* layer );

    /** Get a data defined property pointer
     * @note helpful for Python access
     */
    QgsDataDefined* dataDefinedProperty( QgsPalLayerSettings::DataDefinedProperties p );

    /** Set a property as data defined
     * @note helpful for Python access
     */
    void setDataDefinedProperty( QgsPalLayerSettings::DataDefinedProperties p,
                                 bool active, bool useExpr, const QString& expr, const QString& field );

    /** Set a property to static instead data defined */
    void removeDataDefinedProperty( QgsPalLayerSettings::DataDefinedProperties p );

    /** Convert old property value to new one as delimited values
     * @note not available in python bindings; as temporary solution until refactoring of project settings
     */
    QString updateDataDefinedString( const QString& value );

    /** Get property value as separate values split into Qmap
     * @note not available in python bindings
     */
    QMap<QString, QString> dataDefinedMap( QgsPalLayerSettings::DataDefinedProperties p ) const;

    /** Get data defined property value from expression string or attribute field name
     * @returns value inside QVariant
     * @note not available in python bindings
     */
    //QVariant dataDefinedValue( QgsPalLayerSettings::DataDefinedProperties p, QgsFeature& f, const QgsFields& fields,
    //                           const QgsExpressionContext* context = 0 ) const;

    /** Get data defined property value from expression string or attribute field name
     * @returns true/false whether result is null or invalid
     * @note not available in python bindings
     */
    //bool dataDefinedEvaluate( QgsPalLayerSettings::DataDefinedProperties p, QVariant& exprVal, const QgsExpressionContext* context = 0 ) const;

    /** Whether data definition is active
     */
    bool dataDefinedIsActive( QgsPalLayerSettings::DataDefinedProperties p ) const;

    /** Whether data definition is set to use an expression
     */
    bool dataDefinedUseExpression( QgsPalLayerSettings::DataDefinedProperties p ) const;

    /** Map of current data defined properties
     */
    QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > dataDefinedProperties;


    /** Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @param mapUnitScale a mapUnitScale clamper
     * @return font pixel size
     */
    int sizeToPixel( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor = false, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() ) const;

    /** Calculates size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @param mapUnitScale a mapUnitScale clamper
     * @return size that will render, as double
     */
    double scaleToPixelContext( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor = false, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() ) const;

    /** Map of data defined enum to names and old-style indecies
     * The QPair contains a new string for layer property key, and a reference to old-style numeric key (< QGIS 2.0)
     * @note not available in python bindings;
     */
    // QMap<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > dataDefinedNames() const;
};

class QgsLabelCandidate
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End

  public:
    QgsLabelCandidate( QRectF r, double c );

    QRectF rect;
    double cost;
};

/** \ingroup core
  * Maintains current state of more grainular and temporal values when creating/painting
  * component parts of an individual label (e.g. buffer, background, shadow, etc.).
  */
class QgsLabelComponent
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End

  public:
    QgsLabelComponent();

    // methods

    const QString& text() const;
    void setText( const QString& text );

    const QgsPoint& origin() const;
    void setOrigin( const QgsPoint& point );

    bool useOrigin() const;
    void setUseOrigin( const bool use );

    double rotation() const;
    void setRotation( const double rotation );

    double rotationOffset() const;
    void setRotationOffset( const double rotation );

    bool useRotation() const;
    void setUseRotation( const bool use );

    const QgsPoint& center() const;
    void setCenter( const QgsPoint& point );

    bool useCenter() const;
    void setUseCenter( const bool use );

    const QgsPoint& size() const;
    void setSize( const QgsPoint& point );

    const QgsPoint& offset() const;
    void setOffset( const QgsPoint& point );

    const QPicture* picture() const;
    void setPicture( QPicture* picture );

    double pictureBuffer() const;
    void setPictureBuffer( const double buffer );

    double dpiRatio() const;
    void setDpiRatio( const double ratio );
};


/**
 * Class that stores computed placement from labeling engine.
 * @note added in 2.4
 */
class QgsLabelingResults
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End

  public:
    QgsLabelingResults();
    ~QgsLabelingResults();

    //! return infos about labels at a given (map) position
    QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p ) const;
    //! return infos about labels within a given (map) rectangle
    QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r ) const;

  private:
    QgsLabelingResults( const QgsLabelingResults& );
};


class QgsPalLabeling : QgsLabelingEngineInterface
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End

  public:
    enum DrawLabelType
    {
      LabelText,
      LabelBuffer,
      LabelShape,
      LabelSVG,
      LabelShadow
    };

    QgsPalLabeling();
    ~QgsPalLabeling();

    QgsPalLayerSettings& layer( const QString& layerName );

    void numCandidatePositions( int& candPoint, int& candLine, int& candPolygon );
    void setNumCandidatePositions( int candPoint, int candLine, int candPolygon );

    enum Search { Chain, Popmusic_Tabu, Popmusic_Chain, Popmusic_Tabu_Chain, Falp };

    void setSearchMethod( Search s );
    Search searchMethod() const;

    bool isShowingCandidates() const;
    void setShowingCandidates( bool showing );
    const QList<QgsLabelCandidate>& candidates();

    bool isShowingShadowRectangles() const;
    void setShowingShadowRectangles( bool showing );

    bool isShowingAllLabels() const;
    void setShowingAllLabels( bool showing );

    bool isShowingPartialsLabels() const;
    void setShowingPartialsLabels( bool showing );

    //! @note added in 2.4
    bool isDrawingOutlineLabels() const;
    void setDrawingOutlineLabels( bool outline );

    /** Returns whether the engine will only draw the outline rectangles of labels,
     * not the label contents themselves. Used for debugging and testing purposes.
     * @see setDrawLabelRectOnly
     * @note added in QGIS 2.12
     */
    bool drawLabelRectOnly() const;

    /** Sets whether the engine should only draw the outline rectangles of labels,
     * not the label contents themselves. Used for debugging and testing purposes.
     * @param drawRect set to true to enable rect drawing only
     * @see drawLabelRectOnly
     * @note added in QGIS 2.12
     */
    void setDrawLabelRectOnly( bool drawRect );

    // implemented methods from labeling engine interface

    //! called when we're going to start with rendering
    //! @deprecated since 2.4 - use override with QgsMapSettings
    virtual void init( QgsMapRenderer* mr ) /Deprecated/;
    //! called when we're going to start with rendering
    virtual void init( const QgsMapSettings& mapSettings );
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer );

    //! called to find out whether the layer is used for labeling
    //! @note added in 2.4
    static bool staticWillUseLayer( QgsVectorLayer* layer );
    static bool staticWillUseLayer( const QString& layerID );

    //! clears all PAL layer settings for registered layers
    virtual void clearActiveLayers();
    //! clears data defined objects from PAL layer settings for a registered layer
    virtual void clearActiveLayer( const QString& layerID );
    //! hook called when drawing layer before issuing select()
    virtual int prepareLayer( QgsVectorLayer* layer, QStringList &attrNames, QgsRenderContext& ctx );
    //! adds a diagram layer to the labeling engine
    virtual int addDiagramLayer( QgsVectorLayer* layer, const QgsDiagramLayerSettings *s );

    /** Register a feature for labelling.
     * @param layerID string identifying layer associated with label
     * @param feat feature to label
     * @param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     * @param dxfLayer dxfLayer name
     */
    virtual void registerFeature( const QString& layerID, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext(), QString dxfLayer = QString::null );

    virtual void registerDiagramFeature( const QString& layerID, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() );
    //! called when the map is drawn and labels should be placed
    virtual void drawLabeling( QgsRenderContext& context );
    //! called when we're done with rendering
    virtual void exit();

    //! return infos about labels at a given (map) position
    //! @deprecated since 2.4 - use takeResults() and methods of QgsLabelingResults
    virtual QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p ) /Deprecated/;
    //! return infos about labels within a given (map) rectangle
    //! @deprecated since 2.4 - use takeResults() and methods of QgsLabelingResults
    virtual QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r ) /Deprecated/;

    //! Return pointer to recently computed results (in drawLabeling()) and pass the ownership of results to the caller
    //! @note added in 2.4
    QgsLabelingResults* takeResults() /TransferBack/;

    //! called when passing engine among map renderers
    virtual QgsLabelingEngineInterface* clone() /Factory/;

    //! @note not available in python bindings
    // void drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform );
    //!drawLabel
    //! @note not available in python bindings
    // void drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, DrawLabelType drawType );

    static void drawLabelBuffer( QgsRenderContext& context,
                                 const QgsLabelComponent &component,
                                 const QgsPalLayerSettings& tmpLyr );

    static void drawLabelBackground( QgsRenderContext& context,
                                     QgsLabelComponent component,
                                     const QgsPalLayerSettings& tmpLyr );

    static void drawLabelShadow( QgsRenderContext &context,
                                 const QgsLabelComponent &component,
                                 const QgsPalLayerSettings& tmpLyr );

    //! load/save engine settings to project file
    void loadEngineSettings();
    void saveEngineSettings();
    void clearEngineSettings();
    //! @deprecated since 2.4 - settings are always stored in project
    bool isStoredWithProject() const /Deprecated/;
    //! @deprecated since 2.4 - settings are always stored in project
    void setStoredWithProject( bool store ) /Deprecated/;

    /** Prepares a geometry for registration with PAL. Handles reprojection, rotation, clipping, etc.
     * @param geometry geometry to prepare
     * @param context render context
     * @param ct coordinate transform
     * @param clipGeometry geometry to clip features to, if applicable
     * @returns prepared geometry
     * @note added in QGIS 2.9
     */
    static QgsGeometry* prepareGeometry( const QgsGeometry *geometry, const QgsRenderContext &context, const QgsCoordinateTransform *ct, QgsGeometry *clipGeometry = 0 ) /Factory/;

    /** Checks whether a geometry requires preparation before registration with PAL
     * @param geometry geometry to prepare
     * @param context render context
     * @param ct coordinate transform
     * @param clipGeometry geometry to clip features to, if applicable
     * @returns true if geometry requires preparation
     * @note added in QGIS 2.9
     */
    static bool geometryRequiresPreparation( const QgsGeometry *geometry, const QgsRenderContext &context, const QgsCoordinateTransform *ct, QgsGeometry *clipGeometry = 0 );

    /** Splits a text string to a list of separate lines, using a specified wrap character.
     * The text string will be split on either newline characters or the wrap character.
     * @param text text string to split
     * @param wrapCharacter additional character to wrap on
     * @returns list of text split to lines
     * @note added in QGIS 2.9
     */
    static QStringList splitToLines( const QString& text, const QString& wrapCharacter );

    /** Splits a text string to a list of graphemes, which are the smallest allowable character
     * divisions in the string. This accounts for scripts were individual characters are not
     * allowed to be split apart (eg Arabic and Indic based scripts)
     * @param text string to split
     * @returns list of graphemes
     * @note added in QGIS 2.10
     */
    static QStringList splitToGraphemes( const QString& text );

  protected:
    // update temporary QgsPalLayerSettings with any data defined text style values
    void dataDefinedTextStyle( QgsPalLayerSettings& tmpLyr,
                               const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined text formatting values
    void dataDefinedTextFormatting( QgsPalLayerSettings& tmpLyr,
                                    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined text buffer values
    void dataDefinedTextBuffer( QgsPalLayerSettings& tmpLyr,
                                const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined shape background values
    void dataDefinedShapeBackground( QgsPalLayerSettings& tmpLyr,
                                     const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined drop shadow values
    void dataDefinedDropShadow( QgsPalLayerSettings& tmpLyr,
                                const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues );

    void deleteTemporaryData();

    /** Checks whether a geometry exceeds the minimum required size for a geometry to be labeled.
     * @param context render context
     * @param geom geometry
     * @param minSize minimum size for geometry
     * @returns true if geometry exceeds minimum size
     * @note added in QGIS 2.9
     */
    static bool checkMinimumSizeMM( const QgsRenderContext &context, const QgsGeometry *geom, double minSize );
};
