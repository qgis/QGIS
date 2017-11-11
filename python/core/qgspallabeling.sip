/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspallabeling.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsLabelPosition
{

%TypeHeaderCode
#include "qgspallabeling.h"
%End
  public:
    QgsLabelPosition( int id, double r, const QVector< QgsPointXY > &corners, const QgsRectangle &rect, double w, double h, const QString &layer, const QString &labeltext, const QFont &labelfont, bool upside_down, bool diagram = false, bool pinned = false, const QString &providerId = QString() );

    QgsLabelPosition();
%Docstring
Constructor for QgsLabelPosition
%End

    int featureId;
    double rotation;
    QVector< QgsPointXY > cornerPoints;
    QgsRectangle labelRect;
    double width;
    double height;
    QString layerID;
    QString labelText;
    QFont labelFont;
    bool upsideDown;
    bool isDiagram;
    bool isPinned;
    QString providerID;
%Docstring
.. versionadded:: 2.14
%End
};


class QgsPalLayerSettings
{

%TypeHeaderCode
#include "qgspallabeling.h"
%End
  public:
    QgsPalLayerSettings();
    QgsPalLayerSettings( const QgsPalLayerSettings &s );
    ~QgsPalLayerSettings();


    enum Placement
    {
      AroundPoint,
      OverPoint,
      Line,
      Curved,
      Horizontal,
      Free,
      OrderedPositionsAroundPoint,
      PerimeterCurved,
    };

    enum PredefinedPointPosition
    {
      TopLeft,
      TopSlightlyLeft,
      TopMiddle,
      TopSlightlyRight,
      TopRight,
      MiddleLeft,
      MiddleRight,
      BottomLeft,
      BottomSlightlyLeft,
      BottomMiddle,
      BottomSlightlyRight,
      BottomRight,
    };

    enum OffsetType
    {
      FromPoint,
      FromSymbolBounds,
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
      QuadrantBelowRight,
    };

    enum UpsideDownLabels
    {
      Upright,
      ShowDefined,
      ShowAll
    };

    enum DirectionSymbols
    {
      SymbolLeftRight,
      SymbolAbove,
      SymbolBelow
    };

    enum MultiLineAlign
    {
      MultiLeft,
      MultiCenter,
      MultiRight,
      MultiFollowPlacement
    };

    enum ObstacleType
    {
      PolygonInterior,
      PolygonBoundary,
      PolygonWhole
    };

    enum Property
    {
      // text
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
      FontOpacity,
      FontCase,
      FontLetterSpacing,
      FontWordSpacing,
      FontBlendMode,

      // text
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

      // text
      BufferDraw,
      BufferSize,
      BufferUnit,
      BufferColor,
      BufferTransp,
      BufferOpacity,
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
      ShapeOpacity,
      ShapeBlendMode,
      ShapeFillColor,
      ShapeStrokeColor,
      ShapeStrokeWidth,
      ShapeStrokeWidthUnits,
      ShapeJoinStyle,

      // drop
      ShadowDraw,
      ShadowUnder,
      ShadowOffsetAngle,
      ShadowOffsetDist,
      ShadowOffsetUnits,
      ShadowRadius,
      ShadowRadiusUnits,
      ShadowTransparency,
      ShadowOpacity,
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
      // (data
      PositionX,
      PositionY,
      Hali,
      Vali,
      Rotation,
      LabelRotation,
      RepeatDistance,
      RepeatDistanceUnit,
      Priority,
      PredefinedPositionOrder,

      // rendering
      ScaleVisibility,
      MinScale,
      MinimumScale,
      MaxScale,
      MaximumScale,
      FontLimitPixel,
      FontMinPixel,
      FontMaxPixel,
      IsObstacle,
      ObstacleFactor,
      ZIndex,

      // (data
      Show,
      AlwaysShow
    };

    static const QgsPropertiesDefinition &propertyDefinitions();
%Docstring
 Returns the labeling property definitions.
.. versionadded:: 3.0
 :rtype: QgsPropertiesDefinition
%End

    bool drawLabels;
%Docstring
 Whether to draw labels for this layer. For some layers it may be desirable
 to register their features as obstacles for other labels without requiring
 labels to be drawn for the layer itself. In this case drawLabels can be set
 to false and obstacle set to true, which will result in the layer acting
 as an obstacle but having no labels of its own.
.. versionadded:: 2.12
%End


    QString fieldName;
%Docstring
 Name of field (or an expression) to use for label text.
 If fieldName is an expression, then isExpression should be set to true.
.. seealso:: isExpression
%End

    bool isExpression;
%Docstring
 True if this label is made from a expression string, e.g., FieldName || 'mm'
.. seealso:: fieldName
%End

