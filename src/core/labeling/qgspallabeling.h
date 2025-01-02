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
#include "qgspointxy.h"
#include "qgsmapunitscale.h"
#include "qgsstringutils.h"
#include "qgstextformat.h"
#include "qgspropertycollection.h"
#include "qgslabelplacementsettings.h"
#include "qgslabelobstaclesettings.h"
#include "qgslabelthinningsettings.h"
#include "qgslabellinesettings.h"
#include "qgslabelpointsettings.h"
#include "qgscoordinatetransform.h"
#include "qgsexpression.h"

class QgsTextDocument;
class QgsTextDocumentMetrics;

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
class QgsMapLayer;
class QgsMapSettings;
class QgsLabelFeature;
class QgsLabelingEngine;
class QgsPalLayerSettings;
class QgsVectorLayerLabelProvider;
class QgsDxfExport;
class QgsVectorLayerDiagramProvider;
class QgsExpressionContext;
class QgsCallout;

/**
 * \ingroup core
 * \class QgsPalLayerSettings
 * \brief Contains settings for how a map layer will be labeled.
 */
class CORE_EXPORT QgsPalLayerSettings
{
  public:
    QgsPalLayerSettings();
    QgsPalLayerSettings( const QgsPalLayerSettings &s );
    ~QgsPalLayerSettings();

    //! copy operator - only copies the permanent members
    QgsPalLayerSettings &operator=( const QgsPalLayerSettings &s );

