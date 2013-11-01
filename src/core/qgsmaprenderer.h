/***************************************************************************
    qgsmaprender.h  -  class for rendering map layer set
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPRENDER_H
#define QGSMAPRENDER_H

#include <QMutex>
#include <QSize>
#include <QStringList>
#include <QVector>
#include <QPainter>

#include "qgis.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgsfeature.h"

class QDomDocument;
class QDomNode;
class QPainter;

class QgsMapToPixel;
class QgsMapLayer;
class QgsMapRenderer;
class QgsScaleCalculator;
class QgsCoordinateReferenceSystem;
class QgsDistanceArea;
class QgsVectorLayer;

class QgsPalLayerSettings;
class QgsDiagramLayerSettings;

class CORE_EXPORT QgsLabelPosition
{
  public:
    QgsLabelPosition( int id, double r, const QVector< QgsPoint >& corners, const QgsRectangle& rect, double w, double h, const QString& layer, const QString& labeltext, const QFont& labelfont, bool upside_down, bool diagram = false, bool pinned = false ):
        featureId( id ), rotation( r ), cornerPoints( corners ), labelRect( rect ), width( w ), height( h ), layerID( layer ), labelText( labeltext ), labelFont( labelfont ), upsideDown( upside_down ), isDiagram( diagram ), isPinned( pinned ) {}
    QgsLabelPosition(): featureId( -1 ), rotation( 0 ), labelRect( QgsRectangle() ), width( 0 ), height( 0 ), layerID( "" ), labelText( "" ), labelFont( QFont() ), upsideDown( false ), isDiagram( false ), isPinned( false ) {}
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
};

/** Labeling engine interface.
 * \note Added in QGIS v1.4
 */
class CORE_EXPORT QgsLabelingEngineInterface
{
  public:

    virtual ~QgsLabelingEngineInterface() {}

    //! called when we're going to start with rendering
    virtual void init( QgsMapRenderer* mp ) = 0;
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer ) = 0;
    //! clears all PAL layer settings for registered layers
    //! @note: this method was added in version 1.9
    virtual void clearActiveLayers() = 0;
    //! clears data defined objects from PAL layer settings for a registered layer
    //! @note: this method was added in version 1.9
    virtual void clearActiveLayer( QgsVectorLayer* layer ) = 0;
    //! called when starting rendering of a layer
    //! @note: this method was added in version 1.6
    virtual int prepareLayer( QgsVectorLayer* layer, QSet<int>& attrIndices, QgsRenderContext& ctx ) = 0;
    //! returns PAL layer settings for a registered layer
    //! @note: this method was added in version 1.9
    virtual QgsPalLayerSettings& layer( const QString& layerName ) = 0;
    //! adds a diagram layer to the labeling engine
    virtual int addDiagramLayer( QgsVectorLayer* layer, QgsDiagramLayerSettings* s )
    { Q_UNUSED( layer ); Q_UNUSED( s ); return 0; }
    //! called for every feature
    virtual void registerFeature( QgsVectorLayer* layer, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() ) = 0;
    //! called for every diagram feature
    virtual void registerDiagramFeature( QgsVectorLayer* layer, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() )
    { Q_UNUSED( layer ); Q_UNUSED( feat ); Q_UNUSED( context ); }
    //! called when the map is drawn and labels should be placed
    virtual void drawLabeling( QgsRenderContext& context ) = 0;
    //! called when we're done with rendering
    virtual void exit() = 0;
    //! return infos about labels at a given (map) position
    //! @note: this method was added in version 1.7
    virtual QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p ) = 0;
    //! return infos about labels within a given (map) rectangle
    //! @note: this method was added in version 1.9
    virtual QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r ) = 0;

    //! called when passing engine among map renderers
    virtual QgsLabelingEngineInterface* clone() = 0;
};



/** \ingroup core
 * A non GUI class for rendering a map layer set onto a QPainter.
 */

class CORE_EXPORT QgsMapRenderer : public QObject
{
    Q_OBJECT

  public:

    /**Output units for pen width and point marker width/height*/
    enum OutputUnits
    {
      Millimeters,
      Pixels
      //MAP_UNITS probably supported in future versions
    };

