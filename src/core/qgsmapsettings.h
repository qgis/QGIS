/***************************************************************************
  qgsmapsettings.h
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPSETTINGS_H
#define QGSMAPSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QColor>
#include <QImage>
#include <QPointer>
#include <QSize>
#include <QStringList>

#include "qgscoordinatereferencesystem.h"
#include "qgslabelingenginesettings.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"
#include "qgsscalecalculator.h"
#include "qgsexpressioncontext.h"
#include "qgsmaplayer.h"
#include "qgsgeometry.h"
#include "qgstemporalrangeobject.h"
#include "qgsmapclippingregion.h"
#include "qgsvectorsimplifymethod.h"

class QPainter;

class QgsCoordinateTransform;
class QgsScaleCalculator;
class QgsMapRendererJob;
class QgsRenderedFeatureHandlerInterface;

/**
 * \class QgsLabelBlockingRegion
 * \ingroup core
 *
 * \brief Label blocking region (in map coordinates and CRS).
 *
 * \since QGIS 3.6
*/
class CORE_EXPORT QgsLabelBlockingRegion
{
  public:

    /**
     * Constructor for a label blocking region
     */
    explicit QgsLabelBlockingRegion( const QgsGeometry &geometry )
      : geometry( geometry )
    {}

    //! Geometry of region to avoid placing labels within (in destination map coordinates and CRS)
    QgsGeometry geometry;

};