    // *INDENT-OFF*
    //! Data definable properties.
    enum class Property SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPalLayerSettings, Property ) : int
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
      FontBlendMode = 30, //!< Text blend mode
      FontStretchFactor = 113, //!< Font stretch factor, since QGIS 3.24

      // text formatting
      MultiLineWrapChar = 31,
      AutoWrapLength = 101,
      MultiLineHeight = 32,
      MultiLineAlignment = 33,
      TextOrientation = 110,
      TabStopDistance = 120, //!< Tab stop distance, since QGIS 3.38
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

      // mask buffer
      MaskEnabled = 104, //!< Whether the mask is enabled
      MaskBufferSize = 105, //!< Mask buffer size
      MaskBufferUnit = 106, //!< Mask buffer size unit
      MaskOpacity = 107, //!< Mask opacity
      MaskJoinStyle = 108, //!< Mask join style

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
      MaximumDistance = 119, //!< Maximum distance of label from feature
      DistanceUnits = 81,
      OffsetRotation = 82,
      CurvedCharAngleInOut = 83,
      // (data defined only)
      PositionX = 9, //!< X-coordinate data defined label position
      PositionY = 10, //!< Y-coordinate data defined label position
      PositionPoint = 114, //!< Point-coordinate data defined label position
      Hali = 11, //!< Horizontal alignment for data defined label position (Left, Center, Right)
      Vali = 12, //!< Vertical alignment for data defined label position (Bottom, Base, Half, Cap, Top)
      Rotation = 14, //!< Label rotation (deprecated, for old project compatibility only)
      LabelRotation = 96, //!< Label rotation
      RepeatDistance = 84,
      RepeatDistanceUnit = 86,
      Priority = 87,
      PredefinedPositionOrder = 91,
      LinePlacementOptions = 99, //!< Line placement flags
      OverrunDistance = 102, //!< Distance which labels can extend past either end of linear features
      LabelAllParts = 103, //!< Whether all parts of multi-part features should be labeled
      PolygonLabelOutside = 109, //!< Whether labels outside a polygon feature are permitted, or should be forced \since QGIS 3.14
      LineAnchorPercent = 111, //!< Portion along line at which labels should be anchored \since QGIS 3.16
      LineAnchorClipping = 112, //!< Clipping mode for line anchor calculation \since QGIS 3.20
      LineAnchorType = 115, //!< Line anchor type \since QGIS 3.26
      LineAnchorTextPoint = 116, //!< Line anchor text point \since QGIS 3.26

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
      CalloutDraw = 100, //!< Show callout

      AllowDegradedPlacement = 117, //!< Allow degraded label placements \since QGIS 3.26
      OverlapHandling = 118, //!< Overlap handling technique \since QGIS 3.26

      // (data defined only)
      Show = 15,
      AlwaysShow = 20
    };
    // *INDENT-ON*

    /**
     * Prepare for registration of features.
     * The \a context, \a mapSettings and \a fields parameters give more
     * information about the rendering environment.
     * If target \a crs is not specified, the targetCrs from \a mapSettings
     * will be taken.
     * The parameter \a attributeNames should be updated to contain all the field
     * names which the labeling requires for the rendering.
     *
     * \since QGIS 3.8
     */
    bool prepare( QgsRenderContext &context, QSet<QString> &attributeNames SIP_INOUT, const QgsFields &fields, const QgsMapSettings &mapSettings, const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns all field names referenced by the configuration (e.g. field name or expression, data defined properties).
     * \since QGIS 3.14
     */
    QSet<QString> referencedFields( const QgsRenderContext &context ) const;

    /**
     * Prepares the label settings for rendering.
     *
     * This should be called before rendering any labels, and must be
     * followed by a call to stopRender() in order to gracefully clean up symbols.
     *
     * \since QGIS 3.10
     */
    void startRender( QgsRenderContext &context );

    /**
     * Finalises the label settings after use.
     *
     * This must be called after a call to startRender(), in order to gracefully clean up symbols.
     *
     * \since QGIS 3.10
     */
    void stopRender( QgsRenderContext &context );

    /**
     * Returns TRUE if any component of the label settings requires advanced effects
     * such as blend modes, which require output in raster formats to be fully respected.
     * \since QGIS 3.20
     */
    bool containsAdvancedEffects() const;

    /**
     * Returns the labeling property definitions.
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

    /**
     * Whether to draw labels for this layer. For some layers it may be desirable
     * to register their features as obstacles for other labels without requiring
     * labels to be drawn for the layer itself. In this case drawLabels can be set
     * to FALSE and obstacle set to TRUE, which will result in the layer acting
     * as an obstacle but having no labels of its own.
     */
    bool drawLabels = true;

    //-- text style

    /**
     * Name of field (or an expression) to use for label text.
     * If fieldName is an expression, then isExpression should be set to TRUE.
     * \see isExpression
     */
    QString fieldName;

    /**
     * TRUE if this label is made from a expression string, e.g., FieldName || 'mm'
     * \see fieldName
     */
    bool isExpression = false;

    /**
     * Returns the QgsExpression for this label settings. May be NULLPTR if isExpression is FALSE.
     */
    QgsExpression *getLabelExpression();

    /**
     * \deprecated QGIS 3.10. Use QgsTextFormat::previewBackgroundColor() instead.
     */
    Q_DECL_DEPRECATED QColor previewBkgrdColor = Qt::white;

    //! Substitution collection for automatic text substitution with labels
    QgsStringReplacementCollection substitutions;
    //! True if substitutions should be applied
    bool useSubstitutions = false;

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
     * If TRUE, indicates that when auto wrapping label text the autoWrapLength length indicates the maximum
     * ideal length of text lines. If FALSE, then autoWrapLength indicates the ideal minimum length of text
     * lines.
     *
     * If autoWrapLength is 0 then this value has no effect.
     *
     * \see autoWrapLength
     * \since QGIS 3.4
     */
    bool useMaxLineLengthForAutoWrap = true;

    //! Horizontal alignment of multi-line labels.
    Qgis::LabelMultiLineAlignment multilineAlign = Qgis::LabelMultiLineAlignment::FollowPlacement;

    /**
     * Set to TRUE to format numeric label text as numbers (e.g. inserting thousand separators
     * and fixed number of decimal places).
     * \see decimals
     * \see plusSign
     */
    bool formatNumbers = false;

    /**
     * Number of decimal places to show for numeric labels. formatNumbers must be TRUE for this
     * setting to have an effect.
     * \see formatNumbers
     */
    int decimals = 3;

    /**
     * Whether '+' signs should be prepended to positive numeric labels. formatNumbers must be TRUE for this
     * setting to have an effect.
     * \see formatNumbers
     */
    bool plusSign = false;

    //-- placement

    //! Label placement mode
    Qgis::LabelPlacement placement = Qgis::LabelPlacement::AroundPoint;

    /**
     * Returns the polygon placement flags, which dictate how polygon labels can be placed.
     *
     * \see setPolygonPlacementFlags()
     * \since QGIS 3.14
     */
    Qgis::LabelPolygonPlacementFlags polygonPlacementFlags() const { return mPolygonPlacementFlags; }

    /**
     * Sets the polygon placement \a flags, which dictate how polygon labels can be placed.
     *
     * \see polygonPlacementFlags()
     * \since QGIS 3.14
     */
    void setPolygonPlacementFlags( Qgis::LabelPolygonPlacementFlags flags ) { mPolygonPlacementFlags = flags; }

    /**
     * TRUE if feature centroid should be calculated from the whole feature, or
     * FALSE if only the visible part of the feature should be considered.
     */
    bool centroidWhole = false;

    /**
     * TRUE if centroid positioned labels must be placed inside their corresponding
     * feature polygon, or FALSE if centroids which fall outside the polygon
     * are permitted.
     */
    bool centroidInside = false;

    /**
     * TRUE if only labels which completely fit within a polygon are allowed.
     */
    bool fitInPolygonOnly = false;

    /**
     * Distance from feature to the label. Units are specified via distUnits.
     * \see distUnits
     * \see distMapUnitScale
     */
    double dist = 0;

    /**
     * Units the distance from feature to the label.
     * \see dist
     * \see distMapUnitScale
     */
    Qgis::RenderUnit distUnits = Qgis::RenderUnit::Millimeters;

    /**
     * Map unit scale for label feature distance.
     * \see dist
     * \see distUnits
     */
    QgsMapUnitScale distMapUnitScale;

    //! Offset type for layer (only applies in certain placement modes)
    Qgis::LabelOffsetType offsetType = Qgis::LabelOffsetType::FromPoint;

    /**
     * Distance for repeating labels for a single feature.
     * \see repeatDistanceUnit
     * \see repeatDistanceMapUnitScale
     */
    double repeatDistance = 0;

    /**
     * Units for repeating labels for a single feature.
     * \see repeatDistance
     * \see repeatDistanceMapUnitScale
     */
    Qgis::RenderUnit repeatDistanceUnit = Qgis::RenderUnit::Millimeters;

    /**
     * Map unit scale for repeating labels for a single feature.
     * \see repeatDistance
     * \see repeatDistanceUnit
     */
    QgsMapUnitScale repeatDistanceMapUnitScale;

    /**
     * Horizontal offset of label. Units are specified via offsetUnits.
     * \see yOffset
     * \see offsetUnits
     * \see labelOffsetMapUnitScale
     */
    double xOffset = 0;

    /**
     * Vertical offset of label. Units are specified via offsetUnits.
     * \see xOffset
     * \see offsetUnits
     * \see labelOffsetMapUnitScale
     */
    double yOffset = 0;

    /**
     * Units for offsets of label.
     * \see xOffset
     * \see yOffset
     * \see labelOffsetMapUnitScale
     */
    Qgis::RenderUnit offsetUnits = Qgis::RenderUnit::Millimeters;

    /**
     * Map unit scale for label offset.
     * \see xOffset
     * \see yOffset
     * \see offsetUnits
     */
    QgsMapUnitScale labelOffsetMapUnitScale;

    //! Label rotation, in degrees clockwise
    double angleOffset = 0;

    //! True if label rotation should be preserved during label pin/unpin operations.
    bool preserveRotation = true;

    /**
     * Unit for rotation of labels.
     * \see setRotationUnit()
     * \since QGIS 3.22
     */
    Qgis::AngleUnit rotationUnit() const;

    /**
      * Set unit for rotation of labels.
      * \see rotationUnit()
      * \since QGIS 3.22
      */
    void setRotationUnit( Qgis::AngleUnit angleUnit );

    /**
     * Maximum angle between inside curved label characters (valid range 20.0 to 60.0).
     * \see maxCurvedCharAngleOut
     */
    double maxCurvedCharAngleIn = 25.0;

    /**
     * Maximum angle between outside curved label characters (valid range -20.0 to -95.0)
     * \see maxCurvedCharAngleIn
     */
    double maxCurvedCharAngleOut = -25.0;

    /**
     * Label priority. Valid ranges are from 0 to 10, where 0 = lowest priority
     * and 10 = highest priority.
     */
    int priority = 5;

    //-- rendering

    /**
     * Set to TRUE to limit label visibility to a range of scales.
     * \see maximumScale
     * \see minimumScale
     */
    bool scaleVisibility = false;

    /**
     * The maximum map scale (i.e. most "zoomed in" scale) at which the labels will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no maximum scale visibility.
     *
     * This setting is only considered if scaleVisibility is TRUE.
     *
     * \see minimumScale
     * \see scaleVisibility
    */
    double maximumScale = 0;

    /**
     * The minimum map scale (i.e. most "zoomed out" scale) at which the labels will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no minimum scale visibility.
     *
     * This setting is only considered if scaleVisibility is TRUE.
     *
     * \see maximumScale
     * \see scaleVisibility
    */
    double minimumScale = 0;

    /**
     * TRUE if label sizes should be limited by pixel size.
     * \see fontMinPixelSize
     * \see fontMaxPixelSize
     */
    bool fontLimitPixelSize = false;

    /**
     * Minimum pixel size for showing rendered map unit labels (1 - 1000).
     * \see fontLimitPixelSize
     * \see fontMaxPixelSize
     */
    int fontMinPixelSize = 0;

    /**
     * Maximum pixel size for showing rendered map unit labels (1 - 10000).
     * \see fontLimitPixelSize
     * \see fontMinPixelSize
     */
    int fontMaxPixelSize = 10000;

    //! Controls whether upside down labels are displayed and how they are handled.
    Qgis::UpsideDownLabelHandling upsidedownLabels = Qgis::UpsideDownLabelHandling::FlipUpsideDownLabels;

    /**
     * TRUE if every part of a multi-part feature should be labeled. If FALSE,
     * only the largest part will be labeled.
     */
    bool labelPerPart = false;

    //! Z-Index of label, where labels with a higher z-index are rendered on top of labels with a lower z-index
    double zIndex = 0;

    //! The geometry generator expression. Null if disabled.
    QString geometryGenerator;

    //! The type of the result geometry of the geometry generator.
    Qgis::GeometryType geometryGeneratorType = Qgis::GeometryType::Point;

    //! Defines if the geometry generator is enabled or not. If disabled, the standard geometry will be taken.
    bool geometryGeneratorEnabled = false;

    /**
     * Geometry type of layers associated with these settings.
     * \since QGIS 3.10
     */
    Qgis::GeometryType layerType = Qgis::GeometryType::Unknown;

    /**
     * \brief setLegendString
     * \param legendString the string to show in the legend and preview
     */
    void setLegendString( const QString &legendString ) { mLegendString = legendString; }

    /**
     * \brief legendString
     * \return the string to show in the legend and in the preview icon
     */
    QString legendString() const { return mLegendString; }

    /**
     * Calculates the space required to render the provided \a text in map units.
     * Results will be written to \a labelX and \a labelY.
     *
     * If the text orientation is set to rotation-based, the spaced taken to render
     * vertically oriented text will be written to \a rotatedLabelX and \a rotatedLabelY.
     *
     * \warning This method only returns an approximate label size, and eg will not consider
     * HTML formatted text correctly.
     *
     * \deprecated QGIS 3.40. Will be removed from public API in QGIS 4.0.
     */
    Q_DECL_DEPRECATED void calculateLabelSize( const QFontMetricsF *fm, const QString &text, double &labelX, double &labelY, const QgsFeature *f = nullptr, QgsRenderContext *context = nullptr, double *rotatedLabelX SIP_OUT = nullptr, double *rotatedLabelY SIP_OUT = nullptr ) SIP_DEPRECATED;

    /**
     * Registers a feature for labeling.
     * \param f feature to label
     * \param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     *
     * \warning This method is designed for use by PyQGIS clients only. C++ code should use the
     * variant with additional arguments.
     */
    void registerFeature( const QgsFeature &f, QgsRenderContext &context );

#ifndef SIP_RUN

    /**
     * Registers a feature for labeling.
     * \param feature feature to label
     * \param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields set prior to calling this method.
     * \param obstacleGeometry optional obstacle geometry, if a different geometry to the feature's geometry
     * should be used as an obstacle for labels (e.g., if the feature has been rendered with an offset point
     * symbol, the obstacle geometry should represent the bounds of the offset symbol). If not set,
     * the feature's original geometry will be used as an obstacle for labels.
     * \param symbol feature symbol to label (ownership is not transferred, and \a symbol must exist until the labeling is complete)
     *
     * \returns QgsLabelFeature representing the registered feature, or NULLPTR if the feature will not be labeled
     * in this context.
     *
     * \note Not available in Python bindings
     */
    std::unique_ptr< QgsLabelFeature > registerFeatureWithDetails( const QgsFeature &feature, QgsRenderContext &context,
        QgsGeometry obstacleGeometry = QgsGeometry(), const QgsSymbol *symbol = nullptr );

#endif

    /**
     * Read settings from a DOM element
     */
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    /**
     * Write settings into a DOM element
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Returns a reference to the label's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the label's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \note not available in Python bindings
     */
    const QgsPropertyCollection &dataDefinedProperties() const SIP_SKIP { return mDataDefinedProperties; }

    /**
     * Sets the label's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
     * \see dataDefinedProperties()
     * \see Property
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Returns the label text formatting settings, e.g., font settings, buffer settings, etc.
     * \see setFormat()
     */
    const QgsTextFormat &format() const { return mFormat; }

    /**
     * Sets the label text formatting settings, e.g., font settings, buffer settings, etc.
     * \param format label text format
     * \see format()
     */
    void setFormat( const QgsTextFormat &format ) { mFormat = format; }

    /**
     * Returns the label callout renderer, responsible for drawing label callouts.
     *
     * Ownership is not transferred.
     *
     * \see setCallout()
     * \since QGIS 3.10
     */
    QgsCallout *callout() const { return mCallout.get(); }

    /**
     * Sets the label \a callout renderer, responsible for drawing label callouts.
     *
     * Ownership of \a callout is transferred to the settings.

     * \see callout()
     * \since QGIS 3.10
     */
    void setCallout( QgsCallout *callout SIP_TRANSFER );

    /**
     * Returns the label line settings, which contain settings related to how the label
     * engine places and formats labels for line features (or polygon features which are labeled in
     * a "perimeter" style mode).
     * \see setLineSettings()
     * \note Not available in Python bindings
     * \since QGIS 3.16
     */
    const QgsLabelLineSettings &lineSettings() const SIP_SKIP { return mLineSettings; }

    /**
     * Returns the label line settings, which contain settings related to how the label
     * engine places and formats labels for line features (or polygon features which are labeled in
     * a "perimeter" style mode).
     * \see setLineSettings()
     * \since QGIS 3.16
     */
    QgsLabelLineSettings &lineSettings() { return mLineSettings; }

    /**
     * Sets the label line \a settings, which contain settings related to how the label
     * engine places and formats labels for line features (or polygon features which are labeled in
     * a "perimeter" style mode).
     * \see lineSettings()
     * \since QGIS 3.16
     */
    void setLineSettings( const QgsLabelLineSettings &settings ) { mLineSettings = settings; }

    /**
     * Returns the label point settings, which contain settings related to how the label
     * engine places and formats labels for point features, or polygon features which are
     * labeled in the "around" or "over" centroid placement modes.
     *
     * \see setPointSettings()
     * \note Not available in Python bindings
     * \since QGIS 3.38
     */
    const QgsLabelPointSettings &pointSettings() const SIP_SKIP { return mPointSettings; }

    /**
     * Returns the label point settings, which contain settings related to how the label
     * engine places and formats labels for point features, or polygon features which are
     * labeled in the "around" or "over" centroid placement modes.
     *
    * \see setPointSettings()
    * \since QGIS 3.38
    */
    QgsLabelPointSettings &pointSettings() { return mPointSettings; }

    /**
     * Sets the label point \a settings, which contain settings related to how the label
     * engine places and formats labels for point features, or polygon features which are
     * labeled in the "around" or "over" centroid placement modes.
     *
     * \see pointSettings()
     * \since QGIS 3.38
     */
    void setPointSettings( const QgsLabelPointSettings &settings ) { mPointSettings = settings; }

    /**
     * Returns the label obstacle settings.
     * \see setObstacleSettings()
     * \note Not available in Python bindings
     * \since QGIS 3.10.2
     */
    const QgsLabelObstacleSettings &obstacleSettings() const SIP_SKIP { return mObstacleSettings; }

    /**
     * Returns the label obstacle settings.
     * \see setObstacleSettings()
     * \since QGIS 3.10.2
     */
    QgsLabelObstacleSettings &obstacleSettings() { return mObstacleSettings; }

    /**
     * Sets the label obstacle \a settings.
     * \see obstacleSettings()
     * \since QGIS 3.10.2
     */
    void setObstacleSettings( const QgsLabelObstacleSettings &settings ) { mObstacleSettings = settings; }

    /**
     * Returns the label thinning settings.
     * \see setThinningSettings()
     * \note Not available in Python bindings
     * \since QGIS 3.12
     */
    const QgsLabelThinningSettings &thinningSettings() const SIP_SKIP { return mThinningSettings; }

    /**
     * Returns the label thinning settings.
     * \see setThinningSettings()
     * \since QGIS 3.12
     */
    QgsLabelThinningSettings &thinningSettings() { return mThinningSettings; }

    /**
     * Sets the label thinning \a settings.
     * \see thinningSettings()
     * \since QGIS 3.12
     */
    void setThinningSettings( const QgsLabelThinningSettings &settings ) { mThinningSettings = settings; }

    /**
     * Returns the label placement settings.
     * \see setPlacementSettings()
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    const QgsLabelPlacementSettings &placementSettings() const SIP_SKIP { return mPlacementSettings; }

    /**
     * Returns the label placement settings.
     * \see setPlacementSettings()
     * \since QGIS 3.26
     */
    QgsLabelPlacementSettings &placementSettings() { return mPlacementSettings; }

    /**
     * Sets the label placement \a settings.
     * \see placementSettings()
     * \since QGIS 3.26
     */
    void setPlacementSettings( const QgsLabelPlacementSettings &settings ) { mPlacementSettings = settings; }

    /**
    * Returns a pixmap preview for label \a settings.
    * \param settings label settings
    * \param size target pixmap size
    * \param previewText text to render in preview, or empty for default text
    * \param padding space between icon edge and color ramp
    * \param screen can be used to specify the destination screen properties for the icon. This allows the icon to be generated using the correct DPI and device pixel ratio for the target screen (since QGIS 3.32)
    * \since QGIS 3.10
    */
    static QPixmap labelSettingsPreviewPixmap( const QgsPalLayerSettings &settings, QSize size, const QString &previewText = QString(), int padding = 0, const QgsScreenProperties &screen = QgsScreenProperties() );

    /**
     * Returns the layer's unplaced label visibility.
     *
     * \see setUnplacedVisibility()
     * \since QGIS 3.20
     */
    Qgis::UnplacedLabelVisibility unplacedVisibility() const;

    /**
     * Sets the layer's unplaced label \a visibility.
     *
     * \see unplacedVisibility()
     * \since QGIS 3.20
     */
    void setUnplacedVisibility( Qgis::UnplacedLabelVisibility visibility );

    // temporary stuff: set when layer gets prepared or labeled
    const QgsFeature *mCurFeat = nullptr;
    QgsFields mCurFields;
    int fieldIndex = 0;
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

    /**
     * Calculates the space required to render the provided \a text in map units.
     * Results will be written to \a size.
     *
     * If the text orientation is set to rotation-based, the space taken to render
     * vertically oriented text will be written to \a rotatedSize.
     */
    void calculateLabelSize( const QFontMetricsF &fm, const QString &text, QgsRenderContext &context,
                             const QgsTextFormat &format,
                             QgsTextDocument *document,
                             QgsTextDocumentMetrics *documentMetrics,
                             QSizeF &size, QSizeF &rotatedSize,
                             QRectF &outerBounds );


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
                         Qgis::RenderUnit fontunits,
                         QgsRenderContext &context );

    void parseTextBuffer( QgsRenderContext &context );

    void parseTextMask( QgsRenderContext &context );

    void parseTextFormatting( QgsRenderContext &context );

    void parseShapeBackground( QgsRenderContext &context );

    void parseDropShadow( QgsRenderContext &context );

    /**
     * Checks if a feature is larger than a minimum size (in mm)
     * \returns TRUE if above size, FALSE if below
    */
    bool checkMinimumSizeMM( const QgsRenderContext &ct, const QgsGeometry &geom, double minSize ) const;

    /**
     * Registers a feature as an obstacle only (no label rendered)
     */
    std::unique_ptr< QgsLabelFeature > registerObstacleFeature( const QgsFeature &f, QgsRenderContext &context, const QgsGeometry &obstacleGeometry = QgsGeometry() );

    QMap<Property, QVariant> dataDefinedValues;

    //! Property collection for data defined label settings
    QgsPropertyCollection mDataDefinedProperties;

    QgsExpression *expression = nullptr;

    std::unique_ptr< QFontDatabase > mFontDB;

    QgsTextFormat mFormat;

    std::unique_ptr< QgsCallout > mCallout;

    QgsLabelPlacementSettings mPlacementSettings;
    QgsLabelLineSettings mLineSettings;
    QgsLabelPointSettings mPointSettings;
    QgsLabelObstacleSettings mObstacleSettings;
    QgsLabelThinningSettings mThinningSettings;

    Qgis::LabelPolygonPlacementFlags mPolygonPlacementFlags = Qgis::LabelPolygonPlacementFlag::AllowPlacementInsideOfPolygon;

    QgsExpression mGeometryGeneratorExpression;

    bool mRenderStarted = false;

    QString mLegendString = QObject::tr( "Aa" );

    Qgis::UnplacedLabelVisibility mUnplacedVisibility = Qgis::UnplacedLabelVisibility::FollowEngineSetting;

    //! Unit for rotation of labels.
    Qgis::AngleUnit mRotationUnit = Qgis::AngleUnit::Degrees;

    static void initPropertyDefinitions();
};


