/***************************************************************************
  qgsmeshlayer3drenderer.h
  ------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYER3DRENDERER_H
#define QGSMESHLAYER3DRENDERER_H

#include "qgis_3d.h"

#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgsmesh3dsymbol.h"

#include "qgsphongmaterialsettings.h"
#include "qgsmaplayerref.h"

#include <QObject>

#define SIP_NO_FILE

class QgsMeshLayer;


/**
 * \ingroup core
 * \brief Metadata for mesh layer 3D renderer to allow creation of its instances from XML
 *
 * \warning This is not considered stable API, and may change in future QGIS releases
 *
 * \note Not available in Python bindings
 * \since QGIS 3.6
 */
class _3D_EXPORT QgsMeshLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsMeshLayer3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};


/**
 * \ingroup core
 * \brief 3D renderer that renders all mesh triangles of a mesh layer.
 * \since QGIS 3.6
 */
class _3D_EXPORT QgsMeshLayer3DRenderer : public QgsAbstract3DRenderer
{
  public:
    //! Takes ownership of the symbol object
    explicit QgsMeshLayer3DRenderer( QgsMesh3DSymbol *s SIP_TRANSFER = nullptr );

    //! Sets vector layer associated with the renderer
    void setLayer( QgsMeshLayer *layer );
    //! Returns mesh layer associated with the renderer
    QgsMeshLayer *layer() const;

    //! Sets 3D symbol associated with the renderer
    void setSymbol( QgsMesh3DSymbol *symbol SIP_TRANSFER );
    //! Returns 3D symbol associated with the renderer
    const QgsMesh3DSymbol *symbol() const;

    QString type() const override { return "mesh"; }
    QgsMeshLayer3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( Qgs3DMapSettings *map ) const override SIP_SKIP;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject &project ) override;


  private:
    QgsMapLayerRef mLayerRef;                 //!< Layer used to extract mesh data from
    std::unique_ptr<QgsMesh3DSymbol> mSymbol; //!< 3D symbol that defines appearance

  private:
#ifdef SIP_RUN
    QgsMeshLayer3DRenderer( const QgsMeshLayer3DRenderer & );
    QgsMeshLayer3DRenderer &operator=( const QgsMeshLayer3DRenderer & );
#endif
};

#endif // QGSMESHLAYER3DRENDERER_H
