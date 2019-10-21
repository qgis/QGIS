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

class QgsFeedback;
class QgsMarkerSymbol;

/**
 * \ingroup core
 * Abstract base class for annotation items which are drawn with QgsAnnotationLayers.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsAnnotationItem
{
  public:

    QgsAnnotationItem( const QgsCoordinateReferenceSystem &crs );

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

    QgsCoordinateReferenceSystem crs() const { return QgsCoordinateReferenceSystem(); }

    /**
     * Renders the item to the specified render \a context.
     *
     * The \a feedback argument can be used to detect render cancelations during expensive
     * render operations.
     */
    virtual void render( QgsRenderContext &context, QgsFeedback *feedback ) = 0;

    /**
     * Writes the item's state the an XML \a element.
     */
    virtual bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const = 0;

    int zIndex() const { return 0; }

  private:

    QgsCoordinateReferenceSystem mCrs;

#ifdef SIP_RUN
    QgsAnnotationItem( const QgsAnnotationItem &other );
#endif

};

class CORE_EXPORT QgsMarkerItem : public QgsAnnotationItem
{
  public:

    QgsMarkerItem( QgsPointXY point, const QgsCoordinateReferenceSystem &crs );
    ~QgsMarkerItem() override;

    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    static QgsMarkerItem *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    QgsMarkerItem *clone() override SIP_FACTORY;

    const QgsMarkerSymbol *symbol() const;
    void setSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

  private:

    QgsPointXY mPoint;
    std::unique_ptr< QgsMarkerSymbol > mSymbol;

#ifdef SIP_RUN
    QgsMarkerItem( const QgsMarkerItem &other );
#endif

};

#endif // QGSANNOTATIONITEM_H
