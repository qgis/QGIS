/***************************************************************************
    qgsinvertedpolygonrenderer.h
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINVERTEDPOLYGONRENDERER_H
#define QGSINVERTEDPOLYGONRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsrenderer.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsmergedfeaturerenderer.h"

/**
 * \ingroup core
 * \brief A polygon-only feature renderer used to display features inverted.
 *
 * The exterior of the polygon is turned to an interior
 * and where the exterior theoretically spans the entire plane, allowing
 * to mask the surroundings of some features.
 *
 * It is designed on top of another feature renderer, which is called "embedded"
 * Most of the methods are then only proxies to the embedded renderer.
 *
 * Features are collected to form one "inverted" polygon
 * during renderFeature() and rendered on stopRender().
 *
 */
class CORE_EXPORT QgsInvertedPolygonRenderer : public QgsMergedFeatureRenderer
{
  public:

    /**
     * Constructor
     * \param embeddedRenderer optional embeddedRenderer. If NULLPTR, a default one will be assigned.
     * Ownership will be transferred.
     */
    QgsInvertedPolygonRenderer( QgsFeatureRenderer *embeddedRenderer SIP_TRANSFER = nullptr );

    //! Direct copies are forbidden. Use clone() instead.
    QgsInvertedPolygonRenderer( const QgsInvertedPolygonRenderer & ) = delete;
    //! Direct copies are forbidden. Use clone() instead.
    QgsInvertedPolygonRenderer &operator=( const QgsInvertedPolygonRenderer & ) = delete;

    QgsInvertedPolygonRenderer *clone() const override SIP_FACTORY;

    QString dump() const override;

    //! Creates a renderer out of an XML, for loading
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY; // cppcheck-suppress duplInheritedMember

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;

    /**
     * Returns TRUE if the geometries are to be preprocessed (merged with an union) before rendering.
     *
     * \see setPreprocessingEnabled()
     */
    bool preprocessingEnabled() const { return mOperation == MergeAndInvert; }

    /**
     * Sets whether geometries preprocessing is enabled.
     *
     * When enabled, geometries will be merged with an union before being rendered.
     *
     * This allows fixing some rendering artifacts (when rendering overlapping polygons for instance).
     * This will involve some CPU-demanding computations and is thus disabled by default.
     *
     * \see preprocessingEnabled()
     */
    void setPreprocessingEnabled( bool enabled ) { mOperation = enabled ? MergeAndInvert : InvertOnly; }

    /**
     * Creates a QgsInvertedPolygonRenderer by a conversion from an existing renderer.
     * \returns a new renderer if the conversion was possible, otherwise NULLPTR.
     */
    static QgsInvertedPolygonRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY; // cppcheck-suppress duplInheritedMember

};


#endif // QGSINVERTEDPOLYGONRENDERER_H
