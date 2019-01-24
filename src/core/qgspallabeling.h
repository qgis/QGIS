/***************************************************************************
  qgspallabeling.h
  Smart labeling for vector layers
  -------------------
   begin                : June 2009
   copyright            : (C) Martin Dobias
   email                : wonder dot sk at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//Note: although this file is in the core library, it is not part of the stable API
//and might change at any time!

#ifndef QGSPALLABELING_H
#define QGSPALLABELING_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QString>
#include <QFont>
#include <QFontDatabase>
#include <QColor>
#include <QHash>
#include <QList>
#include <QPainter>
#include <QRectF>
#include <QMap>
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsfields.h"
#include "qgslabelingenginesettings.h"
#include "qgspointxy.h"
#include "qgsmapunitscale.h"
#include "qgsstringutils.h"
#include "qgstextrenderer.h"
#include "qgspropertycollection.h"

namespace pal SIP_SKIP
{
  class Pal;
  class Layer;
  class LabelPosition;
}

class QgsDiagramLayerSettings;
class QgsRectangle;
class QgsMapToPixel;
class QgsFeature;
class QgsTextLabelFeature;
class QgsVectorLayer;
class QgsExpression;
class QFontMetricsF;
class QPainter;
class QPicture;
class QgsGeometry;
class QgsCoordinateTransform;
class QgsLabelSearchTree;
class QgsMapSettings;
class QgsLabelFeature;
class QgsLabelingEngine;
class QgsPalLayerSettings;
class QgsVectorLayerLabelProvider;
class QgsDxfExport;
class QgsVectorLayerDiagramProvider;
class QgsExpressionContext;


/**
 * \ingroup core
 * \class QgsLabelPosition
 */
class CORE_EXPORT QgsLabelPosition
{
  public:
    QgsLabelPosition( int id, double r, const QVector< QgsPointXY > &corners, const QgsRectangle &rect, double w, double h, const QString &layer, const QString &labeltext, const QFont &labelfont, bool upside_down, bool diagram = false, bool pinned = false, const QString &providerId = QString() )
      : featureId( id )
      , rotation( r )
      , cornerPoints( corners )
      , labelRect( rect )
      , width( w )
      , height( h )
      , layerID( layer )
      , labelText( labeltext )
      , labelFont( labelfont )
      , upsideDown( upside_down )
      , isDiagram( diagram )
      , isPinned( pinned )
      , providerID( providerId )
    {}

    //! Constructor for QgsLabelPosition
    QgsLabelPosition() = default;

    int featureId = -1;
    double rotation = 0;
    QVector< QgsPointXY > cornerPoints;
    QgsRectangle labelRect;
    double width = 0;
    double height = 0;
    QString layerID;
    QString labelText;
    QFont labelFont;
    bool upsideDown = false;
    bool isDiagram = false;
    bool isPinned = false;
    //! \since QGIS 2.14
    QString providerID;
};


/**
 * \ingroup core
 * \class QgsPalLayerSettings
 */
class CORE_EXPORT QgsPalLayerSettings
{
  public:
    QgsPalLayerSettings();
    QgsPalLayerSettings( const QgsPalLayerSettings &s );
    ~QgsPalLayerSettings();

    //! copy operator - only copies the permanent members
    QgsPalLayerSettings &operator=( const QgsPalLayerSettings &s );

    /**
     * Placement modes which determine how label candidates are generated for a feature.
     */
    //TODO QGIS 3.0 - move to QgsLabelingEngine
    enum Placement
    {
      AroundPoint, //!< Arranges candidates in a circle around a point (or centroid of a polygon). Applies to point or polygon layers only.
      OverPoint, //! Arranges candidates over a point (or centroid of a polygon), or at a preset offset from the point. Applies to point or polygon layers only.
      Line, //!< Arranges candidates parallel to a generalised line representing the feature or parallel to a polygon's perimeter. Applies to line or polygon layers only.
      Curved, //! Arranges candidates following the curvature of a line feature. Applies to line layers only.
      Horizontal, //!< Arranges horizontal candidates scattered throughout a polygon feature. Applies to polygon layers only.
      Free, //!< Arranges candidates scattered throughout a polygon feature. Candidates are rotated to respect the polygon's orientation. Applies to polygon layers only.
      OrderedPositionsAroundPoint, //!< Candidates are placed in predefined positions around a point. Preference is given to positions with greatest cartographic appeal, e.g., top right, bottom right, etc. Applies to point layers only.
      PerimeterCurved, //! Arranges candidates following the curvature of a polygon's boundary. Applies to polygon layers only.
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

