/***************************************************************************
  qgsabstract3dmapbackgroundsettings.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACT3DMAPBACKGROUNDSETTINGS_H
#define QGSABSTRACT3DMAPBACKGROUNDSETTINGS_H

#include "qgis.h"
#include "qgis_3d.h"

class QDomElement;
class QgsReadWriteContext;

/**
 * \ingroup qgis_3d
 * \brief Base class for all background settings classes used in a 3D map view.
 *
 * QgsAbstract3DMapBackgroundSettings subclasses are responsible for storing
 * the configuration of different background types (skybox, gradient).
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsAbstract3DMapBackgroundSettings
{
  public:
    virtual ~QgsAbstract3DMapBackgroundSettings() = default;

    /**
     * Returns the unique type for this background settings class.
     */
    virtual Qgis::Map3DBackgroundType type() const = 0;

    //! Returns a deep copy of this background settings object.
    virtual QgsAbstract3DMapBackgroundSettings *clone() const = 0 SIP_FACTORY;

    /**
     * Reads settings from a DOM \a element.
     *
     * \see writeXml()
     */
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Writes settings to a DOM \a element.
     *
     * \see readXml()
     */
    virtual void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const = 0;
};

#endif // QGSABSTRACT3DMAPBACKGROUNDSETTINGS_H
