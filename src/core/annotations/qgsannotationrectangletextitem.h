/***************************************************************************
    qgsannotationrectangletextitem.h
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

#ifndef QGSANNOTATIONRECTANGLETEXTITEM_H
#define QGSANNOTATIONRECTANGLETEXTITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"
#include "qgstextformat.h"
#include "qgsmargins.h"

/**
 * \ingroup core
 * \brief An annotation item which renders paragraphs of text within a rectangle.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAnnotationRectangleTextItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationRectangleTextItem, containing the specified \a text
     * within the specified \a bounds rectangle.
     */
    QgsAnnotationRectangleTextItem( const QString &text, const QgsRectangle &bounds );
    ~QgsAnnotationRectangleTextItem() override;

    QString type() const override;
    Qgis::AnnotationItemFlags flags() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QList< QgsAnnotationItemNode > nodesV2( const QgsAnnotationItemEditContext &context ) const override;
    Qgis::AnnotationItemEditOperationResult applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override SIP_FACTORY;

    /**
     * Creates a new rectangle text annotation item.
     */
    static QgsAnnotationRectangleTextItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationRectangleTextItem *clone() const override SIP_FACTORY;
    QgsRectangle boundingBox() const override;

    /**
     * Returns the bounds of the text.
     *
     * The coordinate reference system for the bounds will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see setBounds()
     */
    QgsRectangle bounds() const { return mBounds; }

    /**
     * Sets the \a bounds of the text.
     *
     * The coordinate reference system for the bounds will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see bounds()
     */
    void setBounds( const QgsRectangle &bounds );

    /**
     * Returns the text rendered by the item.
     *
     * \see setText()
     */
    QString text() const { return mText; }

    /**
     * Sets the \a text rendered by the item.
     *
     * \see text()
     */
    void setText( const QString &text ) { mText = text; }

    /**
     * Returns the text format used to render the text.
     *
     * \see setFormat()
     */
    QgsTextFormat format() const;

    /**
     * Sets the text \a format used to render the text.
     *
     * \see format()
     */
    void setFormat( const QgsTextFormat &format );

    /**
     * Returns the text's alignment relative to the bounds() rectangle.
     *
     * \see setAlignment().
     */
    Qt::Alignment alignment() const;

    /**
     * Sets the text's \a alignment relative to the bounds() rectangle.
     *
     * \see alignment().
     */
    void setAlignment( Qt::Alignment alignment );

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

    /**
     * Returns the margins between the outside of the item's frame and the interior text.
     *
     * Units are retrieved via marginsUnit()
     *
     * \see setMargins()
     * \see marginsUnit()
     */
    const QgsMargins &margins() const { return mMargins; }

    /**
     * Sets the \a margins between the outside of the item's frame and the interior text.
     *
     * Units are set via setMarginsUnit()
     *
     * \see margins()
     * \see setMarginsUnit()
     */
    void setMargins( const QgsMargins &margins ) { mMargins = margins; }

    /**
     * Sets the \a unit for the margins between the item's frame and the interior text.
     *
     * \see margins()
     * \see marginsUnit()
    */
    void setMarginsUnit( Qgis::RenderUnit unit ) { mMarginUnit = unit; }

    /**
     * Returns the units for the margins between the item's frame and the interior text.
     *
     * \see setMarginsUnit()
     * \see margins()
    */
    Qgis::RenderUnit marginsUnit() const { return mMarginUnit; }

  private:

    QgsRectangle mBounds;
    QString mText;
    QgsTextFormat mTextFormat;
    Qt::Alignment mAlignment = Qt::AlignLeft;

    bool mDrawBackground = true;
    std::unique_ptr< QgsFillSymbol > mBackgroundSymbol;
    bool mDrawFrame = true;
    std::unique_ptr< QgsFillSymbol > mFrameSymbol;

    QgsMargins mMargins;
    Qgis::RenderUnit mMarginUnit = Qgis::RenderUnit::Millimeters;

#ifdef SIP_RUN
    QgsAnnotationRectangleTextItem( const QgsAnnotationRectangleTextItem &other );
#endif

};
#endif // QGSANNOTATIONRECTANGLETEXTITEM_H