    /**
     * Behavior modifier for label offset and distance, only applies in some
     * label placement modes.
     */
    //TODO QGIS 3.0 - move to QgsLabelingEngine
    enum OffsetType
    {
      FromPoint, //!< Offset distance applies from point geometry
      FromSymbolBounds, //!< Offset distance applies from rendered symbol bounds
    };

    /**
     * Line placement flags, which control how candidates are generated for a linear feature.
     */
    //TODO QGIS 3.0 - move to QgsLabelingEngine, rename to LinePlacementFlag, use Q_DECLARE_FLAGS to make
    //LinePlacementFlags type, and replace use of pal::LineArrangementFlag
    enum LinePlacementFlags
    {
      OnLine    = 1,      //!< Labels can be placed directly over a line feature.
      AboveLine = 2,      /**< Labels can be placed above a line feature. Unless MapOrientation is also specified this mode
                               respects the direction of the line feature, so a line from right to left labels will have labels
                               placed placed below the line feature. */
      BelowLine = 4,      /**< Labels can be placed below a line feature. Unless MapOrientation is also specified this mode
                               respects the direction of the line feature, so a line from right to left labels will have labels
                               placed placed above the line feature. */
      MapOrientation = 8, /**< Signifies that the AboveLine and BelowLine flags should respect the map's orientation rather
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
      QuadrantBelowRight,
    };

    enum UpsideDownLabels
    {
      Upright, //!< Upside-down labels (90 <= angle < 270) are shown upright
      ShowDefined, //!< Show upside down when rotation is layer- or data-defined
      ShowAll //!< Show upside down for all labels, including dynamic ones
    };

    enum DirectionSymbols
    {
      SymbolLeftRight, //!< Place direction symbols on left/right of label
      SymbolAbove, //!< Place direction symbols on above label
      SymbolBelow //!< Place direction symbols on below label
    };

    enum MultiLineAlign
    {
      MultiLeft = 0,
      MultiCenter,
      MultiRight,
      MultiFollowPlacement /*!< Alignment follows placement of label, e.g., labels to the left of a feature
                               will be drawn with right alignment*/
    };

