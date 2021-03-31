/***************************************************************************
  qgssettingsregistrycore.h
  --------------------------------------
  Date                 : February 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSETTINGSREGISTRYCORE_H
#define QGSSETTINGSREGISTRYCORE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsentry.h"

#include "qgslayout.h"
#include "qgslocator.h"

#include <QList>

class QgsSettingsEntryStringList;

/**
 * \ingroup core
 * \class QgsSettingsRegistryCore
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsRegistryCore
{
  public:

    QgsSettingsRegistryCore();
    ~QgsSettingsRegistryCore();

  private:

    QList<const QgsSettingsEntryBase *> mSettingsEntries;

};

#endif // QGSSETTINGSREGISTRYCORE_H
