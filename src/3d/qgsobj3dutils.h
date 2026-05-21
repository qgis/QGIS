/***************************************************************************
  qgsobj3dutils.h
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

#ifndef QGSOBJ3DUTILS_H
#define QGSOBJ3DUTILS_H

///@cond PRIVATE


#include <memory>
#include <vector>

#include "qgis_3d.h"
#include "qgsmaterial.h"

#define SIP_NO_FILE

class QString;
class QgsMaterialContext;

#include <Qt3DCore/QGeometry>

/**
 * \ingroup qgis_3d
 *
 * Utility functions for dealing with OBJ models in 3D map views.
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsObj3DUtils
{
  public:
    //! Geometry and material pair for a single OBJ material group. Material is nullptr when the group has no texture.
    struct ObjMaterialMesh
    {
        std::unique_ptr<Qt3DCore::QGeometry> geometry; //!< Geometry of the material group.
        std::unique_ptr<QgsMaterial> material;         //!< Material of the material group, or nullptr if no texture.
    };

    /**
     * Loads an OBJ file from \a filePath and returns one geometry/material pair
     * per material group found in the file.
     *
     * \a materialContext is used to configure the created materials.
     *
     * If a group has no texture, the material remains a NULLPTR.
     */
    static std::vector<ObjMaterialMesh> buildObjGeometries( const QString &filePath, const QgsMaterialContext &materialContext );
};

///@endcond

#endif // QGSOBJ3DUTILS_H
