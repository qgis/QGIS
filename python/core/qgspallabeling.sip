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
      OnLine    = 1,
      AboveLine = 2,
      BelowLine = 4,
      MapOrientation = 8
    };

    enum UpsideDownLabels
    {
      Upright, // upside-down labels (90 <= angle < 270) are shown upright
      ShowDefined, // show upside down when rotation is layer- or data-defined
      ShowAll // show upside down for all labels, including dynamic ones
    };

    enum DirectionSymbols
    {
      SymbolLeftRight, // place direction symbols on left/right of label
      SymbolAbove, // place direction symbols on above label
      SymbolBelow // place direction symbols on below label
    };

    enum MultiLineAlign
    {
      MultiLeft = 0,
      MultiCenter,
      MultiRight
    };

    enum ShapeType
    {
      ShapeRectangle = 0,
      ShapeSquare,
      ShapeEllipse,
      ShapeCircle,
      ShapeSVG
    };

    enum SizeType
    {
      SizeBuffer = 0,
      SizeFixed,
      SizePercent
    };

    enum RotationType
    {
      RotationSync = 0,
      RotationOffset,
      RotationFixed
    };

    /** Units used for option sizes, before being converted to rendered sizes */
    enum SizeUnit
    {
      Points = 0,
      MM,
      MapUnits,
      Percent
    };

    // update mDataDefinedNames QList in constructor when adding/deleting enum value
    enum DataDefinedProperties
    {
      Size = 0,
      Bold,
      Italic,
      Underline,
      Color,
      Strikeout,
      Family,
      BufferSize,
      BufferColor,
      PositionX, //x-coordinate data defined label position
      PositionY, //y-coordinate data defined label position
      Hali, //horizontal alignment for data defined label position (Left, Center, Right)
      Vali, //vertical alignment for data defined label position (Bottom, Base, Half, Cap, Top)
      LabelDistance,
      Rotation, //data defined rotation (only useful in connection with data defined position)
      Show,
      MinScale,
      MaxScale,
      FontTransp,
      BufferTransp,
      AlwaysShow
    };

    QString fieldName;

    /** Is this label made from a expression string eg FieldName || 'mm'
      */
    bool isExpression;

    /** Returns the QgsExpression for this label settings.
      */
    QgsExpression* getLabelExpression();

    Placement placement;
    unsigned int placementFlags;
    // offset labels of point/centroid features default to center
    // move label to quadrant: left/down, don't move, right/up (-1, 0, 1)
    int xQuadOffset;
    int yQuadOffset;

    // offset from point in mm or map units
    double xOffset;
    double yOffset;
    double angleOffset; // rotation applied to offset labels
    bool centroidWhole; // whether centroid calculated from whole or visible polygon
    QFont textFont;
    QString textNamedStyle;
    QColor textColor;
    int textTransp;
    QPainter::CompositionMode blendMode;
    QColor previewBkgrdColor;
    bool enabled;
    int priority; // 0 = low, 10 = high
    bool obstacle; // whether it's an obstacle
    double dist; // distance from the feature (in mm)
    double vectorScaleFactor; //scale factor painter units->pixels
    double rasterCompressFactor; //pixel resolution scale factor

    // disabled if both are zero
    int scaleMin;
    int scaleMax;
    double bufferSize; //buffer size
    QColor bufferColor;
    int bufferTransp;
    QPainter::CompositionMode bufferBlendMode;
    Qt::PenJoinStyle bufferJoinStyle;
    bool bufferNoFill; //set interior of buffer to 100% transparent

    // shape background
    bool shapeDraw;
    ShapeType shapeType;
    QString shapeSVGFile;
    SizeType shapeSizeType;
    QPointF shapeSize;
    SizeUnit shapeSizeUnits;
    RotationType shapeRotationType;
    double shapeRotation;
    QPointF shapeOffset;
    SizeUnit shapeOffsetUnits;
    QPointF shapeRadii;
    SizeUnit shapeRadiiUnits;
    QColor shapeFillColor;
    QColor shapeBorderColor;
    double shapeBorderWidth;
    SizeUnit shapeBorderWidthUnits;
    Qt::PenJoinStyle shapeJoinStyle;
    int shapeTransparency;
    QPainter::CompositionMode shapeBlendMode;

    bool formatNumbers;
    int decimals;
    bool plusSign;
    bool labelPerPart; // whether to label every feature's part or only the biggest one
    bool displayAll;  // if true, all features will be labelled even though overlaps occur
    bool mergeLines;
    double minFeatureSize; // minimum feature size to be labelled (in mm)
    bool limitNumLabels; // whether to limit the number of labels to be drawn
    int maxNumLabels; // maximum number of labels to be drawn
    // Adds '<' or '>', or user-defined symbol to the label string pointing to the
    // direction of the line / polygon ring
    // Works only if Placement == Line
    bool addDirectionSymbol;
    QString leftDirectionSymbol;
    QString rightDirectionSymbol;
    bool reverseDirectionSymbol;
    DirectionSymbols placeDirectionSymbol; // whether to place left/right, above or below label
    unsigned int upsidedownLabels; // whether, or how, to show upsidedown labels
    double maxCurvedCharAngleIn; // maximum angle between inside curved label characters (defaults to 20.0, range 20.0 to 60.0)
    double maxCurvedCharAngleOut; // maximum angle between outside curved label characters (defaults to -20.0, range -20.0 to -95.0)
    bool fontSizeInMapUnits; //true if font size is in map units (otherwise in points)
    bool fontLimitPixelSize; // true is label should be limited by fontMinPixelSize/fontMaxPixelSize
    int fontMinPixelSize; // minimum pixel size for showing rendered map unit labels (1 - 1000)
    int fontMaxPixelSize; // maximum pixel size for showing rendered map unit labels (1 - 10000)
    bool bufferSizeInMapUnits; //true if buffer is in map units (otherwise in mm)
    bool labelOffsetInMapUnits; //true if label offset is in map units (otherwise in mm)
    bool distInMapUnits; //true if distance is in map units (otherwise in mm)
    QString wrapChar;
    double multilineHeight; //0.0 to 10.0, leading between lines as multiplyer of line height
    MultiLineAlign multilineAlign; // horizontal alignment of multi-line labels
    // called from register feature hook
    void calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY );

    // implementation of register feature hook
    void registerFeature( QgsVectorLayer* layer, QgsFeature& f, const QgsRenderContext& context );

    void readFromLayer( QgsVectorLayer* layer );
    void writeToLayer( QgsVectorLayer* layer );

    /**Set a property as data defined*/
    void setDataDefinedProperty( DataDefinedProperties p, QString attributeName );
    /**Set a property to static instead data defined*/
    void removeDataDefinedProperty( DataDefinedProperties p );

    /**Stores field names for data defined layer properties*/
    // QMap< DataDefinedProperties, QString > dataDefinedProperties;

    bool preserveRotation; // preserve predefined rotation data during label pin/unpin operations

    /** Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @return font pixel size
     */
    int sizeToPixel( double size, const QgsRenderContext& c , SizeUnit unit, bool rasterfactor ) const;

    /** List of data defined enum names
     * @note adding in 1.9
     */
    QList<QString> dataDefinedNames() const;
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

