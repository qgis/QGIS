class QgsLabelPosition
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End
  public:
    QgsLabelPosition( int id, double r, const QVector< QgsPoint >& corners, const QgsRectangle& rect, double w, double h, const QString& layer, const QString& labeltext, const QFont& labelfont, bool upside_down, bool diagram = false, bool pinned = false, const QString& providerId = QString() );
    QgsLabelPosition();
    int featureId;
    double rotation;
    QVector< QgsPoint > cornerPoints;
    QgsRectangle labelRect;
    double width;
    double height;
    QString layerID;
    QString labelText;
    QFont labelFont;
    bool upsideDown;
    bool isDiagram;
    bool isPinned;
    //! @note added in 2.14
    QString providerID;
};


class QgsPalLayerSettings
{
%TypeHeaderCode
#include <qgspallabeling.h>
#include <qgsdiagramrenderer.h>
%End
  public:
    QgsPalLayerSettings();
    QgsPalLayerSettings( const QgsPalLayerSettings& s );
    ~QgsPalLayerSettings();

    //! @note added in 2.4
    static QgsPalLayerSettings fromLayer( QgsVectorLayer* layer );

    /** Placement modes which determine how label candidates are generated for a feature.
     */
    //TODO QGIS 3.0 - move to QgsLabelingEngine
    enum Placement
    {
      AroundPoint, /**< Arranges candidates in a circle around a point (or centroid of a polygon). Applies to point or polygon layers only.*/
      OverPoint, /**  Arranges candidates over a point (or centroid of a polygon), or at a preset offset from the point. Applies to point or polygon layers only.*/
      Line, /**< Arranges candidates parallel to a generalised line representing the feature or parallel to a polygon's perimeter. Applies to line or polygon layers only. */
      Curved, /** Arranges candidates following the curvature of a line feature. Applies to line layers only.*/
      Horizontal, /**< Arranges horizontal candidates scattered throughout a polygon feature. Applies to polygon layers only.*/
      Free, /**< Arranges candidates scattered throughout a polygon feature. Candidates are rotated to respect the polygon's orientation. Applies to polygon layers only.*/
      OrderedPositionsAroundPoint, /**< Candidates are placed in predefined positions around a point. Preference is given to positions with greatest cartographic appeal, e.g., top right, bottom right, etc. Applies to point layers only.*/
      PerimeterCurved, /** Arranges candidates following the curvature of a polygon's boundary. Applies to polygon layers only.*/
    };

    //! Positions for labels when using the QgsPalLabeling::OrderedPositionsAroundPoint placement mode
    //TODO QGIS 3.0 - move to QgsLabelingEngine
    enum PredefinedPointPosition
    {
      TopLeft, //!< Label on top-left of point
      TopSlightlyLeft, //! Label on top of point, slightly left of center
      TopMiddle, //!< Label directly above point
      TopSlightlyRight, //! Label on top of point, slightly right of center
      TopRight, //!< Label on top-right of point
      MiddleLeft, //!< Label on left of point
      MiddleRight, //!< Label on right of point
      BottomLeft, //!< Label on bottom-left of point
      BottomSlightlyLeft, //! Label below point, slightly left of center
      BottomMiddle, //!< Label directly below point
      BottomSlightlyRight, //! Label below point, slightly right of center
      BottomRight, //!< Label on bottom right of point
    };

    //! Behavior modifier for label offset and distance, only applies in some
    //! label placement modes.
    //TODO QGIS 3.0 - move to QgsLabelingEngine
    enum OffsetType
    {
      FromPoint, //!< Offset distance applies from point geometry
      FromSymbolBounds, //!< Offset distance applies from rendered symbol bounds
    };

    /** Line placement flags, which control how candidates are generated for a linear feature.
     */
    //TODO QGIS 3.0 - move to QgsLabelingEngine, rename to LinePlacementFlag, use Q_DECLARE_FLAGS to make
    //LinePlacementFlags type, and replace use of pal::LineArrangementFlag
    enum LinePlacementFlags
    {
      OnLine,          /**< Labels can be placed directly over a line feature.*/
      AboveLine,       /**< Labels can be placed above a line feature. Unless MapOrientation is also specified this mode
                            respects the direction of the line feature, so a line from right to left labels will have labels
                            placed placed below the line feature. */
      BelowLine,       /**< Labels can be placed below a line feature. Unless MapOrientation is also specified this mode
                            respects the direction of the line feature, so a line from right to left labels will have labels
                            placed placed above the line feature. */
      MapOrientation,  /**< Signifies that the AboveLine and BelowLine flags should respect the map's orientation rather
                            than the feature's orientation. For example, AboveLine will always result in label's being placed
                            above a line, regardless of the line's direction. */
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
      MultiFollowPlacement /*!< Alignment follows placement of label, e.g., labels to the left of a feature
                               will be drawn with right alignment*/
    };

