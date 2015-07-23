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

#include <QString>
#include <QFont>
#include <QFontDatabase>
#include <QColor>
#include <QHash>
#include <QList>
#include <QRectF>
#include "qgspoint.h"
#include "qgsmaprenderer.h" // definition of QgsLabelingEngineInterface
#include "qgsdiagramrendererv2.h"
#include "qgsmapunitscale.h"

namespace pal
{
  class Pal;
  class Layer;
  class LabelPosition;
}

class QgsRectangle;
class QgsMapToPixel;
class QgsFeature;
class QgsPalGeometry;
class QgsVectorLayer;
class QgsDataDefined;
class QgsExpression;
class QFontMetricsF;
class QPainter;
class QPicture;
class QgsGeometry;
class QgsMapRenderer;
class QgsCoordinateTransform;
class QgsLabelSearchTree;
class QgsMapSettings;

class CORE_EXPORT QgsPalLayerSettings
{
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
      OnLine    = 1,
      AboveLine = 2,
      BelowLine = 4,
      MapOrientation = 8
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
      MultiLeft = 0,
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

    enum ShadowType
    {
      ShadowLowest = 0,
      ShadowText,
      ShadowBuffer,
      ShadowShape
    };

    // update mDataDefinedNames QMap in constructor when adding/deleting enum value
    enum DataDefinedProperties
    {
      // text style
      Size = 0,
      Bold = 1,
      Italic = 2,
      Underline = 3,
      Color = 4,
      Strikeout = 5,
      Family = 6,
      FontStyle = 21,
      FontSizeUnit = 22,
      FontTransp = 18,
      FontCase = 27,
      FontLetterSpacing = 28,
      FontWordSpacing = 29,
      FontBlendMode = 30,

      // text formatting
      MultiLineWrapChar = 31,
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
      BufferTransp = 19,
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
      ShapeTransparency = 63,
      ShapeBlendMode = 64,
      ShapeFillColor = 58,
      ShapeBorderColor = 59,
      ShapeBorderWidth = 60,
      ShapeBorderWidthUnits = 61,
      ShapeJoinStyle = 62,

      // drop shadow
      ShadowDraw = 65,
      ShadowUnder = 66,
      ShadowOffsetAngle = 67,
      ShadowOffsetDist = 68,
      ShadowOffsetUnits = 69,
      ShadowRadius = 70,
      ShadowRadiusUnits = 71,
      ShadowTransparency = 72,
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
      PositionX = 9, //x-coordinate data defined label position
      PositionY = 10, //y-coordinate data defined label position
      Hali = 11, //horizontal alignment for data defined label position (Left, Center, Right)
      Vali = 12, //vertical alignment for data defined label position (Bottom, Base, Half, Cap, Top)
      Rotation = 14, //data defined rotation
      RepeatDistance = 84,
      RepeatDistanceUnit = 86,
      Priority = 87,

      // rendering
      ScaleVisibility = 23,
      MinScale = 16,
      MaxScale = 17,
      FontLimitPixel = 24,
      FontMinPixel = 25,
      FontMaxPixel = 26,
      IsObstacle = 88,
      ObstacleFactor = 89,

      // (data defined only)
      Show = 15,
      AlwaysShow = 20
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
    void calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY, QgsFeature* f = 0 );

    // implementation of register feature hook
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
    QVariant dataDefinedValue( QgsPalLayerSettings::DataDefinedProperties p, QgsFeature& f, const QgsFields& fields ) const;

    /** Get data defined property value from expression string or attribute field name
     * @returns true/false whether result is null or invalid
     * @note not available in python bindings
     */
    bool dataDefinedEvaluate( QgsPalLayerSettings::DataDefinedProperties p, QVariant& exprVal ) const;

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
    QMap<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > dataDefinedNames() const { return mDataDefinedNames; }

