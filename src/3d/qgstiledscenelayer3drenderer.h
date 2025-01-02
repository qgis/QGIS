/***************************************************************************
  qgstiledscenelayer3drenderer.h
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENELAYER3DRENDERER_H
#define QGSTILEDSCENELAYER3DRENDERER_H

#include "qgis_3d.h"
#include "qgis_sip.h"

#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgsmaplayerref.h"

class QgsTiledSceneLayer;


/**
 * \ingroup 3d
 * \brief Metadata for tiled scene layer 3D renderer to allow creation of its instances from XML
 *
 * \since QGIS 3.34
 */
class _3D_EXPORT QgsTiledSceneLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsTiledSceneLayer3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};


/**
 * \ingroup 3d
 * \brief 3D renderer that renders content of a tiled scene layer
 *
 * \since QGIS 3.34
 */
class _3D_EXPORT QgsTiledSceneLayer3DRenderer : public QgsAbstract3DRenderer
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == QLatin1String( "tiledscene" ) )
    {
      sipType = sipType_QgsTiledSceneLayer3DRenderer;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:
    QgsTiledSceneLayer3DRenderer();

    //! Sets tiled scene layer associated with the renderer
    void setLayer( QgsTiledSceneLayer *layer );
    //! Returns tiled scene layer associated with the renderer
    QgsTiledSceneLayer *layer() const;

    /**
     * Returns the maximum screen error allowed when rendering the tiled scene.
     *
     * Larger values result in a faster render with less content rendered.
     *
     * \see setMaximumScreenError()
     */
    double maximumScreenError() const;

    /**
     * Sets the maximum screen \a error allowed when rendering the tiled scene.
     *
     * Larger values result in a faster render with less content rendered.
     *
     * \see maximumScreenError()
     */
    void setMaximumScreenError( double error );

    /**
     * Returns whether bounding boxes will be visible when rendering the tiled scene.
     *
     * \see setShowBoundingBoxes()
     */
    bool showBoundingBoxes() const;

    /**
     * Sets whether bounding boxes will be visible when rendering the tiled scene.
     *
     * \see showBoundingBoxes()
     */
    void setShowBoundingBoxes( bool showBoundingBoxes );

    virtual QString type() const override { return "tiledscene"; }
    virtual QgsAbstract3DRenderer *clone() const override SIP_FACTORY;
    virtual Qt3DCore::QEntity *createEntity( Qgs3DMapSettings *map ) const override SIP_SKIP;
    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    virtual void resolveReferences( const QgsProject &project ) override;

  private:
    QgsMapLayerRef mLayerRef; //!< Layer used to extract mesh data from
    double mMaximumScreenError = 16.0;
    bool mShowBoundingBoxes = false;

  private:
#ifdef SIP_RUN
    QgsTiledSceneLayer3DRenderer( const QgsTiledSceneLayer3DRenderer & );
    QgsTiledSceneLayer3DRenderer &operator=( const QgsTiledSceneLayer3DRenderer & );
#endif
};

#endif
