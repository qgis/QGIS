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


#include "qgis_3d.h"

#include <QVector>

#define SIP_NO_FILE

class QString;
class QgsMaterial;
class QgsMaterialContext;

namespace Qt3DCore
{
  class QGeometry;
}

class _3D_EXPORT QgsObj3DUtils
{
  public:
    struct ObjMaterialMesh
    {
        Qt3DCore::QGeometry *geometry = nullptr;
        QgsMaterial *material = nullptr;
    };

    static QVector<ObjMaterialMesh> buildObjGeometries( const QString &filePath, const QgsMaterialContext &materialContext );
};

///@endcond

#endif // QGSOBJ3DUTILS_H