    // temporary stuff: set when layer gets prepared or labeled
    // NOTE: not in Python binding
    pal::Layer* palLayer;
    QgsFeature* mCurFeat;
    const QgsFields* mCurFields;
    int fieldIndex;
    const QgsMapToPixel* xform;
    const QgsCoordinateTransform* ct;
    QgsPoint ptZero, ptOne;
    QList<QgsPalGeometry*> geometries;
    QgsGeometry* extentGeom;
    int mFeaturesToLabel; // total features that will probably be labeled, may be less (figured before PAL)
    int mFeatsSendingToPal; // total features tested for sending into PAL (relative to maxNumLabels)
    int mFeatsRegPal; // number of features registered in PAL, when using limitNumLabels

    QString mTextFontFamily;
    bool mTextFontFound;

    bool showingShadowRects; // whether to show debug rectangles for drop shadows

  private:

    void readDataDefinedPropertyMap( QgsVectorLayer* layer,
                                     QMap < QgsPalLayerSettings::DataDefinedProperties,
                                     QgsDataDefined* > & propertyMap );
    void writeDataDefinedPropertyMap( QgsVectorLayer* layer,
                                      const QMap < QgsPalLayerSettings::DataDefinedProperties,
                                      QgsDataDefined* > & propertyMap );
    void readDataDefinedProperty( QgsVectorLayer* layer,
                                  QgsPalLayerSettings::DataDefinedProperties p,
                                  QMap < QgsPalLayerSettings::DataDefinedProperties,
                                  QgsDataDefined* > & propertyMap );

    // convenience data defined evaluation function
    bool dataDefinedValEval( const QString& valType,
                             QgsPalLayerSettings::DataDefinedProperties p,
                             QVariant& exprVal );

    void parseTextStyle( QFont& labelFont,
                         QgsPalLayerSettings::SizeUnit fontunits,
                         const QgsRenderContext& context );

    void parseTextBuffer();

    void parseTextFormatting();

    void parseShapeBackground();

    void parseDropShadow();

    /** Checks if a feature is larger than a minimum size (in mm)
    @return true if above size, false if below*/
    bool checkMinimumSizeMM( const QgsRenderContext& ct, const QgsGeometry* geom, double minSize ) const;

    /** Registers a feature as an obstacle only (no label rendered)
     */
    void registerObstacleFeature( QgsFeature &f, const QgsRenderContext &context, QString dxfLayer );

    QMap<DataDefinedProperties, QVariant> dataDefinedValues;
    QgsExpression* expression;
    QMap<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > mDataDefinedNames;

    QFontDatabase mFontDB;
};

class CORE_EXPORT QgsLabelCandidate
{
  public:
    QgsLabelCandidate( QRectF r, double c ): rect( r ), cost( c ) {}

    QRectF rect;
    double cost;
};

/** \ingroup core
  * Maintains current state of more grainular and temporal values when creating/painting
  * component parts of an individual label (e.g. buffer, background, shadow, etc.).
  */
class CORE_EXPORT QgsLabelComponent
{
  public:
    QgsLabelComponent()
        : mText( QString() )
        , mOrigin( QgsPoint() )
        , mUseOrigin( false )
        , mRotation( 0.0 )
        , mRotationOffset( 0.0 )
        , mUseRotation( false )
        , mCenter( QgsPoint() )
        , mUseCenter( false )
        , mSize( QgsPoint() )
        , mOffset( QgsPoint() )
        , mPicture( 0 )
        , mPictureBuffer( 0.0 )
        , mDpiRatio( 1.0 )
    {}

    // methods

    const QString& text() const { return mText; }
    void setText( const QString& text ) { mText = text; }

    const QgsPoint& origin() const { return mOrigin; }
    void setOrigin( const QgsPoint& point ) { mOrigin = point; }

    bool useOrigin() const { return mUseOrigin; }
    void setUseOrigin( const bool use ) { mUseOrigin = use; }

    double rotation() const { return mRotation; }
    void setRotation( const double rotation ) { mRotation = rotation; }

    double rotationOffset() const { return mRotationOffset; }
    void setRotationOffset( const double rotation ) { mRotationOffset = rotation; }

    bool useRotation() const { return mUseRotation; }
    void setUseRotation( const bool use ) { mUseRotation = use; }

    const QgsPoint& center() const { return mCenter; }
    void setCenter( const QgsPoint& point ) { mCenter = point; }

    bool useCenter() const { return mUseCenter; }
    void setUseCenter( const bool use ) { mUseCenter = use; }

