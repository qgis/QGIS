/***************************************************************************
    qgshighlightmaterial.h
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHIGHLIGHTMATERIAL_H
#define QGSHIGHLIGHTMATERIAL_H

#include "qgis_3d.h"
#include "qgsmaterial.h"

#define SIP_NO_FILE

class _3D_EXPORT QgsHighlightMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    explicit QgsHighlightMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsHighlightMaterial() override;

  private:
    void init();
};

#endif // QGSHIGHLIGHTMATERIAL_H