    /** Valid obstacle types, which affect how features within the layer will act as obstacles
     * for labels.
     */
    //TODO QGIS 3.0 - Move to QgsLabelingEngine
    enum ObstacleType
    {
      PolygonInterior, /*!< avoid placing labels over interior of polygon (prefer placing labels totally
       outside or just slightly inside polygon) */
      PolygonBoundary, /*!< avoid placing labels over boundary of polygon (prefer placing outside or
       completely inside polygon) */
      PolygonWhole /*!< avoid placing labels over ANY part of polygon. Where PolygonInterior will prefer
       to place labels with the smallest area of intersection between the label and the polygon,
       PolygonWhole will penalise any label which intersects with the polygon by an equal amount, so that
       placing labels over any part of the polygon is avoided.*/
    };

    /** Units used for option sizes, before being converted to rendered sizes */
    enum SizeUnit
    {
      Points,
      MM,
      MapUnits,
      Percent
    };

    // update mDataDefinedNames QMap in constructor when adding/deleting enum value
    enum Property
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
      PredefinedPositionOrder,

      // rendering
      ScaleVisibility,
      MinScale,
      MaxScale,
      FontLimitPixel,
      FontMinPixel,
      FontMaxPixel,
      IsObstacle,
      ObstacleFactor,
      ZIndex,

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

    /** Is this label made from a expression string, e.g., FieldName || 'mm'
      */
    bool isExpression;

    /** Returns the QgsExpression for this label settings.
      */
    QgsExpression* getLabelExpression();

    QColor previewBkgrdColor;

    //! Substitution collection for automatic text substitution with labels
    QgsStringReplacementCollection substitutions;
    //! True if substitutions should be applied
    bool useSubstitutions;

    //-- text formatting

    QString wrapChar;
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

    //-- placement

    Placement placement;
    unsigned int placementFlags;

    bool centroidWhole; // whether centroid calculated from whole or visible polygon
    bool centroidInside; // whether centroid-point calculated must be inside polygon

    /** Ordered list of predefined label positions for points. Positions earlier
     * in the list will be prioritized over later positions. Only used when the placement
     * is set to QgsPalLayerSettings::OrderedPositionsAroundPoint.
     * @note not available in Python bindings
     */
    //QVector< QgsPalLayerSettings::PredefinedPointPosition > predefinedPositionOrder;

    /** True if only labels which completely fit within a polygon are allowed.
     */
    bool fitInPolygonOnly;
    double dist; // distance from the feature (in mm)
    bool distInMapUnits; //true if distance is in map units (otherwise in mm)
    QgsMapUnitScale distMapUnitScale;
    //! Offset type for layer (only applies in certain placement modes)
    OffsetType offsetType;

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
    UpsideDownLabels upsidedownLabels; // whether, or how, to show upsidedown labels

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

    //! Z-Index of label, where labels with a higher z-index are rendered on top of labels with a lower z-index
    double zIndex;

    // called from register feature hook
    void calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY, QgsFeature* f = 0, QgsRenderContext* context = 0 );

    /** Register a feature for labeling.
     * @param f feature to label
     * @param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     * @param labelFeature if using QgsLabelingEngine, this will receive the label feature. Not available
     * in Python bindings.
     * @param obstacleGeometry optional obstacle geometry, if a different geometry to the feature's geometry
     * should be used as an obstacle for labels (e.g., if the feature has been rendered with an offset point
     * symbol, the obstacle geometry should represent the bounds of the offset symbol). If not set,
     * the feature's original geometry will be used as an obstacle for labels. Not available
     * in Python bindings.
     */
    void registerFeature( QgsFeature& f, QgsRenderContext& context );

    void readFromLayer( QgsVectorLayer* layer );
    void writeToLayer( QgsVectorLayer* layer );

    /** Read settings from a DOM element
     * @note added in 2.12
     */
    void readXml( QDomElement& elem );

    /** Write settings into a DOM element
     * @note added in 2.12
     */
    QDomElement writeXml( QDomDocument& doc );

    QgsPropertyCollection& dataDefinedProperties();

    //const QgsPropertyCollection& dataDefinedProperties() const;

    void setDataDefinedProperties( const QgsPropertyCollection& collection );

    /** Returns the label text formatting settings, e.g., font settings, buffer settings, etc.
     * @see setFormat()
     * @note added in QGIS 3.0
     */
    const QgsTextFormat& format() const;

    /** Sets the label text formatting settings, e.g., font settings, buffer settings, etc.
     * @param format label text format
     * @see format()
     * @note added in QGIS 3.0
     */
    void setFormat( const QgsTextFormat& format );

    // temporary stuff: set when layer gets prepared or labeled
    QgsFeature* mCurFeat;
    QgsFields mCurFields;
    int fieldIndex;
    const QgsMapToPixel* xform;
    QgsCoordinateTransform ct;

    QgsPoint ptZero;
    QgsPoint ptOne;
    QgsGeometry extentGeom;
    int mFeaturesToLabel; // total features that will probably be labeled, may be less (figured before PAL)
    int mFeatsSendingToPal; // total features tested for sending into PAL (relative to maxNumLabels)
    int mFeatsRegPal; // number of features registered in PAL, when using limitNumLabels

};