    QgsExpression *getLabelExpression();
%Docstring
 Returns the QgsExpression for this label settings. May be None if isExpression is false.
 :rtype: QgsExpression
%End

    QColor previewBkgrdColor;

    QgsStringReplacementCollection substitutions;
%Docstring
Substitution collection for automatic text substitution with labels
%End
    bool useSubstitutions;
%Docstring
True if substitutions should be applied
%End


    QString wrapChar;
%Docstring
 Wrapping character string. If set, any occurrences of this string in the calculated
 label text will be replaced with new line characters.
%End

    MultiLineAlign multilineAlign;
%Docstring
Horizontal alignment of multi-line labels.
%End

    bool addDirectionSymbol;
%Docstring
 If true, '<' or '>' (or custom strings set via leftDirectionSymbol and rightDirectionSymbol)
 will be automatically added to the label text, pointing in the
 direction of the line or polygon ring.
 This setting only affects line or perimeter based labels.
.. seealso:: leftDirectionSymbol
.. seealso:: rightDirectionSymbol
.. seealso:: placeDirectionSymbol
.. seealso:: reverseDirectionSymbol
%End

    QString leftDirectionSymbol;
%Docstring
 String to use for left direction arrows.
.. seealso:: addDirectionSymbol
.. seealso:: rightDirectionSymbol
%End

    QString rightDirectionSymbol;
%Docstring
 String to use for right direction arrows.
.. seealso:: addDirectionSymbol
.. seealso:: leftDirectionSymbol
%End

    DirectionSymbols placeDirectionSymbol;
%Docstring
 Placement option for direction symbols. Controls whether to place symbols to the left/right, above or below label.
.. seealso:: addDirectionSymbol
%End

    bool reverseDirectionSymbol;
%Docstring
True if direction symbols should be reversed
%End

    bool formatNumbers;
%Docstring
 Set to true to format numeric label text as numbers (e.g. inserting thousand separators
 and fixed number of decimal places).
.. seealso:: decimals
.. seealso:: plusSign
%End

    int decimals;
%Docstring
 Number of decimal places to show for numeric labels. formatNumbers must be true for this
 setting to have an effect.
.. seealso:: formatNumbers
%End

    bool plusSign;
%Docstring
 Whether '+' signs should be prepended to positive numeric labels. formatNumbers must be true for this
 setting to have an effect.
.. seealso:: formatNumbers
%End


    Placement placement;
    unsigned int placementFlags;

    bool centroidWhole;
%Docstring
 True if feature centroid should be calculated from the whole feature, or
 false if only the visible part of the feature should be considered.
%End

    bool centroidInside;
%Docstring
 True if centroid positioned labels must be placed inside their corresponding
 feature polygon, or false if centroids which fall outside the polygon
 are permitted.
%End


    bool fitInPolygonOnly;
%Docstring
 True if only labels which completely fit within a polygon are allowed.
%End

    double dist;
%Docstring
 Distance from feature to the label. Units are specified via distUnits.
.. seealso:: distUnits
.. seealso:: distMapUnitScale
%End

    QgsUnitTypes::RenderUnit distUnits;
%Docstring
 Units the distance from feature to the label.
.. seealso:: dist
.. seealso:: distMapUnitScale
%End

    QgsMapUnitScale distMapUnitScale;
%Docstring
 Map unit scale for label feature distance.
.. seealso:: dist
.. seealso:: distUnits
%End

    OffsetType offsetType;
%Docstring
Offset type for layer (only applies in certain placement modes)
%End

    double repeatDistance;
%Docstring
 Distance for repeating labels for a single feature.
.. seealso:: repeatDistanceUnit
.. seealso:: repeatDistanceMapUnitScale
%End

    QgsUnitTypes::RenderUnit repeatDistanceUnit;
%Docstring
 Units for repeating labels for a single feature.
.. seealso:: repeatDistance
.. seealso:: repeatDistanceMapUnitScale
%End

    QgsMapUnitScale repeatDistanceMapUnitScale;
%Docstring
 Map unit scale for repeating labels for a single feature.
.. seealso:: repeatDistance
.. seealso:: repeatDistanceUnit
%End

    QuadrantPosition quadOffset;
%Docstring
 Sets the quadrant in which to offset labels from feature.
%End

    double xOffset;
%Docstring
 Horizontal offset of label. Units are specified via offsetUnits.
.. seealso:: yOffset
.. seealso:: offsetUnits
.. seealso:: labelOffsetMapUnitScale
%End

    double yOffset;
%Docstring
 Vertical offset of label. Units are specified via offsetUnits.
.. seealso:: xOffset
.. seealso:: offsetUnits
.. seealso:: labelOffsetMapUnitScale
%End