    /** Blending modes enum defining the available composition modes that can
     * be used when rendering a layer
     */
    enum BlendMode
    {
      BlendNormal,
      BlendLighten,
      BlendScreen,
      BlendDodge,
      BlendAddition,
      BlendDarken,
      BlendMultiply,
      BlendBurn,
      BlendOverlay,
      BlendSoftLight,
      BlendHardLight,
      BlendDifference,
      BlendSubtract
    };

    //! constructor
    QgsMapRenderer();

    //! destructor
    ~QgsMapRenderer();

    //! starts rendering
    //! @ param forceWidthScale Force a specific scale factor for line widths and marker sizes. Automatically calculated from output device DPI if 0
    Q_DECL_DEPRECATED void render( QPainter* painter, double* forceWidthScale = 0 );

    //! sets extent and checks whether suitable (returns false if not)
    Q_DECL_DEPRECATED bool setExtent( const QgsRectangle& extent );

    //! returns current extent
    Q_DECL_DEPRECATED QgsRectangle extent() const;

    Q_DECL_DEPRECATED const QgsMapToPixel* coordinateTransform() { return &( mRenderContext.mapToPixel() ); }

    //! Scale denominator
    Q_DECL_DEPRECATED double scale() const { return mScale; }
    /**Sets scale for scale based visibility. Normally, the scale is calculated automatically. This
     function is only used to force a preview scale (e.g. for print composer)*/
    Q_DECL_DEPRECATED void setScale( double scale ) {mScale = scale;}
    Q_DECL_DEPRECATED double mapUnitsPerPixel() const { return mMapUnitsPerPixel; }

    Q_DECL_DEPRECATED int width() const { return mSize.width(); }
    Q_DECL_DEPRECATED int height() const { return mSize.height(); }

    //! Recalculate the map scale
    Q_DECL_DEPRECATED void updateScale();

    Q_DECL_DEPRECATED QGis::UnitType mapUnits() const;
    Q_DECL_DEPRECATED void setMapUnits( QGis::UnitType u );

    //! sets whether map image will be for overview
    Q_DECL_DEPRECATED void enableOverviewMode( bool isOverview = true ) { mOverview = isOverview; }

    Q_DECL_DEPRECATED void setOutputSize( QSize size, int dpi );
    Q_DECL_DEPRECATED void setOutputSize( QSizeF size, double dpi );

    //!accessor for output dpi
    Q_DECL_DEPRECATED double outputDpi();
    //!accessor for output size
    Q_DECL_DEPRECATED QSize outputSize();
    Q_DECL_DEPRECATED QSizeF outputSizeF();

