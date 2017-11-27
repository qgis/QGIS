/***************************************************************************
  qgsvectorlayer3drenderer.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYER3DRENDERER_H
#define QGSVECTORLAYER3DRENDERER_H

#include "qgis_3d.h"

#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgsabstract3dsymbol.h"

#include "qgsphongmaterialsettings.h"
#include "qgs3dutils.h"

#include "qgsmaplayerref.h"

#include <QObject>

class QgsVectorLayer;


/**
 * \ingroup core
 * Metadata for vector layer 3D renderer to allow creation of its instances from XML
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsVectorLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsVectorLayer3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    virtual QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};


/**
 * \ingroup core
 * 3D renderer that renders all features of a vector layer with the same 3D symbol.
 * The appearance is completely defined by the symbol.
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsVectorLayer3DRenderer : public QgsAbstract3DRenderer
{
  public:
    //! Takes ownership of the symbol object
    explicit QgsVectorLayer3DRenderer( QgsAbstract3DSymbol *s SIP_TRANSFER = nullptr );
    ~QgsVectorLayer3DRenderer() = default;

    //! Sets vector layer associated with the renderer
    void setLayer( QgsVectorLayer *layer );
    //! Returns vector layer associated with the renderer
    QgsVectorLayer *layer() const;

    //! Sets 3D symbol associated with the renderer. Takes ownership of the symbol
    void setSymbol( QgsAbstract3DSymbol *symbol SIP_TRANSFER );
    //! Returns 3D symbol associated with the renderer
    const QgsAbstract3DSymbol *symbol() const;

    QString type() const override { return "vector"; }
    QgsVectorLayer3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( const Qgs3DMapSettings &map ) const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject &project ) override;

  private:
    QgsMapLayerRef mLayerRef; //!< Layer used to extract polygons from
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol;  //!< 3D symbol that defines appearance
};


#endif // QGSVECTORLAYER3DRENDERER_H