    /**
     * Valid obstacle types, which affect how features within the layer will act as obstacles
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

    //! Data definable properties.
    enum Property
    {
      // text style
      Size = 0, //!< Label size
      Bold = 1, //!< Use bold style
      Italic = 2, //!< Use italic style
      Underline = 3, //!< Use underline
      Color = 4, //!< Text color
      Strikeout = 5, //!< Use strikeout
      Family = 6, //!< Font family
      FontStyle = 21, //!< Font style name
      FontSizeUnit = 22, //!< Font size units
      FontTransp = 18, //!< Text transparency (deprecated)
      FontOpacity = 92, //!< Text opacity
      FontCase = 27, //!< Label text case
      FontLetterSpacing = 28, //!< Letter spacing
      FontWordSpacing = 29, //!< Word spacing
      FontBlendMode = 30, //! Text blend mode

      // text formatting
      MultiLineWrapChar = 31,
      AutoWrapLength = 101,
      MultiLineHeight = 32,
      MultiLineAlignment = 33,
      DirSymbDraw = 34,
      DirSymbLeft = 35,
      DirSymbRight = 36,
      DirSymbPlacement = 37,
      DirSymbReverse = 38,
      NumFormat = 39,
      NumDecimals = 40,
      NumPlusSign = 41,

      // text buffer
      BufferDraw = 42,
      BufferSize = 7,
      BufferUnit = 43,
      BufferColor = 8,
      BufferTransp = 19, //!< Buffer transparency (deprecated)
      BufferOpacity = 94, //!< Buffer opacity
      BufferJoinStyle = 44,
      BufferBlendMode = 45,

      // background
      ShapeDraw = 46,
      ShapeKind = 47,
      ShapeSVGFile = 48,
      ShapeSizeType = 49,
      ShapeSizeX = 50,
      ShapeSizeY = 85,
      ShapeSizeUnits = 51,
      ShapeRotationType = 52,
      ShapeRotation = 53,
      ShapeOffset = 54,
      ShapeOffsetUnits = 55,
      ShapeRadii = 56,
      ShapeRadiiUnits = 57,
      ShapeTransparency = 63, //!< Shape transparency (deprecated)
      ShapeOpacity = 93, //!< Shape opacity
      ShapeBlendMode = 64,
      ShapeFillColor = 58,
      ShapeStrokeColor = 59,
      ShapeStrokeWidth = 60,
      ShapeStrokeWidthUnits = 61,
      ShapeJoinStyle = 62,

      // drop shadow
      ShadowDraw = 65,
      ShadowUnder = 66,
      ShadowOffsetAngle = 67,
      ShadowOffsetDist = 68,
      ShadowOffsetUnits = 69,
      ShadowRadius = 70,
      ShadowRadiusUnits = 71,
      ShadowTransparency = 72, //!< Shadow transparency (deprecated)
      ShadowOpacity = 95, //!< Shadow opacity
      ShadowScale = 73,
      ShadowColor = 74,
      ShadowBlendMode = 75,

      // placement
      CentroidWhole = 76,
      OffsetQuad = 77,
      OffsetXY = 78,
      OffsetUnits = 80,
      LabelDistance = 13,
      DistanceUnits = 81,
      OffsetRotation = 82,
      CurvedCharAngleInOut = 83,
      // (data defined only)
      PositionX = 9, //!< X-coordinate data defined label position
      PositionY = 10, //!< Y-coordinate data defined label position
      Hali = 11, //!< Horizontal alignment for data defined label position (Left, Center, Right)
      Vali = 12, //!< Vertical alignment for data defined label position (Bottom, Base, Half, Cap, Top)
      Rotation = 14, //!< Label rotation (deprecated, for old project compatibility only)
      LabelRotation = 96, //!< Label rotation
      RepeatDistance = 84,
      RepeatDistanceUnit = 86,
      Priority = 87,
      PredefinedPositionOrder = 91,

      // rendering
      ScaleVisibility = 23,
      MinScale = 16, //!< Min scale (deprecated, for old project compatibility only)
      MinimumScale = 97, //!< Minimum map scale (ie most "zoomed out")
      MaxScale = 17, //!< Max scale (deprecated, for old project compatibility only)
      MaximumScale = 98, //!< Maximum map scale (ie most "zoomed in")
      FontLimitPixel = 24,
      FontMinPixel = 25,
      FontMaxPixel = 26,
      IsObstacle = 88,
      ObstacleFactor = 89,
      ZIndex = 90,

      // (data defined only)
      Show = 15,
      AlwaysShow = 20
    };

    /**
     * Returns the labeling property definitions.
     * \since QGIS 3.0
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

    /**
     * Whether to draw labels for this layer. For some layers it may be desirable
     * to register their features as obstacles for other labels without requiring
     * labels to be drawn for the layer itself. In this case drawLabels can be set
     * to false and obstacle set to true, which will result in the layer acting
     * as an obstacle but having no labels of its own.
     * \since QGIS 2.12
     */
    bool drawLabels;

    //-- text style

    /**
     * Name of field (or an expression) to use for label text.
     * If fieldName is an expression, then isExpression should be set to true.
     * \see isExpression
     */
    QString fieldName;

    /**
     * True if this label is made from a expression string, e.g., FieldName || 'mm'
     * \see fieldName
     */
    bool isExpression;

