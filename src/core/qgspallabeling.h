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
class QgsGeometry;
class QgsMapRenderer;
class QgsRectangle;
class QgsCoordinateTransform;
class QgsLabelSearchTree;
class QgsDiagramLayerSettings;

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
    QgsMapRenderer::BlendMode blendMode;
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
    double bufferSize; //buffer size (in mm)
    QColor bufferColor;
    int bufferTransp;
    QgsMapRenderer::BlendMode bufferBlendMode;
    Qt::PenJoinStyle bufferJoinStyle;
    bool bufferNoFill; //set interior of buffer to 100% transparent
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
    //! @note not available in python bindings
    QMap< DataDefinedProperties, QString > dataDefinedProperties;

    bool preserveRotation; // preserve predefined rotation data during label pin/unpin operations

    /**Calculates pixel size (considering output size should be in pixel or map units, scale factors and oversampling)
     @param size size to convert
     @param c rendercontext
     @param buffer whether it buffer size being calculated
     @return font pixel size*/
    int sizeToPixel( double size, const QgsRenderContext& c , bool buffer = false ) const;

    /** List of data defined enum names
     * @note adding in 1.9
     */
    QList<QString> dataDefinedNames() const { return mDataDefinedNames; }

    // temporary stuff: set when layer gets prepared or labeled
    pal::Layer* palLayer;
    int fieldIndex;
    const QgsMapToPixel* xform;
    const QgsCoordinateTransform* ct;
    QgsPoint ptZero, ptOne;
    QList<QgsPalGeometry*> geometries;
    QgsGeometry* extentGeom;
    int mFeaturesToLabel; // total features that will probably be labeled, may be less (figured before PAL)
    int mFeatsSendingToPal; // total features tested for sending into PAL (relative to maxNumLabels)
    int mFeatsRegPal; // number of features registered in PAL, when using limitNumLabels

  private:
    void readDataDefinedPropertyMap( QgsVectorLayer* layer,
                                     QMap < QgsPalLayerSettings::DataDefinedProperties,
                                     QString > & propertyMap );
    void writeDataDefinedPropertyMap( QgsVectorLayer* layer,
                                      const QMap < QgsPalLayerSettings::DataDefinedProperties,
                                      QString > & propertyMap );
    void readDataDefinedProperty( QgsVectorLayer* layer,
                                  QgsPalLayerSettings::DataDefinedProperties p,
                                  QMap < QgsPalLayerSettings::DataDefinedProperties,
                                  QString > & propertyMap );

    /**Checks if a feature is larger than a minimum size (in mm)
    @return true if above size, false if below*/
    bool checkMinimumSizeMM( const QgsRenderContext& ct, QgsGeometry* geom, double minSize ) const;

    QgsExpression* expression;
    QList<QString> mDataDefinedNames;

    QFontDatabase mFontDB;
    /**Updates layer font with one of its named styles */
    void updateFontViaStyle( const QString & fontstyle );
};

class CORE_EXPORT QgsLabelCandidate
{
  public:
    QgsLabelCandidate( QRectF r, double c ): rect( r ), cost( c ) {}

    QRectF rect;
    double cost;
};

class CORE_EXPORT QgsPalLabeling : public QgsLabelingEngineInterface
{
  public:
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

    bool isShowingAllLabels() const { return mShowingAllLabels; }
    void setShowingAllLabels( bool showing ) { mShowingAllLabels = showing; }

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
    virtual QgsLabelingEngineInterface* clone();

    //! @note not available in python bindings
    void drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform );
    //!drawLabel
    //! @note not available in python bindings
    void drawLabel( pal::LabelPosition* label, QPainter* painter, const QFont& f, const QColor& c, const QgsMapToPixel* xform, double bufferSize = -1,
                    const QColor& bufferColor = QColor( 255, 255, 255 ), bool drawBuffer = false );
    static void drawLabelBuffer( QPainter* p, QString text, const QFont& font, double size, QColor color , Qt::PenJoinStyle joinstyle = Qt::BevelJoin, bool noFill = false );

    //! load/save engine settings to project file
    //! @note added in QGIS 1.9
    void loadEngineSettings();
    void saveEngineSettings();
    void clearEngineSettings();
    bool isStoredWithProject() const { return mSavedWithProject; }
    void setStoredWithProject( bool store ) { mSavedWithProject = store; }

  protected:
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

    QgsLabelSearchTree* mLabelSearchTree;
};

#endif // QGSPALLABELING_H
