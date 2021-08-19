/***************************************************************************
    qgsannotationitem.h
    ----------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONITEM_H
#define QGSANNOTATIONITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrendercontext.h"
#include "qgslinestring.h"
#include "qgspolygon.h"

class QgsFeedback;
class QgsMarkerSymbol;
class QgsLineSymbol;
class QgsFillSymbol;

/**
 * \ingroup core
 * \brief Abstract base class for annotation items which are drawn with QgsAnnotationLayers.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationItem
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == QLatin1String( "marker" ) )
    {
      sipType = sipType_QgsAnnotationMarkerItem;
    }
    else if ( sipCpp->type() == QLatin1String( "linestring" ) )
    {
      sipType = sipType_QgsAnnotationLineItem;
    }
    else if ( sipCpp->type() == QLatin1String( "polygon" ) )
    {
      sipType = sipType_QgsAnnotationPolygonItem;
    }
    else if ( sipCpp->type() == QLatin1String( "pointtext" ) )
    {
      sipType = sipType_QgsAnnotationPointTextItem;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for an annotation item.
     */
    QgsAnnotationItem() = default;

#ifndef SIP_RUN
    //! QgsAnnotationItem cannot be copied
    QgsAnnotationItem( const QgsAnnotationItem &other ) = delete;
    //! QgsAnnotationItem cannot be copied
    QgsAnnotationItem &operator=( const QgsAnnotationItem &other ) = delete;
#endif

    virtual ~QgsAnnotationItem() = default;

    /**
     * Returns a clone of the item. Ownership is transferred to the caller.
     */
    virtual QgsAnnotationItem *clone() = 0 SIP_FACTORY;

    /**
     * Returns a unique (untranslated) string identifying the type of item.
     */
    virtual QString type() const = 0;

    /**
     * Returns the bounding box of the item's geographic location, in the parent layer's coordinate reference system.
     */
    virtual QgsRectangle boundingBox() const = 0;

    /**
     * Renders the item to the specified render \a context.
     *
     * The \a feedback argument can be used to detect render cancellations during expensive
     * render operations.
     */
    virtual void render( QgsRenderContext &context, QgsFeedback *feedback ) = 0;

    /**
     * Writes the item's state into an XML \a element.
     */
    virtual bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const = 0;

    /**
     * Reads the item's state from the given DOM \a element.
     */
    virtual bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Returns the item's z index, which controls the order in which annotation items
     * are rendered in the layer.
     *
     * \see setZIndex()
     */
    int zIndex() const { return mZIndex; }

    /**
     * Sets the item's z \a index, which controls the order in which annotation items
     * are rendered in the layer.
     *
     * \see zIndex()
     */
    void setZIndex( int index ) { mZIndex = index; }

  private:

    int mZIndex = 0;

#ifdef SIP_RUN
    QgsAnnotationItem( const QgsAnnotationItem &other );
#endif

};

#endif // QGSANNOTATIONITEM_H