    /**
     * Returns the QgsExpression for this label settings. May be nullptr if isExpression is false.
     */
    QgsExpression *getLabelExpression();

    QColor previewBkgrdColor;

    //! Substitution collection for automatic text substitution with labels
    QgsStringReplacementCollection substitutions;
    //! True if substitutions should be applied
    bool useSubstitutions;

    //-- text formatting

    /**
     * Wrapping character string. If set, any occurrences of this string in the calculated
     * label text will be replaced with new line characters.
     */
    QString wrapChar;

    /**
     * If non-zero, indicates that label text should be automatically wrapped to (ideally) the specified
     * number of characters. If zero, auto wrapping is disabled.
     *
     * \see useMaxLineLengthForAutoWrap
     * \since QGIS 3.4
     */
    int autoWrapLength = 0;

    /**
     * If true, indicates that when auto wrapping label text the autoWrapLength length indicates the maximum
     * ideal length of text lines. If false, then autoWrapLength indicates the ideal minimum length of text
     * lines.
     *
     * If autoWrapLength is 0 then this value has no effect.
     *
     * \see autoWrapLength
     * \since QGIS 3.4
     */
    bool useMaxLineLengthForAutoWrap = true;

    //! Horizontal alignment of multi-line labels.
    MultiLineAlign multilineAlign;

    /**
     * If true, '<' or '>' (or custom strings set via leftDirectionSymbol and rightDirectionSymbol)
     * will be automatically added to the label text, pointing in the
     * direction of the line or polygon ring.
     * This setting only affects line or perimeter based labels.
     * \see leftDirectionSymbol
     * \see rightDirectionSymbol
     * \see placeDirectionSymbol
     * \see reverseDirectionSymbol
     */
    bool addDirectionSymbol;

    /**
     * String to use for left direction arrows.
     * \see addDirectionSymbol
     * \see rightDirectionSymbol
     */
    QString leftDirectionSymbol;

    /**
     * String to use for right direction arrows.
     * \see addDirectionSymbol
     * \see leftDirectionSymbol
     */
    QString rightDirectionSymbol;

    /**
     * Placement option for direction symbols. Controls whether to place symbols to the left/right, above or below label.
     * \see addDirectionSymbol
     */
    DirectionSymbols placeDirectionSymbol;

    //! True if direction symbols should be reversed
    bool reverseDirectionSymbol;

    /**
     * Set to true to format numeric label text as numbers (e.g. inserting thousand separators
     * and fixed number of decimal places).
     * \see decimals
     * \see plusSign
     */
    bool formatNumbers;

    /**
     * Number of decimal places to show for numeric labels. formatNumbers must be true for this
     * setting to have an effect.
     * \see formatNumbers
     */
    int decimals;

    /**
     * Whether '+' signs should be prepended to positive numeric labels. formatNumbers must be true for this
     * setting to have an effect.
     * \see formatNumbers
     */
    bool plusSign;

    //-- placement

    Placement placement;
    unsigned int placementFlags;

    /**
     * True if feature centroid should be calculated from the whole feature, or
     * false if only the visible part of the feature should be considered.
     */
    bool centroidWhole;

    /**
     * True if centroid positioned labels must be placed inside their corresponding
     * feature polygon, or false if centroids which fall outside the polygon
     * are permitted.
     */
    bool centroidInside;

    /**
     * Ordered list of predefined label positions for points. Positions earlier
     * in the list will be prioritized over later positions. Only used when the placement
     * is set to QgsPalLayerSettings::OrderedPositionsAroundPoint.
     * \note not available in Python bindings
     */
    QVector< PredefinedPointPosition > predefinedPositionOrder SIP_SKIP;

    /**
     * True if only labels which completely fit within a polygon are allowed.
     */
    bool fitInPolygonOnly;

    /**
     * Distance from feature to the label. Units are specified via distUnits.
     * \see distUnits
     * \see distMapUnitScale
     */
    double dist;

    /**
     * Units the distance from feature to the label.
     * \see dist
     * \see distMapUnitScale
     */
    QgsUnitTypes::RenderUnit distUnits;