    /**
     * @brief transform bounding box from layer's CRS to output CRS
     * @see layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    Q_DECL_DEPRECATED QgsRectangle layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent );

    /**
     * @brief transform bounding box from output CRS to layer's CRS
     * @see mapToLayerCoordinates( QgsMapLayer* theLayer,QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    Q_DECL_DEPRECATED QgsRectangle outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent );

    /**
     * @brief transform point coordinates from layer's CRS to output CRS
     * @return the transformed point
     */
    Q_DECL_DEPRECATED QgsPoint layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point );

    /**
     * @brief transform rectangle from layer's CRS to output CRS
     * @see layerExtentToOutputExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    Q_DECL_DEPRECATED QgsRectangle layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect );

    /**
     * @brief transform point coordinates from output CRS to layer's CRS
     * @return the transformed point
     */
    Q_DECL_DEPRECATED QgsPoint mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point );

    /**
     * @brief transform rectangle from output CRS to layer's CRS
     * @see outputExtentToLayerExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    Q_DECL_DEPRECATED QgsRectangle mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect );

    //! sets whether to use projections for this layer set
    Q_DECL_DEPRECATED void setProjectionsEnabled( bool enabled );

    //! returns true if projections are enabled for this layer set
    Q_DECL_DEPRECATED bool hasCrsTransformEnabled() const;

    //! sets destination coordinate reference system
    Q_DECL_DEPRECATED void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );

    //! returns CRS of destination coordinate reference system
    Q_DECL_DEPRECATED const QgsCoordinateReferenceSystem& destinationCrs() const;

    Q_DECL_DEPRECATED void setOutputUnits( OutputUnits u ) {mOutputUnits = u;}

    Q_DECL_DEPRECATED OutputUnits outputUnits() const {return mOutputUnits;}

    //! returns current extent of layer set
    Q_DECL_DEPRECATED QgsRectangle fullExtent();

    //! returns current layer set
    Q_DECL_DEPRECATED QStringList& layerSet();

    //! change current layer set
    Q_DECL_DEPRECATED void setLayerSet( const QStringList& layers );

    //! updates extent of the layer set
    Q_DECL_DEPRECATED void updateFullExtent();

    //! read settings
    Q_DECL_DEPRECATED bool readXML( QDomNode & theNode );

    //! write settings
    Q_DECL_DEPRECATED bool writeXML( QDomNode & theNode, QDomDocument & theDoc );

    //! Accessor for render context
    Q_DECL_DEPRECATED QgsRenderContext* rendererContext() {return &mRenderContext;}

    //! Labeling engine (NULL if there's no custom engine)
    //! \note Added in QGIS v1.4
    Q_DECL_DEPRECATED QgsLabelingEngineInterface* labelingEngine() { return mLabelingEngine; }

    //! Set labeling engine. Previous engine (if any) is deleted.
    //! Takes ownership of the engine.
    //! Added in QGIS v1.4
    Q_DECL_DEPRECATED void setLabelingEngine( QgsLabelingEngineInterface* iface );

    //! Returns a QPainter::CompositionMode corresponding to a BlendMode
    //! Added in 1.9
    Q_DECL_DEPRECATED static QPainter::CompositionMode getCompositionMode( const QgsMapRenderer::BlendMode blendMode );
    //! Returns a BlendMode corresponding to a QPainter::CompositionMode
    //! Added in 1.9
    Q_DECL_DEPRECATED static QgsMapRenderer::BlendMode getBlendModeEnum( const QPainter::CompositionMode blendMode );

  signals:

    //! @deprecated in 2.1 - not emitted anymore
    void drawingProgress( int current, int total );

    void hasCrsTransformEnabled( bool flag );

    void destinationSrsChanged();

    //! @deprecated in 2.1 - not emitted anymore
    void updateMap();

    void mapUnitsChanged();

    //! emitted when layer's draw() returned false
    void drawError( QgsMapLayer* );

  public slots:

    //! @deprecated in 2.1 - does nothing
    Q_DECL_DEPRECATED void onDrawingProgress( int current, int total );

  protected:

    //! adjust extent to fit the pixmap size
    void adjustExtentToSize();

    /** Convenience function to project an extent into the layer source
     * CRS, but also split it into two extents if it crosses
     * the +/- 180 degree line. Modifies the given extent to be in the
     * source CRS coordinates, and if it was split, returns true, and
     * also sets the contents of the r2 parameter
     */
    bool splitLayersExtent( QgsMapLayer* layer, QgsRectangle& extent, QgsRectangle& r2 );

    //! indicates drawing in progress
    static bool mDrawing;

    //! map units per pixel
    double mMapUnitsPerPixel;

    //! Map scale denominator at its current zoom level
    double mScale;

    //! scale calculator
    QgsScaleCalculator * mScaleCalculator;

    //! current extent to be drawn
    QgsRectangle mExtent;
    //
    /** Last extent to we drew so we know if we can
        used layer render caching or not. Note there are no
        accessors for this as it is intended to internal
        use only.
        @note added in QGIS 1.4 */
    QgsRectangle mLastExtent;

    //! indicates whether it's map image for overview
    bool mOverview;

    QSizeF mSize;

    //! detemines whether on the fly projection support is enabled
    bool mProjectionsEnabled;

    //! destination spatial reference system of the projection
    QgsCoordinateReferenceSystem* mDestCRS;

    //! stores array of layers to be rendered (identified by string)
    QStringList mLayerSet;

    //! full extent of the layer set
    QgsRectangle mFullExtent;

    //! tool for measuring
    QgsDistanceArea* mDistArea;

    //!Encapsulates context of rendering
    QgsRenderContext mRenderContext;

    //!Output units
    OutputUnits mOutputUnits;

    //! Labeling engine (NULL by default)
    QgsLabelingEngineInterface* mLabelingEngine;

    //! Locks rendering loop for concurrent draws
    QMutex mRenderMutex;

  private:
    const QgsCoordinateTransform* tr( QgsMapLayer *layer );
};

#endif

