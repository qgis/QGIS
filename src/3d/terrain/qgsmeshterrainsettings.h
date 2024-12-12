/***************************************************************************
  qgsmeshterrainsettings.h
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

#ifndef QGSMESHTERRAINSETTINGS_H
#define QGSMESHTERRAINSETTINGS_H

#include "qgis_3d.h"
#include "qgis_sip.h"
#include "qgsabstractterrainsettings.h"
#include "qgsmaplayerref.h"

class QDomElement;
class QgsReadWriteContext;
class QgsProject;
class QgsMeshLayer;
class QgsMesh3DSymbol;

/**
 * \ingroup 3d
 * \brief Terrain settings for a terrain generator that uses uses the Z values of a mesh layer to build a terrain.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.42
 */
class _3D_EXPORT QgsMeshTerrainSettings : public QgsAbstractTerrainSettings
{
  public:
    /**
     * Creates a new instance of a QgsMeshTerrainSettings object.
     */
    static QgsAbstractTerrainSettings *create() SIP_FACTORY;

    QgsMeshTerrainSettings();
    ~QgsMeshTerrainSettings() override;
    QgsMeshTerrainSettings *clone() const final SIP_FACTORY;
    QString type() const final;
    void readXml( const QDomElement &element, const QgsReadWriteContext &context ) final;
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const final;
    void resolveReferences( const QgsProject *project ) final;
    bool equals( const QgsAbstractTerrainSettings *other ) const final;
    std::unique_ptr<QgsTerrainGenerator> createTerrainGenerator( const Qgs3DRenderContext &context ) const override SIP_SKIP;

    /**
     * Sets the mesh \a layer with elevation model to be used for terrain generation.
     * \see layer()
     */
    void setLayer( QgsMeshLayer *layer );

    /**
     * Returns the mesh layer with elevation model to be used for terrain generation.
     *
     * \see setLayer()
     */
    QgsMeshLayer *layer() const;

    /**
     * Returns the symbol used to render the mesh as terrain.
     *
     * \see setSymbol()
     */
    QgsMesh3DSymbol *symbol() const;

    /**
     * Sets the symbol used to render the mesh as terrain.
     *
     * \see symbol()
     */
    void setSymbol( QgsMesh3DSymbol *symbol SIP_TRANSFER );

  private:
#ifdef SIP_RUN
    QgsMeshTerrainSettings( const QgsMeshTerrainSettings & );
#endif
    QgsMapLayerRef mLayer;
    std::unique_ptr<QgsMesh3DSymbol> mSymbol;
};


#endif // QGSMESHTERRAINSETTINGS_H
