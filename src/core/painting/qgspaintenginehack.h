/***************************************************************************
  qgspaintenginehack.cpp - Hack paint engine flags
  ------------------------------------------------
         begin                : July 2012
         copyright            : (C) Juergen E. Fischer
         email                : jef at norbit dot de

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPaintEngine>

#include "qgis_core.h"

/**
 * \ingroup core
 * \brief Hack to workaround Qt #5114 by disabling PatternTransform
 *
 * \note This is not required for builds based on Qt 6.3 or later, or Qt 5.15 builds
 * using KDE's maintained fork. On these versions the class has no effect.
 */
class CORE_EXPORT QgsPaintEngineHack : public QPaintEngine
{
  public:
    void fixFlags();
    static void fixEngineFlags( QPaintEngine *engine );
};
