/***************************************************************************
    qgsannotationpointtextitem.h
    ----------------
    begin                : August 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSANNOTATIONPOINTTEXTITEM_H
#define QGSANNOTATIONPOINTTEXTITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"
#include "qgstextformat.h"


/**
 * \ingroup core
 * \brief An annotation item which renders a text string at a point location.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationPointTextItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationPointTextItem, containing the specified \a text at the specified \a point.
     */
    QgsAnnotationPointTextItem( const QString &text, QgsPointXY point );
    ~QgsAnnotationPointTextItem() override;

    Qgis::AnnotationItemFlags flags() const override;
    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;

    /**
     * Creates a new text at point annotation item.
     */
    static QgsAnnotationPointTextItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationPointTextItem *clone() override SIP_FACTORY;
    QgsRectangle boundingBox() const override;
    QgsRectangle boundingBox( QgsRenderContext &context ) const override;
    QList< QgsAnnotationItemNode > nodes() const override;
    Qgis::AnnotationItemEditOperationResult applyEdit( QgsAbstractAnnotationItemEditOperation *operation ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResults( QgsAbstractAnnotationItemEditOperation *operation ) override SIP_FACTORY;

    /**
     * Returns the point location of the text.
     *
     * The coordinate reference system for the point will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see setPoint()
     */
    QgsPointXY point() const { return mPoint; }

    /**
     * Sets the \a point location of the text.
     *
     * The coordinate reference system for the point will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see point()
     */
    void setPoint( QgsPointXY point ) { mPoint = point; }

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
     * Returns the text's rotation angle, in degrees clockwise.
     *
     * \see setAngle()
     */
    double angle() const { return mAngle; }

    /**
     * Sets the text's rotation \a angle, in degrees clockwise.
     *
     * \see angle()
     */
    void setAngle( double angle ) { mAngle = angle; }

    /**
     * Returns the text's alignment relative to the reference point().
     *
     * \see setAlignment().
     */
    Qt::Alignment alignment() const;

    /**
     * Sets the text's \a alignment relative to the reference point().
     *
     * \see alignment().
     */
    void setAlignment( Qt::Alignment alignment );

  private:

    QString mText;
    QgsPointXY mPoint;
    QgsTextFormat mTextFormat;
    double mAngle = 0;
    Qt::Alignment mAlignment = Qt::AlignHCenter;

#ifdef SIP_RUN
    QgsAnnotationPointTextItem( const QgsAnnotationPointTextItem &other );
#endif

};

#endif // QGSANNOTATIONPOINTTEXTITEM_H
