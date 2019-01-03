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

#include "qgsabstractgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslabelingenginesettings.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"
#include "qgsscalecalculator.h"
#include "qgsexpressioncontext.h"
#include "qgsmaplayer.h"
#include "qgsgeometry.h"

class QPainter;

class QgsCoordinateTransform;
class QgsScaleCalculator;
class QgsMapRendererJob;

/**
 * \class QgsLabelBlockingRegion
 * \ingroup core
 *
 * Label blocking region (in map coordinates and CRS).
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
 * The QgsMapSettings class contains configuration for rendering of the map.
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
class CORE_EXPORT QgsMapSettings
{
  public:
    QgsMapSettings();

    /**
     * Returns geographical coordinates of the rectangle that should be rendered.
     * The actual visible extent used for rendering could be slightly different
     * since the given extent may be expanded in order to fit the aspect ratio
     * of output size. Use visibleExtent() to get the resulting extent.
     */
    QgsRectangle extent() const;

    /**
     * Set coordinates of the rectangle which should be rendered.
     * The actual visible extent used for rendering could be slightly different
     * since the given extent may be expanded in order to fit the aspect ratio
     * of output size. Use visibleExtent() to get the resulting extent.
     */
    void setExtent( const QgsRectangle &rect, bool magnified = true );

    //! Returns the size of the resulting map image
    QSize outputSize() const;
    //! Sets the size of the resulting map image
    void setOutputSize( QSize size );

    /**
     * Returns device pixel ratio
     * Common values are 1 for normal-dpi displays and 2 for high-dpi "retina" displays.
     * \since QGIS 3.4
     */
    float devicePixelRatio() const;

    /**
     * Sets the device pixel ratio
     * Common values are 1 for normal-dpi displays and 2 for high-dpi "retina" displays.
     * \since QGIS 3.4
     */
    void setDevicePixelRatio( float dpr );

    /**
     * Returns the device output size of the map canvas
     * This is equivalent to the output size multiplicated
     * by the device pixel ratio.
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
     * Returns DPI used for conversion between real world units (e.g. mm) and pixels
     * Default value is 96
     */
    double outputDpi() const;
    //! Sets DPI used for conversion between real world units (e.g. mm) and pixels
    void setOutputDpi( double dpi );

    /**
     * Set the magnification factor.
     * \param factor the factor of magnification
     * \see magnificationFactor()
     * \since QGIS 2.16
     */
    void setMagnificationFactor( double factor );

    /**
     * Returns the magnification factor.
     * \see setMagnificationFactor()
     * \since QGIS 2.16
     */
    double magnificationFactor() const;

    /**
     * Gets list of layer IDs for map rendering
     * The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
     */
    QStringList layerIds() const;

    /**
     * Gets list of layers for map rendering
     * The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
     */
    QList<QgsMapLayer *> layers() const;

    /**
     * Set list of layers for map rendering. The layers must be registered in QgsProject.
     * The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
     *
     * \note Any non-spatial layers will be automatically stripped from the list (since they cannot be rendered!).
     */
    void setLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Gets map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
     * \since QGIS 2.8
     */
    QMap<QString, QString> layerStyleOverrides() const;

    /**
     * Set map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
     * \since QGIS 2.8
     */
    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );

    /**
     * Gets custom rendering flags. Layers might honour these to alter their rendering.
     *  \returns custom flags strings, separated by ';'
     * \see setCustomRenderFlags()
     * \since QGIS 2.16
     */
    QString customRenderFlags() const { return mCustomRenderFlags; }

    /**
     * Sets the custom rendering flags. Layers might honour these to alter their rendering.
     * \param customRenderFlags custom flags strings, separated by ';'
     * \see customRenderFlags()
     * \since QGIS 2.16
     */
    void setCustomRenderFlags( const QString &customRenderFlags ) { mCustomRenderFlags = customRenderFlags; }

    //! sets destination coordinate reference system
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
    //! returns CRS of destination coordinate reference system
    QgsCoordinateReferenceSystem destinationCrs() const;

    //! Gets units of map's geographical coordinates - used for scale calculation
    QgsUnitTypes::DistanceUnit mapUnits() const;

    /**
     * Sets the \a ellipsoid by its acronym. Known ellipsoid acronyms can be
     * retrieved using QgsEllipsoidUtils::acronyms().
     * Calculations will only use the ellipsoid if a valid ellipsoid has been set.
     * \returns true if ellipsoid was successfully set
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

    //! Sets the background color of the map
    void setBackgroundColor( const QColor &color ) { mBackgroundColor = color; }
    //! Gets the background color of the map
    QColor backgroundColor() const { return mBackgroundColor; }

    //! Sets color that is used for drawing of selected vector features
    void setSelectionColor( const QColor &color ) { mSelectionColor = color; }
    //! Gets color that is used for drawing of selected vector features
    QColor selectionColor() const { return mSelectionColor; }

    //! Enumeration of flags that adjust the way the map is rendered
    enum Flag
    {
      Antialiasing             = 0x01,  //!< Enable anti-aliasing for map rendering
      DrawEditingInfo          = 0x02,  //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput        = 0x04,  //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects       = 0x08,  //!< Enable layer opacity and blending effects
      DrawLabeling             = 0x10,  //!< Enable drawing of labels on top of the map
      UseRenderingOptimization = 0x20,  //!< Enable vector simplification and other rendering optimizations
      DrawSelection            = 0x40,  //!< Whether vector selections should be shown in the rendered map
      DrawSymbolBounds         = 0x80,  //!< Draw bounds of symbols (for debugging/testing)
      RenderMapTile            = 0x100, //!< Draw map such that there are no problems between adjacent tiles
      RenderPartialOutput      = 0x200, //!< Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0
      RenderPreviewJob         = 0x400, //!< Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering
      // TODO: ignore scale-based visibility (overview)
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    //! Sets combination of flags that will be used for rendering
    void setFlags( QgsMapSettings::Flags flags );
    //! Enable or disable a particular flag (other flags are not affected)
    void setFlag( Flag flag, bool on = true );
    //! Returns combination of flags used for rendering
    Flags flags() const;
    //! Check whether a particular flag is enabled
    bool testFlag( Flag flag ) const;

    /**
     * Returns the text render format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \see setTextRenderFormat()
     * \since QGIS 3.4.3
     */
    QgsRenderContext::TextRenderFormat textRenderFormat() const
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
    void setTextRenderFormat( QgsRenderContext::TextRenderFormat format )
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
    //! Returns the actual extent derived from requested extent that takes takes output image size into account
    QgsRectangle visibleExtent() const;

    /**
     * Returns the visible area as a polygon (may be rotated)
     * \since QGIS 2.8
     */
    QPolygonF visiblePolygon() const;
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

    //! returns current extent of layer set
    QgsRectangle fullExtent() const;

    /* serialization */

    void readXml( QDomNode &node );

    void writeXml( QDomNode &node, QDomDocument &doc );

    /**
     * Sets the segmentation tolerance applied when rendering curved geometries
    \param tolerance the segmentation tolerance*/
    void setSegmentationTolerance( double tolerance ) { mSegmentationTolerance = tolerance; }
    //! Gets the segmentation tolerance applied when rendering curved geometries
    double segmentationTolerance() const { return mSegmentationTolerance; }

    /**
     * Sets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
    \param type the segmentation tolerance typename*/
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

  protected:

    double mDpi;

    QSize mSize;
    float mDevicePixelRatio = 1.0;

    QgsRectangle mExtent;

    double mRotation = 0.0;
    double mMagnificationFactor = 1.0;

    //! list of layers to be rendered (stored as weak pointers)
    QgsWeakMapLayerPointerList mLayers;
    QMap<QString, QString> mLayerStyleOverrides;
    QString mCustomRenderFlags;
    QgsExpressionContext mExpressionContext;

    QgsCoordinateReferenceSystem mDestCRS;
    //! ellipsoid acronym (from table tbl_ellipsoids)
    QString mEllipsoid;

    QColor mBackgroundColor;
    QColor mSelectionColor;

    Flags mFlags;

    QImage::Format mImageFormat = QImage::Format_ARGB32_Premultiplied;

    double mSegmentationTolerance;
    QgsAbstractGeometry::SegmentationToleranceType mSegmentationToleranceType = QgsAbstractGeometry::MaximumAngle;

    QgsLabelingEngineSettings mLabelingEngineSettings;

    // derived properties
    bool mValid = false; //!< Whether the actual settings are valid (set in updateDerived())
    QgsRectangle mVisibleExtent; //!< Extent with some additional white space that matches the output aspect ratio
    double mMapUnitsPerPixel = 1;
    double mScale = 1;

    // utiity stuff
    QgsScaleCalculator mScaleCalculator;
    QgsMapToPixel mMapToPixel;

    QgsCoordinateTransformContext mTransformContext;

    QgsPathResolver mPathResolver;

    QgsRenderContext::TextRenderFormat mTextRenderFormat = QgsRenderContext::TextFormatAlwaysOutlines;

    QgsGeometry mLabelBoundaryGeometry;

#ifdef QGISDEBUG
    bool mHasTransformContext = false;
#endif

    void updateDerived();

  private:

    QList< QgsLabelBlockingRegion > mLabelBlockingRegions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapSettings::Flags )


#endif // QGSMAPSETTINGS_H
