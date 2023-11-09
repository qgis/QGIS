/***************************************************************************
    qgsmaplayeractioncontextgenerator.h
    ---------------------------
    begin                : January 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERACTIONCONTEXTGENERATOR_H
#define QGSMAPLAYERACTIONCONTEXTGENERATOR_H

#include "qgis_sip.h"

#include "qgis.h"
#include "qgis_gui.h"

class QgsMapLayerActionContext;

/**
 * \ingroup gui
 * \brief An interface for objects which can create a QgsMapLayerActionContext.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsMapLayerActionContextGenerator
{
  public:

    virtual ~QgsMapLayerActionContextGenerator();

    /**
     * Creates a QgsMapLayerActionContext.
     */
    virtual QgsMapLayerActionContext createActionContext() = 0;
};

#endif // QGSMAPLAYERACTIONCONTEXTGENERATOR_H