    const QgsPoint& size() const { return mSize; }
    void setSize( const QgsPoint& point ) { mSize = point; }

    const QgsPoint& offset() const { return mOffset; }
    void setOffset( const QgsPoint& point ) { mOffset = point; }

    const QPicture* picture() const { return mPicture; }
    void setPicture( QPicture* picture ) { mPicture = picture; }

    double pictureBuffer() const { return mPictureBuffer; }
    void setPictureBuffer( const double buffer ) { mPictureBuffer = buffer; }

    double dpiRatio() const { return mDpiRatio; }
    void setDpiRatio( const double ratio ) { mDpiRatio = ratio; }

  private:
    // current label component text,
    // e.g. single line in a multi-line label or charcater in curved labeling
    QString mText;
    // current origin point for painting (generally current painter rotation point)
    QgsPoint mOrigin;
    // whether to translate the painter to supplied origin
    bool mUseOrigin;
    // any rotation to be applied to painter (in radians)
    double mRotation;
    // any rotation to be applied to painter (in radians) after initial rotation
    double mRotationOffset;
    // whether to use the rotation to rotate the painter
    bool mUseRotation;
    // current center point of label compnent, after rotation
    QgsPoint mCenter;
    // whether to translate the painter to supplied origin based upon center
    bool mUseCenter;
    // width and height of label component, transformed and ready for painting
    QgsPoint mSize;
    // any translation offsets to be applied before painting, transformed and ready for painting
    QgsPoint mOffset;

    // a stored QPicture of painting for the component
    QPicture* mPicture;
    // buffer for component to accommodate graphic items ignored by QPicture,
    // e.g. half-width of an applied QPen, which would extend beyond boundingRect() of QPicture
    double mPictureBuffer;

    // a ratio of native painter dpi and that of rendering context's painter
    double mDpiRatio;
};


/**
 * Class that stores computed placement from labeling engine.
 * @note added in 2.4
 */
class CORE_EXPORT QgsLabelingResults
{
  public:
    QgsLabelingResults();
    ~QgsLabelingResults();

    //! return infos about labels at a given (map) position
    QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p ) const;
    //! return infos about labels within a given (map) rectangle
    QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r ) const;

  private:
    QgsLabelingResults( const QgsLabelingResults& ) {} // no copying allowed

    QgsLabelSearchTree* mLabelSearchTree;

    friend class QgsPalLabeling;
};

Q_NOWARN_DEPRECATED_PUSH
class CORE_EXPORT QgsPalLabeling : public QgsLabelingEngineInterface
{
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

    QgsPalLayerSettings& layer( const QString& layerName ) override;

    void numCandidatePositions( int& candPoint, int& candLine, int& candPolygon );
    void setNumCandidatePositions( int candPoint, int candLine, int candPolygon );

    enum Search { Chain, Popmusic_Tabu, Popmusic_Chain, Popmusic_Tabu_Chain, Falp };

    void setSearchMethod( Search s );
    Search searchMethod() const;

    bool isShowingCandidates() const { return mShowingCandidates; }
    void setShowingCandidates( bool showing ) { mShowingCandidates = showing; }
    const QList<QgsLabelCandidate>& candidates() { return mCandidates; }

    bool isShowingShadowRectangles() const { return mShowingShadowRects; }
    void setShowingShadowRectangles( bool showing ) { mShowingShadowRects = showing; }

    bool isShowingAllLabels() const { return mShowingAllLabels; }
    void setShowingAllLabels( bool showing ) { mShowingAllLabels = showing; }

    bool isShowingPartialsLabels() const { return mShowingPartialsLabels; }
    void setShowingPartialsLabels( bool showing ) { mShowingPartialsLabels = showing; }

    //! @note added in 2.4
    bool isDrawingOutlineLabels() const { return mDrawOutlineLabels; }
    void setDrawingOutlineLabels( bool outline ) { mDrawOutlineLabels = outline; }

    // implemented methods from labeling engine interface

