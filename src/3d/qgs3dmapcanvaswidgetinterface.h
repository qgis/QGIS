/***************************************************************************
  qgs3dmapcanvaswidgetinterface.h
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

#ifndef QGS3DMAPCANVASWIDGETINTERFACE_H
#define QGS3DMAPCANVASWIDGETINTERFACE_H

#include "qgis.h"
#include "qgis_3d.h"

class Qgs3DEditingToolBar;
class Qgs3DMapCanvas;

/**
 * \ingroup qgis_3d
 * \brief Convenience wrapper to wrap Qgs3DMapCanvasWidget function from Qgs3DMapCanvas.
 *
 * \since QGIS 4.4
 */
class _3D_EXPORT Qgs3DMapCanvasWidgetInterface
{
  public:
    Qgs3DMapCanvasWidgetInterface() = default;
    virtual ~Qgs3DMapCanvasWidgetInterface() = default;

    /**
     * Add new editing toolbar.
     * Takes ownership
     * \param newToolBar new toolbar
     */
    virtual void addEditingToolBar( Qgs3DEditingToolBar *newToolBar ) SIP_SKIP;

    //! Returns all added editing toolbars
    virtual QList<Qgs3DEditingToolBar *> editingToolBars() const SIP_SKIP;

    //! Returns 3D mapCanvas
    virtual Qgs3DMapCanvas *mapCanvas3D() SIP_SKIP;
};

#endif //QGS3DMAPCANVASWIDGETINTERFACE_H
