/***************************************************************************
                          qgspainterswapper.h
                          -------------------
    begin                : July 2019
    copyright            : (C) 2019 Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPAINTERSWAPPER_H
#define QGSPAINTERSWAPPER_H

#include "qgis_core.h"

class QgsRenderContext;
class QPainter;

#define SIP_NO_FILE

/**
 * \ingroup core
 * \class QgsPainterSwapper
 * \brief A class to manage painter saving and restoring required for drawing on a different painter (mask painter for example)
 *
 * \since QGIS 3.12
 */
class QgsPainterSwapper
{
  public:

    /**
     * QgsPainterSwapper constructor
     *
     * The constructor replaces the current painter assigned to \a renderContext, swapping it for the specified \a newPainter.
     * Upon QgsPainterSwapper destruction, the painter previously assigned to the render context will automatically be restored.
     */
    QgsPainterSwapper( QgsRenderContext &renderContext, QPainter *newPainter );

    ~QgsPainterSwapper();

  private:
    QgsRenderContext &mRenderContext;
    QPainter *mOriginalPainter = nullptr;
};

#endif
