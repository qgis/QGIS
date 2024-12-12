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
#include "qgis_sip.h"

#include "qgs3drendererregistry.h"
#include "qgsabstractvectorlayer3drenderer.h"
#include "qgsabstract3dsymbol.h"
#include "qgsphongmaterialsettings.h"

#include <QObject>

class QgsVectorLayer;

/**
 * \ingroup core
 * \brief Metadata for vector layer 3D renderer to allow creation of its instances from XML.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 */
class _3D_EXPORT QgsVectorLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsVectorLayer3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};


/**
 * \ingroup core
 * \brief 3D renderer that renders all features of a vector layer with the same 3D symbol.
 * The appearance is completely defined by the symbol.
 */
class _3D_EXPORT QgsVectorLayer3DRenderer : public QgsAbstractVectorLayer3DRenderer
{
  public:
    //! Takes ownership of the symbol object
    explicit QgsVectorLayer3DRenderer( QgsAbstract3DSymbol *s SIP_TRANSFER = nullptr );

    //! Sets 3D symbol associated with the renderer. Takes ownership of the symbol
    void setSymbol( QgsAbstract3DSymbol *symbol SIP_TRANSFER );
    //! Returns 3D symbol associated with the renderer
    const QgsAbstract3DSymbol *symbol() const;

    QString type() const override { return "vector"; }
    QgsVectorLayer3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( Qgs3DMapSettings *map ) const override SIP_SKIP;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

  private:
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol; //!< 3D symbol that defines appearance

  private:
#ifdef SIP_RUN
    QgsVectorLayer3DRenderer( const QgsVectorLayer3DRenderer & );
    QgsVectorLayer3DRenderer &operator=( const QgsVectorLayer3DRenderer & );
#endif
};


#endif // QGSVECTORLAYER3DRENDERER_H
