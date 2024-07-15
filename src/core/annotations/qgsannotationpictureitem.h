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
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QList< QgsAnnotationItemNode > nodes() const override;
    Qgis::AnnotationItemEditOperationResult applyEdit( QgsAbstractAnnotationItemEditOperation *operation ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResults( QgsAbstractAnnotationItemEditOperation *operation ) override SIP_FACTORY;

    /**
     * Creates a new polygon annotation item.
     */
    static QgsAnnotationPictureItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationPictureItem *clone() const override SIP_FACTORY;
    QgsRectangle boundingBox() const override;

    /**
     * Returns the bounds of the picture.
     *
     * The coordinate reference system for the bounds will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see setBounds()
     */
    QgsRectangle bounds() const { return mBounds; }

    /**
     * Sets the \a bounds of the picture.
     *
     * The coordinate reference system for the bounds will be the parent layer's QgsAnnotationLayer::crs().
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
    bool frameEnabled() const { return mDrawBorder; }

    /**
     * Sets whether the item's frame should be rendered.
     *
     * \see frameEnabled()
     * \see setFrameSymbol()
     */
    void setFrameEnabled( bool enabled ) { mDrawBorder = enabled; }

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
    QgsRectangle mBounds;
    bool mLockAspectRatio = true;
    bool mDrawBackground = false;
    std::unique_ptr< QgsFillSymbol > mBackgroundSymbol;
    bool mDrawBorder = false;
    std::unique_ptr< QgsFillSymbol > mBorderSymbol;

#ifdef SIP_RUN
    QgsAnnotationPictureItem( const QgsAnnotationPictureItem &other );
#endif

};
#endif // QGSANNOTATIONPICTUREITEM_H
