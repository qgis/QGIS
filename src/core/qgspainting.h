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

/**
 * \ingroup core
 * Misc painting enums and functions.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPainting
{
  public:

    /**
     * Blending modes enum defining the available composition modes that can
     * be used when rendering a layer
     */
    enum BlendMode
    {
      BlendNormal,
      BlendLighten,
      BlendScreen,
      BlendDodge,
      BlendAddition,
      BlendDarken,
      BlendMultiply,
      BlendBurn,
      BlendOverlay,
      BlendSoftLight,
      BlendHardLight,
      BlendDifference,
      BlendSubtract,
      BlendSource,
      BlendDestinationOver,
      BlendClear,
      BlendDestination,
      BlendSourceIn,
      BlendDestinationIn,
      BlendSourceOut,
      BlendDestinationOut,
      BlendSourceAtop,
      BlendDestinationAtop,
      BlendXor,
    };

    //! Returns a QPainter::CompositionMode corresponding to a BlendMode
    static QPainter::CompositionMode getCompositionMode( QgsPainting::BlendMode blendMode );
    //! Returns a BlendMode corresponding to a QPainter::CompositionMode
    static QgsPainting::BlendMode getBlendModeEnum( QPainter::CompositionMode blendMode );

};

#endif // QGSPAINTING_H
