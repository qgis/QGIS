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
#include "qgsannotationrectitem.h"
#include "qgstextformat.h"
#include "qgsmargins.h"

/**
 * \ingroup core
 * \brief An annotation item which renders paragraphs of text within a rectangle.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAnnotationRectangleTextItem : public QgsAnnotationRectItem
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
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;

    /**
     * Creates a new rectangle text annotation item.
     */
    static QgsAnnotationRectangleTextItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationRectangleTextItem *clone() const override SIP_FACTORY;

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

  protected:

    void renderInBounds( QgsRenderContext &context, const QRectF &painterBounds, QgsFeedback *feedback ) override;

  private:

    QString mText;
    QgsTextFormat mTextFormat;
    Qt::Alignment mAlignment = Qt::AlignLeft;

    QgsMargins mMargins;
    Qgis::RenderUnit mMarginUnit = Qgis::RenderUnit::Millimeters;

#ifdef SIP_RUN
    QgsAnnotationRectangleTextItem( const QgsAnnotationRectangleTextItem &other );
#endif

};
#endif // QGSANNOTATIONRECTANGLETEXTITEM_H
