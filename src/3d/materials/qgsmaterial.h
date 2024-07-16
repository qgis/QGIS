/***************************************************************************
  qgsmaterial.h
  --------------------------------------
  Date                 : July 2024
  Copyright            : (C) 2024 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMATERIAL_H
#define QGSMATERIAL_H

#define SIP_NO_FILE

#include "qgis_3d.h"

#include <Qt3DRender/QMaterial>

namespace Qt3DRender
{
  class QEffect;
}

///@cond PRIVATE

/**
 * \ingroup 3d
 * \brief Base class for all materials used within QGIS 3D views.
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
  public:

    /*
     * Constructor for QgsMaterial, with the specified \a parent node.
     */
    explicit QgsMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsMaterial() override;

};

#endif // QGSMATERIAL_H
