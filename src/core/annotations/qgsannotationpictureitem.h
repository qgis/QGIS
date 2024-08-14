/***************************************************************************
    qgsannotationpictureitem.h
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

#ifndef QGSANNOTATIONPICTUREITEM_H
#define QGSANNOTATIONPICTUREITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"

/**
 * \ingroup core
 * \brief An annotation item which renders a picture.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAnnotationPictureItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationPictureItem, rendering the specified image \a path
     * within the specified \a bounds geometry.
     */
    QgsAnnotationPictureItem( Qgis::PictureFormat format, const QString &path, const QgsRectangle &bounds );
    ~QgsAnnotationPictureItem() override;

    QString type() const override;
    Qgis::AnnotationItemFlags flags() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QList< QgsAnnotationItemNode > nodesV2( const QgsAnnotationItemEditContext &context ) const override;
    Qgis::AnnotationItemEditOperationResult applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override SIP_FACTORY;

    /**
     * Creates a new polygon annotation item.
     */
    static QgsAnnotationPictureItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationPictureItem *clone() const override SIP_FACTORY;
    QgsRectangle boundingBox() const override;
    QgsRectangle boundingBox( QgsRenderContext &context ) const override;

    /**
     * Returns the bounds of the picture.
     *
     * The coordinate reference system for the bounds will be the parent layer's QgsAnnotationLayer::crs().
     *
     * When the placementMode() is Qgis::AnnotationPlacementMode::FixedSize then the picture will be placed
     * at the center of the bounds.
     *
     * \see setBounds()
     */
    QgsRectangle bounds() const { return mBounds; }

    /**
     * Sets the \a bounds of the picture.
     *
     * The coordinate reference system for the bounds will be the parent layer's QgsAnnotationLayer::crs().
     *
     * When the placementMode() is Qgis::AnnotationPlacementMode::FixedSize then the picture will be placed
     * at the center of the bounds.
     *
     * \see bounds()
     */
    void setBounds( const QgsRectangle &bounds );

    /**
     * Returns the path of the image used to render the item.
     *
     * \see setPath()
     */
    QString path() const { return mPath; }

    /**
     * Returns the picture format.
     */
    Qgis::PictureFormat format() const { return mFormat; }

    /**
     * Sets the \a format and \a path of the image used to render the item.
     *
     * \see path()
     * \see format()
     */
    void setPath( Qgis::PictureFormat format, const QString &path );

    /**
     * Returns the placement mode for the picture.
     *
     * \see setPlacementMode()
     */
    Qgis::AnnotationPlacementMode placementMode() const;

    /**
     * Sets the placement \a mode for the picture.
     *
     * \see placementMode()
     */
    void setPlacementMode( Qgis::AnnotationPlacementMode mode );

    /**
     * Returns the fixed size to use for the picture, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * Units are retrieved via fixedSizeUnit()
     *
     * \see setFixedSize()
     * \see fixedSizeUnit()
     */
    QSizeF fixedSize() const;

    /**
     * Sets the fixed \a size to use for the picture, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * Units are set via setFixedSizeUnit()
     *
     * \see fixedSize()
     * \see setFixedSizeUnit()
     */
    void setFixedSize( const QSizeF &size );

    /**
     * Returns the units to use for fixed picture sizes, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * \see setFixedSizeUnit()
     * \see fixedSize()
     */
    Qgis::RenderUnit fixedSizeUnit() const;

    /**
     * Sets the \a unit to use for fixed picture sizes, when the placementMode() is Qgis::AnnotationPlacementMode::FixedSize.
     *
     * \see fixedSizeUnit()
     * \see setFixedSize()
     */
    void setFixedSizeUnit( Qgis::RenderUnit unit );

    /**
     * Returns TRUE if the aspect ratio of the picture will be retained.
     *
     * \see setLockAspectRatio()
     */
    bool lockAspectRatio() const;

    /**
     * Sets whether the aspect ratio of the picture will be retained.
     *
     * \see lockAspectRatio()
     */
    void setLockAspectRatio( bool locked );

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

  private:

    QString mPath;
    Qgis::PictureFormat mFormat = Qgis::PictureFormat::Unknown;
    Qgis::AnnotationPlacementMode mPlacementMode = Qgis::AnnotationPlacementMode::SpatialBounds;
    QgsRectangle mBounds;

    QSizeF mFixedSize;
    Qgis::RenderUnit mFixedSizeUnit = Qgis::RenderUnit::Millimeters;

    bool mLockAspectRatio = true;
    bool mDrawBackground = false;
    std::unique_ptr< QgsFillSymbol > mBackgroundSymbol;
    bool mDrawFrame = false;
    std::unique_ptr< QgsFillSymbol > mFrameSymbol;

#ifdef SIP_RUN
    QgsAnnotationPictureItem( const QgsAnnotationPictureItem &other );
#endif

};
#endif // QGSANNOTATIONPICTUREITEM_H
