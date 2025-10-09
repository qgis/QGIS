/***************************************************************************
  qgsannotationlayer3drenderer.h
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYER3DRENDERER_H
#define QGSANNOTATIONLAYER3DRENDERER_H

#include "qgis_3d.h"
#include "qgis_sip.h"

#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgstextformat.h"
#include "qgsmaplayerref.h"

class QgsAnnotationLayer;

#ifdef SIP_RUN
// this is needed for the "convert to subclass" code below to compile
% ModuleHeaderCode
#include "qgsannotationlayer3drenderer.h"
  % End
#endif

  /**
 * \ingroup core
 * \brief Metadata for annotation layer 3D renderer to allow creation of its instances from XML.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 4.0
 */
  class _3D_EXPORT QgsAnnotationLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsAnnotationLayer3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};

/**
 * \ingroup qgis_3d
 * \brief 3D renderers for annotation layers.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsAnnotationLayer3DRenderer : public QgsAbstract3DRenderer
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsAnnotationLayer3DRenderer *>( sipCpp ) != nullptr )
      sipType = sipType_QgsAnnotationLayer3DRenderer;
    else
      sipType = nullptr;
    SIP_END
#endif

  public:
    QgsAnnotationLayer3DRenderer();

    /**
     * Sets the annotation layer associated with the renderer.
     *
     * \see layer()
     */
    void setLayer( QgsAnnotationLayer *layer );

    /**
     * Returns the annotation layer associated with the renderer.
     *
     * \see setLayer()
     */
    QgsAnnotationLayer *layer() const;

    QString type() const override;
    QgsAnnotationLayer3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( Qgs3DMapSettings *map ) const override SIP_SKIP;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject &project ) override;

    /**
     * Returns the altitude clamping method, which determines the vertical position of annotations.
     *
     * \see setAltitudeClamping()
     */
    Qgis::AltitudeClamping altitudeClamping() const { return mAltClamping; }

    /**
     * Sets the altitude \a clamping method, which determines the vertical position of annotations.
     *
     * \see altitudeClamping()
     */
    void setAltitudeClamping( Qgis::AltitudeClamping clamping ) { mAltClamping = clamping; }

    /**
     * Returns the z offset, which is a fixed offset amount which should be added to z values for the annotations.
     *
     * \see setZOffset()
     */
    double zOffset() const { return mZOffset; }

    /**
     * Sets the z \a offset, which is a fixed offset amount which will be added to z values for the annotations.
     *
     * \see zOffset()
     */
    void setZOffset( double offset ) { mZOffset = offset; }

    /**
     * Returns TRUE if callout lines are shown, vertically joining the annotations to the terrain.
     *
     * \see setShowCalloutLines()
     */
    bool showCalloutLines() const;

    /**
     * Sets whether callout lines are shown, vertically joining the annotations to the terrain.
     *
     * \see showCalloutLines()
     */
    void setShowCalloutLines( bool show );

    // TODO -- consider exposing via QgsSimpleLineMaterialSettings, for now, for testing only
    /**
     * Sets the callout line \a color.
     *
     * \see calloutLineColor()
     * \note Not available in Python bindings
     */
    SIP_SKIP void setCalloutLineColor( const QColor &color );

    /**
     * Returns the callout line color.
     *
     * \see setCalloutLineColor()
     * \note Not available in Python bindings
     */
    SIP_SKIP QColor calloutLineColor() const;

    /**
     * Sets the callout line \a width.
     *
     * \see calloutLineWidth()
     * \note Not available in Python bindings
     */
    SIP_SKIP void setCalloutLineWidth( double width );

    /**
     * Returns the callout line width.
     *
     * \see setCalloutLineWidth()
     * \note Not available in Python bindings
     */
    SIP_SKIP double calloutLineWidth() const;

    /**
     * Returns the text format to use for rendering text annotations in 3D.
     *
     * \see setTextFormat()
     */
    QgsTextFormat textFormat() const;

    /**
     * Sets the text \a format to use for rendering text annotations in 3D.
     *
     * \see textFormat()
     */
    void setTextFormat( const QgsTextFormat &format );

  private:
#ifdef SIP_RUN
    QgsAnnotationLayer3DRenderer( const QgsAnnotationLayer3DRenderer & );
#endif

    static constexpr double DEFAULT_Z_OFFSET = 50;

    QgsMapLayerRef mLayerRef;
    Qgis::AltitudeClamping mAltClamping = Qgis::AltitudeClamping::Relative;
    double mZOffset = DEFAULT_Z_OFFSET;
    bool mShowCalloutLines = true;
    QColor mCalloutLineColor { 0, 0, 0 };
    double mCalloutLineWidth = 2;
    QgsTextFormat mTextFormat;
};

#endif // QGSANNOTATIONLAYER3DRENDERER_H