/**
 * \ingroup core
 * \brief The QgsMapSettings class contains configuration for rendering of the map.
 * The rendering itself is done by QgsMapRendererJob subclasses.
 *
 * In order to set up QgsMapSettings instance, it is necessary to set at least
 * few members: extent, output size and layers.
 *
 * Some systems use high DPI scaling that is an alternative to the traditional
 * DPI scaling. The operating system provides Qt with a scaling ratio and it
 * scales window, event, and desktop geometry. The Cocoa platform plugin sets
 * the scaling ratio as QWindow::devicePixelRatio().
 * To properly render the map on such systems, the map settings device pixel
 * ratio shall be set accordingly.
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsMapSettings : public QgsTemporalRangeObject
{
  public:
    QgsMapSettings();

    /**
     * Returns geographical coordinates of the rectangle that should be rendered.
     *
     * \warning The actual visible extent used for rendering can be significantly different from this
     * value, since the given extent may be expanded in order to fit the aspect ratio
     * of the outputSize(). Use visibleExtent() to get the actual extent which will be rendered.
     *
     * \see visibleExtent()
     * \see setExtent()
     */
    QgsRectangle extent() const;

    /**
     * Sets the coordinates of the rectangle which should be rendered.
     *
     * \warning The actual visible extent used for rendering can be significantly different
     * from the specified extent, since the given extent may be expanded in order to match the
     * aspect ratio of outputSize(). Use visibleExtent() to retrieve the actual extent to be rendered.
     *
     * \see visibleExtent()
     * \see extent()
     */
    void setExtent( const QgsRectangle &rect, bool magnified = true );

    /**
     * Returns the buffer in map units to use around the visible extent for rendering
     * symbols whose corresponding geometries are outside the visible extent.
     * \see setExtentBuffer()
     * \since QGIS 3.10
     */
    double extentBuffer() const;

    /**
     * Sets the buffer in map units to use around the visible extent for rendering
     * symbols whose corresponding geometries are outside the visible extent. This
     * is useful when using tiles to avoid cut symbols at tile boundaries.
     * \since QGIS 3.10
     */
    void setExtentBuffer( double buffer );

    /**
     * Returns the size of the resulting map image, in pixels.
     *
     * \see deviceOutputSize()
     * \see setOutputSize()
     */
    QSize outputSize() const;

    /**
     * Sets the \a size of the resulting map image, in pixels.
     *
     * \see outputSize()
     */
    void setOutputSize( QSize size );

    /**
     * Returns the device pixel ratio.
     *
     * Common values are 1 for normal-dpi displays and 2 for high-dpi "retina" displays.
     * \since QGIS 3.4
     */
    float devicePixelRatio() const;

    /**
     * Sets the device pixel ratio.
     *
     * Common values are 1 for normal-dpi displays and 2 for high-dpi "retina" displays.
     * \since QGIS 3.4
     */
    void setDevicePixelRatio( float dpr );

    /**
     * Returns the device output size of the map render.
     *
     * This is equivalent to the output size multiplicated
     * by the device pixel ratio.
     *
     * \see outputSize()
     * \see devicePixelRatio()
     * \see setOutputSize()
     *
     * \since QGIS 3.4
     */
    QSize deviceOutputSize() const;

    /**
     * Returns the rotation of the resulting map image, in degrees clockwise.
     * \see setRotation()
     * \since QGIS 2.8
     */
    double rotation() const;

    /**
     * Sets the \a rotation of the resulting map image, in degrees clockwise.
     * \see rotation()
     * \since QGIS 2.8
     */
    void setRotation( double rotation );

    /**
     * Returns the DPI (dots per inch) used for conversion between real world units (e.g. mm) and pixels.
     *
     * The default value is 96 dpi.
     *
     * \see setOutputDpi()
     */
    double outputDpi() const;

    /**
     * Sets the \a dpi (dots per inch) used for conversion between real world units (e.g. mm) and pixels.
     *
     * \see outputDpi()
     */
    void setOutputDpi( double dpi );

    /**
     * Returns the target DPI (dots per inch) to be taken into consideration when rendering.
     *
     * The default value is -1, which states no DPI target is provided.
     *
     * \see setDpiTarget()
     * \since QGIS 3.20
     */
    double dpiTarget() const;

    /**
     * Sets the target \a dpi (dots per inch) to be taken into consideration when rendering.
     *
     * \see dpiTarget()
     * \since QGIS 3.20
     */
    void setDpiTarget( double dpi );

    /**
     * Set the magnification factor.
     * \param factor the factor of magnification
     * \param center optional point to re-center the map
     * \see magnificationFactor()
     * \since QGIS 2.16
     */
    void setMagnificationFactor( double factor, const QgsPointXY *center = nullptr );

    /**
     * Returns the magnification factor.
     * \see setMagnificationFactor()
     * \since QGIS 2.16
     */
    double magnificationFactor() const;

    /**
     * Returns the list of layer IDs which will be rendered in the map.
     *
     * The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top).
     *
     * Since QGIS 3.24, if the \a expandGroupLayers option is TRUE then group layers will be converted to
     * all their child layers.
     *
     * \see layers()
     * \see setLayers()
     */
    QStringList layerIds( bool expandGroupLayers = false ) const;

    /**
     * Returns the list of layers which will be rendered in the map.
     *
     * The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
     *
     * Since QGIS 3.24, if the \a expandGroupLayers option is TRUE then group layers will be converted to
     * all their child layers.
     *
     * \see setLayers()
     * \see layerIds()
     */
    QList<QgsMapLayer *> layers( bool expandGroupLayers = false ) const;

    /**
     * Sets the list of \a layers to render in the map.
     *
     * The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
     *
     * \note Any non-spatial layers will be automatically stripped from the list (since they cannot be rendered!).
     *
     * \see layers()
     * \see layerIds()
     */
    void setLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Returns the map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one.
     *
     * \see setLayerStyleOverrides()
     * \since QGIS 2.8
     */
    QMap<QString, QString> layerStyleOverrides() const;

    /**
     * Sets the map of map layer style \a overrides (key: layer ID, value: style name) where a different style should be used instead of the current one.
     *
     * \see layerStyleOverrides()
     * \since QGIS 2.8
     */
    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );

    /**
     * Returns custom rendering flags. Layers might honour these to alter their rendering.
     * \returns custom flags strings, separated by ';'
     * \see setCustomRenderFlags()
     * \since QGIS 2.16
     * \deprecated use \see customRenderingFlags().
     */
    Q_DECL_DEPRECATED QString customRenderFlags() const { return mCustomRenderFlags; }

    /**
     * Sets the custom rendering flags. Layers might honour these to alter their rendering.
     * \param customRenderFlags custom flags strings, separated by ';'
     * \see customRenderFlags()
     * \since QGIS 2.16
     * \deprecated use \see setCustomRenderingFlag() instead.
     */
    Q_DECL_DEPRECATED void setCustomRenderFlags( const QString &customRenderFlags ) { mCustomRenderFlags = customRenderFlags; }

    /**
     * Returns any custom rendering flags. Layers might honour these to alter their rendering.
     * \returns a map of custom flags
     * \see setCustomRenderingFlag()
     * \since QGIS 3.12
     */
    QVariantMap customRenderingFlags() const { return mCustomRenderingFlags; }

    /**
     * Sets a custom rendering \a flag. Layers might honour these to alter their rendering.
     * \param flag the flag name
     * \param value the flag value
     * \see customRenderingFlags()
     * \since QGIS 3.12
     */
    void setCustomRenderingFlag( const QString &flag, const QVariant &value ) { mCustomRenderingFlags[flag] = value; }

    /**
     * Clears the specified custom rendering \a flag.
     * \param flag the flag name
     * \see setCustomRenderingFlag()
     * \since QGIS 3.12
     */
    void clearCustomRenderingFlag( const QString &flag ) { mCustomRenderingFlags.remove( flag ); }

    /**
     * Sets the destination \a crs (coordinate reference system) for the map render.
     *
     * \see destinationCrs()
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the destination coordinate reference system for the map render.
     *
     * \see setDestinationCrs()
     */
    QgsCoordinateReferenceSystem destinationCrs() const;

    /**
     * Returns the units of the map's geographical coordinates - used for scale calculation.
     */
    QgsUnitTypes::DistanceUnit mapUnits() const;

    /**
     * Sets the \a ellipsoid by its acronym. Known ellipsoid acronyms can be
     * retrieved using QgsEllipsoidUtils::acronyms().
     * Calculations will only use the ellipsoid if a valid ellipsoid has been set.
     * \returns TRUE if ellipsoid was successfully set
     * \see ellipsoid()
     * \since QGIS 3.0
     */
    bool setEllipsoid( const QString &ellipsoid );

    /**
     * Returns ellipsoid's acronym. Calculations will only use the
     * ellipsoid if a valid ellipsoid has been set.
     * \see setEllipsoid()
     * \since QGIS 3.0
     */
    QString ellipsoid() const { return mEllipsoid; }

    /**
     * Sets the background \a color of the map.
     *
     * \see backgroundColor()
     */
    void setBackgroundColor( const QColor &color ) { mBackgroundColor = color; }

    /**
     * Returns the background color of the map.
     *
     * \see setBackgroundColor()
     */
    QColor backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the \a color that is used for drawing of selected vector features.
     *
     * \see selectionColor()
     */
    void setSelectionColor( const QColor &color ) { mSelectionColor = color; }

    /**
     * Returns the color that is used for drawing of selected vector features.
     *
     * \see setSelectionColor()
     */
    QColor selectionColor() const { return mSelectionColor; }

    //! Sets combination of flags that will be used for rendering
    void setFlags( Qgis::MapSettingsFlags flags );
    //! Enable or disable a particular flag (other flags are not affected)
    void setFlag( Qgis::MapSettingsFlag flag, bool on = true );
    //! Returns combination of flags used for rendering
    Qgis::MapSettingsFlags flags() const;
    //! Check whether a particular flag is enabled
    bool testFlag( Qgis::MapSettingsFlag flag ) const;

    /**
     * Returns the text render format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \see setTextRenderFormat()
     * \since QGIS 3.4.3
     */
    Qgis::TextRenderFormat textRenderFormat() const
    {
      return mTextRenderFormat;
    }

    /**
     * Sets the text render \a format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \warning Calling the setLabelingEngineSettings() method will reset the text render format to match the default
     * text render format from the label engine settings.
     *
     * \see textRenderFormat()
     * \since QGIS 3.4.3
     */
    void setTextRenderFormat( Qgis::TextRenderFormat format )
    {
      mTextRenderFormat = format;
      // ensure labeling engine setting is also kept in sync, just in case anyone accesses QgsMapSettings::labelingEngineSettings().defaultTextRenderFormat()
      // instead of correctly calling QgsMapSettings::textRenderFormat(). It can't hurt to be consistent!
      mLabelingEngineSettings.setDefaultTextRenderFormat( format );
    }

    //! sets format of internal QImage
    void setOutputImageFormat( QImage::Format format ) { mImageFormat = format; }
    //! format of internal QImage, default QImage::Format_ARGB32_Premultiplied
    QImage::Format outputImageFormat() const { return mImageFormat; }

    //! Check whether the map settings are valid and can be used for rendering
    bool hasValidSettings() const;
    //! Returns the actual extent derived from requested extent that takes output image size into account
    QgsRectangle visibleExtent() const;

    /**
     * Returns the visible area as a polygon (may be rotated)
     * \since QGIS 2.8
     */
    QPolygonF visiblePolygon() const;

    /**
     * Returns the visible area as a polygon (may be rotated) with extent buffer included
     * \see extentBuffer()
     * \since QGIS 3.22
     */
    QPolygonF visiblePolygonWithBuffer() const;

    //! Returns the distance in geographical coordinates that equals to one pixel in the map
    double mapUnitsPerPixel() const;

    /**
     * Returns the calculated map scale.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    double scale() const;

    /**
     * Sets the expression context. This context is used for all expression evaluation
     * associated with this map settings.
     * \see expressionContext()
     * \since QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext &context ) { mExpressionContext = context; }

    /**
     * Gets the expression context. This context should be used for all expression evaluation
     * associated with this map settings.
     * \see setExpressionContext()
     * \since QGIS 2.12
     */
    const QgsExpressionContext &expressionContext() const { return mExpressionContext; }

    /**
     * Returns the coordinate transform context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see setTransformContext()
     * \since QGIS 3.0
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Sets the coordinate transform \a context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see transformContext()
     * \since QGIS 3.0
     */
    void setTransformContext( const QgsCoordinateTransformContext &context );

    /**
     * Returns the path resolver for conversion between relative and absolute paths
     * during rendering operations, e.g. for resolving relative symbol paths.
     *
     * \see setPathResolver()
     * \since QGIS 3.0
     */
    const QgsPathResolver &pathResolver() const { return mPathResolver; }

    /**
     * Sets the path \a resolver for conversion between relative and absolute paths
     * during rendering operations, e.g. for resolving relative symbol paths.
     *
     * \see pathResolver()
     * \since QGIS 3.0
     */
    void setPathResolver( const QgsPathResolver &resolver ) { mPathResolver = resolver; }

    const QgsMapToPixel &mapToPixel() const { return mMapToPixel; }

    /**
     * Computes an *estimated* conversion factor between layer and map units: layerUnits * layerToMapUnits = mapUnits
     * \param layer The layer
     * \param referenceExtent A reference extent based on which to perform the computation. If not specified, the layer extent is used
     * \since QGIS 2.12
     */
    double layerToMapUnits( const QgsMapLayer *layer, const QgsRectangle &referenceExtent = QgsRectangle() ) const;

    /**
     * \brief transform bounding box from layer's CRS to output CRS
     * \see layerToMapCoordinates( const QgsMapLayer *, QgsRectangle ) const if you want to transform a rectangle
     * \returns a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle layerExtentToOutputExtent( const QgsMapLayer *layer, QgsRectangle extent ) const;

    /**
     * \brief transform bounding box from output CRS to layer's CRS
     * \see mapToLayerCoordinates( const QgsMapLayer *, QgsRectangle ) const if you want to transform a rectangle
     * \returns a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle outputExtentToLayerExtent( const QgsMapLayer *layer, QgsRectangle extent ) const;

    /**
     * \brief transform point coordinates from layer's CRS to output CRS
     * \returns the transformed point
     */
    QgsPointXY layerToMapCoordinates( const QgsMapLayer *layer, QgsPointXY point ) const;

    /**
     * \brief transform point coordinates from layer's CRS to output CRS
     * \returns the transformed point
     * \since QGIS 3.16
     */
    QgsPoint layerToMapCoordinates( const QgsMapLayer *layer, const QgsPoint &point ) const;

    /**
     * \brief transform rectangle from layer's CRS to output CRS
     * \see layerExtentToOutputExtent() if you want to transform a bounding box
     * \returns the transformed rectangle
     */
    QgsRectangle layerToMapCoordinates( const QgsMapLayer *layer, QgsRectangle rect ) const;

    /**
     * \brief transform point coordinates from output CRS to layer's CRS
     * \returns the transformed point
     */
    QgsPointXY mapToLayerCoordinates( const QgsMapLayer *layer, QgsPointXY point ) const;

    /**
     * \brief transform point coordinates from output CRS to layer's CRS
     * \returns the transformed point
     * \since QGIS 3.16
     */
    QgsPoint mapToLayerCoordinates( const QgsMapLayer *layer, const QgsPoint &point ) const;

    /**
     * \brief transform rectangle from output CRS to layer's CRS
     * \see outputExtentToLayerExtent() if you want to transform a bounding box
     * \returns the transformed rectangle
     */
    QgsRectangle mapToLayerCoordinates( const QgsMapLayer *layer, QgsRectangle rect ) const;

    /**
     * Returns the coordinate transform from layer's CRS to destination CRS
     * \returns transform - may be invalid if the transform is not needed
     */
    QgsCoordinateTransform layerTransform( const QgsMapLayer *layer ) const;

    /**
     * \brief Compute the extent such that its \a center is at the specified
     * position (mapped to the destinatonCrs) and the zoom factor corresponds
     * to the specified \a scale
     * \param center the center, in map coordinates
     * \param scale the desired zoom factor (the x part of 1:x)
     * \returns an extent which can be passed to QgsMapCanvas::setExtent
     * \see computeScaleForExtent()
     * \since QGIS 3.22
     */
    QgsRectangle computeExtentForScale( const QgsPointXY &center, double scale ) const;

    /**
     * \brief Compute the scale that corresponds to the specified \a extent
     * \param extent the extent, as passed to \see QgsMapCanvas::setExtent
     * \returns the scale denominator
     * \see computeExtentForScale()
     * \note This function does not consider any map rotation
     * \since QGIS 3.22
     */
    double computeScaleForExtent( const QgsRectangle &extent ) const;

    //! returns current extent of layer set
    QgsRectangle fullExtent() const;

    /* serialization */

    void readXml( QDomNode &node );

    void writeXml( QDomNode &node, QDomDocument &doc );

    /**
     * Sets the segmentation tolerance applied when rendering curved geometries
     * \param tolerance the segmentation tolerance
    */
    void setSegmentationTolerance( double tolerance ) { mSegmentationTolerance = tolerance; }
    //! Gets the segmentation tolerance applied when rendering curved geometries
    double segmentationTolerance() const { return mSegmentationTolerance; }

    /**
     * Sets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
     * \param type the segmentation tolerance typename
    */
    void setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type ) { mSegmentationToleranceType = type; }
    //! Gets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
    QgsAbstractGeometry::SegmentationToleranceType segmentationToleranceType() const { return mSegmentationToleranceType; }

    /**
     * Sets the global configuration of the labeling engine.
     *
     * \note Calling this method will reset the textRenderFormat() to match the default
     * text render format from the label engine \a settings.
     *
     * \see labelingEngineSettings()
     *
     * \since QGIS 3.0
     */
    void setLabelingEngineSettings( const QgsLabelingEngineSettings &settings )
    {
      mLabelingEngineSettings = settings;
      mTextRenderFormat = settings.defaultTextRenderFormat();
    }

    /**
     * Returns the global configuration of the labeling engine.
     *
     * \see setLabelingEngineSettings()
     *
     * \since QGIS 3.0
     */
    const QgsLabelingEngineSettings &labelingEngineSettings() const { return mLabelingEngineSettings; }

    /**
     * Returns the label boundary geometry, which restricts where in the rendered map labels are permitted to be
     * placed. By default this is a null geometry, which indicates that labels can be placed anywhere within
     * the map's visiblePolygon().
     *
     * The geometry is specified using the map's destinationCrs().
     *
     * \see setLabelBoundaryGeometry()
     * \see labelBlockingRegions()
     * \since QGIS 3.6
     */
    QgsGeometry labelBoundaryGeometry() const;

    /**
     * Sets the label \a boundary geometry, which restricts where in the rendered map labels are permitted to be
     * placed.
     *
     * A null \a boundary geometry (the default) indicates that labels can be placed anywhere within
     * the map's visiblePolygon().
     *
     * The geometry is specified using the map's destinationCrs().
     *
     * \see labelBoundaryGeometry()
     * \see setLabelBlockingRegions()
     * \since QGIS 3.6
     */
    void setLabelBoundaryGeometry( const QgsGeometry &boundary );

    /**
     * Sets a list of \a regions to avoid placing labels within.
     * \see labelBlockingRegions()
     * \see setLabelBoundaryGeometry()
     * \since QGIS 3.6
     */
    void setLabelBlockingRegions( const QList< QgsLabelBlockingRegion > &regions ) { mLabelBlockingRegions = regions; }

    /**
     * Returns the list of regions to avoid placing labels within.
     * \see setLabelBlockingRegions()
     * \see labelBoundaryGeometry()
     * \since QGIS 3.6
     */
    QList< QgsLabelBlockingRegion > labelBlockingRegions() const { return mLabelBlockingRegions; }

    /**
     * Adds a new clipping \a region to the map settings.
     *
     * \see clippingRegions()
     * \see setClippingRegions()
     *
     * \since QGIS 3.16
     */
    void addClippingRegion( const QgsMapClippingRegion &region );

    /**
     * Sets the list of clipping \a regions to apply to the map.
     *
     * \see addClippingRegion()
     * \see clippingRegions()
     *
     * \since QGIS 3.16
     */
    void setClippingRegions( const QList< QgsMapClippingRegion > &regions );

    /**
     * Returns the list of clipping regions to apply to the map.
     *
     * \see addClippingRegion()
     * \see setClippingRegions()
     *
     * \since QGIS 3.16
     */
    QList< QgsMapClippingRegion > clippingRegions() const;

    /**
     * Sets the simplification setting to use when rendering vector layers.
     *
     * If the simplify \a method is enabled, it will override all other layer-specific simplification
     * settings and will apply to all vector layers rendered for the map.
     *
     * This can be used to specify global simplification methods to apply during map exports,
     * e.g. to allow vector layers to be simplified to an appropriate maximum level of detail
     * during PDF exports.
     *
     * The default is to use no global simplification, and fallback to individual layer's settings instead.
     *
     * \see simplifyMethod()
     *
     * \since QGIS 3.10
     */
    void setSimplifyMethod( const QgsVectorSimplifyMethod &method ) { mSimplifyMethod = method; }

    /**
     * Returns the simplification settings to use when rendering vector layers.
     *
     * If enabled, it will override all other layer-specific simplification
     * settings and will apply to all vector layers rendered for the map.
     *
     * The default is to use no global simplification, and fallback to individual layer's settings instead.
     *
     * \see setSimplifyMethod()
     * \since QGIS 3.10
     */
    const QgsVectorSimplifyMethod &simplifyMethod() const { return mSimplifyMethod; }

    /**
     * Adds a rendered feature \a handler to use while rendering the map settings.
     *
     * Ownership of \a handler is NOT transferred, and it is the caller's responsibility to ensure
     * that the handler exists for the lifetime of the map render job.
     *
     * \see renderedFeatureHandlers()
     * \since QGIS 3.10
     */
    void addRenderedFeatureHandler( QgsRenderedFeatureHandlerInterface *handler );

    /**
     * Returns the list of rendered feature handlers to use while rendering the map settings.
     * \see addRenderedFeatureHandler()
     * \since QGIS 3.10
     */
    QList< QgsRenderedFeatureHandlerInterface * > renderedFeatureHandlers() const;

    /**
     * Returns the range of z-values which will be visible in the map.
     *
     * \see setZRange()
     * \since QGIS 3.18
     */
    QgsDoubleRange zRange() const;

    /**
     * Sets the \a range of z-values which will be visible in the map.
     *
     * \see zRange()
     * \since QGIS 3.18
     */
    void setZRange( const QgsDoubleRange &range );

    /**
     * Returns the rendering usage
     *
     * \see setRendererUsage()
     * \since QGIS 3.24
     */
    Qgis::RendererUsage rendererUsage() const;

    /**
     * Sets the rendering usage
     *
     * \note This usage not alter how the map gets rendered but the intention is that data provider
     * knows the context of rendering and may report that to the backend.
     *
     * \see rendererUsage()
     * \since QGIS 3.24
     */
    void setRendererUsage( Qgis::RendererUsage rendererUsage );

  protected:

    double mDpi = 96.0;
    double mDpiTarget = -1;

    QSize mSize;
    float mDevicePixelRatio = 1.0;

    QgsRectangle mExtent;
    double mExtentBuffer = 0.0;

    double mRotation = 0.0;
    double mMagnificationFactor = 1.0;

    //! list of layers to be rendered (stored as weak pointers)
    QgsWeakMapLayerPointerList mLayers;
    QMap<QString, QString> mLayerStyleOverrides;
    QString mCustomRenderFlags;
    QVariantMap mCustomRenderingFlags;
    QgsExpressionContext mExpressionContext;

    QgsCoordinateReferenceSystem mDestCRS;
    //! ellipsoid acronym (from table tbl_ellipsoids)
    QString mEllipsoid;

    QColor mBackgroundColor;
    QColor mSelectionColor;

    Qgis::MapSettingsFlags mFlags;

    QImage::Format mImageFormat = QImage::Format_ARGB32_Premultiplied;

    double mSegmentationTolerance;
    QgsAbstractGeometry::SegmentationToleranceType mSegmentationToleranceType = QgsAbstractGeometry::MaximumAngle;

    QgsLabelingEngineSettings mLabelingEngineSettings;

    // derived properties
    //! Whether the actual settings are valid (set in updateDerived())
    bool mValid = false;
    //! Extent with some additional white space that matches the output aspect ratio
    QgsRectangle mVisibleExtent;
    double mMapUnitsPerPixel = 1;
    double mScale = 1;

    // utiity stuff
    QgsScaleCalculator mScaleCalculator;
    QgsMapToPixel mMapToPixel;

    QgsCoordinateTransformContext mTransformContext;

    QgsPathResolver mPathResolver;

    Qgis::TextRenderFormat mTextRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;

    QgsGeometry mLabelBoundaryGeometry;

    QgsVectorSimplifyMethod mSimplifyMethod;

    Qgis::RendererUsage mRendererUsage = Qgis::RendererUsage::Unknown;

#ifdef QGISDEBUG
    bool mHasTransformContext = false;
#endif

    void updateDerived();

  private:

    QList< QgsLabelBlockingRegion > mLabelBlockingRegions;
    QList< QgsMapClippingRegion > mClippingRegions;
    QList< QgsRenderedFeatureHandlerInterface * > mRenderedFeatureHandlers;

    QgsDoubleRange mZRange;

};

#endif // QGSMAPSETTINGS_H
