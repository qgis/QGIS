/***************************************************************************
                          qgselevationprofilecanvas.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILECANVAS_H
#define QGSELEVATIONPROFILECANVAS_H

#include "qgsconfig.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsplotcanvas.h"
#include "qgsmaplayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprofilepoint.h"

class QgsElevationProfilePlotItem;
class QgsElevationProfileCrossHairsItem;
class QgsAbstractProfileResults;
class QgsProfilePlotRenderer;
class QgsCurve;
class Qgs2DPlot;
class QgsProfileSnapContext;

/**
 * \ingroup gui
 * \brief A canvas for elevation profiles.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsElevationProfileCanvas : public QgsPlotCanvas
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsElevationProfileCanvas, with the specified \a parent widget.
     */
    QgsElevationProfileCanvas( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsElevationProfileCanvas() override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsPoint toMapCoordinates( const QgsPointXY &point ) const override;
    QgsPointXY toCanvasCoordinates( const QgsPoint &point ) const override;
    void resizeEvent( QResizeEvent *event ) override;
    void paintEvent( QPaintEvent *event ) override;
    void cancelJobs() override SIP_SKIP;
    void panContentsBy( double dx, double dy ) override;
    void centerPlotOn( double x, double y ) override;
    void scalePlot( double factor ) override;
    QgsPointXY snapToPlot( QPoint point ) override;

    /**
     * Scales the plot axis by the given factors.
     */
    void scalePlot( double xFactor, double yFactor );

    void zoomToRect( const QRectF &rect ) override;
    void wheelZoom( QWheelEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *e ) override;

    /**
     * Returns the interior rectangle representing the surface of the plot, in canvas coordinates.
     */
    QRectF plotArea() const;

    /**
     * Triggers a complete regeneration of the profile, causing the profile extraction to perform in the
     * background.
     */
    void refresh() override;

    /**
     * Sets the \a project associated with the profile.
     *
     * This must be set before any layers which utilize terrain based elevation settings can be
     * included in the canvas.
     */
    void setProject( QgsProject *project );

    /**
     * Sets the list of \a layers to include in the profile.
     *
     * \see layers()
     */
    void setLayers( const QList< QgsMapLayer * > &layers );

    /**
     * Returns the list of layers included in the profile.
     *
     * \see setLayers()
     */
    QList< QgsMapLayer * > layers() const;

    /**
     * Sets the \a crs associated with the canvas' map coordinates.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the profile \a curve.
     *
     * The CRS associated with \a curve is set via setCrs().
     *
     * Ownership is transferred to the plot canvas.
     *
     * \see profileCurve()
     */
    void setProfileCurve( QgsCurve *curve SIP_TRANSFER );

    /**
     * Returns the profile curve.
     *
     * The CRS associated with the curve is retrieved via crs().
     *
     * \see setProfileCurve()
     */
    QgsCurve *profileCurve() const;

    /**
     * Sets the profile tolerance (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results.
     *
     * \see tolerance()
     */
    void setTolerance( double tolerance );

    /**
     * Returns the tolerance of the profile (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results.
     *
     * \see setTolerance()
     */
    double tolerance() const { return mTolerance; }

    /**
     * Sets the visible area of the plot.
     */
    void setVisiblePlotRange( double minimumDistance, double maximumDistance, double minimumElevation, double maximumElevation );

    /**
     * Returns a reference to the 2D plot used by the widget.
     *
     * \note Not available in Python bindings
     */
    const Qgs2DPlot &plot() const SIP_SKIP;

    /**
     * Renders a portion of the profile using the specified render \a context.
     */
    void render( QgsRenderContext &context, double width, double height, const Qgs2DPlot &plotSettings );

  signals:

    /**
     * Emitted when the number of active background jobs changes.
     */
    void activeJobCountChanged( int count );

    /**
     * Emitted when the mouse hovers over the specified point (in canvas coordinates).
     */
    void canvasPointHovered( const QgsPointXY &point );

  public slots:

    /**
     * Zooms to the full extent of the profile.
     */
    void zoomFull();

    /**
     * Clears the current profile.
     */
    void clear();

    /**
     * Sets whether snapping of cursor points is enabled.
     */
    void setSnappingEnabled( bool enabled );

  private slots:

    void generationFinished();
    void onLayerProfileGenerationPropertyChanged();
    void onLayerProfileRendererPropertyChanged();
    void regenerateResultsForLayer();
    void scheduleDeferredRegeneration();
    void scheduleDeferredRedraw();
    void startDeferredRegeneration();
    void startDeferredRedraw();
    void refineResults();

  private:

    /**
     * Converts a canvas point to the equivalent plot point.
     */
    QgsProfilePoint canvasPointToPlotPoint( QPointF point ) const;

    /**
     * Converts a plot point to the equivalent canvas point.
     */
    QgsPointXY plotPointToCanvasPoint( const QgsProfilePoint &point ) const;

    QgsProfileSnapContext snapContext() const;

    void setupLayerConnections( QgsMapLayer *layer, bool isDisconnect );

    QgsCoordinateReferenceSystem mCrs;
    QgsProject *mProject = nullptr;

    QgsWeakMapLayerPointerList mLayers;

    QgsElevationProfilePlotItem *mPlotItem = nullptr;
    QgsElevationProfileCrossHairsItem *mCrossHairsItem = nullptr;

    QgsProfilePlotRenderer *mCurrentJob = nullptr;
    QTimer *mDeferredRegenerationTimer = nullptr;
    bool mDeferredRegenerationScheduled = false;
    QTimer *mDeferredRedrawTimer = nullptr;
    bool mDeferredRedrawScheduled = false;

    std::unique_ptr< QgsCurve > mProfileCurve;
    double mTolerance = 0;

    bool mFirstDrawOccurred = false;

    bool mSnappingEnabled = true;

    bool mZoomFullWhenJobFinished = true;

    bool mForceRegenerationAfterCurrentJobCompletes = false;

    static constexpr double MAX_ERROR_PIXELS = 2;
};

#endif // QGSELEVATIONPROFILECANVAS_H
