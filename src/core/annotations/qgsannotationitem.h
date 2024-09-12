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
#include "qgsgeometry.h"
#include "qgscallout.h"

class QgsFeedback;
class QgsMarkerSymbol;
class QgsLineSymbol;
class QgsFillSymbol;
class QgsAnnotationItemNode;
class QgsAnnotationItemEditContext;
class QgsAbstractAnnotationItemEditOperation;
class QgsAnnotationItemEditOperationTransientResults;
class QgsRenderContext;
class QgsReadWriteContext;

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
    else if ( sipCpp->type() == QLatin1String( "linetext" ) )
    {
      sipType = sipType_QgsAnnotationLineTextItem;
    }
    else if ( sipCpp->type() == QLatin1String( "recttext" ) )
    {
      sipType = sipType_QgsAnnotationRectangleTextItem;
    }
    else if ( sipCpp->type() == QLatin1String( "picture" ) )
    {
      sipType = sipType_QgsAnnotationPictureItem;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    QgsAnnotationItem();

#ifndef SIP_RUN
    QgsAnnotationItem( const QgsAnnotationItem &other ) = delete;
    QgsAnnotationItem &operator=( const QgsAnnotationItem &other ) = delete;
#endif

    virtual ~QgsAnnotationItem();

    /**
     * Returns item flags.
     *
     * \since QGIS 3.22
     */
    virtual Qgis::AnnotationItemFlags flags() const;

    /**
     * Returns a clone of the item. Ownership is transferred to the caller.
     *
     * Implementations should include a call to copyCommonProperties() to copy the base class properties.
     *
     * \see copyCommonProperties()
     */
    virtual QgsAnnotationItem *clone() const = 0 SIP_FACTORY;

    /**
     * Returns a unique (untranslated) string identifying the type of item.
     */
    virtual QString type() const = 0;

    /**
     * Returns the bounding box of the item's geographic location, in the parent layer's coordinate reference system.
     */
    virtual QgsRectangle boundingBox() const = 0;

    /**
     * Returns the bounding box of the item's geographic location, in the parent layer's coordinate reference system.
     */
    virtual QgsRectangle boundingBox( QgsRenderContext &context ) const { Q_UNUSED( context ) return boundingBox();}

    /**
     * Renders the item to the specified render \a context.
     *
     * The \a feedback argument can be used to detect render cancellations during expensive
     * render operations.
     */
    virtual void render( QgsRenderContext &context, QgsFeedback *feedback ) = 0;

    /**
     * Writes the item's state into an XML \a element.
     *
     * Implementations should include a call to writeCommonProperties() to store the base class properties.
     *
     * \see readXml()
     * \see writeCommonProperties()
     */
    virtual bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const = 0;

    /**
     * Reads the item's state from the given DOM \a element.
     *
     * Implementations should include a call to readCommonProperties() to read the base class properties.
     *
     * \see writeXml()
     * \see readCommonProperties()
     */
    virtual bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Applies an edit \a operation to the item.
     *
     * \deprecated QGIS 3.40. Use applyEditV2() instead.
     */
    Q_DECL_DEPRECATED virtual Qgis::AnnotationItemEditOperationResult applyEdit( QgsAbstractAnnotationItemEditOperation *operation ) SIP_DEPRECATED;

    /**
     * Applies an edit \a operation to the item.
     *
     * \since QGIS 3.40
     */
    virtual Qgis::AnnotationItemEditOperationResult applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context );

    /**
     * Retrieves the results of a transient (in progress) edit \a operation on the item.
     *
     * \deprecated QGIS 3.40. Use transientEditResultsV2() instead.
     */
    Q_DECL_DEPRECATED virtual QgsAnnotationItemEditOperationTransientResults *transientEditResults( QgsAbstractAnnotationItemEditOperation *operation ) SIP_FACTORY;

    /**
     * Retrieves the results of a transient (in progress) edit \a operation on the item.
     *
     * \since QGIS 3.40
     */
    virtual QgsAnnotationItemEditOperationTransientResults *transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context ) SIP_FACTORY;

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

    /**
     * Returns TRUE if the item is enabled and will be rendered in the layer.
     *
     * \see setEnabled()
     * \since QGIS 3.36
     */
    bool enabled() const { return mEnabled; }

    /**
     * Sets if the item will be rendered or not in the layer.
     *
     * \see enabled()
     * \since QGIS 3.36
     */
    void setEnabled( bool enabled ) { mEnabled = enabled; }

    /**
     * Returns the nodes for the item, used for editing the item.
     *
     * \deprecated QGIS 3.40. Use nodesV2() instead.
     */
    Q_DECL_DEPRECATED virtual QList< QgsAnnotationItemNode > nodes() const SIP_DEPRECATED;

    /**
     * Returns the nodes for the item, used for editing the item.
     *
     * \since QGIS 3.40
     */
    virtual QList< QgsAnnotationItemNode > nodesV2( const QgsAnnotationItemEditContext &context ) const;

    /**
     * Returns TRUE if the annotation item uses a symbology reference scale.
     *
     * \see setUseSymbologyReferenceScale()
     * \see symbologyReferenceScale()
     */
    bool useSymbologyReferenceScale() const { return mUseReferenceScale; }

    /**
     * Sets whether the annotation item uses a symbology reference scale.
     *
     * \see useSymbologyReferenceScale()
     * \see setSymbologyReferenceScale()
     */
    void setUseSymbologyReferenceScale( bool enabled ) { mUseReferenceScale = enabled; }

    /**
     * Returns the annotation's symbology reference scale.
     *
     * The reference scale will only be used if useSymbologyReferenceScale() returns TRUE.
     *
     * This represents the desired scale denominator for the rendered map, eg 1000.0 for a 1:1000 map render.
     *
     * The symbology reference scale is an optional property which specifies the reference
     * scale at which symbology in paper units (such a millimeters or points) is fixed
     * to. For instance, if the scale is 1000 then a 2mm thick line will be rendered at
     * exactly 2mm thick when a map is rendered at 1:1000, or 1mm thick when rendered at 1:2000, or 4mm thick at 1:500.
     *
     * \see setSymbologyReferenceScale()
     * \see useSymbologyReferenceScale()
     */
    double symbologyReferenceScale() const { return mReferenceScale; }

    /**
     * Sets the annotation's symbology reference \a scale.
     *
     * The reference scale will only be used if useSymbologyReferenceScale() returns TRUE.
     *
     * This represents the desired scale denominator for the rendered map, eg 1000.0 for a 1:1000 map render.
     *
     * The symbology reference scale is an optional property which specifies the reference
     * scale at which symbology in paper units (such a millimeters or points) is fixed
     * to. For instance, if the scale is 1000 then a 2mm thick line will be rendered at
     * exactly 2mm thick when a map is rendered at 1:1000, or 1mm thick when rendered at 1:2000, or 4mm thick at 1:500.
     *
     * \see symbologyReferenceScale()
     * \see setUseSymbologyReferenceScale()
     */
    void setSymbologyReferenceScale( double scale ) { mReferenceScale = scale; }

    /**
     * Returns the item's callout renderer, responsible for drawing item callouts.
     *
     * Ownership is not transferred.
     *
     * By default items do not have a callout, and it is necessary to be explicitly set
     * a callout style (via setCallout() ) and set the callout anchor geometry (via set
     * setCalloutAnchor() ).
     *
     * \note Callouts are only supported by items which return Qgis::AnnotationItemFlag::SupportsCallouts from flags().
     *
     * \see setCallout()
     * \see calloutAnchor()
     * \since QGIS 3.40
     */
    QgsCallout *callout() const;

    /**
     * Sets the item's \a callout renderer, responsible for drawing item callouts.
     *
     * Ownership of \a callout is transferred to the item.
     *
     * \note Callouts are only supported by items which return Qgis::AnnotationItemFlag::SupportsCallouts from flags().
     *
     * \see callout()
     * \see setCalloutAnchor()
     * \since QGIS 3.40
     */
    void setCallout( QgsCallout *callout SIP_TRANSFER );

    /**
     * Returns the callout's anchor geometry.
     *
     * The anchor dictates the geometry which the option item callout() should connect to. Depending on the
     * callout subclass and anchor geometry type, the actual shape of the rendered callout may vary.
     *
     * The callout anchor geometry is in the parent layer's coordinate reference system.
     *
     * \see callout()
     * \see setCalloutAnchor()
     *
     * \since QGIS 3.40
     */
    QgsGeometry calloutAnchor() const;

    /**
     * Sets the callout's \a anchor geometry.
     *
     * The anchor dictates the geometry which the option item callout() should connect to. Depending on the
     * callout subclass and anchor geometry type, the actual shape of the rendered callout may vary.
     *
     * The callout \a anchor geometry must be specified in the parent layer's coordinate reference system.
     *
     * \see setCallout()
     * \see calloutAnchor()
     *
     * \since QGIS 3.40
     */
    void setCalloutAnchor( const QgsGeometry &anchor );

    /**
     * Returns the (optional) offset of the annotation item from the calloutAnchor().
     *
     * Some annotation item subclasses support placement relative to the callout anchor. For these
     * items, the offset from callout defines how far (in screen/page units) the item should be
     * placed from the anchor point.
     *
     * Units are defined by offsetFromCalloutUnit()
     *
     * \see setOffsetFromCallout()
     * \since QGIS 3.40
     */
    QSizeF offsetFromCallout() const;

    /**
     * Sets the offset of the annotation item from the calloutAnchor().
     *
     * Some annotation item subclasses support placement relative to the callout anchor. For these
     * items, the offset from callout defines how far (in screen/page units) the item should be
     * placed from the anchor point.
     *
     * Units are defined by offsetFromCalloutUnit()
     *
     * \see offsetFromCallout()
     * \since QGIS 3.40
     */
    void setOffsetFromCallout( const QSizeF &offset );

    /**
     * Returns the units for the offsetFromCallout().
     *
     * \see offsetFromCallout()
     * \see setOffsetFromCalloutUnit()
     *
     * \since QGIS 3.40
     */
    Qgis::RenderUnit offsetFromCalloutUnit() const;

    /**
     * Sets the \a unit for the offsetFromCallout().
     *
     * \see setOffsetFromCallout()
     * \see offsetFromCalloutUnit()
     *
     * \since QGIS 3.40
     */
    void setOffsetFromCalloutUnit( Qgis::RenderUnit unit );

  protected:

    /**
     * Copies common properties from the base class from an \a other item.
     *
     * \since QGIS 3.22
     */
    virtual void copyCommonProperties( const QgsAnnotationItem *other );

    /**
     * Writes common properties from the base class into an XML \a element.
     *
     * \see writeXml()
     * \since QGIS 3.22
     */
    virtual bool writeCommonProperties( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Reads common properties from the base class from the given DOM \a element.
     *
     * \see readXml()
     * \since QGIS 3.22
     */
    virtual bool readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Renders the item's callout.
     *
     * The item must have valid callout() set.
     *
     * \since QGIS 3.40
     */
    void renderCallout( QgsRenderContext &context, const QRectF &rect, double angle, QgsCallout::QgsCalloutContext &calloutContext, QgsFeedback *feedback );

  private:

    int mZIndex = 0;
    bool mEnabled = true;
    bool mUseReferenceScale = false;
    double mReferenceScale = 0;

    std::unique_ptr< QgsCallout > mCallout;
    QgsGeometry mCalloutAnchor;
    QSizeF mOffsetFromCallout;
    Qgis::RenderUnit mOffsetFromCalloutUnit = Qgis::RenderUnit::Millimeters;

#ifdef SIP_RUN
    QgsAnnotationItem( const QgsAnnotationItem &other );
#endif

};

#endif // QGSANNOTATIONITEM_H
