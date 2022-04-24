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
#include "qgis_sip.h"

#include <QList>

class Qgs3DMapSettings;
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
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsLightSource SIP_ABSTRACT
{

  public:

    virtual ~QgsLightSource();

    /**
     * Creates entities representing the light source.
     */
    virtual QList< Qt3DCore::QEntity * > createEntities( const Qgs3DMapSettings &map, Qt3DCore::QEntity *parent ) const = 0 SIP_SKIP;

    /**
     * Writes the light source's configuration to a new DOM element and returns it.
     *
     * \see readXml()
     */
    virtual QDomElement writeXml( QDomDocument &doc ) const = 0;

    /**
     * Reads configuration from a DOM element previously written using writeXml().
     *
     * \see writeXml()
     */
    virtual void readXml( const QDomElement &elem ) = 0;
};


#endif // QGSLIGHTSOURCE_H