    //! called when we're going to start with rendering
    //! @deprecated since 2.4 - use override with QgsMapSettings
    Q_DECL_DEPRECATED virtual void init( QgsMapRenderer* mr ) override;
    //! called when we're going to start with rendering
    virtual void init( const QgsMapSettings& mapSettings ) override;
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer ) override;

    //! called to find out whether the layer is used for labeling
    //! @note added in 2.4
    static bool staticWillUseLayer( QgsVectorLayer* layer );
    static bool staticWillUseLayer( const QString& layerID );

    //! clears all PAL layer settings for registered layers
    virtual void clearActiveLayers() override;
    //! clears data defined objects from PAL layer settings for a registered layer
    virtual void clearActiveLayer( const QString& layerID ) override;
    //! hook called when drawing layer before issuing select()
    virtual int prepareLayer( QgsVectorLayer* layer, QStringList &attrNames, QgsRenderContext& ctx ) override;
    //! adds a diagram layer to the labeling engine
    virtual int addDiagramLayer( QgsVectorLayer* layer, const QgsDiagramLayerSettings *s ) override;
    //! hook called when drawing for every feature in a layer
    virtual void registerFeature( const QString& layerID, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext(), QString dxfLayer = QString::null ) override;
    virtual void registerDiagramFeature( const QString& layerID, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() ) override;
    //! called when the map is drawn and labels should be placed
    virtual void drawLabeling( QgsRenderContext& context ) override;
    //! called when we're done with rendering
    virtual void exit() override;

    //! return infos about labels at a given (map) position
    //! @deprecated since 2.4 - use takeResults() and methods of QgsLabelingResults
    Q_DECL_DEPRECATED virtual QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p ) override;
    //! return infos about labels within a given (map) rectangle
    //! @deprecated since 2.4 - use takeResults() and methods of QgsLabelingResults
    Q_DECL_DEPRECATED virtual QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r ) override;

    //! Return pointer to recently computed results (in drawLabeling()) and pass the ownership of results to the caller
    //! @note added in 2.4
    QgsLabelingResults* takeResults();

    //! called when passing engine among map renderers
    virtual QgsLabelingEngineInterface* clone() override;

    //! @note not available in python bindings
    void drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform );
    //!drawLabel
    //! @note not available in python bindings
    virtual void drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, DrawLabelType drawType, double dpiRatio = 1.0 );

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
    Q_DECL_DEPRECATED bool isStoredWithProject() const { return true; }
    //! @deprecated since 2.4 - settings are always stored in project
    Q_DECL_DEPRECATED void setStoredWithProject( bool store ) { Q_UNUSED( store ); }

    /** Prepares a geometry for registration with PAL. Handles reprojection, rotation, clipping, etc.
     * @param geometry geometry to prepare
     * @param context render context
     * @param ct coordinate transform
     * @param clipGeometry geometry to clip features to, if applicable
     * @returns prepared geometry, the caller takes ownership
     * @note added in QGIS 2.9
     */
    static QgsGeometry* prepareGeometry( const QgsGeometry *geometry, const QgsRenderContext &context, const QgsCoordinateTransform *ct, QgsGeometry *clipGeometry = 0 );

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

    // hashtable of layer settings, being filled during labeling (key = layer ID)
    QHash<QString, QgsPalLayerSettings> mActiveLayers;
    // hashtable of active diagram layers (key = layer ID)
    QHash<QString, QgsDiagramLayerSettings> mActiveDiagramLayers;
    QgsPalLayerSettings mInvalidLayerSettings;

    const QgsMapSettings* mMapSettings;
    int mCandPoint, mCandLine, mCandPolygon;
    Search mSearch;

    pal::Pal* mPal;

    // list of candidates from last labeling
    QList<QgsLabelCandidate> mCandidates;
    bool mShowingCandidates;
    bool mShowingAllLabels; // whether to avoid collisions or not
    bool mShowingShadowRects; // whether to show debugging rectangles for drop shadows
    bool mShowingPartialsLabels; // whether to avoid partials labels or not
    bool mDrawOutlineLabels; // whether to draw labels as text or outlines

    QgsLabelingResults* mResults;

    friend class QgsPalLayerSettings;
};
Q_NOWARN_DEPRECATED_POP


#endif // QGSPALLABELING_H
