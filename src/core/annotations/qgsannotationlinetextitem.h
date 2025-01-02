/***************************************************************************
    qgsannotationlinetextitem.h
    ----------------
    begin                : March 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSANNOTATIONLINETEXTITEM_H
#define QGSANNOTATIONLINETEXTITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"
#include "qgstextformat.h"

class QgsCurve;

/**
 * \ingroup core
 * \brief An annotation item which renders text along a line geometry.
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsAnnotationLineTextItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationLineTextItem, with the specified \a curve and \a text.
     */
    QgsAnnotationLineTextItem( const QString &text, QgsCurve *curve SIP_TRANSFER );
    ~QgsAnnotationLineTextItem() override;

    Qgis::AnnotationItemFlags flags() const override;
    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QList< QgsAnnotationItemNode > nodesV2( const QgsAnnotationItemEditContext &context ) const override;
    Qgis::AnnotationItemEditOperationResult applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) override SIP_FACTORY;

    /**
     * Creates a new linestring annotation item.
     */
    static QgsAnnotationLineTextItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsRectangle boundingBox() const override;
    QgsRectangle boundingBox( QgsRenderContext &context ) const override;

    QgsAnnotationLineTextItem *clone() const override SIP_FACTORY;

    /**
     * Returns the geometry of the item.
     *
     * The coordinate reference system for the line will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see setGeometry()
     */
    const QgsCurve *geometry() const { return mCurve.get(); }

    /**
     * Sets the \a geometry of the item. Ownership of \a geometry is transferred.
     *
     * The coordinate reference system for the line will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see geometry()
     */
    void setGeometry( QgsCurve *geometry SIP_TRANSFER );

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
     * Returns the offset distance from the line geometry() to the text's baseline. Units are specified through offsetFromLineUnit().
     *
     * \see setOffsetFromLine()
     * \see offsetFromLineUnit()
     */
    double offsetFromLine() const { return mOffsetFromLineDistance; }

    /**
     * Sets the offset \a distance from the line geometry() to the text's baseline. Units are specified through setOffsetFromLineUnit().
     *
     * \see offsetFromLine()
     * \see setOffsetFromLineUnit()
     */
    void setOffsetFromLine( double distance ) { mOffsetFromLineDistance = distance; }

    /**
     * Sets the \a unit for the offset from line geometry() distance.
     *
     * \see offsetFromLineUnit()
     * \see setOffsetFromLine()
    */
    void setOffsetFromLineUnit( Qgis::RenderUnit unit ) { mOffsetFromLineUnit = unit; }

    /**
     * Returns the units for the offset from line geometry() distance.
     *
     * \see setOffsetFromLineUnit()
     * \see offsetFromLine()
    */
    Qgis::RenderUnit offsetFromLineUnit() const { return mOffsetFromLineUnit; }

    /**
     * Sets the map unit \a scale for the offset from line geometry() distance.
     *
     * \see offsetFromLineMapUnitScale()
     * \see setOffsetFromLineUnit()
     * \see setOffsetFromLine()
     */
    void setOffsetFromLineMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetFromLineScale = scale; }

    /**
     * Returns the map unit scale for the offset from line geometry() distance.
     *
     * \see setOffsetFromLineMapUnitScale()
     * \see offsetFromLineUnit()
     * \see offsetFromLine()
     */
    const QgsMapUnitScale &offsetFromLineMapUnitScale() const { return mOffsetFromLineScale; }

  private:

    QString mText;
    std::unique_ptr< QgsCurve > mCurve;
    QgsTextFormat mTextFormat;

    double mOffsetFromLineDistance = 0;
    Qgis::RenderUnit mOffsetFromLineUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mOffsetFromLineScale;

#ifdef SIP_RUN
    QgsAnnotationLineTextItem( const QgsAnnotationLineTextItem &other );
#endif

};

#endif // QGSANNOTATIONLINETEXTITEM_H
