/***************************************************************************
    qgspainting.h
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPAINTING_H
#define QGSPAINTING_H

#include <QPainter>

#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 * \brief Misc painting enums and functions.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPainting
{
  public:

    //! Returns a QPainter::CompositionMode corresponding to a BlendMode
    static QPainter::CompositionMode getCompositionMode( Qgis::BlendMode blendMode );
    //! Returns a BlendMode corresponding to a QPainter::CompositionMode
    static Qgis::BlendMode getBlendModeEnum( QPainter::CompositionMode blendMode );

    /**
     * Returns TRUE if \a mode is a clipping blend mode.
     *
     * \since QGIS 3.30
     */
    static bool isClippingMode( Qgis::BlendMode mode );

};

#endif // QGSPAINTING_H