class QgsLabelCandidate
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End

  public:
    QgsLabelCandidate( const QRectF& r, double c );

    QRectF rect;
    double cost;
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


class QgsPalLabeling
{
%TypeHeaderCode
#include <qgspallabeling.h>
%End

  public:

    QgsPalLabeling();
    ~QgsPalLabeling();

    void numCandidatePositions( int& candPoint, int& candLine, int& candPolygon );
    void setNumCandidatePositions( int candPoint, int candLine, int candPolygon );

    enum Search { Chain, Popmusic_Tabu, Popmusic_Chain, Popmusic_Tabu_Chain, Falp };

    void setSearchMethod( Search s );
    Search searchMethod() const;

    bool isShowingCandidates() const;
    void setShowingCandidates( bool showing );

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

    //! called to find out whether the layer is used for labeling
    //! @note added in 2.4
    static bool staticWillUseLayer( QgsVectorLayer* layer );
    static bool staticWillUseLayer( const QString& layerID );

    //! @note not available in python bindings
    // void drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform );

    //! load/save engine settings to project file
    void loadEngineSettings();
    void saveEngineSettings();
    void clearEngineSettings();

    /** Prepares a geometry for registration with PAL. Handles reprojection, rotation, clipping, etc.
     * @param geometry geometry to prepare
     * @param context render context
     * @param ct coordinate transform, or invalid transform if no transformation required
     * @param clipGeometry geometry to clip features to, if applicable
     * @returns prepared geometry
     * @note added in QGIS 2.9
     */
    static QgsGeometry prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform& ct, QgsGeometry *clipGeometry = 0 ) /Factory/;

    /** Checks whether a geometry requires preparation before registration with PAL
     * @param geometry geometry to prepare
     * @param context render context
     * @param @param ct coordinate transform, or invalid transform if no transformation required
     * @param clipGeometry geometry to clip features to, if applicable
     * @returns true if geometry requires preparation
     * @note added in QGIS 2.9
     */
    static bool geometryRequiresPreparation( const QgsGeometry& geometry, QgsRenderContext &context, const QgsCoordinateTransform& ct, QgsGeometry *clipGeometry = 0 );

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
     * allowed to be split apart (e.g., Arabic and Indic based scripts)
     * @param text string to split
     * @returns list of graphemes
     * @note added in QGIS 2.10
     */
    static QStringList splitToGraphemes( const QString& text );

  protected:
    // update temporary QgsPalLayerSettings with any data defined text style values
    void dataDefinedTextStyle( QgsPalLayerSettings& tmpLyr,
                               const QMap< QgsPalLayerSettings::Property, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined text formatting values
    void dataDefinedTextFormatting( QgsPalLayerSettings& tmpLyr,
                                    const QMap< QgsPalLayerSettings::Property, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined text buffer values
    void dataDefinedTextBuffer( QgsPalLayerSettings& tmpLyr,
                                const QMap< QgsPalLayerSettings::Property, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined shape background values
    void dataDefinedShapeBackground( QgsPalLayerSettings& tmpLyr,
                                     const QMap< QgsPalLayerSettings::Property, QVariant >& ddValues );

    // update temporary QgsPalLayerSettings with any data defined drop shadow values
    void dataDefinedDropShadow( QgsPalLayerSettings& tmpLyr,
                                const QMap< QgsPalLayerSettings::Property, QVariant >& ddValues );

    void deleteTemporaryData();

    /** Checks whether a geometry exceeds the minimum required size for a geometry to be labeled.
     * @param context render context
     * @param geom geometry
     * @param minSize minimum size for geometry
     * @returns true if geometry exceeds minimum size
     * @note added in QGIS 2.9
     */
    static bool checkMinimumSizeMM( const QgsRenderContext &context, const QgsGeometry* geom, double minSize );
};