    QgsUnitTypes::RenderUnit offsetUnits;
%Docstring
 Units for offsets of label.
.. seealso:: xOffset
.. seealso:: yOffset
.. seealso:: labelOffsetMapUnitScale
%End

    QgsMapUnitScale labelOffsetMapUnitScale;
%Docstring
 Map unit scale for label offset.
.. seealso:: xOffset
.. seealso:: yOffset
.. seealso:: offsetUnits
%End

    double angleOffset;
%Docstring
Label rotation, in degrees clockwise
%End

    bool preserveRotation;
%Docstring
True if label rotation should be preserved during label pin/unpin operations.
%End

    double maxCurvedCharAngleIn;
%Docstring
 Maximum angle between inside curved label characters (valid range 20.0 to 60.0).
.. seealso:: maxCurvedCharAngleOut
%End

    double maxCurvedCharAngleOut;
%Docstring
 Maximum angle between outside curved label characters (valid range -20.0 to -95.0)
.. seealso:: maxCurvedCharAngleIn
%End

    int priority;
%Docstring
 Label priority. Valid ranges are from 0 to 10, where 0 = lowest priority
 and 10 = highest priority.
%End


    bool scaleVisibility;
%Docstring
 Set to true to limit label visibility to a range of scales.
.. seealso:: maximumScale
.. seealso:: minimumScale
%End

    double maximumScale;
%Docstring
 The maximum map scale (i.e. most "zoomed in" scale) at which the labels will be visible.
 The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
 A scale of 0 indicates no maximum scale visibility.

 This setting is only considered if scaleVisibility is true.

.. seealso:: minimumScale
.. seealso:: scaleVisibility
%End

    double minimumScale;
%Docstring
 The minimum map scale (i.e. most "zoomed out" scale) at which the labels will be visible.
 The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
 A scale of 0 indicates no minimum scale visibility.

 This setting is only considered if scaleVisibility is true.

.. seealso:: maximumScale
.. seealso:: scaleVisibility
%End

    bool fontLimitPixelSize;
%Docstring
 True if label sizes should be limited by pixel size.
.. seealso:: fontMinPixelSize
.. seealso:: fontMaxPixelSize
%End

    int fontMinPixelSize;
%Docstring
 Minimum pixel size for showing rendered map unit labels (1 - 1000).
.. seealso:: fontLimitPixelSize
.. seealso:: fontMaxPixelSize
%End

    int fontMaxPixelSize;
%Docstring
 Maximum pixel size for showing rendered map unit labels (1 - 10000).
.. seealso:: fontLimitPixelSize
.. seealso:: fontMinPixelSize
%End

    bool displayAll;
%Docstring
If true, all features will be labelled even when overlaps occur.
%End

    UpsideDownLabels upsidedownLabels;
%Docstring
Controls whether upside down labels are displayed and how they are handled.
%End

    bool labelPerPart;
%Docstring
 True if every part of a multi-part feature should be labeled. If false,
 only the largest part will be labeled.
%End

    bool mergeLines;
%Docstring
 True if connected line features with identical label text should be merged
 prior to generating label positions.
%End

    bool limitNumLabels;
%Docstring
 True if the number of labels drawn should be limited.
.. seealso:: maxNumLabels
%End

    int maxNumLabels;
%Docstring
 The maximum number of labels which should be drawn for this layer.
 This only has an effect if limitNumLabels is true.
.. seealso:: limitNumLabels
%End

    double minFeatureSize;
%Docstring
 Minimum feature size (in millimeters) for a feature to be labelled.
%End

    bool obstacle;
%Docstring
 True if features for layer are obstacles to labels of other layers.
.. seealso:: obstacleFactor
.. seealso:: obstacleType
%End

    double obstacleFactor;
%Docstring
 Obstacle factor, where 1.0 = default, < 1.0 more likely to be covered by labels,
 > 1.0 less likely to be covered
.. seealso:: obstacle
.. seealso:: obstacleType
%End

    ObstacleType obstacleType;
%Docstring
 Controls how features act as obstacles for labels.
.. seealso:: obstacle
.. seealso:: obstacleFactor
%End

    double zIndex;
%Docstring
Z-Index of label, where labels with a higher z-index are rendered on top of labels with a lower z-index
%End

    void calculateLabelSize( const QFontMetricsF *fm, QString text, double &labelX, double &labelY, QgsFeature *f = 0, QgsRenderContext *context = 0 );

    void registerFeature( QgsFeature &f, QgsRenderContext &context  );

%Docstring
 Register a feature for labeling.
 \param f feature to label
 \param context render context. The QgsExpressionContext contained within the render context
 must have already had the feature and fields sets prior to calling this method.
 \param labelFeature if using QgsLabelingEngine, this will receive the label feature. Not available
 in Python bindings.
 \param obstacleGeometry optional obstacle geometry, if a different geometry to the feature's geometry
 should be used as an obstacle for labels (e.g., if the feature has been rendered with an offset point
 symbol, the obstacle geometry should represent the bounds of the offset symbol). If not set,
 the feature's original geometry will be used as an obstacle for labels. Not available
 in Python bindings.
%End