    /**
     * Map unit scale for label feature distance.
     * \see dist
     * \see distUnits
     */
    QgsMapUnitScale distMapUnitScale;

    //! Offset type for layer (only applies in certain placement modes)
    OffsetType offsetType;

    /**
     * Distance for repeating labels for a single feature.
     * \see repeatDistanceUnit
     * \see repeatDistanceMapUnitScale
     */
    double repeatDistance;

    /**
     * Units for repeating labels for a single feature.
     * \see repeatDistance
     * \see repeatDistanceMapUnitScale
     */
    QgsUnitTypes::RenderUnit repeatDistanceUnit;

    /**
     * Map unit scale for repeating labels for a single feature.
     * \see repeatDistance
     * \see repeatDistanceUnit
     */
    QgsMapUnitScale repeatDistanceMapUnitScale;

    /**
     * Sets the quadrant in which to offset labels from feature.
     */
    QuadrantPosition quadOffset;

    /**
     * Horizontal offset of label. Units are specified via offsetUnits.
     * \see yOffset
     * \see offsetUnits
     * \see labelOffsetMapUnitScale
     */
    double xOffset;

    /**
     * Vertical offset of label. Units are specified via offsetUnits.
     * \see xOffset
     * \see offsetUnits
     * \see labelOffsetMapUnitScale
     */
    double yOffset;

    /**
     * Units for offsets of label.
     * \see xOffset
     * \see yOffset
     * \see labelOffsetMapUnitScale
     */
    QgsUnitTypes::RenderUnit offsetUnits;

    /**
     * Map unit scale for label offset.
     * \see xOffset
     * \see yOffset
     * \see offsetUnits
     */
    QgsMapUnitScale labelOffsetMapUnitScale;

    //! Label rotation, in degrees clockwise
    double angleOffset;

    //! True if label rotation should be preserved during label pin/unpin operations.
    bool preserveRotation;

    /**
     * Maximum angle between inside curved label characters (valid range 20.0 to 60.0).
     * \see maxCurvedCharAngleOut
     */
    double maxCurvedCharAngleIn;

    /**
     * Maximum angle between outside curved label characters (valid range -20.0 to -95.0)
     * \see maxCurvedCharAngleIn
     */
    double maxCurvedCharAngleOut;

    /**
     * Label priority. Valid ranges are from 0 to 10, where 0 = lowest priority
     * and 10 = highest priority.
     */
    int priority;

    //-- rendering

    /**
     * Set to true to limit label visibility to a range of scales.
     * \see maximumScale
     * \see minimumScale
     */
    bool scaleVisibility;

    /**
     * The maximum map scale (i.e. most "zoomed in" scale) at which the labels will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no maximum scale visibility.
     *
     * This setting is only considered if scaleVisibility is true.
     *
     * \see minimumScale
     * \see scaleVisibility
    */
    double maximumScale;

    /**
     * The minimum map scale (i.e. most "zoomed out" scale) at which the labels will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no minimum scale visibility.
     *
     * This setting is only considered if scaleVisibility is true.
     *
     * \see maximumScale
     * \see scaleVisibility
    */
    double minimumScale;

    /**
     * True if label sizes should be limited by pixel size.
     * \see fontMinPixelSize
     * \see fontMaxPixelSize
     */
    bool fontLimitPixelSize;

    /**
     * Minimum pixel size for showing rendered map unit labels (1 - 1000).
     * \see fontLimitPixelSize
     * \see fontMaxPixelSize
     */
    int fontMinPixelSize;

    /**
     * Maximum pixel size for showing rendered map unit labels (1 - 10000).
     * \see fontLimitPixelSize
     * \see fontMinPixelSize
     */
    int fontMaxPixelSize;

    //! If true, all features will be labelled even when overlaps occur.
    bool displayAll;

    //! Controls whether upside down labels are displayed and how they are handled.
    UpsideDownLabels upsidedownLabels = Upright;

    /**
     * True if every part of a multi-part feature should be labeled. If false,
     * only the largest part will be labeled.
     */
    bool labelPerPart;

