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
%End

    bool drawLabels;


    QString fieldName;

    bool isExpression;

    QgsExpression *getLabelExpression();
%Docstring
Returns the QgsExpression for this label settings. May be None if isExpression is false.
%End

    QColor previewBkgrdColor;

    QgsStringReplacementCollection substitutions;
    bool useSubstitutions;


    QString wrapChar;

    MultiLineAlign multilineAlign;

    bool addDirectionSymbol;

    QString leftDirectionSymbol;

    QString rightDirectionSymbol;

    DirectionSymbols placeDirectionSymbol;

    bool reverseDirectionSymbol;

    bool formatNumbers;

    int decimals;

    bool plusSign;


    Placement placement;
    unsigned int placementFlags;

    bool centroidWhole;

    bool centroidInside;


    bool fitInPolygonOnly;

    double dist;

    QgsUnitTypes::RenderUnit distUnits;

    QgsMapUnitScale distMapUnitScale;

    OffsetType offsetType;

    double repeatDistance;

    QgsUnitTypes::RenderUnit repeatDistanceUnit;

    QgsMapUnitScale repeatDistanceMapUnitScale;

    QuadrantPosition quadOffset;

    double xOffset;

    double yOffset;

    QgsUnitTypes::RenderUnit offsetUnits;

    QgsMapUnitScale labelOffsetMapUnitScale;

    double angleOffset;

    bool preserveRotation;

    double maxCurvedCharAngleIn;

    double maxCurvedCharAngleOut;

    int priority;


    bool scaleVisibility;

    double maximumScale;

    double minimumScale;

    bool fontLimitPixelSize;

    int fontMinPixelSize;

    int fontMaxPixelSize;

    bool displayAll;

    UpsideDownLabels upsidedownLabels;

    bool labelPerPart;

    bool mergeLines;

    bool limitNumLabels;

    int maxNumLabels;

    double minFeatureSize;

    bool obstacle;

    double obstacleFactor;

    ObstacleType obstacleType;

    double zIndex;

    void calculateLabelSize( const QFontMetricsF *fm, QString text, double &labelX, double &labelY, QgsFeature *f = 0, QgsRenderContext *context = 0 );

    void registerFeature( QgsFeature &f, QgsRenderContext &context  );

%Docstring
Register a feature for labeling.

:param f: feature to label
:param context: render context. The QgsExpressionContext contained within the render context
must have already had the feature and fields sets prior to calling this method.
:param labelFeature: if using :py:class:`QgsLabelingEngine`, this will receive the label feature. Not available
in Python bindings.
:param obstacleGeometry: optional obstacle geometry, if a different geometry to the feature's geometry
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
%End

    QgsPropertyCollection &dataDefinedProperties();
%Docstring
Returns a reference to the label's property collection, used for data defined overrides.

.. versionadded:: 3.0

.. seealso:: :py:func:`setDataDefinedProperties`
%End


    void setDataDefinedProperties( const QgsPropertyCollection &collection );
%Docstring
Sets the label's property collection, used for data defined overrides.

:param collection: property collection. Existing properties will be replaced.

.. versionadded:: 3.0

.. seealso:: :py:func:`dataDefinedProperties`
%End

    const QgsTextFormat &format() const;
%Docstring
Returns the label text formatting settings, e.g., font settings, buffer settings, etc.

.. seealso:: :py:func:`setFormat`

.. versionadded:: 3.0
%End

    void setFormat( const QgsTextFormat &format );
%Docstring
Sets the label text formatting settings, e.g., font settings, buffer settings, etc.

:param format: label text format

.. seealso:: :py:func:`format`

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
%End
    QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle &r ) const;
%Docstring
return infos about labels within a given (map) rectangle
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
%End


    static QgsGeometry prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry() ) /Factory/;
%Docstring
Prepares a geometry for registration with PAL. Handles reprojection, rotation, clipping, etc.

:param geometry: geometry to prepare
:param context: render context
:param ct: coordinate transform, or invalid transform if no transformation required
:param clipGeometry: geometry to clip features to, if applicable

:return: prepared geometry

.. versionadded:: 2.9
%End

    static bool geometryRequiresPreparation( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry() );
%Docstring
Checks whether a geometry requires preparation before registration with PAL

:param geometry: geometry to prepare
:param context: render context
:param ct: coordinate transform, or invalid transform if no transformation required
:param clipGeometry: geometry to clip features to, if applicable

:return: true if geometry requires preparation

.. versionadded:: 2.9
%End

    static QStringList splitToLines( const QString &text, const QString &wrapCharacter );
%Docstring
Splits a text string to a list of separate lines, using a specified wrap character.
The text string will be split on either newline characters or the wrap character.

:param text: text string to split
:param wrapCharacter: additional character to wrap on

:return: list of text split to lines

.. versionadded:: 2.9
%End

    static QStringList splitToGraphemes( const QString &text );
%Docstring
Splits a text string to a list of graphemes, which are the smallest allowable character
divisions in the string. This accounts for scripts were individual characters are not
allowed to be split apart (e.g., Arabic and Indic based scripts)

:param text: string to split

:return: list of graphemes

.. versionadded:: 2.10
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspallabeling.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
