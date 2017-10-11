/***************************************************************************
  qgsabstract3dsymbol.h
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

#ifndef QGSABSTRACT3DSYMBOL_H
#define QGSABSTRACT3DSYMBOL_H

#include "qgis_3d.h"
#include "qgis_sip.h"

class QDomElement;
class QString;

class QgsReadWriteContext;


/**
 * \ingroup 3d
 * Abstract base class for 3D symbols that are used by VectorLayer3DRenderer objects.
 *
 * 3D symbol objects define appearance of GIS data.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsAbstract3DSymbol
{
  public:
    virtual ~QgsAbstract3DSymbol() = default;

    //! Returns identifier of symbol type. Each 3D symbol implementation should return a different type.
    virtual QString type() const = 0;
    //! Returns a new instance of the symbol with the same settings
    virtual QgsAbstract3DSymbol *clone() const = 0 SIP_FACTORY;

    //! Writes symbol configuration to the given DOM element
    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
    //! Reads symbol configuration from the given DOM element
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
};


#endif // QGSABSTRACT3DSYMBOL_H
