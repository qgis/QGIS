/***************************************************************************
                             qgslayoutitemregistryguiutils.h
                             -------------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMREGISTRYGUIUTILS_H
#define QGSLAYOUTITEMREGISTRYGUIUTILS_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgslayoutitemregistry.h"

class QgsLayoutViewRubberBand;

/**
 * \ingroup gui
 * A group of static utilities for working with the gui based portions of
 * QgsLayoutItemRegistry.
 *
 * This class is designed to allow Python item subclasses to override the
 * default GUI based QgsLayoutItemAbstractMetadata methods, which
 * cannot be directly overridden from Python subclasses.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutItemRegistryGuiUtils
{
  public:

    /**
     * Sets a \a prototype for the rubber bands for the layout item with specified \a type.
     * Python subclasses of QgsLayoutItem must call this method to register their prototypes,
     * as the usual c++ QgsLayoutItemAbstractMetadata are not accessible via the Python bindings.
     */
    static void setItemRubberBandPrototype( int type, QgsLayoutViewRubberBand *prototype SIP_TRANSFER );


};

#endif // QGSLAYOUTITEMREGISTRYGUIUTILS_H
