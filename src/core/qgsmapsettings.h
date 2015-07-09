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

#include <QColor>
#include <QImage>
#include <QSize>
#include <QStringList>

#include "qgscoordinatereferencesystem.h"
#include "qgsdatumtransformstore.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"
#include "qgsscalecalculator.h"

class QPainter;

class QgsCoordinateTransform;
class QgsScaleCalculator;
class QgsMapRendererJob;
class QgsMapLayer;


/**
 * The QgsMapSettings class contains configuration for rendering of the map.
 * The rendering itself is done by QgsMapRendererJob subclasses.
 *
 * In order to set up QgsMapSettings instance, it is necessary to set at least
 * few members: extent, output size and layers.
 *
 * QgsMapSettings and QgsMapRendererJob (+subclasses) are intended to replace
 * QgsMapRenderer class that existed before QGIS 2.4. The advantage of the new
 * classes is that they separate the settings from the rendering and provide
 * asynchronous API for map rendering.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsMapSettings
{
  public:
    QgsMapSettings();

    //! Return geographical coordinates of the rectangle that should be rendered.
    //! The actual visible extent used for rendering could be slightly different
    //! since the given extent may be expanded in order to fit the aspect ratio
    //! of output size. Use visibleExtent() to get the resulting extent.
    QgsRectangle extent() const;
    //! Set coordinates of the rectangle which should be rendered.
    //! The actual visible extent used for rendering could be slightly different
    //! since the given extent may be expanded in order to fit the aspect ratio
    //! of output size. Use visibleExtent() to get the resulting extent.
    void setExtent( const QgsRectangle& rect );

    //! Return the size of the resulting map image
    QSize outputSize() const;
    //! Set the size of the resulting map image
    void setOutputSize( const QSize& size );

    //! Return the rotation of the resulting map image
    //! Units are clockwise degrees
    //! @note added in 2.8
    double rotation() const;
    //! Set the rotation of the resulting map image
    //! Units are clockwise degrees
    //! @note added in 2.8
    void setRotation( double degrees );

    //! Return DPI used for conversion between real world units (e.g. mm) and pixels
    //! Default value is 96
    int outputDpi() const;
    //! Set DPI used for conversion between real world units (e.g. mm) and pixels
    void setOutputDpi( int dpi );

    //! Get list of layer IDs for map rendering
    //! The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
    QStringList layers() const;
    //! Set list of layer IDs for map rendering. The layers must be registered in QgsMapLayerRegistry.
    //! The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
    void setLayers( const QStringList& layers );

    //! Get map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
    //! @note added in 2.8
    QMap<QString, QString> layerStyleOverrides() const;
    //! Set map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
    //! @note added in 2.8
    void setLayerStyleOverrides( const QMap<QString, QString>& overrides );

    //! sets whether to use projections for this layer set
    void setCrsTransformEnabled( bool enabled );
    //! returns true if projections are enabled for this layer set
    bool hasCrsTransformEnabled() const;

    //! sets destination coordinate reference system
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );
    //! returns CRS of destination coordinate reference system
    const QgsCoordinateReferenceSystem& destinationCrs() const;

    //! Get units of map's geographical coordinates - used for scale calculation
    QGis::UnitType mapUnits() const;
    //! Set units of map's geographical coordinates - used for scale calculation
    void setMapUnits( QGis::UnitType u );

    //! Set the background color of the map
    void setBackgroundColor( const QColor& color ) { mBackgroundColor = color; }
    //! Get the background color of the map
    QColor backgroundColor() const { return mBackgroundColor; }

    //! Set color that is used for drawing of selected vector features
    void setSelectionColor( const QColor& color ) { mSelectionColor = color; }
    //! Get color that is used for drawing of selected vector features
    QColor selectionColor() const { return mSelectionColor; }

    //! Enumeration of flags that adjust the way how map is rendered
    enum Flag
    {
      Antialiasing       = 0x01,  //!< Enable anti-aliasin for map rendering
      DrawEditingInfo    = 0x02,  //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput  = 0x04,  //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects = 0x08,  //!< Enable layer transparency and blending effects
      DrawLabeling       = 0x10,  //!< Enable drawing of labels on top of the map
      UseRenderingOptimization = 0x20, //!< Enable vector simplification and other rendering optimizations
      DrawSelection      = 0x40,  //!< Whether vector selections should be shown in the rendered map
      // TODO: ignore scale-based visibility (overview)
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    //! Set combination of flags that will be used for rendering
    void setFlags( Flags flags );
    //! Enable or disable a particular flag (other flags are not affected)
    void setFlag( Flag flag, bool on = true );
    //! Return combination of flags used for rendering
    Flags flags() const;
    //! Check whether a particular flag is enabled
    bool testFlag( Flag flag ) const;

    //! sets format of internal QImage
    void setOutputImageFormat( QImage::Format format ) { mImageFormat = format; }
    //! format of internal QImage, default QImage::Format_ARGB32_Premultiplied
    QImage::Format outputImageFormat() const { return mImageFormat; }

    //! Check whether the map settings are valid and can be used for rendering
    bool hasValidSettings() const;
    //! Return the actual extent derived from requested extent that takes takes output image size into account
    QgsRectangle visibleExtent() const;
    //! Return the visible area as a polygon (may be rotated)
    //! @note added in 2.8
    QPolygonF visiblePolygon() const;
    //! Return the distance in geographical coordinates that equals to one pixel in the map
    double mapUnitsPerPixel() const;
    //! Return the calculated scale of the map
    double scale() const;


    // -- utility functions --

    const QgsDatumTransformStore& datumTransformStore() const { return mDatumTransformStore; }
    QgsDatumTransformStore& datumTransformStore() { return mDatumTransformStore; }

    const QgsMapToPixel& mapToPixel() const { return mMapToPixel; }

    /**
     * @brief transform bounding box from layer's CRS to output CRS
     * @see layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent ) const;

    /**
     * @brief transform bounding box from output CRS to layer's CRS
     * @see mapToLayerCoordinates( QgsMapLayer* theLayer,QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent ) const;

    /**
     * @brief transform point coordinates from layer's CRS to output CRS
     * @return the transformed point
     */
    QgsPoint layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point ) const;

    /**
     * @brief transform rectangle from layer's CRS to output CRS
     * @see layerExtentToOutputExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) const;

    /**
     * @brief transform point coordinates from output CRS to layer's CRS
     * @return the transformed point
     */
    QgsPoint mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point ) const;

    /**
     * @brief transform rectangle from output CRS to layer's CRS
     * @see outputExtentToLayerExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) const;

    /**
     * @brief Return coordinate transform from layer's CRS to destination CRS
     * @param layer
     * @return transform - may be null if the transform is not needed
     */
    const QgsCoordinateTransform* layerTransform( QgsMapLayer *layer ) const;

    //! returns current extent of layer set
    QgsRectangle fullExtent() const;

    /* serialization */

    void readXML( QDomNode& theNode );

    void writeXML( QDomNode& theNode, QDomDocument& theDoc );

  protected:

    int mDpi;

    QSize mSize;

    QgsRectangle mExtent;

    double mRotation;

    QStringList mLayers;
    QMap<QString, QString> mLayerStyleOverrides;

    bool mProjectionsEnabled;
    QgsCoordinateReferenceSystem mDestCRS;
    QgsDatumTransformStore mDatumTransformStore;

    QColor mBackgroundColor;
    QColor mSelectionColor;

    Flags mFlags;

    QImage::Format mImageFormat;

    // derived properties
    bool mValid; //!< whether the actual settings are valid (set in updateDerived())
    QgsRectangle mVisibleExtent; //!< extent with some additional white space that matches the output aspect ratio
    double mMapUnitsPerPixel;
    double mScale;


    // utiity stuff
    QgsScaleCalculator mScaleCalculator;
    QgsMapToPixel mMapToPixel;

    void updateDerived();
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapSettings::Flags )


#endif // QGSMAPSETTINGS_H
