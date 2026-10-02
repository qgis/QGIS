/***************************************************************************
  qgs3dmapinterface.h
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPINTERFACE_H
#define QGS3DMAPINTERFACE_H

#include "qgis.h"
#include "qgis_3d.h"

#define SIP_NO_FILE

class Qgs3DEditingToolBar;
class Qgs3DMapCanvas;
class QgsLateralPanelWidget;


/**
 * \ingroup qgis_3d
 * \brief Convenience wrapper to wrap Qgs3DMapCanvasWidget function from Qgs3DMapCanvas.
 *
 * \since QGIS 4.4
 */
class _3D_EXPORT Qgs3DMapInterface
{
  public:
    Qgs3DMapInterface() = default;
    virtual ~Qgs3DMapInterface() = default;

    /**
     * Add new editing toolbar.
     * Takes ownership
     * \param newToolBar new toolbar
     */
    virtual void addEditingToolBar( Qgs3DEditingToolBar *newToolBar ) = 0;

    //! Returns all added editing toolbars
    virtual QList<Qgs3DEditingToolBar *> editingToolBars() const = 0;

    //! Returns 3D mapCanvas
    virtual Qgs3DMapCanvas *mapCanvas3D() = 0;

    //! Returns lateral panel widget
    virtual QgsLateralPanelWidget *lateralPanel() const = 0;
};

#endif //QGS3DMAPINTERFACE_H
