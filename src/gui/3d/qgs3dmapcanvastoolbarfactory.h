/***************************************************************************
  qgs3dmapcanvastoolbarfactory.h
  --------------------------------------
  Date                 : November 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPCANVASTOOLBARFACTORY_H
#define QGS3DMAPCANVASTOOLBARFACTORY_H

#include "qgis_gui.h"
#include "qgis_sip.h"

class Qgs3DMapCanvas;
class QToolBar;
class QWidget;

/**
 * \ingroup gui
 * \class Qgs3DMapCanvasToolBarFactory
 * \brief Factory for creating custom toolbars inside 3D map views.
 *
 * Factories are registered through QgisInterface::register3DMapCanvasToolBarFactory().
 * The created toolbar is shown below the 3D map view's own toolbars.
 *
 * \since QGIS 4.4.1
 */
class GUI_EXPORT Qgs3DMapCanvasToolBarFactory
{
  public:
    virtual ~Qgs3DMapCanvasToolBarFactory();

    /**
     * Creates a new toolbar for the 3D map \a canvas with the specified \a parent.
     */
    virtual QToolBar *createToolBar( Qgs3DMapCanvas *canvas, QWidget *parent ) = 0 SIP_FACTORY;
};

#endif // QGS3DMAPCANVASTOOLBARFACTORY_H