    /**
     * True if connected line features with identical label text should be merged
     * prior to generating label positions.
     */
    bool mergeLines;

    /**
     * True if the number of labels drawn should be limited.
     * \see maxNumLabels
     */
    bool limitNumLabels;

    /**
     * The maximum number of labels which should be drawn for this layer.
     * This only has an effect if limitNumLabels is true.
     * \see limitNumLabels
     */
    int maxNumLabels;

    /**
     * Minimum feature size (in millimeters) for a feature to be labelled.
     */
    double minFeatureSize;

    /**
     * True if features for layer are obstacles to labels of other layers.
     * \see obstacleFactor
     * \see obstacleType
     */
    bool obstacle;

    /**
     * Obstacle factor, where 1.0 = default, < 1.0 more likely to be covered by labels,
     * > 1.0 less likely to be covered
     * \see obstacle
     * \see obstacleType
     */
    double obstacleFactor;

    /**
     * Controls how features act as obstacles for labels.
     * \see obstacle
     * \see obstacleFactor
     */
    ObstacleType obstacleType;

    //! Z-Index of label, where labels with a higher z-index are rendered on top of labels with a lower z-index
    double zIndex;

    // called from register feature hook
    void calculateLabelSize( const QFontMetricsF *fm, QString text, double &labelX, double &labelY, QgsFeature *f = nullptr, QgsRenderContext *context = nullptr );

    /**
     * Register a feature for labeling.
     * \param f feature to label
     * \param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     * \param labelFeature if using QgsLabelingEngine, this will receive the label feature. Not available
     * in Python bindings.
     * \param obstacleGeometry optional obstacle geometry, if a different geometry to the feature's geometry
     * should be used as an obstacle for labels (e.g., if the feature has been rendered with an offset point
     * symbol, the obstacle geometry should represent the bounds of the offset symbol). If not set,
     * the feature's original geometry will be used as an obstacle for labels. Not available
     * in Python bindings.
     */
    void registerFeature( QgsFeature &f, QgsRenderContext &context,
                          QgsLabelFeature **labelFeature SIP_PYARGREMOVE = nullptr,
                          QgsGeometry obstacleGeometry SIP_PYARGREMOVE = QgsGeometry() );

    /**
     * Read settings from a DOM element
     * \since QGIS 2.12
     */
    void readXml( QDomElement &elem, const QgsReadWriteContext &context );

    /**
     * Write settings into a DOM element
     * \since QGIS 2.12
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * Returns a reference to the label's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \since QGIS 3.0
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the label's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    const QgsPropertyCollection &dataDefinedProperties() const SIP_SKIP { return mDataDefinedProperties; }

    /**
     * Sets the label's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
     * \see dataDefinedProperties()
     * \see Property
     * \since QGIS 3.0
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Returns the label text formatting settings, e.g., font settings, buffer settings, etc.
     * \see setFormat()
     * \since QGIS 3.0
     */
    const QgsTextFormat &format() const { return mFormat; }

    /**
     * Sets the label text formatting settings, e.g., font settings, buffer settings, etc.
     * \param format label text format
     * \see format()
     * \since QGIS 3.0
     */
    void setFormat( const QgsTextFormat &format ) { mFormat = format; }

    // temporary stuff: set when layer gets prepared or labeled
    QgsFeature *mCurFeat = nullptr;
    QgsFields mCurFields;
    int fieldIndex;
    const QgsMapToPixel *xform = nullptr;
    QgsCoordinateTransform ct;

    QgsPointXY ptZero;
    QgsPointXY ptOne;
    QgsGeometry extentGeom;
    int mFeaturesToLabel = 0; // total features that will probably be labeled, may be less (figured before PAL)
    int mFeatsSendingToPal = 0; // total features tested for sending into PAL (relative to maxNumLabels)
    int mFeatsRegPal = 0; // number of features registered in PAL, when using limitNumLabels

  private:

    friend class QgsVectorLayer;  // to allow calling readFromLayerCustomProperties()

