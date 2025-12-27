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

///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief A single color material for highlighting features.
 * Uses the highlight color and opacity defined in qgis settings Map/highlight
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsHighlightMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsHighlightMaterial, with the specified \a parent node.
     */
    explicit QgsHighlightMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsHighlightMaterial() override;

  private:
    void init();
};

///@endcond PRIVATE

#endif // QGSHIGHLIGHTMATERIAL_H
