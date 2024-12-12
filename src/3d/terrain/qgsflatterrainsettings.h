/***************************************************************************
  qgsflatterrainsettings.h
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFLATTERRAINSETTINGS_H
#define QGSFLATTERRAINSETTINGS_H

#include "qgis_3d.h"
#include "qgis_sip.h"
#include "qgsabstractterrainsettings.h"
#include "qgsrectangle.h"

class QDomElement;
class QgsReadWriteContext;
class QgsProject;

/**
 * \ingroup 3d
 * \brief Terrain settings for a simple flat area terrain.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.42
 */
class _3D_EXPORT QgsFlatTerrainSettings : public QgsAbstractTerrainSettings
{
  public:
    /**
     * Creates a new instance of a QgsFlatTerrainSettings object.
     */
    static QgsAbstractTerrainSettings *create() SIP_FACTORY;

    QgsFlatTerrainSettings *clone() const final SIP_FACTORY;
    QString type() const final;
    void readXml( const QDomElement &element, const QgsReadWriteContext &context ) final;
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const final;
    bool equals( const QgsAbstractTerrainSettings *other ) const final;
    std::unique_ptr<QgsTerrainGenerator> createTerrainGenerator( const Qgs3DRenderContext &context ) const override SIP_SKIP;
};


#endif // QGSFLATTERRAINSETTINGS_H
