/***************************************************************************
                         qgs3d.h
                         --------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGS3D_H
#define QGS3D_H

#include "qgis_3d.h"
#include "qgis_sip.h"

class QgsMaterialRegistry;
class Qgs3DTerrainRegistry;

/**
 * \ingroup gui
 * \brief Qgs3D is a singleton class containing various registries and other global members
 * related to 3D classes.
 * \since QGIS 3.16
 */
class _3D_EXPORT Qgs3D
{
  public:
    Qgs3D( const Qgs3D &other ) = delete;
    Qgs3D &operator=( const Qgs3D &other ) = delete;

    /**
     * Returns a pointer to the singleton instance.
     */
    static Qgs3D *instance();

    ~Qgs3D();

    /**
     * Initializes the 3D framework.
     */
    static void initialize();

    /**
     * Returns the material registry, used for managing 3D materials.
     */
    static QgsMaterialRegistry *materialRegistry();

    /**
     * Returns the terrain registry, used for managing 3D terrains.
     */
    static Qgs3DTerrainRegistry *terrainRegistry();

  private:
    Qgs3D();

#ifdef SIP_RUN
    Qgs3D( const Qgs3D &other );
#endif

    bool mInitialized = false;

    QgsMaterialRegistry *mMaterialRegistry = nullptr;
    Qgs3DTerrainRegistry *mTerrainRegistry = nullptr;
};

#endif // QGS3D_H