    /**
     * Reads labeling configuration from layer's custom properties to support loading of simple labeling from QGIS 2.x projects.
     * \since QGIS 3.0
     */
    void readFromLayerCustomProperties( QgsVectorLayer *layer );

    /**
     * Reads data defined properties from a QGIS 2.x project.
     */
    void readOldDataDefinedPropertyMap( QgsVectorLayer *layer, QDomElement *parentElem );

    /**
     * Reads a data defined property from a QGIS 2.x project.
     */
    void readOldDataDefinedProperty( QgsVectorLayer *layer, QgsPalLayerSettings::Property p );

    enum DataDefinedValueType
    {
      DDBool,
      DDInt,
      DDIntPos,
      DDDouble,
      DDDoublePos,
      DDRotation180,
      DDOpacity, //!< Data defined opacity (double between 0 and 100)
      DDString,
      DDUnits,
      DDColor,
      DDJoinStyle,
      DDBlendMode,
      DDPointF,
      DDSizeF, //!< Data defined size
    };

    // convenience data defined evaluation function
    bool dataDefinedValEval( DataDefinedValueType valType,
                             QgsPalLayerSettings::Property p,
                             QVariant &exprVal, QgsExpressionContext &context, const QVariant &originalValue = QVariant() );

    void parseTextStyle( QFont &labelFont,
                         QgsUnitTypes::RenderUnit fontunits,
                         QgsRenderContext &context );

    void parseTextBuffer( QgsRenderContext &context );

    void parseTextFormatting( QgsRenderContext &context );

    void parseShapeBackground( QgsRenderContext &context );

    void parseDropShadow( QgsRenderContext &context );

    /**
     * Checks if a feature is larger than a minimum size (in mm)
    \returns true if above size, false if below*/
    bool checkMinimumSizeMM( const QgsRenderContext &ct, const QgsGeometry &geom, double minSize ) const;

    /**
     * Registers a feature as an obstacle only (no label rendered)
     */
    void registerObstacleFeature( QgsFeature &f, QgsRenderContext &context, QgsLabelFeature **obstacleFeature, const QgsGeometry &obstacleGeometry = QgsGeometry() );

    QMap<Property, QVariant> dataDefinedValues;

    //! Property collection for data defined label settings
    QgsPropertyCollection mDataDefinedProperties;

    QgsExpression *expression = nullptr;

    QFontDatabase mFontDB;

    QgsTextFormat mFormat;

    static const QVector< PredefinedPointPosition > DEFAULT_PLACEMENT_ORDER;

    static void initPropertyDefinitions();

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

};

/**
 * \ingroup core
 */
class CORE_EXPORT QgsLabelCandidate
{
  public:
    QgsLabelCandidate( const QRectF &r, double c ): rect( r ), cost( c ) {}

    QRectF rect;
    double cost;
};




/**
 * \ingroup core
 * Class that stores computed placement from labeling engine.
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsLabelingResults
{
  public:
    QgsLabelingResults();
    ~QgsLabelingResults();

    //! QgsLabelingResults cannot be copied.
    QgsLabelingResults( const QgsLabelingResults & ) = delete;
    //! QgsLabelingResults cannot be copied.
    QgsLabelingResults &operator=( const QgsLabelingResults &rh ) = delete;

    //! Returns infos about labels at a given (map) position
    QList<QgsLabelPosition> labelsAtPosition( const QgsPointXY &p ) const;
    //! Returns infos about labels within a given (map) rectangle
    QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle &r ) const;

  private:
#ifdef SIP_RUN
    QgsLabelingResults( const QgsLabelingResults & );
#endif

    QgsLabelSearchTree *mLabelSearchTree = nullptr;

    friend class QgsPalLabeling;
    friend class QgsVectorLayerLabelProvider;
    friend class QgsVectorLayerDiagramProvider;
};

/**
 * \ingroup core
 * \class QgsPalLabeling
 */
class CORE_EXPORT QgsPalLabeling
{
  public:

