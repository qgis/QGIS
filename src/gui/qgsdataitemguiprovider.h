/***************************************************************************
  qgsdataitemguiprovider.h
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATAITEMGUIPROVIDER_H
#define QGSDATAITEMGUIPROVIDER_H

#include "qgis_gui.h"

class QString;

/**
 * \class QgsDataItemGuiProvider
 * \ingroup gui
 *
 * Abstract base class for providers which affect how QgsDataItem items behave
 * within the application GUI.
 *
 * Providers must be registered via QgsDataItemGuiProviderRegistry.
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsDataItemGuiProvider
{
  public:

    virtual ~QgsDataItemGuiProvider() = default;

    /**
     * Returns the provider's name.
     */
    virtual QString name() = 0;

};

#endif // QGSDATAITEMGUIPROVIDER_H
