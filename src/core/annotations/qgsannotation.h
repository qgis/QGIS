/***************************************************************************
                             qgsannotation.h
                             ---------------
    begin                : January 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSANNOTATION_H
#define QGSANNOTATION_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgspointxy.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmargins.h"
#include "qgsmaplayer.h"
#include "qgsfeature.h"

#include <QPointer>

class QgsRenderContext;
class QgsMarkerSymbol;
class QgsFillSymbol;

/**
 * \ingroup core
 * \class QgsAnnotation
 *
 * \brief Abstract base class for annotation items which are drawn over a map.
 *
 * QgsAnnotation is an abstract base class for map annotation items. These annotations can be
 * drawn within a map, and have either a fixed map position (retrieved using mapPosition())
 * or are placed relative to the map's frame (retrieved using relativePosition()).
 * Annotations with a fixed map position also have a corresponding
 * QgsCoordinateReferenceSystem, which can be determined by calling mapPositionCrs().
 *
 * Derived classes should implement their custom painting routines within
 * a renderAnnotation() override.
 *
 */

class CORE_EXPORT QgsAnnotation : public QObject
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsTextAnnotation * >( sipCpp ) )
      sipType = sipType_QgsTextAnnotation;
    else if ( dynamic_cast< QgsSvgAnnotation * >( sipCpp ) )
      sipType = sipType_QgsSvgAnnotation;
    else if ( dynamic_cast< QgsHtmlAnnotation * >( sipCpp ) )
      sipType = sipType_QgsHtmlAnnotation;
    else
      sipType = NULL;
    SIP_END
