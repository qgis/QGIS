/***************************************************************************
                              qgsmapcanvasinteractionblocker.h
                              --------------------------------
  begin                : May 2020
  copyright            : (C) 2020 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCANVASINTERACTIONBLOCKER_H
#define QGSMAPCANVASINTERACTIONBLOCKER_H

#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \class QgsMapCanvasInteractionBlocker
 * \ingroup gui
 * \brief An interface for objects which block interactions with a QgsMapCanvas.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsMapCanvasInteractionBlocker
{
  public:
    /**
     * Available interactions to block.
     */
    enum class Interaction : int SIP_ENUM_BASETYPE( IntFlag )
    {
      MapPanOnSingleClick = 1 << 0, //!< A map pan interaction caused by a single click and release on the map canvas
    };

    virtual ~QgsMapCanvasInteractionBlocker() = default;

    /**
     * Returns TRUE if the specified \a interaction should be blocked.
     */
    virtual bool blockCanvasInteraction( Interaction interaction ) const = 0;
};

#endif // QGSMAPCANVASINTERACTIONBLOCKER_H