/**
 * \ingroup core
 * \class QgsPalLabeling
 * \brief PAL labeling utilities.
 */
class CORE_EXPORT QgsPalLabeling
{
  public:

    /**
     * Called to find out whether a specified \a layer is used for labeling.
     */
    static bool staticWillUseLayer( const QgsMapLayer *layer );

    /**
     * Prepares a geometry for registration with PAL. Handles reprojection, rotation, clipping, etc.
     * \param geometry geometry to prepare
     * \param context render context
     * \param ct coordinate transform, or invalid transform if no transformation required
     * \param clipGeometry geometry to clip features to, if applicable
     * \param mergeLines TRUE if touching lines from this layer will be merged and treated as single features during labeling
     * \returns prepared geometry
     */
    static QgsGeometry prepareGeometry( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry(), bool mergeLines = false ) SIP_FACTORY;

    /**
     * Checks whether a geometry requires preparation before registration with PAL
     * \param geometry geometry to prepare
     * \param context render context
     * \param ct coordinate transform, or invalid transform if no transformation required
     * \param clipGeometry geometry to clip features to, if applicable
     * \param mergeLines TRUE if touching lines from this layer will be merged and treated as single features during labeling
     * \returns TRUE if geometry requires preparation
     */
    static bool geometryRequiresPreparation( const QgsGeometry &geometry, QgsRenderContext &context, const QgsCoordinateTransform &ct, const QgsGeometry &clipGeometry = QgsGeometry(), bool mergeLines = false );

