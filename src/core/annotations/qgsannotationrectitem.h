/***************************************************************************
    qgsannotationrectitem.h
    ----------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#ifndef QGSANNOTATIONRECTITEM_H
#define QGSANNOTATIONRECTITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"

/**
 * \ingroup core
 * \brief Abstract base class for annotation items which render annotations in a rectangular shape.
 *
 * Subclasses should implement the pure virtual render() method which takes a painter bounds argument.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAnnotationRectItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationRectItem, rendering the annotation
     * within the specified \a bounds geometry.
     */
    QgsAnnotationRectItem( const QgsRectangle &bounds );
    ~QgsAnnotationRectItem() override;

    Qgis::AnnotationItemFlags flags() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    QList< QgsAnnotationItemNode > nodesV2( const QgsAnnotationItemEditContext &context ) const override;
    Qgis::AnnotationItemEditOperationResult applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override SIP_FACTORY;
    QgsRectangle boundingBox() const override;
    QgsRectangle boundingBox( QgsRenderContext &context ) const override;

    /**
     * Returns the bounds of the item.
     *
     * The coordinate reference system for the item will be the parent layer's QgsAnnotationLayer::crs().
     *
     * When the placementMode() is Qgis::AnnotationPlacementMode::FixedSize then the item will be placed
     * at the center of the bounds.
     *
     * \see setBounds()
     */
    QgsRectangle bounds() const { return mBounds; }

    /**
     * Sets the \a bounds of the item.
     *
     * The coordinate reference system for the bounds will be the parent layer's QgsAnnotationLayer::crs().
     *
     * When the placementMode() is Qgis::AnnotationPlacementMode::FixedSize then the item will be placed
     * at the center of the bounds.
     *
     * \see bounds()
     */
    void setBounds( const QgsRectangle &bounds );

    /**
     * Returns the placement mode for the item.
     *
     * \see setPlacementMode()
     */
    Qgis::AnnotationPlacementMode placementMode() const;

    /**
     * Sets the placement \a mode for the item.
     *
     * \see placementMode()
     */
    void setPlacementMode( Qgis::AnnotationPlacementMode mode );

    /**
     * Returns the fixed size to use for the item, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * Units are retrieved via fixedSizeUnit()
     *
     * \see setFixedSize()
     * \see fixedSizeUnit()
     */
    QSizeF fixedSize() const;

    /**
     * Sets the fixed \a size to use for the item, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * Units are set via setFixedSizeUnit()
     *
     * \see fixedSize()
     * \see setFixedSizeUnit()
     */
    void setFixedSize( const QSizeF &size );

    /**
     * Returns the units to use for fixed item sizes, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * \see setFixedSizeUnit()
     * \see fixedSize()
     */
    Qgis::RenderUnit fixedSizeUnit() const;

    /**
     * Sets the \a unit to use for fixed item sizes, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * \see fixedSizeUnit()
     * \see setFixedSize()
     */
    void setFixedSizeUnit( Qgis::RenderUnit unit );

    /**
     * Returns TRUE if the item's background should be rendered.
     *
     * \see setBackgroundEnabled()
     * \see backgroundSymbol()
     */
    bool backgroundEnabled() const { return mDrawBackground; }

    /**
     * Sets whether the item's background should be rendered.
     *
     * \see backgroundEnabled()
     * \see setBackgroundSymbol()
     */
    void setBackgroundEnabled( bool enabled ) { mDrawBackground = enabled; }

    /**
     * Returns the symbol used to render the item's background.
     *
     * \see backgroundEnabled()
     * \see setBackgroundSymbol()
     */
    const QgsFillSymbol *backgroundSymbol() const;

    /**
     * Sets the \a symbol used to render the item's background.
     *
     * The item takes ownership of the symbol.
     *
     * \see backgroundSymbol()
     * \see setBackgroundEnabled()
     */
    void setBackgroundSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns TRUE if the item's frame should be rendered.
     *
     * \see setFrameEnabled()
     * \see frameSymbol()
     */
    bool frameEnabled() const { return mDrawFrame; }

    /**
     * Sets whether the item's frame should be rendered.
     *
     * \see frameEnabled()
     * \see setFrameSymbol()
     */
    void setFrameEnabled( bool enabled ) { mDrawFrame = enabled; }

    /**
     * Returns the symbol used to render the item's frame.
     *
     * \see frameEnabled()
     * \see setFrameSymbol()
     */
    const QgsFillSymbol *frameSymbol() const;

    /**
     * Sets the \a symbol used to render the item's frame.
     *
     * The item takes ownership of the symbol.
     *
     * \see frameSymbol()
     * \see setBackgroundEnabled()
     */
    void setFrameSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

  protected:
    void copyCommonProperties( const QgsAnnotationItem *other ) override;
    bool writeCommonProperties( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context ) override;

    /**
     * Renders the item to the specified render \a context.
     *
     * The \a painterRect argument specifies the bounds in painter units where the rectangular
     * item should be rendered within.
     *
     * The \a feedback argument can be used to detect render cancellations during expensive
     * render operations.
     */
    virtual void renderInBounds( QgsRenderContext &context, const QRectF &painterRect, QgsFeedback *feedback ) = 0;

  private:

    Qgis::AnnotationPlacementMode mPlacementMode = Qgis::AnnotationPlacementMode::SpatialBounds;
    QgsRectangle mBounds;

    QSizeF mFixedSize;
    Qgis::RenderUnit mFixedSizeUnit = Qgis::RenderUnit::Millimeters;
    bool mDrawBackground = false;
    std::unique_ptr< QgsFillSymbol > mBackgroundSymbol;
    bool mDrawFrame = false;
    std::unique_ptr< QgsFillSymbol > mFrameSymbol;

#ifdef SIP_RUN
    QgsAnnotationRectItem( const QgsAnnotationRectItem &other );
#endif

};
#endif // QGSANNOTATIONRECTITEM_H
