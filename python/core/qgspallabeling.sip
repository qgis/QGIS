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
    QgsLabelPosition( int id, double r, const QVector< QgsPoint > &corners, const QgsRectangle &rect, double w, double h, const QString &layer, const QString &labeltext, const QFont &labelfont, bool upside_down, bool diagram = false, bool pinned = false, const QString &providerId = QString() );
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


    enum SizeUnit
    {
      Points,
      MM,
      MapUnits,
      Percent
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

    bool isExpression;
%Docstring
 Is this label made from a expression string, e.g., FieldName || 'mm'
%End

    QgsExpression *getLabelExpression();
%Docstring
 Returns the QgsExpression for this label settings.
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
    MultiLineAlign multilineAlign; // horizontal alignment of multi-line labels

    bool addDirectionSymbol;
    QString leftDirectionSymbol;
    QString rightDirectionSymbol;
    DirectionSymbols placeDirectionSymbol; // whether to place left/right, above or below label
    bool reverseDirectionSymbol;

    bool formatNumbers;
    int decimals;
    bool plusSign;


    Placement placement;
    unsigned int placementFlags;

    bool centroidWhole; // whether centroid calculated from whole or visible polygon
    bool centroidInside; // whether centroid-point calculated must be inside polygon


    bool fitInPolygonOnly;
%Docstring
 True if only labels which completely fit within a polygon are allowed.
%End
    double dist; // distance from the feature (in mm)
    bool distInMapUnits; //true if distance is in map units (otherwise in mm)
    QgsMapUnitScale distMapUnitScale;
    OffsetType offsetType;
%Docstring
Offset type for layer (only applies in certain placement modes)
%End

    double repeatDistance;
    SizeUnit repeatDistanceUnit;
    QgsMapUnitScale repeatDistanceMapUnitScale;

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

    double obstacleFactor;
%Docstring
 Obstacle factor, where 1.0 = default, < 1.0 more likely to be covered by labels,
 > 1.0 less likely to be covered
%End

    ObstacleType obstacleType;
%Docstring
 Controls how features act as obstacles for labels
%End

    double zIndex;
%Docstring
Z-Index of label, where labels with a higher z-index are rendered on top of labels with a lower z-index
%End

    void calculateLabelSize( const QFontMetricsF *fm, QString text, double &labelX, double &labelY, QgsFeature *f = 0, QgsRenderContext *context = 0 );

    void registerFeature( QgsFeature &f, QgsRenderContext &context );

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


    QList<QgsLabelPosition> labelsAtPosition( const QgsPoint &p ) const;
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
.. versionadded:: 2.4
 :rtype: bool
%End


    static QgsGeometry prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, QgsGeometry *clipGeometry = 0 ) /Factory/;
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

    static bool geometryRequiresPreparation( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, QgsGeometry *clipGeometry = 0 );
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

  protected:
    static void dataDefinedTextStyle( QgsPalLayerSettings &tmpLyr,
                                      const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );
%Docstring
Update temporary QgsPalLayerSettings with any data defined text style values
%End

    static void dataDefinedTextFormatting( QgsPalLayerSettings &tmpLyr,
                                           const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );
%Docstring
Update temporary QgsPalLayerSettings with any data defined text formatting values
%End

    static void dataDefinedTextBuffer( QgsPalLayerSettings &tmpLyr,
                                       const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );
%Docstring
Update temporary QgsPalLayerSettings with any data defined text buffer values
%End

    static void dataDefinedShapeBackground( QgsPalLayerSettings &tmpLyr,
                                            const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );
%Docstring
Update temporary QgsPalLayerSettings with any data defined shape background values
%End

    static void dataDefinedDropShadow( QgsPalLayerSettings &tmpLyr,
                                       const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );
%Docstring
Update temporary QgsPalLayerSettings with any data defined drop shadow values
%End


    static bool checkMinimumSizeMM( const QgsRenderContext &context, const QgsGeometry *geom, double minSize );
%Docstring
 Checks whether a geometry exceeds the minimum required size for a geometry to be labeled.
 \param context render context
 \param geom geometry
 \param minSize minimum size for geometry
 :return: true if geometry exceeds minimum size
.. versionadded:: 2.9
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspallabeling.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