    /**
     * Splits a \a text string to a list of separate lines, using a specified wrap character (\a wrapCharacter).
     * The text string will be split on either newline characters or the wrap character.
     *
     * Since QGIS 3.4 the \a autoWrapLength argument can be used to specify an ideal length of line to automatically
     * wrap text to (automatic wrapping is disabled if \a autoWrapLength is 0). This automatic wrapping is performed
     * after processing wrapping using \a wrapCharacter. When auto wrapping is enabled, the \a useMaxLineLengthWhenAutoWrapping
     * argument controls whether the lines should be wrapped to an ideal maximum of \a autoWrapLength characters, or
     * if FALSE then the lines are wrapped to an ideal minimum length of \a autoWrapLength characters.
     *
     */
    static QStringList splitToLines( const QString &text, const QString &wrapCharacter, int autoWrapLength = 0, bool useMaxLineLengthWhenAutoWrapping = true );

    /**
     * Splits a text string to a list of graphemes, which are the smallest allowable character
     * divisions in the string. This accounts for scripts were individual characters are not
     * allowed to be split apart (e.g., Arabic and Indic based scripts)
     * \param text string to split
     * \returns list of graphemes
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

    //! Update temporary QgsPalLayerSettings with any data defined mask values
    static void dataDefinedTextMask( QgsPalLayerSettings &tmpLyr,
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
     * \returns TRUE if geometry exceeds minimum size
     */
    static bool checkMinimumSizeMM( const QgsRenderContext &context, const QgsGeometry &geom, double minSize );

    friend class QgsPalLayerSettings;
};


#endif // QGSPALLABELING_H
