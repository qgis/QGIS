/***************************************************************************
  qgsabstract3drenderer.h
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

#ifndef QGSABSTRACT3DRENDERER_H
#define QGSABSTRACT3DRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>

class QDomElement;
class QgsProject;
class QgsReadWriteContext;
class Qgs3DMapSettings;

#ifndef SIP_RUN
namespace Qt3DCore
{
  class QEntity;
}
#endif

/**
 * \ingroup core
 * Base class for all renderers that may to participate in 3D view.
 *
 * 3D renderers implement the method createEntity() that returns a new 3D entity - that entity
 * will be added to the 3D scene to represent data in renderer's display style.
 *
 * Renderers may store some custom properties (e.g. materials, sizes) that are written to and read from
 * XML. It is therefore not recommended to store large amount of data within a renderer (e.g. arrays of vertices).
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAbstract3DRenderer SIP_ABSTRACT
{
  public:
    virtual ~QgsAbstract3DRenderer() = default;

    //! Returns unique identifier of the renderer class (used to identify subclass)
    virtual QString type() const = 0;
    //! Returns a cloned instance
    virtual QgsAbstract3DRenderer *clone() const = 0 SIP_FACTORY;
    //! Returns a 3D entity that will be used to show renderer's data in 3D scene
    virtual Qt3DCore::QEntity *createEntity( const Qgs3DMapSettings &map ) const = 0 SIP_SKIP;

    //! Writes renderer's properties to given XML element
    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
    //! Reads renderer's properties from given XML element
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
    //! Resolves references to other objects - second phase of loading - after readXml()
    virtual void resolveReferences( const QgsProject &project );

  protected:
    //! Default constructor
    QgsAbstract3DRenderer() = default;

  private:
#ifdef SIP_RUN
    QgsAbstract3DRenderer( const QgsAbstract3DRenderer & );
    QgsAbstract3DRenderer &operator=( const QgsAbstract3DRenderer & );
#endif

    Q_DISABLE_COPY( QgsAbstract3DRenderer )
};


#endif // QGSABSTRACT3DRENDERER_H
