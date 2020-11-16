/***************************************************************************
  qgspointcloudlayer3drenderer.h
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYER3DRENDERER_H
#define QGSPOINTCLOUDLAYER3DRENDERER_H

#include "qgis_3d.h"
#include "qgis_sip.h"

#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgsmaplayerref.h"

#include <QObject>

class QgsPointCloudLayer;
class QgsPointCloud3DSymbol;

#ifndef SIP_RUN

/**
 * \ingroup core
 * Metadata for point cloud layer 3D renderer to allow creation of its instances from XML
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsPointCloudLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata SIP_SKIP
{
  public:
    QgsPointCloudLayer3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};

#endif

/**
 * \ingroup core
 * 3D renderer that renders all points from a point cloud layer
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsPointCloudLayer3DRenderer : public QgsAbstract3DRenderer
{
  public:
    //! Takes ownership of the symbol object
    explicit QgsPointCloudLayer3DRenderer();

    //! Sets point cloud layer associated with the renderer
    void setLayer( QgsPointCloudLayer *layer );
    //! Returns point cloud layer associated with the renderer
    QgsPointCloudLayer *layer() const;

    QString type() const override;
    QgsPointCloudLayer3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( const Qgs3DMapSettings &map ) const override SIP_SKIP;

    /**
     * Sets the 3D \a symbol associated with the renderer.
      * Ownership of \a symbol is transferred to the renderer.
      * \see symbol()
      */
    void setSymbol( QgsPointCloud3DSymbol *symbol SIP_TRANSFER );
    //! Returns 3D symbol associated with the renderer
    const QgsPointCloud3DSymbol *symbol() const { return mSymbol.get(); }

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject &project ) override;

  private:
    QgsMapLayerRef mLayerRef; //!< Layer used to extract mesh data from
    std::unique_ptr< QgsPointCloud3DSymbol > mSymbol;

  private:
#ifdef SIP_RUN
    QgsPointCloudLayer3DRenderer( const QgsPointCloudLayer3DRenderer & );
    QgsPointCloudLayer3DRenderer &operator=( const QgsPointCloudLayer3DRenderer & );
#endif
};


#endif // QGSPOINTCLOUDLAYER3DRENDERER_H