    void readXml( QDomElement &elem, const QgsReadWriteContext &context );
%Docstring
 Read settings from a DOM element
.. versionadded:: 2.12
%End

    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context );
%Docstring
 Write settings into a DOM element
.. versionadded:: 2.12
 :rtype: QDomElement
%End

    QgsPropertyCollection &dataDefinedProperties();
%Docstring
 Returns a reference to the label's property collection, used for data defined overrides.
.. versionadded:: 3.0
.. seealso:: setDataDefinedProperties()
 :rtype: QgsPropertyCollection
%End


    void setDataDefinedProperties( const QgsPropertyCollection &collection );
%Docstring
 Sets the label's property collection, used for data defined overrides.
 \param collection property collection. Existing properties will be replaced.
.. versionadded:: 3.0
.. seealso:: dataDefinedProperties()
%End

    const QgsTextFormat &format() const;
%Docstring
 Returns the label text formatting settings, e.g., font settings, buffer settings, etc.
.. seealso:: setFormat()
.. versionadded:: 3.0
 :rtype: QgsTextFormat
%End

    void setFormat( const QgsTextFormat &format );
%Docstring
 Sets the label text formatting settings, e.g., font settings, buffer settings, etc.
 \param format label text format
.. seealso:: format()
.. versionadded:: 3.0
%End

    QgsFeature *mCurFeat;
    QgsFields mCurFields;
    int fieldIndex;
    const QgsMapToPixel *xform;
    QgsCoordinateTransform ct;

    QgsPointXY ptZero;
    QgsPointXY ptOne;
    QgsGeometry extentGeom;
    int mFeaturesToLabel;
    int mFeatsSendingToPal;
    int mFeatsRegPal;

};

class QgsLabelCandidate
{

%TypeHeaderCode
#include "qgspallabeling.h"
%End
  public:
    QgsLabelCandidate( const QRectF &r, double c );

    QRectF rect;
    double cost;
};




class QgsLabelingResults
{
%Docstring
 Class that stores computed placement from labeling engine.
.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgspallabeling.h"
%End
  public:
    QgsLabelingResults();
    ~QgsLabelingResults();


    QList<QgsLabelPosition> labelsAtPosition( const QgsPointXY &p ) const;
%Docstring
return infos about labels at a given (map) position
 :rtype: list of QgsLabelPosition
%End
    QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle &r ) const;
%Docstring
return infos about labels within a given (map) rectangle
 :rtype: list of QgsLabelPosition
%End

  private:
    QgsLabelingResults( const QgsLabelingResults & );
};

class QgsPalLabeling
{

%TypeHeaderCode
#include "qgspallabeling.h"
%End
  public:

    static bool staticWillUseLayer( QgsVectorLayer *layer );
%Docstring
 called to find out whether the layer is used for labeling
.. versionadded:: 2.4
 :rtype: bool
%End


    static QgsGeometry prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry() ) /Factory/;
%Docstring
 Prepares a geometry for registration with PAL. Handles reprojection, rotation, clipping, etc.
 \param geometry geometry to prepare
 \param context render context
 \param ct coordinate transform, or invalid transform if no transformation required
 \param clipGeometry geometry to clip features to, if applicable
 :return: prepared geometry
.. versionadded:: 2.9
 :rtype: QgsGeometry
%End

    static bool geometryRequiresPreparation( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry() );
%Docstring
 Checks whether a geometry requires preparation before registration with PAL
 \param geometry geometry to prepare
 \param context render context
 \param ct coordinate transform, or invalid transform if no transformation required
 \param clipGeometry geometry to clip features to, if applicable
 :return: true if geometry requires preparation
.. versionadded:: 2.9
 :rtype: bool
%End

    static QStringList splitToLines( const QString &text, const QString &wrapCharacter );
%Docstring
 Splits a text string to a list of separate lines, using a specified wrap character.
 The text string will be split on either newline characters or the wrap character.
 \param text text string to split
 \param wrapCharacter additional character to wrap on
 :return: list of text split to lines
.. versionadded:: 2.9
 :rtype: list of str
%End

    static QStringList splitToGraphemes( const QString &text );
%Docstring
 Splits a text string to a list of graphemes, which are the smallest allowable character
 divisions in the string. This accounts for scripts were individual characters are not
 allowed to be split apart (e.g., Arabic and Indic based scripts)
 \param text string to split
 :return: list of graphemes
.. versionadded:: 2.10
 :rtype: list of str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspallabeling.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
