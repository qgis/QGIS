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


#include <vector>

#include "qgis_3d.h"
#include "qgs3dutils.h"

#define SIP_NO_FILE

class QString;
class QgsMaterialContext;

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
    /**
     * Loads an OBJ file from \a filePath and returns one geometry/material pair
     * per material group found in the file.
     *
     * \a materialContext is used to configure the created materials.
     *
     * If a group has no texture, the material remains a NULLPTR.
     */
    static std::vector<QgsMeshNodeData> buildObjGeometries( const QString &filePath, const QgsMaterialContext &materialContext );
};

///@endcond

#endif // QGSOBJ3DUTILS_H