class QgsPalLabeling : QgsLabelingEngineInterface
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End

  public:
    enum DrawLabelType
    {
      LabelText = 0,
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

    bool isShowingAllLabels() const;
    void setShowingAllLabels( bool showing );

    // implemented methods from labeling engine interface

    //! called when we're going to start with rendering
    virtual void init( QgsMapRenderer* mr );
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer );
    //! hook called when drawing layer before issuing select()
    virtual int prepareLayer( QgsVectorLayer* layer, QSet<int>& attrIndices, QgsRenderContext& ctx );
    //! adds a diagram layer to the labeling engine
    virtual int addDiagramLayer( QgsVectorLayer* layer, QgsDiagramLayerSettings *s );
    //! hook called when drawing for every feature in a layer
    virtual void registerFeature( QgsVectorLayer* layer, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() );
    virtual void registerDiagramFeature( QgsVectorLayer* layer, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() );
    //! called when the map is drawn and labels should be placed
    virtual void drawLabeling( QgsRenderContext& context );
    //! called when we're done with rendering
    virtual void exit();
    //! return infos about labels at a given (map) position
    virtual QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p );
    //! return infos about labels within a given (map) rectangle
    virtual QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r );

    //! called when passing engine among map renderers
    virtual QgsLabelingEngineInterface* clone() /Factory/;

/*
    void drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform );
    //!drawLabel
    void drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmplyr, DrawLabelType drawType );
*/
    static void drawLabelBuffer( QgsRenderContext& context, QString text, const QgsPalLayerSettings& tmpLyr );

    static void drawLabelBackground( QgsRenderContext& context,
                                     const QgsPoint& centerPt, double labelRotation, double labelWidth, double labelHeight,
                                     const QgsPalLayerSettings& tmpLyr );

    //! load/save engine settings to project file
    //! @note added in QGIS 1.9
    void loadEngineSettings();
    void saveEngineSettings();
    void clearEngineSettings();
    bool isStoredWithProject() const;
    void setStoredWithProject( bool store );
};
