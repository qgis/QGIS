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
class QgsOverlayObjectPositionManager;
class QgsVectorLayer;

struct QgsDiagramLayerSettings;

struct CORE_EXPORT QgsLabelPosition
{
  QgsLabelPosition( int id, double r, const QVector< QgsPoint >& corners, const QgsRectangle& rect, double w, double h, const QString& layer, bool upside_down, bool diagram = false, bool pinned = false ):
      featureId( id ), rotation( r ), cornerPoints( corners ), labelRect( rect ), width( w ), height( h ), layerID( layer ), upsideDown( upside_down ), isDiagram( diagram ), isPinned( pinned ) {}
  QgsLabelPosition(): featureId( -1 ), rotation( 0 ), labelRect( QgsRectangle() ), width( 0 ), height( 0 ), layerID( "" ), upsideDown( false ), isDiagram( false ), isPinned( false ) {}
  int featureId;
  double rotation;
  QVector< QgsPoint > cornerPoints;
  QgsRectangle labelRect;
  double width;
  double height;
  QString layerID;
  bool upsideDown;
  bool isDiagram;
  bool isPinned;
};

/** Labeling engine interface.
 * \note Added in QGIS v1.4
 */
class QgsLabelingEngineInterface
{
  public:

    virtual ~QgsLabelingEngineInterface() {}

    //! called when we're going to start with rendering
    virtual void init( QgsMapRenderer* mp ) = 0;
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer ) = 0;
    //! called when starting rendering of a layer
    //! @note: this method was added in version 1.6
    virtual int prepareLayer( QgsVectorLayer* layer, QSet<int>& attrIndices, QgsRenderContext& ctx ) = 0;
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

    //! constructor
    QgsMapRenderer();

    //! destructor
    ~QgsMapRenderer();

    //! starts rendering
    //! @ param forceWidthScale Force a specific scale factor for line widths and marker sizes. Automatically calculated from output device DPI if 0
    void render( QPainter* painter, double* forceWidthScale = 0 );

    //! sets extent and checks whether suitable (returns false if not)
    bool setExtent( const QgsRectangle& extent );

    //! returns current extent
    QgsRectangle extent() const;

    const QgsMapToPixel* coordinateTransform() { return &( mRenderContext.mapToPixel() ); }

    double scale() const { return mScale; }
    /**Sets scale for scale based visibility. Normally, the scale is calculated automatically. This
     function is only used to force a preview scale (e.g. for print composer)*/
    void setScale( double scale ) {mScale = scale;}
    double mapUnitsPerPixel() const { return mMapUnitsPerPixel; }

    int width() const { return mSize.width(); };
    int height() const { return mSize.height(); };

    //! Recalculate the map scale
    void updateScale();

    //! Return the measuring object
    //! @deprecated
    Q_DECL_DEPRECATED QgsDistanceArea *distanceArea() { return mDistArea; }
    QGis::UnitType mapUnits() const;
    void setMapUnits( QGis::UnitType u );

    //! sets whether map image will be for overview
    void enableOverviewMode( bool isOverview = true ) { mOverview = isOverview; }

    void setOutputSize( QSize size, int dpi );
    void setOutputSize( QSizeF size, double dpi );

    //!accessor for output dpi
    double outputDpi();
    //!accessor for output size
    QSize outputSize();
    QSizeF outputSizeF();

    //! transform extent in layer's CRS to extent in output CRS
    QgsRectangle layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent );

    //! transform extent in output CRS to extent in layer's CRS
    QgsRectangle outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent );

    //! transform coordinates from layer's CRS to output CRS
    QgsPoint layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point );

    //! transform coordinates from output CRS to layer's CRS
    QgsPoint mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point );

    //! transform rect's coordinates from output CRS to layer's CRS
    QgsRectangle mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect );

    //! sets whether to use projections for this layer set
    void setProjectionsEnabled( bool enabled );

    //! returns true if projections are enabled for this layer set
    bool hasCrsTransformEnabled();

    /** sets destination coordinate reference system
     * @note deprecated by qgis 1.7
     * @see setDestinationCrs
     */
    Q_DECL_DEPRECATED void setDestinationSrs( const QgsCoordinateReferenceSystem& srs ) { setDestinationCrs( srs ); };

    /** returns CRS of destination coordinate reference system
     * @note deprecated by qgis 1.7
     * @see destinationCrs
     */
    Q_DECL_DEPRECATED const QgsCoordinateReferenceSystem& destinationSrs() { return destinationCrs(); };

    //! sets destination coordinate reference system
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );

    //! returns CRS of destination coordinate reference system
    const QgsCoordinateReferenceSystem& destinationCrs() const;

    void setOutputUnits( OutputUnits u ) {mOutputUnits = u;}

    OutputUnits outputUnits() const {return mOutputUnits;}

    //! returns current extent of layer set
    QgsRectangle fullExtent();

    //! returns current layer set
    QStringList& layerSet();

    //! change current layer set
    void setLayerSet( const QStringList& layers );

    //! updates extent of the layer set
    void updateFullExtent();

    //! read settings
    bool readXML( QDomNode & theNode );

    //! write settings
    bool writeXML( QDomNode & theNode, QDomDocument & theDoc );

    //! Accessor for render context
    QgsRenderContext* rendererContext() {return &mRenderContext;}

    //! Labeling engine (NULL if there's no custom engine)
    //! \note Added in QGIS v1.4
    QgsLabelingEngineInterface* labelingEngine() { return mLabelingEngine; }

    //! Set labeling engine. Previous engine (if any) is deleted.
    //! Takes ownership of the engine.
    //! Added in QGIS v1.4
    void setLabelingEngine( QgsLabelingEngineInterface* iface );

  signals:

    void drawingProgress( int current, int total );

    void hasCrsTransformEnabled( bool flag );

    void destinationSrsChanged();

    void updateMap();

    void mapUnitsChanged();

    //! emitted when layer's draw() returned false
    void drawError( QgsMapLayer* );

  public slots:

    //! called by signal from layer current being drawn
    void onDrawingProgress( int current, int total );

    //! invalidate cached layer CRS
    void invalidateCachedLayerCrs();

    //! cached layer was destroyed
    void cachedLayerDestroyed();

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

    /**Creates an overlay object position manager subclass according to the current settings
    @note this method was added in version 1.1*/
    QgsOverlayObjectPositionManager* overlayManagerFromSettings();

    //! indicates drawing in progress
    static bool mDrawing;

    //! map units per pixel
    double mMapUnitsPerPixel;

    //! Map scale at its current zool level
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
    QgsCoordinateTransform *tr( QgsMapLayer *layer );
    QgsCoordinateTransform *mCachedTr;
    QgsMapLayer *mCachedTrForLayer;
};

#endif