    /**
     * called to find out whether the layer is used for labeling
     * \since QGIS 2.4
     */
    static bool staticWillUseLayer( QgsVectorLayer *layer );

    //! \note not available in Python bindings
    static void drawLabelCandidateRect( pal::LabelPosition *lp, QPainter *painter, const QgsMapToPixel *xform, QList<QgsLabelCandidate> *candidates = nullptr ) SIP_SKIP;

    /**
     * Prepares a geometry for registration with PAL. Handles reprojection, rotation, clipping, etc.
     * \param geometry geometry to prepare
     * \param context render context
     * \param ct coordinate transform, or invalid transform if no transformation required
     * \param clipGeometry geometry to clip features to, if applicable
     * \returns prepared geometry
     * \since QGIS 2.9
     */
    static QgsGeometry prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry() ) SIP_FACTORY;

    /**
     * Checks whether a geometry requires preparation before registration with PAL
     * \param geometry geometry to prepare
     * \param context render context
     * \param ct coordinate transform, or invalid transform if no transformation required
     * \param clipGeometry geometry to clip features to, if applicable
     * \returns true if geometry requires preparation
     * \since QGIS 2.9
     */
    static bool geometryRequiresPreparation( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry() );

    /**
     * Splits a \a text string to a list of separate lines, using a specified wrap character (\a wrapCharacter).
     * The text string will be split on either newline characters or the wrap character.
     *
     * Since QGIS 3.4 the \a autoWrapLength argument can be used to specify an ideal length of line to automatically
     * wrap text to (automatic wrapping is disabled if \a autoWrapLength is 0). This automatic wrapping is performed
     * after processing wrapping using \a wrapCharacter. When auto wrapping is enabled, the \a useMaxLineLengthWhenAutoWrapping
     * argument controls whether the lines should be wrapped to an ideal maximum of \a autoWrapLength characters, or
     * if false then the lines are wrapped to an ideal minimum length of \a autoWrapLength characters.
     *
     * \since QGIS 2.9
     */
    static QStringList splitToLines( const QString &text, const QString &wrapCharacter, int autoWrapLength = 0, bool useMaxLineLengthWhenAutoWrapping = true );

    /**
     * Splits a text string to a list of graphemes, which are the smallest allowable character
     * divisions in the string. This accounts for scripts were individual characters are not
     * allowed to be split apart (e.g., Arabic and Indic based scripts)
     * \param text string to split
     * \returns list of graphemes
     * \since QGIS 2.10
     */
    static QStringList splitToGraphemes( const QString &text );

  private:
    //! Update temporary QgsPalLayerSettings with any data defined text style values
    static void dataDefinedTextStyle( QgsPalLayerSettings &tmpLyr,
                                      const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );

    //! Update temporary QgsPalLayerSettings with any data defined text formatting values
    static void dataDefinedTextFormatting( QgsPalLayerSettings &tmpLyr,
                                           const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );

    //! Update temporary QgsPalLayerSettings with any data defined text buffer values
    static void dataDefinedTextBuffer( QgsPalLayerSettings &tmpLyr,
                                       const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );

    //! Update temporary QgsPalLayerSettings with any data defined shape background values
    static void dataDefinedShapeBackground( QgsPalLayerSettings &tmpLyr,
                                            const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );

    //! Update temporary QgsPalLayerSettings with any data defined drop shadow values
    static void dataDefinedDropShadow( QgsPalLayerSettings &tmpLyr,
                                       const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues );

    friend class QgsVectorLayerLabelProvider; // to allow calling the static methods above
    friend class QgsDxfExport;                // to allow calling the static methods above

    /**
     * Checks whether a geometry exceeds the minimum required size for a geometry to be labeled.
     * \param context render context
     * \param geom geometry
     * \param minSize minimum size for geometry
     * \returns true if geometry exceeds minimum size
     * \since QGIS 2.9
     */
    static bool checkMinimumSizeMM( const QgsRenderContext &context, const QgsGeometry &geom, double minSize );

    friend class QgsPalLayerSettings;
};

#endif // QGSPALLABELING_H
