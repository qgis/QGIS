/***************************************************************************
  qgs3drendererregistry.h
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

#ifndef QGS3DRENDERERREGISTRY_H
#define QGS3DRENDERERREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QMap>

class QDomElement;
class QgsAbstract3DRenderer;
class QgsReadWriteContext;


/**
 * Base metadata class for 3D renderers. Instances of derived classes may be registered in Qgs3DRendererRegistry.
 * \ingroup core
 * \since QGIS 3.0
 */
class CORE_EXPORT Qgs3DRendererAbstractMetadata
{
  public:

    virtual ~Qgs3DRendererAbstractMetadata() = default;

    /**
     * Returns unique identifier of the 3D renderer class
     */
    QString type() const;

    /**
     * Returns new instance of the renderer given the DOM element. Returns NULLPTR on error.
     * Pure virtual function: must be implemented in derived classes.
     */
    virtual QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) = 0 SIP_FACTORY;

  protected:

    /**
     * Constructor of the base class
     */
    explicit Qgs3DRendererAbstractMetadata( const QString &type );

  protected:
    //! Type used within QGIS for identification (the same what renderer's type() returns)
    QString mType;
};


/**
 * Keeps track of available 3D renderers. Should be accessed through QgsApplication::renderer3DRegistry() singleton.
 * \ingroup core
 * \since QGIS 3.0
 */
class CORE_EXPORT Qgs3DRendererRegistry
{
  public:
    //! Creates registry of 3D renderers
    Qgs3DRendererRegistry() = default;

    ~Qgs3DRendererRegistry();

    /**
     * Registers a new 3D renderer type. The call takes ownership of the passed metadata object.
     */
    void addRenderer( Qgs3DRendererAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Unregisters a 3D renderer type
     */
    void removeRenderer( const QString &type );

    /**
     * Returns metadata for a 3D renderer type (may be used to create a new instance of the type)
     */
    Qgs3DRendererAbstractMetadata *rendererMetadata( const QString &type ) const;

    /**
     * Returns a list of all available 3D renderer types.
     */
    QStringList renderersList() const;

  private:
    QMap<QString, Qgs3DRendererAbstractMetadata *> mRenderers;
};

#endif // QGS3DRENDERERREGISTRY_H
