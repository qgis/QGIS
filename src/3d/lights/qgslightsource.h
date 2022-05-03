/***************************************************************************
                          qgslightsource.h
                          ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLIGHTSOURCE_H
#define QGSLIGHTSOURCE_H

#include "qgis_3d.h"
#include "qgis.h"
#include "qgis_sip.h"
#include "qgsreadwritecontext.h"

#include <QList>

class Qgs3DMapSettings;
class QgsProject;
class QDomElement;
class QDomDocument;

#ifndef SIP_RUN
namespace Qt3DCore
{
  class QEntity;
}
#endif

/**
 * \ingroup 3d
 * \brief Base class for light sources in 3d scenes.
 *
 * \since QGIS 3.26
 */
class _3D_EXPORT QgsLightSource SIP_ABSTRACT
{

  public:

    virtual ~QgsLightSource();

    /**
     * Returns the light source type.
     */
    virtual Qgis::LightSourceType type() const = 0;

    /**
     * Returns a copy of the light source.
     */
    virtual QgsLightSource *clone() const = 0 SIP_FACTORY;

    /**
     * Creates an entity representing the light source.
     */
    virtual Qt3DCore::QEntity *createEntity( const Qgs3DMapSettings &map, Qt3DCore::QEntity *parent ) const = 0 SIP_SKIP;

    /**
     * Writes the light source's configuration to a new DOM element and returns it.
     *
     * \see readXml()
     */
    virtual QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const = 0;

    /**
     * Reads configuration from a DOM element previously written using writeXml().
     *
     * \see writeXml()
     */
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() ) = 0;

    /**
     * After reading from XML, resolve references to any layers that have been read as layer IDs.
     */
    virtual void resolveReferences( const QgsProject &project );

    /**
     * Creates a new light source from an XML element.
     */
    static QgsLightSource *createFromXml( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;
};


#endif // QGSLIGHTSOURCE_H
