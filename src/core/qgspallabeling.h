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

class QFontMetricsF;
class QPainter;
class QPicture;
class QgsGeometry;
class QgsMapRenderer;
class QgsRectangle;
class QgsCoordinateTransform;
class QgsLabelSearchTree;

#include <QString>
#include <QFont>
#include <QFontDatabase>
#include <QColor>
#include <QHash>
#include <QList>
#include <QRectF>

namespace pal
{
  class Pal;
  class Layer;
  class LabelPosition;
}

class QgsMapToPixel;
class QgsFeature;

#include "qgspoint.h"
#include "qgsrectangle.h"
#include "qgsmaprenderer.h" // definition of QgsLabelingEngineInterface
#include "qgsexpression.h"
#include "qgsdatadefined.h"
#include "qgsdiagramrendererv2.h"

class QgsPalGeometry;
class QgsVectorLayer;

class CORE_EXPORT QgsPalLayerSettings
{
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

      // rendering
      ScaleVisibility = 23,
      MinScale = 16,
      MaxScale = 17,
      FontLimitPixel = 24,
      FontMinPixel = 25,
      FontMaxPixel = 26,
      // (data defined only)
      Show = 15,
      AlwaysShow = 20
    };


    // whether to label this layer
    bool enabled;

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
    RotationType shapeRotationType;
    double shapeRotation;
    QPointF shapeOffset;
    SizeUnit shapeOffsetUnits;
    QPointF shapeRadii;
    SizeUnit shapeRadiiUnits;
    int shapeTransparency;
    QPainter::CompositionMode shapeBlendMode;
    QColor shapeFillColor;
    QColor shapeBorderColor;
    double shapeBorderWidth;
    SizeUnit shapeBorderWidthUnits;
    Qt::PenJoinStyle shapeJoinStyle;

    //-- drop shadow

    bool shadowDraw;
    ShadowType shadowUnder;
    int shadowOffsetAngle;
    double shadowOffsetDist;
    SizeUnit shadowOffsetUnits;
    bool shadowOffsetGlobal;
    double shadowRadius;
    SizeUnit shadowRadiusUnits;
    bool shadowRadiusAlphaOnly;
    int shadowTransparency;
    int shadowScale;
    QColor shadowColor;
    QPainter::CompositionMode shadowBlendMode;

    //-- placement

    Placement placement;
    unsigned int placementFlags;

    bool centroidWhole; // whether centroid calculated from whole or visible polygon
    double dist; // distance from the feature (in mm)
    bool distInMapUnits; //true if distance is in map units (otherwise in mm)

    // offset labels of point/centroid features default to center
    // move label to quadrant: left/down, don't move, right/up (-1, 0, 1)
    QuadrantPosition quadOffset;

    double xOffset; // offset from point in mm or map units
    double yOffset; // offset from point in mm or map units
    bool labelOffsetInMapUnits; //true if label offset is in map units (otherwise in mm)
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

    //-- scale factors
    double vectorScaleFactor; //scale factor painter units->pixels
    double rasterCompressFactor; //pixel resolution scale factor

    // called from register feature hook
    void calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY, QgsFeature* f = 0 );

    // implementation of register feature hook
    void registerFeature( QgsVectorLayer* layer, QgsFeature& f, const QgsRenderContext& context );

    void readFromLayer( QgsVectorLayer* layer );
    void writeToLayer( QgsVectorLayer* layer );

    /** Get a data defined property pointer
     * @note added in 1.9, helpful for Python access
     */
    QgsDataDefined* dataDefinedProperty( DataDefinedProperties p );

    /** Get current data defined properties
     * @note added in 1.9, helpful for Python access
     */
    QList<QgsPalLayerSettings::DataDefinedProperties> dataDefinedPropertyList();

    /** Set a property as data defined
     * @note added in 1.9, helpful for Python access
     */
    void setDataDefinedProperty( QgsPalLayerSettings::DataDefinedProperties p,
                                 bool active, bool useExpr, const QString& expr, const QString& field );

    /** Set a property to static instead data defined */
    void removeDataDefinedProperty( QgsPalLayerSettings::DataDefinedProperties p );

    /** Convert old property value to new one as delimited values
     * @note not available in python bindings; added in 1.9, as temporary solution until refactoring of project settings
     */
    QString updateDataDefinedString( const QString& value );

    /** Get property value as separate values split into Qmap
     * @note not available in python bindings; added in 1.9
     */
    QMap<QString, QString> dataDefinedMap( QgsPalLayerSettings::DataDefinedProperties p ) const;

    /** Get data defined property value from expression string or attribute field name
     * @returns value inside QVariant
     * @note not available in python bindings; added in 1.9
     */
    QVariant dataDefinedValue( QgsPalLayerSettings::DataDefinedProperties p, QgsFeature& f, const QgsFields& fields ) const;

    /** Get data defined property value from expression string or attribute field name
     * @returns true/false whether result is null or invalid
     * @note not available in python bindings; added in 1.9
     */
    bool dataDefinedEvaluate( QgsPalLayerSettings::DataDefinedProperties p, QVariant& exprVal ) const;

    /** Whether data definition is active
     */
    bool dataDefinedIsActive( QgsPalLayerSettings::DataDefinedProperties p ) const;

    /** Whether data definition is set to use an expression
     */
    bool dataDefinedUseExpression( QgsPalLayerSettings::DataDefinedProperties p ) const;

    /**Stores field names for data defined layer properties*/
    //! @note not available in python bindings
    QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > dataDefinedProperties;


    /** Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @return font pixel size
     */
    int sizeToPixel( double size, const QgsRenderContext& c , SizeUnit unit, bool rasterfactor = false ) const;

    /** Calculates size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @return size that will render, as double
     * @note added in 1.9, as a better precision replacement for sizeToPixel
     */
    double scaleToPixelContext( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor = false ) const;

    /** Map of data defined enum to names and old-style indecies
     * The QPair contains a new string for layer property key, and a reference to old-style numeric key (< QGIS 2.0)
     * @note not available in python bindings; added in 1.9
     */
    QMap<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > dataDefinedNames() const { return mDataDefinedNames; }

    // temporary stuff: set when layer gets prepared or labeled
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

    /**Checks if a feature is larger than a minimum size (in mm)
    @return true if above size, false if below*/
    bool checkMinimumSizeMM( const QgsRenderContext& ct, QgsGeometry* geom, double minSize ) const;


    QMap<DataDefinedProperties, QVariant> dataDefinedValues;
    QgsExpression* expression;
    QMap<QgsPalLayerSettings::DataDefinedProperties, QPair<QString, int> > mDataDefinedNames;

    QFontDatabase mFontDB;
    /**Updates font with named style */
    void updateFontViaStyle( QFont& font, const QString & fontstyle );
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
    QgsLabelComponent(): mText( QString() )
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

    const QString& text() { return mText; }
    void setText( const QString& text ) { mText = text; }

    const QgsPoint& origin() { return mOrigin; }
    void setOrigin( QgsPoint point ) { mOrigin = point; }

    bool useOrigin() const { return mUseOrigin; }
    void setUseOrigin( bool use ) { mUseOrigin = use; }

    double rotation() const { return mRotation; }
    void setRotation( double rotation ) { mRotation = rotation; }

    double rotationOffset() const { return mRotationOffset; }
    void setRotationOffset( double rotation ) { mRotationOffset = rotation; }

    bool useRotation() const { return mUseRotation; }
    void setUseRotation( bool use ) { mUseRotation = use; }

    const QgsPoint& center() { return mCenter; }
    void setCenter( QgsPoint point ) { mCenter = point; }

    bool useCenter() const { return mUseCenter; }
    void setUseCenter( bool use ) { mUseCenter = use; }

    const QgsPoint& size() { return mSize; }
    void setSize( QgsPoint point ) { mSize = point; }

    const QgsPoint& offset() { return mOffset; }
    void setOffset( QgsPoint point ) { mOffset = point; }

    const QPicture* picture() { return mPicture; }
    void setPicture( QPicture* picture ) { mPicture = picture; }

    double pictureBuffer() const { return mPictureBuffer; }
    void setPictureBuffer( double buffer ) { mPictureBuffer = buffer; }

    double dpiRatio() const { return mDpiRatio; }
    void setDpiRatio( double ratio ) { mDpiRatio = ratio; }

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

    QgsPalLayerSettings& layer( const QString& layerName );

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

    // implemented methods from labeling engine interface

    //! called when we're going to start with rendering
    virtual void init( QgsMapRenderer* mr );
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer );
    //! clears all PAL layer settings for registered layers
    //! @note: this method was added in version 1.9
    virtual void clearActiveLayers();
    //! clears data defined objects from PAL layer settings for a registered layer
    //! @note: this method was added in version 1.9
    virtual void clearActiveLayer( QgsVectorLayer* layer );
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
    virtual QgsLabelingEngineInterface* clone();

    //! @note not available in python bindings
    void drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform );
    //!drawLabel
    //! @note not available in python bindings
    void drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, DrawLabelType drawType );

    static void drawLabelBuffer( QgsRenderContext& context,
                                 QgsLabelComponent component,
                                 const QgsPalLayerSettings& tmpLyr );

    static void drawLabelBackground( QgsRenderContext& context,
                                     QgsLabelComponent component,
                                     const QgsPalLayerSettings& tmpLyr );

    static void drawLabelShadow( QgsRenderContext& context,
                                 QgsLabelComponent component,
                                 const QgsPalLayerSettings& tmpLyr );

    //! load/save engine settings to project file
    //! @note added in QGIS 1.9
    void loadEngineSettings();
    void saveEngineSettings();
    void clearEngineSettings();
    bool isStoredWithProject() const { return mSavedWithProject; }
    void setStoredWithProject( bool store ) { mSavedWithProject = store; }

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

    // hashtable of layer settings, being filled during labeling
    QHash<QgsVectorLayer*, QgsPalLayerSettings> mActiveLayers;
    // hashtable of active diagram layers
    QHash<QgsVectorLayer*, QgsDiagramLayerSettings> mActiveDiagramLayers;
    QgsPalLayerSettings mInvalidLayerSettings;

    QgsMapRenderer* mMapRenderer;
    int mCandPoint, mCandLine, mCandPolygon;
    Search mSearch;

    pal::Pal* mPal;

    // list of candidates from last labeling
    QList<QgsLabelCandidate> mCandidates;
    bool mShowingCandidates;
    bool mShowingAllLabels; // whether to avoid collisions or not
    bool mSavedWithProject; // whether engine settings have been read from project file
    bool mShowingShadowRects; // whether to show debugging rectangles for drop shadows

    QgsLabelSearchTree* mLabelSearchTree;
};

#endif // QGSPALLABELING_H