#endif


    Q_OBJECT
    Q_PROPERTY( bool visible READ isVisible WRITE setVisible )
    Q_PROPERTY( bool hasFixedMapPosition READ hasFixedMapPosition WRITE setHasFixedMapPosition )
    Q_PROPERTY( QgsPointXY mapPosition READ mapPosition WRITE setMapPosition )
    Q_PROPERTY( QSizeF frameSize READ frameSize WRITE setFrameSize )

  public:

    /**
     * Constructor for QgsAnnotation.
     */
    QgsAnnotation( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsAnnotation() override;

    /**
     * Clones the annotation, returning a new copy of the annotation
     * reflecting the annotation's current state.
     */
    virtual QgsAnnotation *clone() const = 0 SIP_FACTORY;

    /**
     * Returns TRUE if the annotation is visible and should be rendered.
     * \see setVisible()
     */
    bool isVisible() const { return mVisible; }

    /**
     * Sets whether the annotation is visible and should be rendered.
     * \see isVisible()
     */
    void setVisible( bool visible );

    /**
     * Returns TRUE if the annotation is attached to a fixed map position, or
     * FALSE if the annotation uses a position relative to the current map
     * extent.
     * \see setHasFixedMapPosition()
     * \see mapPosition()
     * \see relativePosition()
     */
    bool hasFixedMapPosition() const { return mHasFixedMapPosition; }

    /**
     * Sets whether the annotation is attached to a fixed map position, or
     * uses a position relative to the current map extent.
     * \see hasFixedMapPosition()
     */
    void setHasFixedMapPosition( bool fixed );

    /**
     * Returns the map position of the annotation, if it is attached to a fixed map
     * position.
     * \see setMapPosition()
     * \see hasFixedMapPosition()
     * \see mapPositionCrs()
     */
    QgsPointXY mapPosition() const { return mMapPosition; }

    /**
     * Sets the map position of the annotation, if it is attached to a fixed map
     * position.
     * \see mapPosition()
     */
    void setMapPosition( const QgsPointXY &position );

    /**
     * Returns the CRS of the map position, or an invalid CRS if the annotation does
     * not have a fixed map position.
     * \see setMapPositionCrs()
     */
    QgsCoordinateReferenceSystem mapPositionCrs() const { return mMapPositionCrs; }

    /**
     * Sets the CRS of the map position.
     * \see mapPositionCrs()
     */
    void setMapPositionCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the relative position of the annotation, if it is not attached to a fixed map
     * position. The coordinates in the return point should be between 0 and 1, and represent
     * the relative percentage for the position compared to the map width and height.
     * \see setRelativePosition()
     */
    QPointF relativePosition() const { return mRelativePosition; }

    /**
     * Sets the relative position of the annotation, if it is not attached to a fixed map
     * position. The coordinates in the return point should be between 0 and 1, and represent
     * the relative percentage for the position compared to the map width and height.
     * \see relativePosition()
     */
    void setRelativePosition( QPointF position );

    /**
     * Sets the annotation's frame's offset (in pixels) from the mapPosition() reference point.
     * \see frameOffsetFromReferencePoint()
     * \deprecated QGIS 3.40. Use setFrameOffsetFromReferencePointMm() instead.
     */
    Q_DECL_DEPRECATED void setFrameOffsetFromReferencePoint( QPointF offset ) SIP_DEPRECATED;

    /**
     * Returns the annotation's frame's offset (in pixels) from the mapPosition() reference point.
     * \see setFrameOffsetFromReferencePoint()
     * \deprecated QGIS 3.40. Use frameOffsetFromReferencePointMm() instead.
     */
    Q_DECL_DEPRECATED QPointF frameOffsetFromReferencePoint() const SIP_DEPRECATED;

    /**
     * Sets the annotation's frame's offset (in millimeters) from the mapPosition() reference point.
     * \see frameOffsetFromReferencePointMm()
     * \since QGIS 3.4.8
     */
    void setFrameOffsetFromReferencePointMm( QPointF offset );

    /**
     * Returns the annotation's frame's offset (in millimeters) from the mapPosition() reference point.
     * \see setFrameOffsetFromReferencePointMm()
     * \since QGIS 3.4.8
     */
    QPointF frameOffsetFromReferencePointMm() const { return mOffsetFromReferencePoint; }

    /**
     * Sets the size (in pixels) of the annotation's frame (the main area in which
     * the annotation's content is drawn).
     * \see frameSize()
     * \deprecated QGIS 3.40. Use setFrameSizeMm() instead.
     */
    Q_DECL_DEPRECATED void setFrameSize( QSizeF size ) SIP_DEPRECATED;

    /**
     * Returns the size (in pixels) of the annotation's frame (the main area in which
     * the annotation's content is drawn).
     * \see setFrameSize()
     * \deprecated QGIS 3.40. Use frameSizeMm() instead.
     */
    Q_DECL_DEPRECATED QSizeF frameSize() const SIP_DEPRECATED;

    /**
     * Sets the size (in millimeters) of the annotation's frame (the main area in which
     * the annotation's content is drawn).
     * \see frameSizeMm()
     * \since QGIS 3.4.8
     */
    void setFrameSizeMm( QSizeF size );

    /**
     * Returns the size (in millimeters) of the annotation's frame (the main area in which
     * the annotation's content is drawn).
     * \see setFrameSizeMm()
     * \since QGIS 3.4.8
     */
    QSizeF frameSizeMm() const { return mFrameSize; }

    /**
     * Sets the margins (in millimeters) between the outside of the frame and the annotation
     * content.
     * \see contentsMargin()
     */
    void setContentsMargin( const QgsMargins &margins );

    /**
     * Returns the margins (in millimeters) between the outside of the frame and the annotation
     * content.
     * \see setContentsMargin()
     */
    QgsMargins contentsMargin() const { return mContentsMargins; }

    /**
     * Sets the fill symbol used for rendering the annotation frame. Ownership
     * of the symbol is transferred to the annotation.
     * \see fillSymbol()
     */
    void setFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbol that is used for rendering the annotation frame.
     * \see setFillSymbol()
     */
    QgsFillSymbol *fillSymbol() const;

    /**
     * Renders the annotation to a target render context.
     */
    void render( QgsRenderContext &context ) const;

    /**
     * Writes the annotation state to a DOM element. Derived classes should
     * call _writeXml() within their implementation of this method.
     * \see readXml()
     * \see _writeXml()
     */
    virtual void writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    /**
     * Restores the annotation's state from a DOM element. Derived classes should
     * call _readXml() within their implementation of this method.
     * \see writeXml()
     * \see _readXml()
     */
    virtual void readXml( const QDomElement &itemElem, const QgsReadWriteContext &context ) = 0;

    /**
     * Sets the symbol that is drawn at the annotation's map position. Ownership
     * of the symbol is transferred to the annotation.
     * \see markerSymbol()
     */
    void setMarkerSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbol that is drawn at the annotation's map position.
     * \see setMarkerSymbol()
     */
    QgsMarkerSymbol *markerSymbol() const { return mMarkerSymbol.get(); }

    /**
     * Returns the map layer associated with the annotation. Annotations can be
     * associated with a map layer if their visibility should be synchronized
     * with the layer's visibility.
     * \see setMapLayer()
     */
    QgsMapLayer *mapLayer() const { return mMapLayer.data(); }

    /**
     * Sets the map layer associated with the annotation. Annotations can be
     * associated with a map layer if their visibility should be synchronized
     * with the layer's visibility.
     * \see mapLayer()
     */
    void setMapLayer( QgsMapLayer *layer );

    /**
     * Returns the feature associated with the annotation, or an invalid
     * feature if none has been set.
     * \see setAssociatedFeature()
     */
    QgsFeature associatedFeature() const { return mFeature; }

    /**
     * Sets the feature associated with the annotation.
     * \see associatedFeature()
     */
    virtual void setAssociatedFeature( const QgsFeature &feature );

    /**
     * Accepts the specified style entity \a visitor, causing it to visit all style entities associated
     * within the annotation.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     *
     * \since QGIS 3.10
     */
    virtual bool accept( QgsStyleEntityVisitorInterface *visitor ) const;

  signals:

    //! Emitted whenever the annotation's appearance changes
    void appearanceChanged();

    /**
     * Emitted when the annotation's position has changed and items need
     * to be moved to reflect this.
     */
    void moved();

    /**
     * Emitted when the map layer associated with the annotation changes.
     */
    void mapLayerChanged();

  protected:

    /**
     * Renders the annotation's contents to a target /a context at the specified /a size.
     * Derived classes should implement their custom annotation drawing logic here.
     */
    virtual void renderAnnotation( QgsRenderContext &context, QSizeF size ) const = 0;

    /**
     * Returns the minimum frame size for the annotation. Subclasses should implement this if they
     * cannot be resized smaller than a certain minimum size.
     */
    virtual QSizeF minimumFrameSize() const;

    /**
     * Writes common annotation properties to a DOM element.
     * This method should be called from subclasses in their writeXml method.
     * \see writeXml()
     * \see _readXml()
     */
    void _writeXml( QDomElement &itemElem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Reads common annotation properties from a DOM element.
     * This method should be called from subclasses in their readXml method.
     * \see readXml()
     * \see _writeXml()
     */
    void _readXml( const QDomElement &annotationElem, const QgsReadWriteContext &context );

    /**
     * Copies common annotation properties to the \a targe
     * annotation.
     * Can be used within QgsAnnotation::clone() implementations
     * to assist with creating copies.
     */
    void copyCommonProperties( QgsAnnotation *target ) const;

  private:

    //! Draws the annotation frame to a destination painter
    void drawFrame( QgsRenderContext &context ) const;

    //! Draws the map position marker symbol to a destination painter
    void drawMarkerSymbol( QgsRenderContext &context ) const;

    bool mVisible = true;

    //! True if the item stays at the same map position, FALSE if the item stays on same screen position
    bool mHasFixedMapPosition = true;

    //! Map position (for fixed position items)
    QgsPointXY mMapPosition;

    //! CRS of the map position
    QgsCoordinateReferenceSystem mMapPositionCrs;

    //! Relative position (for non-fixed items) (0-1)
    QPointF mRelativePosition;

    //! Describes the shift of the item content box to the reference point in mm
    QPointF mOffsetFromReferencePoint = QPointF( 13, -13 );

    //! Size of the frame in mm (without balloon)
    QSizeF mFrameSize;

    //! Point symbol that is to be drawn at the map reference location
    std::unique_ptr<QgsMarkerSymbol> mMarkerSymbol;

    QgsMargins mContentsMargins;

    //! Fill symbol used for drawing annotation
    std::unique_ptr<QgsFillSymbol> mFillSymbol;

    //! Associated layer (or NULLPTR if not attached to a layer)
    QgsWeakMapLayerPointer mMapLayer;

    //! Associated feature, or invalid feature if no feature associated
    QgsFeature mFeature;

    //! Roughly 10 pixels at 96 dpi, to match earlier version rendering
    double mSegmentPointWidthMm = 2.64;

};

#endif // QGSANNOTATION_H

